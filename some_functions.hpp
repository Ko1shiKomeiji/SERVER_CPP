#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>

std::string read_file(const std::string filename);
std::string parse_and_sort_JSON(const std::string& jsonString, const std::string& sort_type);
std::string sort_words(const std::string& input, const std::string& sort_type);
void writefile(const std::string& filename, const std::string& data);