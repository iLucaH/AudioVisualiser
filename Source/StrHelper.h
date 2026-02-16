/*
  ==============================================================================

    StrHelper.h
    Created: 16 Feb 2026 10:00:25am
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <regex>
#include <string>

inline bool isValidFilename(const std::string& filename) {
    if (filename.empty() || filename.length() > 255 || filename.length() < 5)
        return false;

    std::regex pattern(R"(^[a-zA-Z0-9_\-\.]+$)");
    return std::regex_match(filename, pattern);
}

inline bool isMP4(const std::string&filename) {
    std::string last = filename.substr(filename.length() - 4);
    return last == ".mp4";
}

inline bool isValidVidFileStr(const std::string& filename) {
    return isValidFilename(filename) && isMP4(filename);
}
