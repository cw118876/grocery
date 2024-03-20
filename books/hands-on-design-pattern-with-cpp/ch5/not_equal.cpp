#include <iostream>

template <typename D>
struct B {
  bool operator==(const D& rhs) {
    return !static_cast<D*>(this)->operator!=(rhs);
  }
};

class D : public B<D> {
 public:
  explicit D(int i) : i_{i} {}
  bool operator!=(const D& rhs) { return i_ != rhs.i_; }

 private:
  int i_ = 0;
};

int main(int argc, const char* argv[]) {
  D d1{1}, d2{2}, d3{1};
  std::cout << "d1 == d2: " << (d1 == d2) << " d1 != d2 " << (d1 != d2)
            << std::endl;
  std::cout << "d1 == d3: " << (d1 == d3) << " d1 != d3 " << (d1 != d3)
            << std::endl;
  return 0;
}