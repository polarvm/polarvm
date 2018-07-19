void __METHOD_NAME__()
{
   __METHOD_PREPARE__
   __CUR_FILENAME__
   if (m_currentGenerateFilename.empty()) {
      throw std::runtime_error("output filename for " + currentFileName + " is not set");
   }
   __METHOD_GENERATE_CODE__
   writeCodeToFile(code);
}
