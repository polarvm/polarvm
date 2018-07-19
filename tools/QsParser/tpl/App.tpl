#include <iostream>
#include <string>
#include <fstream>
#include <experimental/filesystem>
#include <cstdio>

namespace fs = std::experimental::filesystem;

__INCLUDES__

__GENERATOR_CLS__

int main(int argc, char *argv[])
{
   try {
      QsGenerator generator("__BASE_DIR__");
      generator.generate();
      return 0;
   } catch (std::exception &e) {
      std::cerr << e.what() << std::endl;
      return -1;
   }
}
