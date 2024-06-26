#include <iostream>
#include <utility>
#include <vector>

using V = std::vector<int>;

int main() {
  {  // OK, member swap
    V v({1, 2, 3, 4});
    std::cout << "Initial v:";
    for (auto x : v)
      std::cout << " " << x;
    std::cout << std::endl;
    V t;
    v.swap(t);
    std::cout << "Final v:";
    for (auto x : v)
      std::cout << " " << x;
    std::cout << std::endl;
  }

#if 0
    { // Does not compile, swap() needs an rvalue
        V v({1, 2, 3, 4});
        std::cout << "Initial v:"; for (auto x: v) std::cout << " " << x; std::cout << std::endl;
        v.swap(V()); // non-const lvalue reference to type 'vector<...>' cannot bind to a temporary of type 'vector<...>' 
        std::cout << "Final v:"; for (auto x: v) std::cout << " " << x; std::cout << std::endl;
    }
#endif

  {  // Compiles fine, member function call on a temporary
    V v({1, 2, 3, 4});
    std::cout << "Initial v:";
    for (auto x : v)
      std::cout << " " << x;
    std::cout << std::endl;
    V().swap(v);
    std::cout << "Final v:";
    for (auto x : v)
      std::cout << " " << x;
    std::cout << std::endl;
  }

  {  // OK, non-member swap
    V v({1, 2, 3, 4});
    std::cout << "Initial v:";
    for (auto x : v)
      std::cout << " " << x;
    std::cout << std::endl;
    V t;
    swap(v, t);
    std::cout << "Final v:";
    for (auto x : v)
      std::cout << " " << x;
    std::cout << std::endl;
  }

#if 0
    { // Does not compile, swap() needs an rvalue
        V v({1, 2, 3, 4});
        std::cout << "Initial v:"; for (auto x: v) std::cout << " " << x; std::cout << std::endl;
        swap(v, V()); //error: no matching function for call to 'swap'
        std::cout << "Final v:"; for (auto x: v) std::cout << " " << x; std::cout << std::endl;
    }
#endif  // 0
}