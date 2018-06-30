// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/06/09.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Unix specific implementation of the Path API.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only generic UNIX code that
//===          is guaranteed to work on *all* UNIX variants.
//===----------------------------------------------------------------------===//

#include "polar/global/platform/Unix.h"
#include "polar/utils/FileSystem.h"
#include "polar/basic/adt/StringRef.h"
#include "polar/utils/Chrono.h"
#include "polar/utils/Path.h"
#include "polar/utils/ErrorCode.h"
#include "polar/utils/Process.h"
#include <limits.h>
#include <stdio.h>
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif

#include <dirent.h>
#include <pwd.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <sys/attr.h>
#endif

// Both stdio.h and cstdio are included via different paths and
// stdcxx's cstdio doesn't include stdio.h, so it doesn't #undef the macros
// either.
#undef ferror
#undef feof

// For GNU Hurd
#if defined(__GNU__) && !defined(PATH_MAX)
# define PATH_MAX 4096
#endif

#include <sys/types.h>
#if !defined(__APPLE__) && !defined(__OpenBSD__) && !defined(__FreeBSD__) &&   \
   !defined(__linux__)
#include <sys/statvfs.h>
#define STATVFS statvfs
#define FSTATVFS fstatvfs
#define STATVFS_F_FRSIZE(vfs) vfs.f_frsize
#else
#if defined(__OpenBSD__) || defined(__FreeBSD__)
#include <sys/mount.h>
#include <sys/param.h>
#elif defined(__linux__)
#if defined(HAVE_LINUX_MAGIC_H)
#include <linux/magic.h>
#else
#if defined(HAVE_LINUX_NFS_FS_H)
#include <linux/nfs_fs.h>
#endif
#if defined(HAVE_LINUX_SMB_H)
#include <linux/smb.h>
#endif
#endif
#include <sys/vfs.h>
#else
#include <sys/mount.h>
#endif
#define STATVFS statfs
#define FSTATVFS fstatfs
#define STATVFS_F_FRSIZE(vfs) static_cast<uint64_t>(vfs.f_bsize)
#endif

#if defined(__NetBSD__)
#define STATVFS_F_FLAG(vfs) (vfs).f_flag
#else
#define STATVFS_F_FLAG(vfs) (vfs).f_flags
#endif


namespace polar {
namespace fs {

using polar::sys::Process;

namespace {
#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) ||     \
   defined(__minix) || defined(__FreeBSD_kernel__) || defined(__linux__) ||   \
   defined(__CYGWIN__) || defined(__DragonFly__) || defined(_AIX)
int
test_dir(char ret[PATH_MAX], const char *dir, const char *bin)
{
   struct stat sb;
   char fullpath[PATH_MAX];
   snprintf(fullpath, PATH_MAX, "%s/%s", dir, bin);
   if (!realpath(fullpath, ret)) {
      return 1;
   }
   if (stat(fullpath, &sb) != 0) {
      return 1;
   }
   return 0;
}

char *
get_program_path(char ret[PATH_MAX], const char *bin)
{
   char *pv, *s, *t;

   /* First approach: absolute path. */
   if (bin[0] == '/') {
      if (test_dir(ret, "/", bin) == 0) {
         return ret;
      }
      return nullptr;
   }

   /* Second approach: relative path. */
   if (strchr(bin, '/')) {
      char cwd[PATH_MAX];
      if (!getcwd(cwd, PATH_MAX)) {
         return nullptr;
      }
      if (test_dir(ret, cwd, bin) == 0) {
         return ret;
      }
      return nullptr;
   }

   /* Third approach: $PATH */
   if ((pv = getenv("PATH")) == nullptr) {
      return nullptr;
   }
   s = pv = strdup(pv);
   if (!pv) {
      return nullptr;
   }

   while ((t = strsep(&s, ":")) != nullptr) {
      if (test_dir(ret, t, bin) == 0) {
         free(pv);
         return ret;
      }
   }
   free(pv);
   return nullptr;
}
#endif // __FreeBSD__ || __NetBSD__ || __FreeBSD_kernel__

} // anonymous namespacd

/// get_main_executable - Return the path to the main executable, given the
/// value of argv[0] from program startup.
std::string get_main_executable(const char *argv0, void *mainAddr) {
#if defined(__APPLE__)
   // On OS X the executable path is saved to the stack by dyld. Reading it
   // from there is much faster than calling dladdr, especially for large
   // binaries with symbols.
   char exePath[MAXPATHLEN];
   uint32_t size = sizeof(exePath);
   if (_NSGetExecutablePath(exePath, &size) == 0) {
      char linkPath[MAXPATHLEN];
      if (::realpath(exePath, linkPath)) {
         return linkPath;
      }
   }
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) ||   \
   defined(__minix) || defined(__DragonFly__) ||                              \
   defined(__FreeBSD_kernel__) || defined(_AIX)
   char exePath[PATH_MAX];

   if (get_program_path(exePath, argv0) != NULL) {
      return exePath;
   }
#elif defined(__linux__) || defined(__CYGWIN__)
   char exePath[MAXPATHLEN];
   StringRef aPath("/proc/self/exe");
   if (fs::exists(aPath)) {
      // /proc is not always mounted under Linux (chroot for example).
      ssize_t len = readlink(aPath.getStr().c_str(), exePath, sizeof(exePath));
      if (len >= 0) {
         return std::string(exePath, len);
      }
   } else {
      // Fall back to the classical detection.
      if (get_program_path(exePath, argv0)) {
         return exePath;
      }
   }
#elif defined(HAVE_DLFCN_H) && defined(HAVE_DLADDR)
   // Use dladdr to get executable path if available.
   Dl_info DLInfo;
   int err = dladdr(mainAddr, &DLInfo);
   if (err == 0) {
      return "";
   }

   // If the filename is a symlink, we need to resolve and return the location of
   // the actual executable.
   char linkPath[MAXPATHLEN];
   if (realPath(DLInfo.dli_fname, linkPath)) {
      return linkPath;
   }

#else
#error get_main_executable is not implemented on this host yet.
#endif
   return "";
}

TimePoint<> BasicFileStatus::getLastAccessedTime() const
{
   return polar::utils::to_time_point(m_fsStatusAtime);
}

TimePoint<> BasicFileStatus::getLastModificationTime() const
{
   return polar::utils::to_time_point(m_fsStatusMtime);
}

UniqueId FileStatus::getUniqueId() const
{
   return UniqueId(m_fsStatusDev, m_fsStatusInode);
}

uint32_t FileStatus::getLinkCount() const
{
   return m_fsStatusNlinks;
}

OptionalError<SpaceInfo> disk_space(const Twine &path)
{
   struct STATVFS vfs;
   if (::STATVFS(path.getStr().c_str(), &vfs)) {
      return std::error_code(errno, std::generic_category());
   }
   auto frSize = STATVFS_F_FRSIZE(vfs);
   SpaceInfo spaceInfo;
   spaceInfo.capacity = static_cast<uint64_t>(vfs.f_blocks) * frSize;
   spaceInfo.free = static_cast<uint64_t>(vfs.f_bfree) * frSize;
   spaceInfo.available = static_cast<uint64_t>(vfs.f_bavail) * frSize;
   return spaceInfo;
}

std::error_code current_path(SmallVectorImpl<char> &result)
{
   result.clear();
   const char *pwd = ::getenv("PWD");
   FileStatus pwdStatus, dotStatus;
   if (pwd && path::is_absolute(pwd) &&
       !status(pwd, pwdStatus) &&
       !status(".", dotStatus) &&
       pwdStatus.getUniqueId() == dotStatus.getUniqueId()) {
      result.append(pwd, pwd + strlen(pwd));
      return std::error_code();
   }

#ifdef MAXPATHLEN
   result.reserve(MAXPATHLEN);
#else
   // For GNU Hurd
   result.reserve(1024);
#endif

   while (true) {
      if (::getcwd(result.getData(), result.getCapacity()) == nullptr) {
         // See if there was a real error.
         if (errno != ENOMEM) {
            return std::error_code(errno, std::generic_category());
         }
         // Otherwise there just wasn't enough space.
         result.reserve(result.getCapacity() * 2);
      } else {
         break;
      }
   }
   result.setSize(strlen(result.getData()));
   return std::error_code();
}

std::error_code set_current_path(const Twine &path)
{
   SmallString<128> pathStorage;
   StringRef p = path.toNullTerminatedStringRef(pathStorage);
   if (::chdir(p.begin()) == -1) {
      return std::error_code(errno, std::generic_category());
   }
   return std::error_code();
}

std::error_code create_directory(const Twine &path, bool ignoreExisting,
                                 Permission perms)
{
   SmallString<128> pathStorage;
   StringRef p = path.toNullTerminatedStringRef(pathStorage);
   if (::mkdir(p.begin(), perms) == -1) {
      if (errno != EEXIST || !ignoreExisting) {
         return std::error_code(errno, std::generic_category());
      }
   }
   return std::error_code();
}

// Note that we are using symbolic link because hard links are not supported by
// all filesystems (SMB doesn't).
std::error_code create_link(const Twine &to, const Twine &from)
{
   // Get arguments.
   SmallString<128> fromStorage;
   SmallString<128> toStorage;
   StringRef f = from.toNullTerminatedStringRef(fromStorage);
   StringRef t = to.toNullTerminatedStringRef(toStorage);
   if (::symlink(t.begin(), f.begin()) == -1) {
      return std::error_code(errno, std::generic_category());
   }
   return std::error_code();
}

std::error_code create_hard_link(const Twine &to, const Twine &from)
{
   // Get arguments.
   SmallString<128> fromStorage;
   SmallString<128> toStorage;
   StringRef f = from.toNullTerminatedStringRef(fromStorage);
   StringRef t = to.toNullTerminatedStringRef(toStorage);

   if (::link(t.begin(), f.begin()) == -1) {
      return std::error_code(errno, std::generic_category());
   }
   return std::error_code();
}

std::error_code remove(const Twine &path, bool IgnoreNonExisting)
{
   SmallString<128> pathStorage;
   StringRef p = path.toNullTerminatedStringRef(pathStorage);
   struct stat buf;
   if (lstat(p.begin(), &buf) != 0) {
      if (errno != ENOENT || !IgnoreNonExisting) {
         return std::error_code(errno, std::generic_category());
      }
      return std::error_code();
   }

   // Note: this check catches strange situations. In all cases, LLVM should
   // only be involved in the creation and deletion of regular files.  This
   // check ensures that what we're trying to erase is a regular file. It
   // effectively prevents LLVM from erasing things like /dev/null, any block
   // special file, or other things that aren't "regular" files.
   if (!S_ISREG(buf.st_mode) && !S_ISDIR(buf.st_mode) && !S_ISLNK(buf.st_mode)) {
      return make_error_code(ErrorCode::operation_not_permitted);
   }

   if (::remove(p.begin()) == -1) {
      if (errno != ENOENT || !IgnoreNonExisting) {
         return std::error_code(errno, std::generic_category());
      }
   }

   return std::error_code();
}

static bool is_local_impl(struct STATVFS &vfs) {
#if defined(__linux__)
#ifndef NFS_SUPER_MAGIC
#define NFS_SUPER_MAGIC 0x6969
#endif
#ifndef SMB_SUPER_MAGIC
#define SMB_SUPER_MAGIC 0x517B
#endif
#ifndef CIFS_MAGIC_NUMBER
#define CIFS_MAGIC_NUMBER 0xFF534D42
#endif
   switch ((uint32_t)vfs.f_type) {
   case NFS_SUPER_MAGIC:
   case SMB_SUPER_MAGIC:
   case CIFS_MAGIC_NUMBER:
      return false;
   default:
      return true;
   }
#elif defined(__CYGWIN__)
   // Cygwin doesn't expose this information; would need to use Win32 API.
   return false;
#elif defined(__Fuchsia__)
   // Fuchsia doesn't yet support remote filesystem mounts.
   return true;
#elif defined(__sun)
   // statvfs::f_basetype contains a null-terminated FSType name of the mounted target
   StringRef fstype(Vfs.f_basetype);
   // NFS is the only non-local fstype??
   return !fstype.equals("nfs");
#else
   return !!(STATVFS_F_FLAG(vfs) & MNT_LOCAL);
#endif
}

std::error_code is_local(const Twine &path, bool &result)
{
   struct STATVFS vfs;
   if (::STATVFS(path.getStr().c_str(), &vfs)) {
      return std::error_code(errno, std::generic_category());
   }
   result = is_local_impl(vfs);
   return std::error_code();
}

std::error_code is_local(int fd, bool &result)
{
   struct STATVFS vfs;
   if (::FSTATVFS(fd, &vfs)) {
      return std::error_code(errno, std::generic_category());
   }
   result = is_local_impl(vfs);
   return std::error_code();
}

std::error_code rename(const Twine &from, const Twine &to)
{
   // Get arguments.
   SmallString<128> fromStorage;
   SmallString<128> toStorage;
   StringRef f = from.toNullTerminatedStringRef(fromStorage);
   StringRef t = to.toNullTerminatedStringRef(toStorage);
   if (::rename(f.begin(), t.begin()) == -1) {
      return std::error_code(errno, std::generic_category());
   }
   return std::error_code();
}

std::error_code resize_file(int fd, uint64_t size)
{
#if defined(HAVE_POSIX_FALLOCATE)
   // If we have posix_fallocate use it. Unlike ftruncate it always allocates
   // space, so we get an error if the disk is full.
   if (int error = ::posix_fallocate(fd, 0, size)) {
      if (error != EINVAL && error != EOPNOTSUPP) {
         return std::error_code(error, std::generic_category());
      }
   }
#endif
   // Use ftruncate as a fallback. It may or may not allocate space. At least on
   // OS X with HFS+ it does.
   if (::ftruncate(fd, size) == -1) {
      return std::error_code(errno, std::generic_category());
   }
   return std::error_code();
}

namespace {
int convert_access_mode(AccessMode mode)
{
   switch (mode) {
   case AccessMode::Exist:
      return F_OK;
   case AccessMode::Write:
      return W_OK;
   case AccessMode::Execute:
      return R_OK | X_OK; // scripts also need R_OK.
   }
   polar_unreachable("invalid enum");
}
} // anonymous namespace

std::error_code access(const Twine &path, AccessMode mode)
{
   SmallString<128> pathStorage;
   StringRef p = path.toNullTerminatedStringRef(pathStorage);
   if (::access(p.begin(), convert_access_mode(mode)) == -1) {
      return std::error_code(errno, std::generic_category());
   }
   if (mode == AccessMode::Execute) {
      // Don't say that directories are executable.
      struct stat buf;
      if (0 != stat(p.begin(), &buf)) {
         return ErrorCode::permission_denied;
      }
      if (!S_ISREG(buf.st_mode)) {
         return ErrorCode::permission_denied;
      }
   }

   return std::error_code();
}

bool can_execute(const Twine &path)
{
   return !access(path, AccessMode::Execute);
}

bool equivalent(FileStatus lhs, FileStatus rhs)
{
   assert(status_known(lhs) && status_known(rhs));
   return lhs.m_fsStatusDev == rhs.m_fsStatusDev &&
         lhs.m_fsStatusInode == rhs.m_fsStatusInode;
}

std::error_code equivalent(const Twine &lhs, const Twine &rhs, bool &result)
{
   FileStatus fsLhs, fsRhs;
   if (std::error_code erroCode = status(lhs, fsLhs)) {
      return erroCode;
   }
   if (std::error_code erroCode = status(rhs, fsRhs)) {
      return erroCode;
   }
   result = equivalent(fsLhs, fsRhs);
   return std::error_code();
}

namespace {

void expand_tilde_expr(SmallVectorImpl<char> &pathVector)
{
   StringRef pathStr(pathVector.begin(), pathVector.getSize());
   if (pathStr.empty() || !pathStr.startsWith("~")) {
      return;
   }
   pathStr = pathStr.dropFront();
   StringRef expr =
         pathStr.takeUntil([](char c) { return path::is_separator(c); });
   StringRef remainder = pathStr.substr(expr.getSize() + 1);
   SmallString<128> storage;
   if (expr.empty()) {
      // This is just ~/..., resolve it to the current user's home dir.
      if (!path::home_directory(storage)) {
         // For some reason we couldn't get the home directory.  Just exit.
         return;
      }
      // Overwrite the first character and insert the rest.
      pathVector[0] = storage[0];
      pathVector.insert(pathVector.begin() + 1, storage.begin() + 1, storage.end());
      return;
   }

   // This is a string of the form ~username/, look up this user's entry in the
   // password database.
   struct passwd *entry = nullptr;
   std::string user = expr.getStr();
   entry = ::getpwnam(user.c_str());

   if (!entry) {
      // Unable to look up the entry, just return back the original path.
      return;
   }

   storage = remainder;
   pathVector.clear();
   pathVector.append(entry->pw_dir, entry->pw_dir + strlen(entry->pw_dir));
   path::append(pathVector, storage);
}

std::error_code fill_status(int statRet, const struct stat &status,
                            FileStatus &result) {
   if (statRet != 0) {
      std::error_code errorCode(errno, std::generic_category());
      if (errorCode == ErrorCode::no_such_file_or_directory) {
         result = FileStatus(FileType::file_not_found);
      } else {
         result = FileStatus(FileType::status_error);
      }
      return errorCode;
   }

   FileType type = FileType::type_unknown;

   if (S_ISDIR(status.st_mode)) {
      type = FileType::directory_file;
   } else if (S_ISREG(status.st_mode)) {
      type = FileType::regular_file;
   } else if (S_ISBLK(status.st_mode)) {
      type = FileType::block_file;
   } else if (S_ISCHR(status.st_mode)) {
      type = FileType::character_file;
   } else if (S_ISFIFO(status.st_mode)) {
      type = FileType::fifo_file;
   } else if (S_ISSOCK(status.st_mode)) {
      type = FileType::socket_file;
   } else if (S_ISLNK(status.st_mode)) {
      type = FileType::symlink_file;
   }
   Permission perms = static_cast<Permission>(status.st_mode) & all_perms;
   result = FileStatus(type, perms, status.st_dev, status.st_nlink,
                       status.st_ino, status.st_atime, status.st_mtime,
                       status.st_uid, status.st_gid, status.st_size);

   return std::error_code();
}

} // anonymous namespace

std::error_code status(const Twine &path, FileStatus &result, bool follow)
{
   SmallString<128> pathStorage;
   StringRef p = path.toNullTerminatedStringRef(pathStorage);

   struct stat status;
   int statRet = (follow ? ::stat : ::lstat)(p.begin(), &status);
   return fill_status(statRet, status, result);
}

std::error_code status(int fd, FileStatus &result)
{
   struct stat status;
   int statRet = ::fstat(fd, &status);
   return fill_status(statRet, status, result);
}

std::error_code set_permissions(const Twine &path, Permission permissions)
{
   SmallString<128> pathStorage;
   StringRef p = path.toNullTerminatedStringRef(pathStorage);

   if (::chmod(p.begin(), permissions)) {
      return std::error_code(errno, std::generic_category());
   }
   return std::error_code();
}

std::error_code set_last_modification_and_access_time(int fd, TimePoint<> time)
{
#if defined(HAVE_FUTIMENS)
   timespec times[2];
   times[0] = times[1] = to_time_spec(time);
   if (::futimens(fd, times)) {
      return std::error_code(errno, std::generic_category());
   }
   return std::error_code();
#elif defined(HAVE_FUTIMES)
   timeval times[2];
   times[0] = times[1] = to_time_spec(
            std::chrono::time_point_cast<std::chrono::microseconds>(time));
   if (::futimes(fd, times)) {
      return std::error_code(errno, std::generic_category());
   }
   return std::error_code();
#else
#warning Missing futimes() and futimens()
   return make_error_code(ErrorCode::function_not_supported);
#endif
}

std::error_code MappedFileRegion::init(int fd, uint64_t offset,
                                       MapMode mode)
{
   assert(m_size != 0);

   int flags = (mode == readwrite) ? MAP_SHARED : MAP_PRIVATE;
   int prot = (mode == readonly) ? PROT_READ : (PROT_READ | PROT_WRITE);
#if defined(__APPLE__)
   //----------------------------------------------------------------------
   // Newer versions of MacOSX have a flag that will allow us to read from
   // binaries whose code signature is invalid without crashing by using
   // the MAP_RESILIENT_CODESIGN flag. Also if a file from removable media
   // is mapped we can avoid crashing and return zeroes to any pages we try
   // to read if the media becomes unavailable by using the
   // MAP_RESILIENT_MEDIA flag.  These flags are only usable when mapping
   // with PROT_READ, so take care not to specify them otherwise.
   //----------------------------------------------------------------------
   if (mode == readonly) {
#if defined(MAP_RESILIENT_CODESIGN)
      flags |= MAP_RESILIENT_CODESIGN;
#endif
#if defined(MAP_RESILIENT_MEDIA)
      flags |= MAP_RESILIENT_MEDIA;
#endif
   }
#endif // #if defined (__APPLE__)

   m_mapping = ::mmap(nullptr, m_size, prot, flags, fd, offset);
   if (m_mapping == MAP_FAILED) {
      return std::error_code(errno, std::generic_category());
   }
   return std::error_code();
}

MappedFileRegion::MappedFileRegion(int fd, MapMode mode, size_t length,
                                   uint64_t offset, std::error_code &errorCode)
   : m_size(length), m_mapping(), m_fd(fd), m_mode(mode)
{
   (void)fd;
   (void)mode;
   errorCode = init(fd, offset, mode);
   if (errorCode) {
      m_mapping = nullptr;
   }
}

MappedFileRegion::~MappedFileRegion()
{
   if (m_mapping) {
      ::munmap(m_mapping, m_size);
   }
}

size_t MappedFileRegion::getSize() const
{
   assert(m_mapping && "Mapping failed but used anyway!");
   return m_size;
}

char *MappedFileRegion::getData() const
{
   assert(m_mapping && "Mapping failed but used anyway!");
   return reinterpret_cast<char*>(m_mapping);
}

const char *MappedFileRegion::getConstData() const
{
   assert(m_mapping && "Mapping failed but used anyway!");
   return reinterpret_cast<const char*>(m_mapping);
}

int MappedFileRegion::getAlignment()
{
   return Process::getPageSize();
}

namespace internal {
std::error_code directory_iterator_construct(internal::DirIterState &iter,
                                             StringRef path,
                                             bool followSymlinks)
{
   SmallString<128> pathNull(path);
   DIR *directory = ::opendir(pathNull.getCStr());
   if (!directory) {
      return std::error_code(errno, std::generic_category());
   }
   iter.m_iterationHandle = reinterpret_cast<intptr_t>(directory);
   // Add something for replace_filename to replace.
   path::append(pathNull, ".");
   iter.m_currentEntry = DirectoryEntry(pathNull.getStr(), followSymlinks);
   return directory_iterator_increment(iter);
}

std::error_code DirectoryIterator_destruct(DirIterState &iter)
{
   if (iter.m_iterationHandle) {
      ::closedir(reinterpret_cast<DIR *>(iter.m_iterationHandle));
   }
   iter.m_iterationHandle = 0;
   iter.m_currentEntry = DirectoryEntry();
   return std::error_code();
}

std::error_code directory_iterator_increment(DirIterState &iter)
{
   errno = 0;
   dirent *curDir = ::readdir(reinterpret_cast<DIR *>(iter.m_iterationHandle));
   if (curDir == nullptr && errno != 0) {
      return std::error_code(errno, std::generic_category());
   } else if (curDir != nullptr) {
      StringRef name(curDir->d_name);
      if ((name.getSize() == 1 && name[0] == '.') ||
          (name.getSize() == 2 && name[0] == '.' && name[1] == '.')) {
         return directory_iterator_increment(iter);
      }
      iter.m_currentEntry.replaceFilename(name);
   } else {
      return DirectoryIterator_destruct(iter);
   }
   return std::error_code();
}
} // internal

OptionalError<BasicFileStatus> DirectoryEntry::getStatus() const
{
   FileStatus status;
   if (auto errorCode = fs::status(m_path, status, m_followSymlinks)) {
      return errorCode;
   }
   return status;
}

namespace {
#if !defined(F_GETPATH)
bool has_proc_self_fd()
{
   // If we have a /proc filesystem mounted, we can quickly establish the
   // real name of the file with readlink
   static const bool result = (::access("/proc/self/fd", R_OK) == 0);
   return result;
}
#endif
} // anonymous namespace

std::error_code open_file_for_read(const Twine &name, int &resultFD,
                                   SmallVectorImpl<char> *realPath) {
   SmallString<128> storage;
   StringRef p = name.toNullTerminatedStringRef(storage);
   int openFlags = O_RDONLY;
#ifdef O_CLOEXEC
   openFlags |= O_CLOEXEC;
#endif
   if ((resultFD = sys::retry_after_signal(-1, open, p.begin(), openFlags)) < 0) {
      return std::error_code(errno, std::generic_category());
   }
#ifndef O_CLOEXEC
   int r = fcntl(resultFD, F_SETFD, FD_CLOEXEC);
   (void)r;
   assert(r == 0 && "fcntl(F_SETFD, FD_CLOEXEC) failed");
#endif
   // Attempt to get the real name of the file, if the user asked
   if(!realPath) {
      return std::error_code();
   }
   realPath->clear();
#if defined(F_GETPATH)
   // When F_GETPATH is availble, it is the quickest way to get
   // the real path name.
   char buffer[MAXPATHLEN];
   if (::fcntl(resultFD, F_GETPATH, buffer) != -1) {
      realPath->append(buffer, buffer + strlen(buffer));
   }
#else
   char buffer[PATH_MAX];
   if (has_proc_self_fd()) {
      char procPath[64];
      snprintf(procPath, sizeof(procPath), "/proc/self/fd/%d", resultFD);
      ssize_t charCount = ::readlink(procPath, buffer, sizeof(buffer));
      if (charCount > 0) {
         realPath->append(buffer, buffer + charCount);
      }
   } else {
      // Use ::realPath to get the real path name
      if (::realpath(p.begin(), buffer) != nullptr) {
         realPath->append(buffer, buffer + strlen(buffer));
      }
   }
#endif
   return std::error_code();
}

std::error_code open_file_for_write(const Twine &name, int &resultFD,
                                    OpenFlags flags, unsigned mode)
{
   // Verify that we don't have both "append" and "excl".
   assert((!(flags & fs::F_Excl) || !(flags & fs::F_Append)) &&
          "Cannot specify both 'excl' and 'append' file creation flags!");

   int openFlags = O_CREAT;

#ifdef O_CLOEXEC
   openFlags |= O_CLOEXEC;
#endif

   if (flags & F_RW) {
      openFlags |= O_RDWR;
   } else {
      openFlags |= O_WRONLY;
   }
   if (flags & F_Append) {
      openFlags |= O_APPEND;
   } else if (!(flags & F_NoTrunc)) {
      openFlags |= O_TRUNC;
   }
   if (flags & F_Excl) {
      openFlags |= O_EXCL;
   }
   SmallString<128> storage;
   StringRef p = name.toNullTerminatedStringRef(storage);
   if ((resultFD = sys::retry_after_signal(-1, open, p.begin(), openFlags, mode)) < 0) {
      return std::error_code(errno, std::generic_category());
   }
#ifndef O_CLOEXEC
   int r = fcntl(resultFD, F_SETFD, FD_CLOEXEC);
   (void)r;
   assert(r == 0 && "fcntl(F_SETFD, FD_CLOEXEC) failed");
#endif
   return std::error_code();
}

template <typename T>
static std::error_code remove_directories_impl(const T &entry,
                                               bool ignoreErrors)
{
   std::error_code errorCode;
   DirectoryIterator begin(entry, errorCode, false);
   DirectoryIterator end;
   while (begin != end) {
      auto &item = *begin;
      OptionalError<BasicFileStatus> st = item.getStatus();
      if (!st && !ignoreErrors) {
         return st.getError();
      }
      if (is_directory(*st)) {
         errorCode = remove_directories_impl(item, ignoreErrors);
         if (errorCode && !ignoreErrors) {
            return errorCode;
         }
      }

      errorCode = fs::remove(item.getPath(), true);
      if (errorCode && !ignoreErrors) {
         return errorCode;
      }
      begin.increment(errorCode);
      if (errorCode && !ignoreErrors) {
         return errorCode;
      }
   }
   return std::error_code();
}

std::error_code remove_directories(const Twine &path, bool ignoreErrors)
{
   auto errorCode = remove_directories_impl(path, ignoreErrors);
   if (errorCode && !ignoreErrors) {
      return errorCode;
   }
   errorCode = fs::remove(path, true);
   if (errorCode && !ignoreErrors) {
      return errorCode;
   }
   return std::error_code();
}

std::error_code real_path(const Twine &path, SmallVectorImpl<char> &dest,
                          bool expandTilde)
{
   dest.clear();
   if (path.isTriviallyEmpty()) {
      return std::error_code();
   }
   if (expandTilde) {
      SmallString<128> sorage;
      path.toVector(sorage);
      expand_tilde_expr(sorage);
      return real_path(sorage, dest, false);
   }

   int fd;
   std::error_code errorCode = open_file_for_read(path, fd, &dest);
   if (errorCode) {
      return errorCode;
   }
   ::close(fd);
   return std::error_code();
}

namespace path {
bool home_directory(SmallVectorImpl<char> &result)
{
   char *requestedDir = getenv("HOME");
   if (!requestedDir) {
      struct passwd *pw = getpwuid(getuid());
      if (pw && pw->pw_dir) {
         requestedDir = pw->pw_dir;
      }
   }
   if (!requestedDir) {
      return false;
   }
   result.clear();
   result.append(requestedDir, requestedDir + strlen(requestedDir));
   return true;
}

namespace {
bool get_darwin_conf_dir(bool tempDir, SmallVectorImpl<char> &result)
{
#if defined(_CS_DARWIN_USER_TEMP_DIR) && defined(_CS_DARWIN_USER_CACHE_DIR)
   // On Darwin, use DARWIN_USER_TEMP_DIR or DARWIN_USER_CACHE_DIR.
   // macros defined in <unistd.h> on darwin >= 9
   int confName = tempDir ? _CS_DARWIN_USER_TEMP_DIR
                          : _CS_DARWIN_USER_CACHE_DIR;
   size_t confLen = confstr(confName, nullptr, 0);
   if (confLen > 0) {
      do {
         result.resize(confLen);
         confLen = confstr(confName, result.getData(), result.getSize());
      } while (confLen > 0 && confLen != result.getSize());

      if (confLen > 0) {
         assert(result.back() == 0);
         result.pop_back();
         return true;
      }
      result.clear();
   }
#endif
   return false;
}

} // anonymous namespace

bool get_user_cache_dir(SmallVectorImpl<char> &result)
{
   // First try using XDG_CACHE_HOME env variable,
   // as specified in XDG Base Directory Specification at
   // http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html
   if (const char *xdgCacheDir = std::getenv("XDG_CACHE_HOME")) {
      result.clear();
      result.append(xdgCacheDir, xdgCacheDir + strlen(xdgCacheDir));
      return true;
   }

   // Try Darwin configuration query
   if (get_darwin_conf_dir(false, result))
      return true;

   // Use "$HOME/.cache" if $HOME is available
   if (home_directory(result)) {
      append(result, ".cache");
      return true;
   }

   return false;
}

namespace {

const char *get_env_temp_dir()
{
   // Check whether the temporary directory is specified by an environment
   // variable.
   const char *environmentVariables[] = {"TMPDIR", "TMP", "TEMP", "tempDir"};
   for (const char *env : environmentVariables) {
      if (const char *dir = std::getenv(env)) {
         return dir;
      }
   }
   return nullptr;
}

const char *get_default_temp_dir(bool erasedOnReboot) {
#ifdef P_tmpdir
   if ((bool)P_tmpdir) {
      return P_tmpdir;
   }

#endif

   if (erasedOnReboot) {
      return "/tmp";
   }
   return "/var/tmp";
}

} // anonymous namespace


void system_temp_directory(bool erasedOnReboot, SmallVectorImpl<char> &result)
{
   result.clear();

   if (erasedOnReboot) {
      // There is no env variable for the cache directory.
      if (const char *requestedDir = get_env_temp_dir()) {
         result.append(requestedDir, requestedDir + strlen(requestedDir));
         return;
      }
   }

   if (get_darwin_conf_dir(erasedOnReboot, result))
      return;

   const char *requestedDir = get_default_temp_dir(erasedOnReboot);
   result.append(requestedDir, requestedDir + strlen(requestedDir));
}
} // path

} // end namespace fs
} // polar



