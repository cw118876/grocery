#include <iostream>

template <typename D>
struct NotEqual {
  friend bool operator!=(const D& lhs, const D& rhs) { return !(lhs == rhs); }
};

class C : public NotEqual<C> {
 public:
  explicit C(int i) : i_{i} {}
  friend bool operator==(const C& lhs, const C& rhs) {
    return lhs.i_ == rhs.i_;
  }

 private:
  int i_ = 0;
};

struct D : public NotEqual<D> {
  explicit D(int i) : i_{i} {}
  friend bool operator==(const D& lhs, const D& rhs) {
    return lhs.i_ == rhs.i_;
  }
  int i_ = 0;
};

int main(int argc, const char* argv[]) {
  C c1{1}, c2{2}, c3{1};
  std::cout << "c1==c2: " << (c1 == c2) << " c1!=c2 " << (c1 != c2)
            << std::endl;
  std::cout << "c1==c3: " << (c1 == c3) << " c1!=c3 " << (c1 != c3)
            << std::endl;

  D d1{1}, d2{2}, d3{1};
  std::cout << "d1==d2: " << (d1 == d2) << " d1!=d2 " << (d1 != d2)
            << std::endl;
  std::cout << "d1==d3: " << (d1 == d3) << " d1!=d3 " << (d1 != d3)
            << std::endl;

  return 0;
}