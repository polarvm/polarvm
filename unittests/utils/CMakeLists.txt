polar_add_unittest(PolarBaseLibTests BasicUtilsTest
   ArrayRecyclerTest.cpp
   AlignOfTest.cpp
   AllocatorTest.cpp
   ARMAttributeParserTest.cpp
   BinaryStreamTest.cpp
   BranchProbabilityTest.cpp
   ChronoTest.cpp
   ConvertUtfTest.cpp
   CommandLineTest.cpp
   CastingTest.cpp
   CachePruningTest.cpp
   CompressionTest.cpp
   CrashRecoveryTest.cpp
   DataExtractorTest.cpp
   DebugTest.cpp
   EndianStreamTest.cpp
   EndianTest.cpp
   ErrorNumberTest.cpp
   ErrorTest.cpp
   FastHashTest.cpp
   FormatVariadicTest.cpp
   FileOutputBufferTest.cpp
   FormattedRawOutStreamTest.cpp
   GlobPatternTest.cpp
   HostTest.cpp
   MathExtrasTest.cpp

   OptionalErrorTest.cpp
   RawOutStreamTest.cpp
   SwapByteOrderTest.cpp
   PathTest.cpp
   UnicodeTest.cpp

   )

target_link_libraries(BasicUtilsTest PRIVATE TestSupport)
