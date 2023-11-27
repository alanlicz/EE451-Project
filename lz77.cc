#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>
#include <vector>

struct LZ77Token {
    int offset;
    int length;
    char nextChar;
};

std::vector<LZ77Token> LZ77Compress(const std::string& input, int windowSize) {
    std::vector<LZ77Token> tokens;
    int inputSize = input.length();

    for (int i = 0; i < inputSize; ++i) {
        int maxLength = 0;
        int maxOffset = 0;
        int maxWindowStart = std::max(0, i - windowSize);

        for (int j = maxWindowStart; j < i; ++j) {
            int length = 0;
            while (i + length < inputSize &&
                   input[j + length] == input[i + length]) {
                length++;
                if (j + length >= i) break;
            }

            if (length > maxLength) {
                maxLength = length;
                maxOffset = i - j;
            }
        }

        char nextChar = i + maxLength < inputSize ? input[i + maxLength] : '\0';
        tokens.push_back({maxOffset, maxLength, nextChar});
        i += maxLength;
    }

    return tokens;
}

std::string LZ77Decompress(const std::vector<LZ77Token>& tokens) {
    std::string output;
    for (const auto& token : tokens) {
        int start = output.length() - token.offset;
        for (int i = 0; i < token.length; ++i) {
            output += output[start + i];
        }
        if (token.nextChar != '\0') {
            output += token.nextChar;
        }
    }
    return output;
}

int main() {
    std::string input = "ababcbababaa";
    int windowSize = 8;
    struct timespec start, stop;
    double time;

    if (clock_gettime(CLOCK_REALTIME, &start) == -1) {
        perror("clock gettime");
    }

    auto compressed = LZ77Compress(input, windowSize);
    std::string decompressed = LZ77Decompress(compressed);

    if (clock_gettime(CLOCK_REALTIME, &stop) == -1) {
        perror("clock gettime");
    }
    time = (stop.tv_sec - start.tv_sec) +
           (double)(stop.tv_nsec - start.tv_nsec) / 1e9;

    std::cout << "Time: " << time << std::endl;

    std::cout << "Original: " << input << std::endl;
    std::cout << "Compressed Data: ";
    for (const auto& token : compressed) {
        std::cout << "(" << token.offset << ", " << token.length << ", "
                  << token.nextChar << ") ";
    }
    std::cout << std::endl;
    std::cout << "Decompressed: " << decompressed << std::endl;

    return 0;
}
