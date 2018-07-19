class QsGenerator
{
public:
   QsGenerator(const std::string &baseDir)
      : m_baseDir(baseDir)
   {

   }

   void generate()
   {
      __GENERATOR_METHOD_LIST__

   }


protected:
   __METHOD_LIST__

   void setOutputFile(const std::string &filename)
   {
      std::cout << "generating " << filename << std::endl;
      m_currentGenerateFilename = filename;
   }

   void writeCodeToFile(std::string &code)
   {
      std::string outputFilename = m_baseDir + "/" + m_currentGenerateFilename;
      fs::path filePath(outputFilename);
      fs::path dirPath = filePath.parent_path();
      std::error_code errorCode;
      if (!fs::exists(dirPath, errorCode)) {
         fs::create_directories(dirPath);
      }
      char errorMsg[1024];
      std::ofstream ostrm(outputFilename, std::ios_base::trunc);
      if (!ostrm.is_open()) {
         snprintf(errorMsg, 1024, "open file: %s failed", m_currentGenerateFilename.c_str());
         throw std::runtime_error(errorMsg);
      }
      ostrm << code;
      if (!ostrm) {
         snprintf(errorMsg, 1024, "write generate result error into : %s", m_currentGenerateFilename.c_str());
         throw std::runtime_error(errorMsg);
      }
      ostrm.close();
   }
protected:
   std::string m_currentGenerateFilename;
   std::string m_baseDir;
};
