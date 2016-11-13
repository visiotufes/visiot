#ifndef FIND_ENTITIES_HPP
#define FIND_ENTITIES_HPP

#include <string>
#include <vector>
#include <utils/entity.hpp>
#include "service-client.hpp"

namespace is {

using namespace std;

vector<entity> find_entities(string broker, string filter);

} // ::is

#endif // FIND_ENTITIES_HPP