// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
// 
// Created by softboy on 2018/05/30.

#ifndef POLAR_UTILS_RAW_OS_OUT_STREAM_H
#define POLAR_UTILS_RAW_OS_OUT_STREAM_H

#include "polar/basic/adt/SmallVector.h"
#include "polar/basic/adt/StringRef.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <system_error>

namespace polar {

namespace system {
namespace fs {

enum class OpenFlags : unsigned;

} // fs
} // system

namespace utils {

class FormatvObjectBase;
class FormatObjectBase;
class FormattedString;
class FormattedNumber;
class FormattedBytes;

/// This class implements an extremely fast bulk output stream that can *only*
/// output to a stream.  It does not support seeking, reopening, rewinding, line
/// buffered disciplines etc. It is a simple buffer that outputs
/// a chunk at a time.
class RawOstream {
private:
   /// The buffer is handled in such a way that the buffer is
   /// uninitialized, unbuffered, or out of space when m_outBufCur >=
   /// m_outBufEnd. Thus a single comparison suffices to determine if we
   /// need to take the slow path to write a single character.
   ///
   /// The buffer is in one of three states:
   ///  1. Unbuffered (m_bufferMode == Unbuffered)
   ///  1. Uninitialized (m_bufferMode != Unbuffered && m_outBufStart == 0).
   ///  2. Buffered (m_bufferMode != Unbuffered && m_outBufStart != 0 &&
   ///               m_outBufEnd - m_outBufStart >= 1).
   ///
   /// If buffered, then the RawOstream owns the buffer if (m_bufferMode ==
   /// InternalBuffer); otherwise the buffer has been set via SetBuffer and is
   /// managed by the subclass.
   ///
   /// If a subclass installs an external buffer using SetBuffer then it can wait
   /// for a \see write_impl() call to handle the data which has been put into
   /// this buffer.
   char *m_outBufStart;
   char *m_outBufEnd;
   char *m_outBufCur;
   
   enum class BufferKind
   {
      Unbuffered = 0,
      InternalBuffer,
      ExternalBuffer
   } m_bufferMode;
   
public:
   // color order matches ANSI escape sequence, don't change
   enum class Colors
   {
      BLACK = 0,
      RED,
      GREEN,
      YELLOW,
      BLUE,
      MAGENTA,
      CYAN,
      WHITE,
      SAVEDCOLOR
   };
   
   explicit RawOstream(bool unbuffered = false)
      : m_bufferMode(unbuffered ? BufferKind::Unbuffered : BufferKind::InternalBuffer)
   {
      // Start out ready to flush.
      m_outBufStart = m_outBufEnd = m_outBufCur = nullptr;
   }
   
   RawOstream(const RawOstream &) = delete;
   void operator=(const RawOstream &) = delete;
   
   virtual ~RawOstream();
   
   /// tell - Return the current offset with the file.
   uint64_t tell() const
   {
      return getCurrentPos() + getNumBytesInBuffer();
   }
   
   //===--------------------------------------------------------------------===//
   // Configuration Interface
   //===--------------------------------------------------------------------===//
   
   /// Set the stream to be buffered, with an automatically determined buffer
   /// size.
   void setBuffered();
   
   /// Set the stream to be buffered, using the specified buffer size.
   void setBufferSize(size_t Size)
   {
      flush();
      setBufferAndMode(new char[Size], Size, BufferKind::InternalBuffer);
   }
   
   size_t getBufferSize() const
   {
      // If we're supposed to be buffered but haven't actually gotten around
      // to allocating the buffer yet, return the value that would be used.
      if (m_bufferMode != BufferKind::Unbuffered && m_outBufStart == nullptr) {
         return preferred_buffer_size();
      }
      
      // Otherwise just return the size of the allocated buffer.
      return m_outBufEnd - m_outBufStart;
   }
   
   /// Set the stream to be unbuffered. When unbuffered, the stream will flush
   /// after every write. This routine will also flush the buffer immediately
   /// when the stream is being set to unbuffered.
   void setUnbuffered()
   {
      flush();
      setBufferAndMode(nullptr, 0, BufferKind::Unbuffered);
   }
   
   size_t getNumBytesInBuffer() const
   {
      return m_outBufCur - m_outBufStart;
   }
   
   //===--------------------------------------------------------------------===//
   // Data Output Interface
   //===--------------------------------------------------------------------===//
   
   void flush()
   {
      if (m_outBufCur != m_outBufStart) {
         flushNonEmpty();
      }
   }
   
   RawOstream &operator<<(char character)
   {
      if (m_outBufCur >= m_outBufEnd) {
         return write(character);
      }
      *m_outBufCur++ = character;
      return *this;
   }
   
   RawOstream &operator<<(unsigned char character)
   {
      if (m_outBufCur >= m_outBufEnd) {
         return write(character);
      }
      *m_outBufCur++ = character;
      return *this;
   }
   
   RawOstream &operator<<(signed char character) {
      if (m_outBufCur >= m_outBufEnd) {
         return write(character);
      }
      *m_outBufCur++ = character;
      return *this;
   }
   
   RawOstream &operator<<(StringRef str) {
      // Inline fast path, particularly for strings with a known length.
      size_t size = str.getSize();
      
      // Make sure we can use the fast path.
      if (size > (size_t)(m_outBufEnd - m_outBufCur))
         return write(str.getData(), size);
      
      if (size) {
         memcpy(m_outBufCur, str.getData(), size);
         m_outBufCur += size;
      }
      return *this;
   }
   
   RawOstream &operator<<(const char *str)
   {
      // Inline fast path, particularly for constant strings where a sufficiently
      // smart compiler will simplify strlen.
      
      return this->operator<<(StringRef(str));
   }
   
   RawOstream &operator<<(const std::string &str)
   {
      // Avoid the fast path, it would only increase code size for a marginal win.
      return write(str.data(), str.length());
   }
   
   RawOstream &operator<<(const polar::basic::SmallVectorImpl<char> &str)
   {
      return write(str.getData(), str.getSize());
   }
   
   RawOstream &operator<<(unsigned long num);
   RawOstream &operator<<(long num);
   RawOstream &operator<<(unsigned long long num);
   RawOstream &operator<<(long long num);
   RawOstream &operator<<(const void *ptr);
   
   RawOstream &operator<<(unsigned int num)
   {
      return this->operator<<(static_cast<unsigned long>(num));
   }
   
   RawOstream &operator<<(int N) {
      return this->operator<<(static_cast<long>(N));
   }
   
   RawOstream &operator<<(double N);
   
   /// Output \p N in hexadecimal, without any prefix or padding.
   RawOstream &write_hex(unsigned long long N);
   
   /// Output a formatted UUID with dash separators.
   using uuid_t = uint8_t[16];
   RawOstream &write_uuid(const uuid_t UUID);
   
   /// Output \p Str, turning '\\', '\t', '\n', '"', and anything that doesn't
   /// satisfy std::isprint into an escape sequence.
   RawOstream &write_escaped(StringRef Str, bool UseHexEscapes = false);
   
   RawOstream &write(unsigned char C);
   RawOstream &write(const char *Ptr, size_t Size);
   
   // Formatted output, see the format() function in Support/Format.h.
   RawOstream &operator<<(const format_object_base &Fmt);
   
   // Formatted output, see the leftJustify() function in Support/Format.h.
   RawOstream &operator<<(const FormattedString &);
   
   // Formatted output, see the formatHex() function in Support/Format.h.
   RawOstream &operator<<(const FormattedNumber &);
   
   // Formatted output, see the formatv() function in Support/FormatVariadic.h.
   RawOstream &operator<<(const formatv_object_base &);
   
   // Formatted output, see the format_bytes() function in Support/Format.h.
   RawOstream &operator<<(const FormattedBytes &);
   
   /// indent - Insert 'NumSpaces' spaces.
   RawOstream &indent(unsigned NumSpaces);
   
   /// Changes the foreground color of text that will be output from this point
   /// forward.
   /// @param Color ANSI color to use, the special SAVEDCOLOR can be used to
   /// change only the bold attribute, and keep colors untouched
   /// @param Bold bold/brighter text, default false
   /// @param BG if true change the background, default: change foreground
   /// @returns itself so it can be used within << invocations
   virtual RawOstream &changeColor(enum Colors Color,
                                    bool Bold = false,
                                    bool BG = false) {
      (void)Color;
      (void)Bold;
      (void)BG;
      return *this;
   }
   
   /// Resets the colors to terminal defaults. Call this when you are done
   /// outputting colored text, or before program exit.
   virtual RawOstream &resetColor() { return *this; }
   
   /// Reverses the foreground and background colors.
   virtual RawOstream &reverseColor() { return *this; }
   
   /// This function determines if this stream is connected to a "tty" or
   /// "console" window. That is, the output would be displayed to the user
   /// rather than being put on a pipe or stored in a file.
   virtual bool is_displayed() const { return false; }
   
   /// This function determines if this stream is displayed and supports colors.
   virtual bool has_colors() const { return is_displayed(); }
   
   //===--------------------------------------------------------------------===//
   // Subclass Interface
   //===--------------------------------------------------------------------===//
   
private:
   /// The is the piece of the class that is implemented by subclasses.  This
   /// writes the \p Size bytes starting at
   /// \p Ptr to the underlying stream.
   ///
   /// This function is guaranteed to only be called at a point at which it is
   /// safe for the subclass to install a new buffer via SetBuffer.
   ///
   /// \param Ptr The start of the data to be written. For buffered streams this
   /// is guaranteed to be the start of the buffer.
   ///
   /// \param Size The number of bytes to be written.
   ///
   /// \invariant { Size > 0 }
   virtual void write_impl(const char *Ptr, size_t Size) = 0;
   
   // An out of line virtual method to provide a home for the class vtable.
   virtual void handle();
   
   /// Return the current position within the stream, not counting the bytes
   /// currently in the buffer.
   virtual uint64_t getCurrentPos() const = 0;
   
protected:
   /// Use the provided buffer as the RawOstream buffer. This is intended for
   /// use only by subclasses which can arrange for the output to go directly
   /// into the desired output buffer, instead of being copied on each flush.
   void SetBuffer(char *BufferStart, size_t Size) {
      setBufferAndMode(BufferStart, Size, ExternalBuffer);
   }
   
   /// Return an efficient buffer size for the underlying output mechanism.
   virtual size_t preferred_buffer_size() const;
   
   /// Return the beginning of the current stream buffer, or 0 if the stream is
   /// unbuffered.
   const char *getBufferStart() const { return m_outBufStart; }
   
   //===--------------------------------------------------------------------===//
   // Private Interface
   //===--------------------------------------------------------------------===//
private:
   /// Install the given buffer and mode.
   void setBufferAndMode(char *BufferStart, size_t Size, BufferKind Mode);
   
   /// Flush the current buffer, which is known to be non-empty. This outputs the
   /// currently buffered data and resets the buffer to empty.
   void flushNonEmpty();
   
   /// Copy data into the buffer. Size must not be greater than the number of
   /// unused bytes in the buffer.
   void copy_to_buffer(const char *Ptr, size_t Size);
};

/// An abstract base class for streams implementations that also support a
/// pwrite operation. This is useful for code that can mostly stream out data,
/// but needs to patch in a header that needs to know the output size.
class raw_pwrite_stream : public RawOstream {
   virtual void pwrite_impl(const char *Ptr, size_t Size, uint64_t Offset) = 0;
   
public:
   explicit raw_pwrite_stream(bool Unbuffered = false)
      : RawOstream(Unbuffered) {}
   void pwrite(const char *Ptr, size_t Size, uint64_t Offset) {
#ifndef NDBEBUG
      uint64_t Pos = tell();
      // /dev/null always reports a pos of 0, so we cannot perform this check
      // in that case.
      if (Pos)
         assert(Size + Offset <= Pos && "We don't support extending the stream");
#endif
      pwrite_impl(Ptr, Size, Offset);
   }
};

//===----------------------------------------------------------------------===//
// File Output Streams
//===----------------------------------------------------------------------===//

/// A RawOstream that writes to a file descriptor.
///
class raw_fd_ostream : public raw_pwrite_stream {
   int FD;
   bool ShouldClose;
   
   std::error_code EC;
   
   uint64_t pos;
   
   bool SupportsSeeking;
   
   /// See RawOstream::write_impl.
   void write_impl(const char *Ptr, size_t Size) override;
   
   void pwrite_impl(const char *Ptr, size_t Size, uint64_t Offset) override;
   
   /// Return the current position within the stream, not counting the bytes
   /// currently in the buffer.
   uint64_t getCurrentPos() const override { return pos; }
   
   /// Determine an efficient buffer size.
   size_t preferred_buffer_size() const override;
   
   /// Set the flag indicating that an output error has been encountered.
   void error_detected(std::error_code EC) { this->EC = EC; }
   
public:
   /// Open the specified file for writing. If an error occurs, information
   /// about the error is put into EC, and the stream should be immediately
   /// destroyed;
   /// \p Flags allows optional flags to control how the file will be opened.
   ///
   /// As a special case, if Filename is "-", then the stream will use
   /// STDOUT_FILENO instead of opening a file. This will not close the stdout
   /// descriptor.
   raw_fd_ostream(StringRef Filename, std::error_code &EC,
                  sys::fs::OpenFlags Flags);
   
   /// FD is the file descriptor that this writes to.  If ShouldClose is true,
   /// this closes the file when the stream is destroyed. If FD is for stdout or
   /// stderr, it will not be closed.
   raw_fd_ostream(int fd, bool shouldClose, bool unbuffered=false);
   
   ~raw_fd_ostream() override;
   
   /// Manually flush the stream and close the file. Note that this does not call
   /// fsync.
   void close();
   
   bool supportsSeeking() { return SupportsSeeking; }
   
   /// Flushes the stream and repositions the underlying file descriptor position
   /// to the offset specified from the beginning of the file.
   uint64_t seek(uint64_t off);
   
   RawOstream &changeColor(enum Colors colors, bool bold=false,
                            bool bg=false) override;
   RawOstream &resetColor() override;
   
   RawOstream &reverseColor() override;
   
   bool is_displayed() const override;
   
   bool has_colors() const override;
   
   std::error_code error() const { return EC; }
   
   /// Return the value of the flag in this raw_fd_ostream indicating whether an
   /// output error has been encountered.
   /// This doesn't implicitly flush any pending output.  Also, it doesn't
   /// guarantee to detect all errors unless the stream has been closed.
   bool has_error() const { return bool(EC); }
   
   /// Set the flag read by has_error() to false. If the error flag is set at the
   /// time when this RawOstream's destructor is called, report_fatal_error is
   /// called to report the error. Use clear_error() after handling the error to
   /// avoid this behavior.
   ///
   ///   "Errors should never pass silently.
   ///    Unless explicitly silenced."
   ///      - from The Zen of Python, by Tim Peters
   ///
   void clear_error() { EC = std::error_code(); }
};

/// This returns a reference to a RawOstream for standard output. Use it like:
/// outs() << "foo" << "bar";
RawOstream &outs();

/// This returns a reference to a RawOstream for standard error. Use it like:
/// errs() << "foo" << "bar";
RawOstream &errs();

/// This returns a reference to a RawOstream which simply discards output.
RawOstream &nulls();

//===----------------------------------------------------------------------===//
// Output Stream Adaptors
//===----------------------------------------------------------------------===//

/// A RawOstream that writes to an std::string.  This is a simple adaptor
/// class. This class does not encounter output errors.
class raw_string_ostream : public RawOstream {
   std::string &OS;
   
   /// See RawOstream::write_impl.
   void write_impl(const char *Ptr, size_t Size) override;
   
   /// Return the current position within the stream, not counting the bytes
   /// currently in the buffer.
   uint64_t getCurrentPos() const override { return OS.size(); }
   
public:
   explicit raw_string_ostream(std::string &O) : OS(O) {}
   ~raw_string_ostream() override;
   
   /// Flushes the stream contents to the target string and returns  the string's
   /// reference.
   std::string& str() {
      flush();
      return OS;
   }
};

/// A RawOstream that writes to an SmallVector or SmallString.  This is a
/// simple adaptor class. This class does not encounter output errors.
/// raw_svector_ostream operates without a buffer, delegating all memory
/// management to the SmallString. Thus the SmallString is always up-to-date,
/// may be used directly and there is no need to call flush().
class raw_svector_ostream : public raw_pwrite_stream {
   SmallVectorImpl<char> &OS;
   
   /// See RawOstream::write_impl.
   void write_impl(const char *Ptr, size_t Size) override;
   
   void pwrite_impl(const char *Ptr, size_t Size, uint64_t Offset) override;
   
   /// Return the current position within the stream.
   uint64_t getCurrentPos() const override;
   
public:
   /// Construct a new raw_svector_ostream.
   ///
   /// \param O The vector to write to; this should generally have at least 128
   /// bytes free to avoid any extraneous memory overhead.
   explicit raw_svector_ostream(SmallVectorImpl<char> &O) : OS(O) {
      setUnbuffered();
   }
   
   ~raw_svector_ostream() override = default;
   
   void flush() = delete;
   
   /// Return a StringRef for the vector contents.
   StringRef str() { return StringRef(OS.data(), OS.size()); }
};

/// A RawOstream that discards all output.
class raw_null_ostream : public raw_pwrite_stream {
   /// See RawOstream::write_impl.
   void write_impl(const char *Ptr, size_t size) override;
   void pwrite_impl(const char *Ptr, size_t Size, uint64_t Offset) override;
   
   /// Return the current position within the stream, not counting the bytes
   /// currently in the buffer.
   uint64_t getCurrentPos() const override;
   
public:
   explicit raw_null_ostream() = default;
   ~raw_null_ostream() override;
};

class buffer_ostream : public raw_svector_ostream {
   RawOstream &OS;
   SmallVector<char, 0> Buffer;
   
public:
   buffer_ostream(RawOstream &OS) : raw_svector_ostream(Buffer), OS(OS) {}
   ~buffer_ostream() override { OS << str(); }
};

} // utils
} // polar

#endif // POLAR_UTILS_RAW_OS_OUT_STREAM_H
