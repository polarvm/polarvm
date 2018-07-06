// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/07/06.

//===----------------------------------------------------------------------===//
//
// This file implements the SourceMgr class.  This class is used as a simple
// substrate for diagnostics, #include handling, and other low level things for
// simple parsers.
//
//===----------------------------------------------------------------------===//

#include "polar/utils/SourceMgr.h"
#include "polar/basic/adt/ArrayRef.h"
#include "polar/basic/adt/STLExtras.h"
#include "polar/basic/adt/SmallVector.h"
#include "polar/basic/adt/StringRef.h"
#include "polar/basic/adt/Twine.h"
#include "polar/utils/OptionalError.h"
#include "polar/utils/Locale.h"
#include "polar/utils/MemoryBuffer.h"
#include "polar/utils/Path.h"
#include "polar/utils/SourceLocation.h"
#include "polar/utils/RawOutStream.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>

namespace polar {
namespace utils {

using polar::basic::make_array_ref;
using polar::basic::find_if;

static const size_t sg_tabStop = 8;

namespace {

struct LineNoCacheTy
{
   const char *m_lastQuery;
   unsigned m_lastQueryBufferId;
   unsigned m_lineNoOfQuery;
};

} // end anonymous namespace

static LineNoCacheTy *getCache(void *ptr)
{
   return (LineNoCacheTy*)ptr;
}

SourceMgr::~SourceMgr()
{
   delete getCache(m_lineNoCache);
}

unsigned SourceMgr::addIncludeFile(const std::string &filename,
                                   SMLocation includeLoc,
                                   std::string &includedFile)
{
   includedFile = filename;
   OptionalError<std::unique_ptr<MemoryBuffer>> newBufOrErr =
         MemoryBuffer::getFile(includedFile);

   // If the file didn't exist directly, see if it's in an include path.
   for (unsigned i = 0, e = m_includeDirectories.size(); i != e && !newBufOrErr;
        ++i) {
      includedFile =
            m_includeDirectories[i] + fs::path::get_separator().getData() + filename;
      newBufOrErr = MemoryBuffer::getFile(includedFile);
   }

   if (!newBufOrErr) {
      return 0;
   }
   return addNewSourceBuffer(std::move(*newBufOrErr), includeLoc);
}

unsigned SourceMgr::findBufferContainingLoc(SMLocation loc) const
{
   for (unsigned i = 0, e = m_buffers.size(); i != e; ++i) {
      if (loc.getPointer() >= m_buffers[i].m_buffer->getBufferStart() &&
          // Use <= here so that a pointer to the null at the end of the buffer
          // is included as part of the buffer.
          loc.getPointer() <= m_buffers[i].m_buffer->getBufferEnd()) {
         return i + 1;
      }
   }
   return 0;
}

std::pair<unsigned, unsigned>
SourceMgr::getLineAndColumn(SMLocation loc, unsigned bufferID) const
{
   if (!bufferID) {
      bufferID = findBufferContainingLoc(loc);
   }
   assert(bufferID && "Invalid Location!");
   const MemoryBuffer *buff = getMemoryBuffer(bufferID);

   // Count the number of \n's between the start of the file and the specified
   // location.
   unsigned m_lineNo = 1;

   const char *bufStart = buff->getBufferStart();
   const char *ptr = bufStart;

   // If we have a line number cache, and if the query is to a later point in the
   // same file, start searching from the last query location.  This optimizes
   // for the case when multiple diagnostics come out of one file in order.
   if (LineNoCacheTy *cache = getCache(m_lineNoCache))
      if (cache->m_lastQueryBufferId == bufferID &&
          cache->m_lastQuery <= loc.getPointer()) {
         ptr = cache->m_lastQuery;
         m_lineNo = cache->m_lineNoOfQuery;
      }

   // Scan for the location being queried, keeping track of the number of lines
   // we see.
   for (; SMLocation::getFromPointer(ptr) != loc; ++ptr) {
      if (*ptr == '\n') {
         ++m_lineNo;
      }
   }
   // Allocate the line number cache if it doesn't exist.
   if (!m_lineNoCache) {
      m_lineNoCache = new LineNoCacheTy();
   }
   // Update the line # cache.
   LineNoCacheTy &cache = *getCache(m_lineNoCache);
   cache.m_lastQueryBufferId = bufferID;
   cache.m_lastQuery = ptr;
   cache.m_lineNoOfQuery = m_lineNo;

   size_t newlineOffs = StringRef(bufStart, ptr - bufStart).findLastOf("\n\r");
   if (newlineOffs == StringRef::npos) {
      newlineOffs = ~(size_t)0;
   }
   return std::make_pair(m_lineNo, ptr - bufStart - newlineOffs);
}

void SourceMgr::printIncludeStack(SMLocation includeLoc, RawOutStream &outstream) const
{
   if (includeLoc == SMLocation()) {
      return;  // Top of stack.
   }
   unsigned curBuffer = findBufferContainingLoc(includeLoc);
   assert(curBuffer && "Invalid or unspecified location!");
   printIncludeStack(getBufferInfo(curBuffer).m_includeLoc, outstream);
   outstream << "Included from "
             << getBufferInfo(curBuffer).m_buffer->getBufferIdentifier()
             << ":" << findLineNumber(includeLoc, curBuffer) << ":\n";
}

SMDiagnostic SourceMgr::getMessage(SMLocation loc, SourceMgr::DiagKind m_kind,
                                   const Twine &msg,
                                   ArrayRef<SMRange> m_ranges,
                                   ArrayRef<SMFixIt> m_fixIts) const
{
   // First thing to do: find the current buffer containing the specified
   // location to pull out the source line.
   SmallVector<std::pair<unsigned, unsigned>, 4> colRanges;
   std::pair<unsigned, unsigned> lineAndCol;
   StringRef bufferID = "<unknown>";
   std::string lineStr;
   if (loc.isValid()) {
      unsigned curBuffer = findBufferContainingLoc(loc);
      assert(curBuffer && "Invalid or unspecified location!");

      const MemoryBuffer *curMB = getMemoryBuffer(curBuffer);
      bufferID = curMB->getBufferIdentifier();

      // Scan backward to find the start of the line.
      const char *lineStart = loc.getPointer();
      const char *bufStart = curMB->getBufferStart();
      while (lineStart != bufStart && lineStart[-1] != '\n' &&
             lineStart[-1] != '\r') {
         --lineStart;
      }
      // Get the end of the line.
      const char *lineEnd = loc.getPointer();
      const char *bufEnd = curMB->getBufferEnd();
      while (lineEnd != bufEnd && lineEnd[0] != '\n' && lineEnd[0] != '\r') {
         ++lineEnd;
      }
      lineStr = std::string(lineStart, lineEnd);
      // Convert any m_ranges to column m_ranges that only intersect the line of the
      // location.
      for (unsigned i = 0, e = m_ranges.getSize(); i != e; ++i) {
         SMRange range = m_ranges[i];
         if (!range.isValid()) {
            continue;
         }
         // If the line doesn't contain any part of the range, then ignore it.
         if (range.m_start.getPointer() > lineEnd || range.m_end.getPointer() < lineStart) {
            continue;
         }
         // Ignore pieces of the range that go onto other lines.
         if (range.m_start.getPointer() < lineStart) {
            range.m_start = SMLocation::getFromPointer(lineStart);
         }
         if (range.m_end.getPointer() > lineEnd) {
            range.m_end = SMLocation::getFromPointer(lineEnd);
         }
         // Translate from SMLocation m_ranges to column m_ranges.
         // FIXME: Handle multibyte characters.
         colRanges.push_back(std::make_pair(range.m_start.getPointer()-lineStart,
                                            range.m_end.getPointer()-lineStart));
      }

      lineAndCol = getLineAndColumn(loc, curBuffer);
   }

   return SMDiagnostic(*this, loc, bufferID, lineAndCol.first,
                       lineAndCol.second - 1, m_kind, msg.getStr(),
                       lineStr, colRanges, m_fixIts);
}

void SourceMgr::printMessage(RawOutStream &outstream, const SMDiagnostic &diagnostic,
                             bool showColors) const
{
   // Report the message with the diagnostic handler if present.
   if (m_diagHandler) {
      m_diagHandler(diagnostic, m_diagContext);
      return;
   }

   if (diagnostic.getLocation().isValid()) {
      unsigned curBuffer = findBufferContainingLoc(diagnostic.getLocation());
      assert(curBuffer && "Invalid or unspecified location!");
      printIncludeStack(getBufferInfo(curBuffer).m_includeLoc, outstream);
   }

   diagnostic.print(nullptr, outstream, showColors);
}

void SourceMgr::printMessage(RawOutStream &outstream, SMLocation loc,
                             SourceMgr::DiagKind m_kind,
                             const Twine &msg, ArrayRef<SMRange> m_ranges,
                             ArrayRef<SMFixIt> m_fixIts, bool showColors) const
{
   printMessage(outstream, getMessage(loc, m_kind, msg, m_ranges, m_fixIts), showColors);
}

void SourceMgr::printMessage(SMLocation loc, SourceMgr::DiagKind m_kind,
                             const Twine &msg, ArrayRef<SMRange> m_ranges,
                             ArrayRef<SMFixIt> m_fixIts, bool showColors) const
{
   printMessage(error_stream(), loc, m_kind, msg, m_ranges, m_fixIts, showColors);
}

//===----------------------------------------------------------------------===//
// SMDiagnostic Implementation
//===----------------------------------------------------------------------===//

SMDiagnostic::SMDiagnostic(const SourceMgr &sm, SMLocation location, StringRef filename,
                           int line, int column, SourceMgr::DiagKind m_kind,
                           StringRef msg, StringRef lineStr,
                           ArrayRef<std::pair<unsigned,unsigned>> m_ranges,
                           ArrayRef<SMFixIt> hints)
   : m_sourceMgr(&sm),
     m_location(location),
     m_filename(filename),
     m_lineNo(line),
     m_columnNo(column),
     m_kind(m_kind),
     m_message(msg),
     m_lineContents(lineStr),
     m_ranges(m_ranges.getVector()),
     m_fixIts(hints.begin(), hints.end())
{
   std::sort(m_fixIts.begin(), m_fixIts.end());
}

static void build_fixit_line(std::string &caretLine, std::string &fixItLine,
                             ArrayRef<SMFixIt> fixIts, ArrayRef<char> sourceLine)
{
   if (fixIts.empty()) {
      return;
   }
   const char *lineStart = sourceLine.begin();
   const char *lineEnd = sourceLine.end();

   size_t prevHintEndCol = 0;

   for (ArrayRef<SMFixIt>::iterator iter = fixIts.begin(), endMark = fixIts.end();
        iter != endMark; ++iter) {
      // If the fixit contains a newline or tab, ignore it.
      if (iter->getText().findFirstOf("\n\r\t") != StringRef::npos) {
         continue;
      }
      SMRange range = iter->getRange();
      // If the line doesn't contain any part of the range, then ignore it.
      if (range.m_start.getPointer() > lineEnd || range.m_end.getPointer() < lineStart) {
         continue;
      }
      // Translate from SMLocation to column.
      // Ignore pieces of the range that go onto other lines.
      // FIXME: Handle multibyte characters in the source line.
      unsigned firstCol;
      if (range.m_start.getPointer() < lineStart) {
         firstCol = 0;
      } else {
         firstCol = range.m_start.getPointer() - lineStart;
      }
      // If we inserted a long previous hint, push this one forwards, and add
      // an extra space to show that this is not part of the previous
      // completion. This is sort of the best we can do when two hints appear
      // to overlap.
      //
      // Note that if this hint is located immediately after the previous
      // hint, no space will be added, since the location is more important.
      unsigned hintCol = firstCol;
      if (hintCol < prevHintEndCol) {
         hintCol = prevHintEndCol + 1;
      }

      // FIXME: This assertion is intended to catch unintended use of multibyte
      // characters in fixits. If we decide to do this, we'll have to track
      // separate byte widths for the source and fixit lines.
      assert((size_t)sys::locale::column_width(iter->getText()) ==
             iter->getText().getSize());

      // This relies on one byte per column in our fixit hints.
      unsigned lastColumnModified = hintCol + iter->getText().getSize();
      if (lastColumnModified > fixItLine.size()) {
         fixItLine.resize(lastColumnModified, ' ');
      }
      std::copy(iter->getText().begin(), iter->getText().end(),
                fixItLine.begin() + hintCol);

      prevHintEndCol = lastColumnModified;

      // For replacements, mark the removal range with '~'.
      // FIXME: Handle multibyte characters in the source line.
      unsigned lastCol;
      if (range.m_end.getPointer() >= lineEnd) {
         lastCol = lineEnd - lineStart;
      } else{
         lastCol = range.m_end.getPointer() - lineStart;
      }
      std::fill(&caretLine[firstCol], &caretLine[lastCol], '~');
   }
}

static void print_source_line(RawOutStream &stream, StringRef m_lineContents) {
   // Print out the source line one character at a time, so we can expand tabs.
   for (unsigned i = 0, e = m_lineContents.getSize(), outCol = 0; i != e; ++i) {
      if (m_lineContents[i] != '\t') {
         stream << m_lineContents[i];
         ++outCol;
         continue;
      }

      // If we have a tab, emit at least one space, then round up to 8 columns.
      do {
         stream << ' ';
         ++outCol;
      } while ((outCol % sg_tabStop) != 0);
   }
   stream << '\n';
}

static bool is_non_ascii(char c)
{
   return c & 0x80;
}

void SMDiagnostic::print(const char *progName, RawOutStream &stream, bool showColors,
                         bool showKindLabel) const
{
   // Display colors only if outstream supports colors.
   showColors &= stream.hasColors();
   if (showColors) {
      stream.changeColor(RawOutStream::Colors::SAVEDCOLOR, true);
   }
   if (progName && progName[0]) {
      stream << progName << ": ";
   }
   if (!m_filename.empty()) {
      if (m_filename == "-") {
         stream << "<stdin>";
      } else {
         stream << m_filename;
      }
      if (m_lineNo != -1) {
         stream << ':' << m_lineNo;
         if (m_columnNo != -1) {
            stream << ':' << (m_columnNo + 1);
         }
      }
      stream << ": ";
   }

   if (showKindLabel) {
      switch (m_kind) {
      case SourceMgr::DK_Error:
         if (showColors) {
            stream.changeColor(RawOutStream::Colors::RED, true);
         }
         stream << "error: ";
         break;
      case SourceMgr::DK_Warning:
         if (showColors) {
            stream.changeColor(RawOutStream::Colors::MAGENTA, true);
         }
         stream << "warning: ";
         break;
      case SourceMgr::DK_Note:
         if (showColors) {
            stream.changeColor(RawOutStream::Colors::BLACK, true);
         }
         stream << "note: ";
         break;
      case SourceMgr::DK_Remark:
         if (showColors) {
            stream.changeColor(RawOutStream::Colors::BLUE, true);
         }
         stream << "remark: ";
         break;
      }

      if (showColors) {
         stream.resetColor();
         stream.changeColor(RawOutStream::Colors::SAVEDCOLOR, true);
      }
   }
   stream << m_message << '\n';
   if (showColors) {
      stream.resetColor();
   }
   if (m_lineNo == -1 || m_columnNo == -1) {
      return;
   }
   // FIXME: If there are multibyte or multi-column characters in the source, all
   // our m_ranges will be wrong. To do this properly, we'll need a byte-to-column
   // map like Clang's TextDiagnostic. For now, we'll just handle tabs by
   // expanding them later, and bail out rather than show incorrect m_ranges and
   // misaligned fixits for any other odd characters.
   if (find_if(m_lineContents, is_non_ascii) != m_lineContents.end()) {
      print_source_line(stream, m_lineContents);
      return;
   }
   size_t m_numColumns = m_lineContents.size();

   // Build the line with the caret and m_ranges.
   std::string caretLine(m_numColumns+1, ' ');

   // Expand any ranges.
   for (unsigned r = 0, e = m_ranges.size(); r != e; ++r) {
      std::pair<unsigned, unsigned> range = m_ranges[r];
      std::fill(&caretLine[range.first],
            &caretLine[std::min((size_t)range.second, caretLine.size())],
            '~');
   }

   // Add any fix-its.
   // FIXME: Find the beginning of the line properly for multibyte characters.
   std::string fixItInsertionLine;
   build_fixit_line(caretLine, fixItInsertionLine, m_fixIts,
                    make_array_ref(m_location.getPointer() - m_columnNo,
                                   m_lineContents.size()));

   // Finally, plop on the caret.
   if (unsigned(m_columnNo) <= m_numColumns) {
      caretLine[m_columnNo] = '^';
   } else {
      caretLine[m_numColumns] = '^';
   }
   // ... and remove trailing whitespace so the output doesn't wrap for it.  We
   // know that the line isn't completely empty because it has the caret in it at
   // least.
   caretLine.erase(caretLine.find_last_not_of(' ')+1);
   print_source_line(stream, m_lineContents);
   if (showColors) {
      stream.changeColor(RawOutStream::Colors::GREEN, true);
   }
   // Print out the caret line, matching tabs in the source line.
   for (unsigned i = 0, e = caretLine.size(), outCol = 0; i != e; ++i) {
      if (i >= m_lineContents.size() || m_lineContents[i] != '\t') {
         stream << caretLine[i];
         ++outCol;
         continue;
      }

      // Okay, we have a tab.  Insert the appropriate number of characters.
      do {
         stream << caretLine[i];
         ++outCol;
      } while ((outCol % sg_tabStop) != 0);
   }
   stream << '\n';

   if (showColors) {
      stream.resetColor();
   }
   // Print out the replacement line, matching tabs in the source line.
   if (fixItInsertionLine.empty()) {
      return;
   }


   for (size_t i = 0, e = fixItInsertionLine.size(), outCol = 0; i < e; ++i) {
      if (i >= m_lineContents.size() || m_lineContents[i] != '\t') {
         stream << fixItInsertionLine[i];
         ++outCol;
         continue;
      }

      // Okay, we have a tab.  Insert the appropriate number of characters.
      do {
         stream << fixItInsertionLine[i];
         // FIXME: This is trying not to break up replacements, but then to re-sync
         // with the tabs between replacements. This will fail, though, if two
         // fix-it replacements are exactly adjacent, or if a fix-it contains a
         // space. Really we should be precomputing column widths, which we'll
         // need anyway for multibyte chars.
         if (fixItInsertionLine[i] != ' ') {
            ++i;
         }
         ++outCol;
      } while (((outCol % sg_tabStop) != 0) && i != e);
   }
   stream << '\n';
}

} // utils
} // polar
