#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <jsoncpp/json/json.h>
#include <regex>
#include <filesystem>

std::string read_file(const std::string filename) {
    bool exist = std::filesystem::exists(filename);
    if (exist) {
        std::ifstream file(filename);
        std::stringstream buffer;
        buffer << file.rdbuf() << "\0";
        return buffer.str();
    } else {
        return "";
    }
}

void writefile(const std::string& filename, const std::string& data) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << data;
        file.close();
    }
}

std::string sort_words(const std::string& input, const std::string& sort_type) {
    std::istringstream iss(input);
    std::vector<std::string> words;
    std::string word;
    while (iss >> word) {
        words.push_back(word);
    }
    if (sort_type == "ASC") {
        std::sort(words.begin(), words.end());
    } else {
        std::sort(words.begin(), words.end(), std::greater<>());
    }
    
    
    std::ostringstream oss;
    for (const auto& sortedWord : words) {
        oss << sortedWord << " ";
    }
    
    std::string sortedString = oss.str();
    if (!sortedString.empty()) {
        sortedString.pop_back();
    }
    
    return sortedString;
}

std::string parse_and_sort_JSON(const std::string& jsonString, const std::string& sort_type) {
    Json::Value root;
    Json::CharReaderBuilder reader;
    std::istringstream iss(jsonString);
    std::string result;

    if (Json::parseFromStream(reader, iss, &root, nullptr)) {
        if (root.isArray()) {
            std::vector<std::string> strings;
            std::vector<int> numbers;

            for (const auto& element : root) {
                if (element.isString()) {
                    strings.push_back(element.asString());
                } else if (element.isNumeric()) {
                    numbers.push_back(element.asInt());
                }
            }

            if (sort_type == "ASC") {
                for (auto& element: strings) {
                    element = sort_words(element, sort_type);
                }
                std::sort(numbers.begin(), numbers.end());
            } else {
                for (auto& element: strings) {
                    element = sort_words(element, sort_type);         
                }
                std::sort(numbers.begin(), numbers.end(), std::greater<>());
            }

            result += "[";
            if (numbers.size() == 0) {
                for (const auto& str : strings) {
                    result += '"' + str + '"' + ", ";
                }
            } else {
                for (const auto& num : numbers) {          
                    result += std::to_string(num) + ", ";
                }
            }
            result = result.substr(0, result.length() - 2);
            result += "]";
        } else {
            perror("root element isn't array");
        }
    } else {
        perror("JSON parse fault");
    }

    return result;
}