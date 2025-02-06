#ifndef OURPAINT_HEADERS_ID_H_
#define OURPAINT_HEADERS_ID_H_

#include <fstream>

struct ID {
  long long int id;

  ID(long long int i = 0) : id(i) {}

  ID &operator=(const ID &x);
};

std::ifstream &operator>>(std::ifstream &in, ID &x);

bool operator>(const ID &left, const ID &right);
bool operator<(const ID &left, const ID &right);
bool operator==(const ID &left, const ID &right);

namespace std {
template <>
struct hash<ID> {
  std::size_t operator()(const ID &id) const {
    return std::hash<long long int>()(id.id);
  }
};
}

#endif // ! OURPAINT_HEADERS_ID_H_