// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/06/03.

#include "polar/utils/Allocator.h"
#include "polar/utils/RawOutStream.h"

namespace polar {
namespace utils {

namespace internal {

void print_bump_ptr_allocator_stats(unsigned numSlabs, size_t bytesAllocated,
                                    size_t totalMemory)
{
   error_stream() << "\nNumber of memory regions: " << numSlabs << '\n'
                  << "Bytes used: " << bytesAllocated << '\n'
                  << "Bytes allocated: " << totalMemory << '\n'
                  << "Bytes wasted: " << (totalMemory - bytesAllocated)
                  << " (includes alignment, etc)\n";
}
} // internal

void print_recycler_stats(size_t size,
                          size_t align,
                          size_t freeListSize)
{
   error_stream() << "Recycler element size: " << size << '\n'
                  << "Recycler element alignment: " << align << '\n'
                  << "Number of elements free for recycling: " << freeListSize << '\n';
}

} // utils
} // polar
