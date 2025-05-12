
#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype> // Required for isspace

// Function to trim whitespace from the beginning of a string
inline std::string ltrim(const std::string &s) {
    std::string result = s;
    result.erase(result.begin(), std::find_if(result.begin(), result.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    return result;
}

// Function to trim whitespace from the end of a string
inline std::string rtrim(const std::string &s) {
    std::string result = s;
    result.erase(std::find_if(result.rbegin(), result.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), result.end());
    return result;
}

// Function to trim whitespace from both ends of a string
inline std::string trim(const std::string &s) {
    return ltrim(rtrim(s));
}

// Function to split a string by a delimiter string
// This version handles multiple character delimiters and empty strings between delimiters better.
inline std::vector<std::string> split(const std::string& s, const std::string& delimiter) {
    std::vector<std::string> tokens;
    std::string::size_type pos_start = 0, pos_end;
    std::string::size_type delim_len = delimiter.length();
    std::string token;

    if (s.empty()) {
        return tokens;
    }

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr(pos_start, pos_end - pos_start);
        // Only add non-empty tokens, or handle as per specific requirements if empty tokens are significant
        // For grammar parsing, usually we want to split by space and ignore multiple spaces.
        // If splitting by "|", "S -> A | " should give "A" and an empty string for the part after last "|".
        // The current GrammarReader.cpp code trims alternatives after splitting by "|", so empty parts become empty strings.
        // And then it splits alternatives by space, and filters empty strings there.
        tokens.push_back(token); // Keep the token as is, let the caller trim or filter.
        pos_start = pos_end + delim_len;
    }

    tokens.push_back(s.substr(pos_start)); // Add the last part
    return tokens;
}

// Overload for splitting by a single character delimiter (often more efficient)
inline std::vector<std::string> split(const std::string& s, char delimiter_char) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter_char)) {
        tokens.push_back(token);
    }
    return tokens;
}

#endif // UTILS_H

