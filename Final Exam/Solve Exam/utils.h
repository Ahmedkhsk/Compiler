#pragma once
#include <string>
#include <vector>
#include <cctype> 



    inline std::vector<std::string> split(const std::string& str, const std::string& delimiter) {
        std::vector<std::string> result;
        int pos = 0;
        int start = 0;
        int delimLength = delimiter.length();

        while ((pos = str.find(delimiter, start)) != std::string::npos) {
            result.push_back(str.substr(start, pos - start));
            start = pos + delimLength;
        }

        // Add the last part
        result.push_back(str.substr(start));
        return result;
    }
