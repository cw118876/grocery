#include <iostream>
#include <utility>

using std::swap;

namespace N {
class C {
 public:
  explicit C(int i) : i_{i} {}
  C(const C& rhs) : i_{rhs.i_} {}
  C& operator=(const C& rhs) {
    i_ = rhs.i_;
    std::cout << "copy assignment" << std::endl;
    return *this;
  }
  void swap(C& rhs) noexcept {
    std::cout << "C:swap " << std::endl;
    ::swap(i_, rhs.i_);
  }
  friend std::ostream& operator<<(std::ostream& stream, const C& c) {
    stream << c.i_;
    return stream;
  }

 private:
  int i_;
};

void swap(C& lhs, C& rhs) noexcept {
  std::cout << "N::swap" << std::endl;
  lhs.swap(rhs);
}

}  // namespace N

namespace std {
void swap(N::C& lhs, N::C& rhs) noexcept {
  std::cout << "Custom std::swap" << std::endl;
  lhs.swap(rhs);
}

}  // namespace std

int main(int argc, const char* argv[]) {
  N::C c1{1}, c2{2};
  std::cout << "original c1: " << c1 << " original C2: " << c2 << std::endl;
  std::swap(c1, c2);
  std::cout << "After calling std::swap\n";
  std::cout << "std::swapped c1: " << c1 << "std::swapped c2: " << c2
            << std::endl;

  swap(c1, c2);
  std::cout << "After calling swap\n";
  std::cout << "swapped c1: " << c1 << "Swapped c2: " << c2 << std::endl;
  return 0;
}