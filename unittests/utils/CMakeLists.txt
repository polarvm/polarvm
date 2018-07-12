polar_add_unittest(PolarBaseLibTests BasicUtilsTest
   ArrayRecyclerTest.cpp
   AlignOfTest.cpp
   AllocatorTest.cpp
   ARMAttributeParserTest.cpp
   BinaryStreamTest.cpp
   ChronoTest.cpp
   ConvertUtfTest.cpp
   CommandLineTest.cpp
   CastingTest.cpp
   FastHashTest.cpp
   FormatVariadicTest.cpp
   RawOutStreamTest.cpp
   SwapByteOrderTest.cpp
   PathTest.cpp
   UnicodeTest.cpp

   )

target_link_libraries(BasicUtilsTest PRIVATE TestSupport)
