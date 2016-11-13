#include "ptgrey-utils.hpp"

namespace is {
namespace utils {

std::vector<std::string> split(const std::string &s, char delim) {
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> elems;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

int ip_to_int(const char* ip) {
	std::string data(ip);
	auto split_data = split(data,'.');
	int IP = 0;
	if (split_data.size()==4) {
		int sl_num = 24;
		int mask = 0xff000000;
		for ( auto& d : split_data) {
			int octets = stoi(d); 
			if ( ( octets >> 8) && 0xffffff00 ) {
				return 0; //erro em algum
			}
			IP += (octets << sl_num);
			sl_num -= 8;
		}
		return IP;
	}
	return 0;
}

} // ::utils
} // ::is