#include "file_reader.h"

#include <fstream>
#include <sstream>

std::string FileReader::readFile(const std::string& fileName) {
    std::ifstream file(fileName);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + fileName);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
