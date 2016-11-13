#ifndef PTGREY_UTILS_HPP
#define PTGREY_UTILS_HPP

#include <string>
#include <sstream>
#include <vector>

using namespace std;

namespace is {
namespace utils {

vector<string> split(const string &s, char delim);
int ip_to_int(const char* ip);

} // ::utils
} // ::is

#endif // PTGREY_UTILS_HPP