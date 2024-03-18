#include <iostream>
#include <utility>

class A {
 public:
  explicit A(int i) : i_{i} {}
  A(const A& rhs) : i_{rhs.i_} { std::cout << "A(const A&)" << std::endl; }
  A& operator=(const A& rhs) {
    i_ = rhs.i_;
    std::cout << "A& operator(const A&)" << std::endl;
    return *this;
  }
  A(A&&) = delete;
  A& operator=(A&& rhs)= delete;
  friend std::ostream& operator<<(std::ostream& os, const A& rhs) {
    os << rhs.i_;
    return os;
  }

 private:
  int i_;
};

int main(int argc, const char* argv[]) {
  A a1{1}, a2{2};
  std::cout << "Initial\n" << std::endl;
  std::cout << "a1: " << a1 << " a2 " << a2 << std::endl;

  std::swap(a1, a2);  // Not the best way.
  std::cout << "After std::swap\n";
  std::cout << "a1: " << a1 << " a2 " << a2 << std::endl;
  return 0;
}