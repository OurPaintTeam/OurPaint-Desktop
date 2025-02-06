#include "ID.h"

ID& ID::operator=(const ID& x) {
  if (this != &x) {
    id = x.id;
  }
  return *this;
}

bool operator>(const ID& left, const ID& right) {
  return left.id > right.id;
}

bool operator<(const ID& left, const ID& right) {
  return left.id < right.id;
}

bool operator==(const ID& left, const ID& right) {
  return left.id == right.id;
}

std::ifstream& operator>> (std::ifstream& in, ID& x) {
  in >> x.id;
  return in;
}