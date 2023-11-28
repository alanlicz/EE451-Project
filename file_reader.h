#ifndef FILE_READER_H
#define FILE_READER_H

#include <string>

class FileReader {
   public:
    std::string readFile(const std::string& fileName);
};

#endif  // FILE_READER_H
