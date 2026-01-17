// std_string.cpp
#include "link_str.h"
#include <algorithm>

namespace SysString {

    std::string trim(const std::string& str) {
        const std::string whitespace = " \t\n\r";
        size_t first = str.find_first_not_of(whitespace);
        if (std::string::npos == first) return "";
        size_t last = str.find_last_not_of(whitespace);
        return str.substr(first, (last - first + 1));
    }

    std::string replace(std::string str, const std::string& from, const std::string& to) {
        if (from.empty()) return str;
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
        return str;
    }

    std::vector<std::string> split(std::string str, const std::string& delimiter) {
        std::vector<std::string> list;
        size_t pos = 0;
        std::string token;
        while ((pos = str.find(delimiter)) != std::string::npos) {
            token = str.substr(0, pos);
            list.push_back(token);
            str.erase(0, pos + delimiter.length());
        }
        list.push_back(str); // Sisa string terakhir
        return list;
    }

    std::string merge(const std::vector<std::string>& list, const std::string& delimiter) {
        std::string result = "";
        for (size_t i = 0; i < list.size(); ++i) {
            result += list[i];
            if (i < list.size() - 1) result += delimiter;
        }
        return result;
    }
}
