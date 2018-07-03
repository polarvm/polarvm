// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/07/01.

#ifndef POLAR_UTILS_DYNAMIC_LIBRARY_H
#define POLAR_UTILS_DYNAMIC_LIBRARY_H

#include <string>
#include <vector>
#include <algorithm>
#include "polar/global/Global.h"
#include "polar/basic/adt/StlExtras.h"

namespace polar {

// forward declare class with namespace
namespace basic {
class StringRef;
} // basic

namespace sys {

using polar::basic::StringRef;

/// This class provides a portable interface to dynamic libraries which also
/// might be known as shared libraries, shared objects, dynamic shared
/// objects, or dynamic link libraries. Regardless of the terminology or the
/// operating system interface, this class provides a portable interface that
/// allows dynamic libraries to be loaded and searched for externally
/// defined symbols. This is typically used to provide "plug-in" support.
/// It also allows for symbols to be defined which don't live in any library,
/// but rather the main program itself, useful on Windows where the main
/// executable cannot be searched.
///
/// Note: there is currently no interface for temporarily loading a library,
/// or for unloading libraries when the LLVM library is unloaded.
class DynamicLibrary
{
   // Placeholder whose address represents an invalid library.
   // We use this instead of NULL or a pointer-int pair because the OS library
   // might define 0 or 1 to be "special" handles, such as "search all".
   static char sm_invalid;

   // Opaque data used to interface with OS-specific dynamic library handling.
   void *m_data;

public:
   explicit DynamicLibrary(void *data = &sm_invalid) : m_data(data)
   {}

   /// Returns true if the object refers to a valid library.
   bool isValid() const
   {
      return m_data != &sm_invalid;
   }

   /// Searches through the library for the symbol \p symbolName. If it is
   /// found, the address of that symbol is returned. If not, NULL is returned.
   /// Note that NULL will also be returned if the library failed to load.
   /// Use isValid() to distinguish these cases if it is important.
   /// Note that this will \e not search symbols explicitly registered by
   /// addSymbol().
   void *getAddressOfSymbol(const char *symbolName);

   /// This function permanently loads the dynamic library at the given path.
   /// The library will only be unloaded when llvm_shutdown() is called.
   /// This returns a valid DynamicLibrary instance on success and an invalid
   /// instance on failure (see isValid()). \p *errMsg will only be modified
   /// if the library fails to load.
   ///
   /// It is safe to call this function multiple times for the same library.
   /// @brief Open a dynamic library permanently.
   static DynamicLibrary getPermanentLibrary(const char *filename,
                                             std::string *errMsg = nullptr);

   /// Registers an externally loaded library. The library will be unloaded
   /// when the program terminates.
   ///
   /// It is safe to call this function multiple times for the same library,
   /// though ownership is only taken if there was no error.
   ///
   /// \returns An empty \p DynamicLibrary if the library was already loaded.
   static DynamicLibrary addPermanentLibrary(void *handle,
                                             std::string *errMsg = nullptr);

   /// This function permanently loads the dynamic library at the given path.
   /// Use this instead of getPermanentLibrary() when you won't need to get
   /// symbols from the library itself.
   ///
   /// It is safe to call this function multiple times for the same library.
   static bool LoadLibraryPermanently(const char *filename,
                                      std::string *errorMsg = nullptr)
   {
      return !getPermanentLibrary(filename, errorMsg).isValid();
   }

   enum SearchOrdering
   {
      /// SO_Linker - Search as a call to dlsym(dlopen(NULL)) would when
      /// DynamicLibrary::getPermanentLibrary(NULL) has been called or
      /// search the list of explcitly loaded symbols if not.
      SO_Linker,
      /// SO_LoadedFirst - Search all loaded libraries, then as SO_Linker would.
      SO_LoadedFirst,
      /// SO_LoadedLast - Search as SO_Linker would, then loaded libraries.
      /// Only useful to search if libraries with RTLD_LOCAL have been added.
      SO_LoadedLast,
      /// SO_LoadOrder - Or this in to search libraries in the ordered loaded.
      /// The default bahaviour is to search loaded libraries in reverse.
      SO_LoadOrder = 4
   };
   static SearchOrdering sm_searchOrder; // = SO_Linker

   /// This function will search through all previously loaded dynamic
   /// libraries for the symbol \p symbolName. If it is found, the address of
   /// that symbol is returned. If not, null is returned. Note that this will
   /// search permanently loaded libraries (getPermanentLibrary()) as well
   /// as explicitly registered symbols (addSymbol()).
   /// @throws std::string on error.
   /// @brief Search through libraries for address of a symbol
   static void *searchForAddressOfSymbol(const char *symbolName);

   /// @brief Convenience function for C++ophiles.
   static void *searchForAddressOfSymbol(const std::string &symbolName)
   {
      return searchForAddressOfSymbol(symbolName.c_str());
   }

   /// This functions permanently adds the symbol \p symbolName with the
   /// value \p symbolValue.  These symbols are searched before any
   /// libraries.
   /// @brief Add searchable symbol/value pair.
   static void addSymbol(StringRef symbolName, void *symbolValue);

   class HandleSet;
};

// All methods for HandleSet should be used holding sg_symbolsMutex.
class DynamicLibrary::HandleSet
{
   typedef std::vector<void *> HandleList;
   HandleList m_handles;
   void *m_process;

public:
   static void *dllOpen(const char *filename, std::string *errorMsg);
   static void dllClose(void *handle);
   static void *dllSym(void *handle, const char *symbol);

   HandleSet() : m_process(nullptr)
   {}
   ~HandleSet();

   HandleList::iterator find(void *handle)
   {
      return std::find(m_handles.begin(), m_handles.end(), handle);
   }

   bool contains(void *handle)
   {
      return handle == m_process || find(handle) != m_handles.end();
   }

   bool addLibrary(void *handle, bool isProcess = false, bool canClose = true) {
#ifdef POLAR_ON_WIN32
      assert((handle == this ? isProcess : !isProcess) && "Bad handle.");
#endif

      if (POLAR_LIKELY(!isProcess)) {
         if (find(handle) != m_handles.end()) {
            if (canClose) {
               dllClose(handle);
            }
            return false;
         }
         m_handles.push_back(handle);
      } else {
#ifndef POLAR_ON_WIN32
         if (m_process) {
            if (canClose) {
               dllClose(m_process);
            }
            if (m_process == handle) {
               return false;
            }
         }
#endif
         m_process = handle;
      }
      return true;
   }

   void *libLookup(const char *symbol, DynamicLibrary::SearchOrdering order) {
      if (order & SearchOrdering::SO_LoadOrder) {
         for (void *handle : m_handles) {
            if (void *ptr = dllSym(handle, symbol)) {
               return ptr;
            }
         }
      } else {
         for (void *handle : polar::basic::reverse(m_handles)) {
            if (void *ptr = dllSym(handle, symbol)) {
               return ptr;
            }
         }
      }
      return nullptr;
   }

   void *lookup(const char *symbol, DynamicLibrary::SearchOrdering order)
   {
      assert(!((order & SearchOrdering::SO_LoadedFirst) && (order & SearchOrdering::SO_LoadedLast)) &&
             "Invalid Ordering");

      if (!m_process || (order & SearchOrdering::SO_LoadedFirst)) {
         if (void *ptr = libLookup(symbol, order)) {
            return ptr;
         }
      }
      if (m_process) {
         // Use OS facilities to search the current binary and all loaded libs.
         if (void *ptr = dllSym(m_process, symbol)) {
            return ptr;
         }
         // Search any libs that might have been skipped because of RTLD_LOCAL.
         if (order & SearchOrdering::SO_LoadedLast) {
            if (void *ptr = libLookup(symbol, order)) {
               return ptr;
            }
         }
      }
      return nullptr;
   }
};

} // utils
} // polar

#endif // POLAR_UTILS_DYNAMIC_LIBRARY_H
