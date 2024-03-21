#include <iostream>
#include <utility>
#include <vector>

static int bad_i = -1;

struct C {
  C() = default;
  explicit C(int i) : i_{i} {}
  C(const C& rhs) : i_{rhs.i_} {
    if (i_ == bad_i) {
      throw i_;
    }
  }
  int i_ = 0;
};

using Vec = std::vector<C>;

C transform(C x) { return C{2 * x.i_}; }

void transform_not_safe(const Vec& in, Vec& out) {
  out.resize(0);
  out.reserve(in.size());
  for (const auto& x : in) {
    out.push_back(transform(x));
  }
}

void transform_safe(const Vec& in, Vec& out) {
  Vec tmp{};
  tmp.reserve(in.size());
  for (const auto& x : in) {
    tmp.push_back(transform(x));
  }
  out.swap(tmp);
}

void printVec(const Vec& v, std::ostream& os) {
  os << "Vector size: " << v.size() << " Capacity: " << v.capacity()
     << std::endl;
  for (const auto& x : v) {
    os << x.i_ << " ";
  }
  os << std::endl;
}

int main(int argc, const char* argv[]) {
  Vec v1{C{1}, C{2}, C{3}, C{4}};
  {  // No exception
    bad_i = -1;
    Vec v2;
    transform_not_safe(v1, v2);
    printVec(v2, std::cout);
  }
  {  // exception- partially constructed result;
    bad_i = 6;
    Vec v2;
    try {
      transform_not_safe(v1, v2);
    } catch (...) {
    }
    printVec(v2, std::cout);
  }
  {  // exception - intial content lost
    bad_i = 1;
    Vec v2{C{5}};
    try {
      transform_not_safe(v1, v2);
    } catch (...) {
    }
    printVec(v2, std::cout);
  }
  {  // No exception
    bad_i = -1;
    Vec v2;
    transform_safe(v1, v2);
    printVec(v2, std::cout);
  }
  {  // exception- rollback ;
    bad_i = 6;
    Vec v2;
    try {
      transform_safe(v1, v2);
    } catch (...) {
    }
    printVec(v2, std::cout);
  }
  {  // exception - intial remains
    bad_i = 1;
    Vec v2{C{5}};
    try {
      transform_safe(v1, v2);
    } catch (...) {
    }
    printVec(v2, std::cout);
  }

  return 0;
}