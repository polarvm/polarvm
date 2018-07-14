// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/07/15.

#include "polar/utils/Program.h"
#include "polar/utils/CommandLine.h"
#include "polar/utils/ConvertUTF.h"
#include "polar/utils/FileSystem.h"
#include "polar/utils/Path.h"
#include "gtest/gtest.h"
#include <stdlib.h>
#if defined(__APPLE__)
# include <crt_externs.h>
#elif !defined(_MSC_VER)
// Forward declare environ in case it's not provided by stdlib.h.
extern char **environ;
#endif

#if defined(POLAR_ON_UNIX)
#include <unistd.h>
void sleep_for(unsigned int seconds)
{
   sleep(seconds);
}
#elif defined(POLAR_ON_WIN32)
#include <windows.h>
void sleep_for(unsigned int seconds) {
   Sleep(seconds * 1000);
}
#else
#error sleep_for is not implemented on your platform.
#endif

#define ASSERT_NO_ERROR(x)                                                     \
   if (std::error_code ASSERT_NO_ERROR_ec = x) {                                \
   SmallString<128> MessageStorage;                                           \
   RawSvectorOutStream Message(MessageStorage);                               \
   Message << #x ": did not return errc::success.\n"                          \
   << "error number: " << ASSERT_NO_ERROR_ec.value() << "\n"          \
   << "error message: " << ASSERT_NO_ERROR_ec.message() << "\n";      \
   GTEST_FATAL_FAILURE_(MessageStorage.getCStr());                              \
   } else {                                                                     \
   }
// From TestMain.cpp.
extern const char *TestMainArgv0;

using namespace polar;
using namespace polar::basic;
using namespace polar::utils;
using namespace polar::sys;

namespace {

static cmd::Opt<std::string>
ProgramTestStringArg1("program-test-string-arg1");
static cmd::Opt<std::string>
ProgramTestStringArg2("program-test-string-arg2");

class ProgramEnvTest : public testing::Test {
   std::vector<const char *> EnvTable;
   std::vector<std::string> EnvStorage;

protected:
   void SetUp() override {
      auto EnvP = [] {
#if defined(POLAR_ON_WIN32)
         _wgetenv(L"TMP"); // Populate _wenviron, initially is null
         return _wenviron;
#elif defined(__APPLE__)
         return *_NSGetEnviron();
#else
         return environ;
#endif
      }();
      ASSERT_TRUE(EnvP);

      auto prepareEnvVar = [this](decltype(*EnvP) Var) {
#if defined(POLAR_ON_WIN32)
         // On Windows convert UTF16 encoded variable to UTF8
         auto Len = wcslen(Var);
         ArrayRef<char> Ref{reinterpret_cast<char const *>(Var),
                  Len * sizeof(*Var)};
         EnvStorage.emplace_back();
         auto convStatus = convertUTF16ToUTF8String(Ref, EnvStorage.back());
         EXPECT_TRUE(convStatus);
         return EnvStorage.back().c_str();
#else
         (void)this;
         return Var;
#endif
      };

      while (*EnvP != nullptr) {
         EnvTable.emplace_back(prepareEnvVar(*EnvP));
         ++EnvP;
      }
   }

   void TearDown() override {
      EnvTable.clear();
      EnvStorage.clear();
   }

   void addEnvVar(const char *Var) {
      ASSERT_TRUE(EnvTable.empty() || EnvTable.back()) << "Env table sealed";
      EnvTable.emplace_back(Var);
   }

   const char **getEnviron() {
      if (EnvTable.back() != nullptr)
         EnvTable.emplace_back(nullptr); // Seal table.
      return &EnvTable[0];
   }
};

#ifdef POLAR_ON_WIN32
TEST_F(ProgramEnvTest, testCreateProcessLongPath)
{
   if (getenv("POLAR_PROGRAM_TEST_LONG_PATH"))
      exit(0);

   // getMainExecutable returns an absolute path; prepend the long-path prefix.
   std::string MyAbsExe =
         fs::getMainExecutable(TestMainArgv0, &ProgramTestStringArg1);
   std::string MyExe;
   if (!StringRef(MyAbsExe).startswith("\\\\?\\"))
      MyExe.append("\\\\?\\");
   MyExe.append(MyAbsExe);

   const char *ArgV[] = {
      MyExe.c_str(),
      "--gtest_filter=ProgramEnvTest.CreateProcessLongPath",
      nullptr
   };

   // Add POLAR_PROGRAM_TEST_LONG_PATH to the environment of the child.
   addEnvVar("POLAR_PROGRAM_TEST_LONG_PATH=1");

   // Redirect stdout to a long path.
   SmallString<128> TestDirectory;
   ASSERT_NO_ERROR(
            fs::createUniqueDirectory("program-redirect-test", TestDirectory));
   SmallString<256> LongPath(TestDirectory);
   LongPath.push_back('\\');
   // MAX_PATH = 260
   LongPath.append(260 - TestDirectory.size(), 'a');

   std::string Error;
   bool ExecutionFailed;
   std::optional<StringRef> Redirects[] = {None, LongPath.str(), None};
   int RC = execute_and_wait(MyExe, ArgV, getEnviron(), Redirects,
                           /*secondsToWait=*/ 10, /*memoryLimit=*/ 0, &Error,
                           &ExecutionFailed);
   EXPECT_FALSE(ExecutionFailed) << Error;
   EXPECT_EQ(0, RC);

   // Remove the long stdout.
   ASSERT_NO_ERROR(fs::remove(Twine(LongPath)));
   ASSERT_NO_ERROR(fs::remove(Twine(TestDirectory)));
}
#endif

TEST_F(ProgramEnvTest, testCreateProcessTrailingSlash) {
   if (getenv("POLAR_PROGRAM_TEST_CHILD")) {
      if (ProgramTestStringArg1 == "has\\\\ trailing\\" &&
          ProgramTestStringArg2 == "has\\\\ trailing\\") {
         exit(0);  // Success!  The arguments were passed and parsed.
      }
      exit(1);
   }

   std::string my_exe =
         fs::get_main_executable(TestMainArgv0, &ProgramTestStringArg1);
   const char *argv[] = {
      my_exe.c_str(),
      "--gtest_filter=ProgramEnvTest.CreateProcessTrailingSlash",
      "-program-test-string-arg1", "has\\\\ trailing\\",
      "-program-test-string-arg2", "has\\\\ trailing\\",
      nullptr
   };

   // Add POLAR_PROGRAM_TEST_CHILD to the environment of the child.
   addEnvVar("POLAR_PROGRAM_TEST_CHILD=1");

   std::string error;
   bool ExecutionFailed;
   // Redirect stdout and stdin to NUL, but let stderr through.
#ifdef POLAR_ON_WIN32
   StringRef nul("NUL");
#else
   StringRef nul("/dev/null");
#endif
   std::optional<StringRef> redirects[] = { nul, nul, std::nullopt };
   int rc = execute_and_wait(my_exe, argv, getEnviron(), redirects,
                             /*secondsToWait=*/ 10, /*memoryLimit=*/ 0, &error,
                             &ExecutionFailed);
   EXPECT_FALSE(ExecutionFailed) << error;
   EXPECT_EQ(0, rc);
}

TEST_F(ProgramEnvTest, TestExecuteNoWait)
{

   if (getenv("POLAR_PROGRAM_TEST_EXECUTE_NO_WAIT")) {
      sleep_for(/*seconds*/ 1);
      exit(0);
   }

   std::string Executable =
         fs::get_main_executable(TestMainArgv0, &ProgramTestStringArg1);
   const char *argv[] = {
      Executable.c_str(),
      "--gtest_filter=ProgramEnvTest.TestExecuteNoWait",
      nullptr
   };

   // Add POLAR_PROGRAM_TEST_EXECUTE_NO_WAIT to the environment of the child.
   addEnvVar("POLAR_PROGRAM_TEST_EXECUTE_NO_WAIT=1");

   std::string Error;
   bool ExecutionFailed;
   ProcessInfo PI1 = execute_no_wait(Executable, argv, getEnviron(), {}, 0, &Error,
                                     &ExecutionFailed);
   ASSERT_FALSE(ExecutionFailed) << Error;
   ASSERT_NE(PI1.m_pid, ProcessInfo::InvalidPid) << "Invalid process id";

   unsigned loopCount = 0;

   // Test that Wait() with WaitUntilTerminates=true works. In this case,
   // loopCount should only be incremented once.
   while (true) {
      ++loopCount;
      ProcessInfo WaitResult = wait(PI1, 0, true, &Error);
      ASSERT_TRUE(Error.empty());
      if (WaitResult.m_pid == PI1.m_pid)
         break;
   }

   EXPECT_EQ(loopCount, 1u) << "loopCount should be 1";

   ProcessInfo pi2 = execute_no_wait(Executable, argv, getEnviron(), {}, 0, &Error,
                                     &ExecutionFailed);
   ASSERT_FALSE(ExecutionFailed) << Error;
   ASSERT_NE(pi2.m_pid, ProcessInfo::InvalidPid) << "Invalid process id";

   // Test that Wait() with SecondsToWait=0 performs a non-blocking wait. In this
   // cse, loopCount should be greater than 1 (more than one increment occurs).
   while (true) {
      ++loopCount;
      ProcessInfo WaitResult = wait(pi2, 0, false, &Error);
      ASSERT_TRUE(Error.empty());
      if (WaitResult.m_pid == pi2.m_pid)
         break;
   }

   ASSERT_GT(loopCount, 1u) << "loopCount should be >1";
}

TEST_F(ProgramEnvTest, testExecuteAndWaitTimeout)
{
   if (getenv("POLAR_PROGRAM_TEST_TIMEOUT")) {
      sleep_for(/*seconds*/ 10);
      exit(0);
   }

   std::string Executable =
         fs::get_main_executable(TestMainArgv0, &ProgramTestStringArg1);
   const char *argv[] = {
      Executable.c_str(),
      "--gtest_filter=ProgramEnvTest.TestExecuteAndWaitTimeout",
      nullptr
   };

   // Add POLAR_PROGRAM_TEST_TIMEOUT to the environment of the child.
   addEnvVar("POLAR_PROGRAM_TEST_TIMEOUT=1");

   std::string Error;
   bool ExecutionFailed;
   int RetCode =
         execute_and_wait(Executable, argv, getEnviron(), {}, /*secondsToWait=*/1, 0,
                        &Error, &ExecutionFailed);
   ASSERT_EQ(-2, RetCode);
}

TEST(ProgramTest, testExecuteNegative)
{
   std::string Executable = "i_dont_exist";
   const char *argv[] = { Executable.c_str(), nullptr };

   {
      std::string Error;
      bool ExecutionFailed;
      int RetCode = execute_and_wait(Executable, argv, nullptr, {}, 0, 0, &Error,
                                   &ExecutionFailed);
      ASSERT_TRUE(RetCode < 0) << "On error execute_and_wait should return 0 or "
                                  "positive value indicating the result code";
      ASSERT_TRUE(ExecutionFailed);
      ASSERT_FALSE(Error.empty());
   }

   {
      std::string Error;
      bool ExecutionFailed;
      ProcessInfo PI = execute_no_wait(Executable, argv, nullptr, {}, 0, &Error,
                                     &ExecutionFailed);
      ASSERT_EQ(PI.m_pid, ProcessInfo::InvalidPid)
            << "On error execute_no_wait should return an invalid ProcessInfo";
      ASSERT_TRUE(ExecutionFailed);
      ASSERT_FALSE(Error.empty());
   }

}

#ifdef POLAR_ON_WIN32
const char utf16le_text[] =
      "\x6c\x00\x69\x00\x6e\x00\x67\x00\xfc\x00\x69\x00\xe7\x00\x61\x00";
const char utf16be_text[] =
      "\x00\x6c\x00\x69\x00\x6e\x00\x67\x00\xfc\x00\x69\x00\xe7\x00\x61";
#endif
const char utf8_text[] = "\x6c\x69\x6e\x67\xc3\xbc\x69\xc3\xa7\x61";

TEST(ProgramTest, testWriteWithSystemEncoding) {
   SmallString<128> TestDirectory;
   ASSERT_NO_ERROR(fs::create_unique_directory("program-test", TestDirectory));
   error_stream() << "Test Directory: " << TestDirectory << '\n';
   error_stream().flush();
   SmallString<128> file_pathname(TestDirectory);
   fs::path::append(file_pathname, "international-file.txt");
   // Only on Windows we should encode in UTF16. For other systems, use UTF8
   ASSERT_NO_ERROR(sys::write_file_with_encoding(file_pathname.getCStr(), utf8_text,
                                              sys::WEM_UTF16));
   int fd = 0;
   ASSERT_NO_ERROR(fs::open_file_for_read(file_pathname.getCStr(), fd));
#if defined(POLAR_ON_WIN32)
   char buf[18];
   ASSERT_EQ(::read(fd, buf, 18), 18);
   if (strncmp(buf, "\xfe\xff", 2) == 0) { // UTF16-BE
      ASSERT_EQ(strncmp(&buf[2], utf16be_text, 16), 0);
   } else if (strncmp(buf, "\xff\xfe", 2) == 0) { // UTF16-LE
      ASSERT_EQ(strncmp(&buf[2], utf16le_text, 16), 0);
   } else {
      FAIL() << "Invalid BOM in UTF-16 file";
   }
#else
   char buf[10];
   ASSERT_EQ(::read(fd, buf, 10), 10);
   ASSERT_EQ(strncmp(buf, utf8_text, 10), 0);
#endif
   ::close(fd);
   ASSERT_NO_ERROR(fs::remove(file_pathname.getStr()));
   ASSERT_NO_ERROR(fs::remove(TestDirectory.getStr()));
}

} // anonymous namespace
