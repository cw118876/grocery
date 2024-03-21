#include <iostream>

template <typename D>
struct Registry {
  inline static size_t count = 0;   // inline variable since C++17
  inline static D* head = nullptr;  // inline variable sinse C++17

  Registry() {
    ++count;
    prev_ = nullptr;
    next_ = head;
    head = static_cast<D*>(this);
    if (next_) {
      next_->prev_ = head;
    }
  }
  Registry(const Registry& rhs) {
    ++count;
    prev_ = nullptr;
    next_ = head;
    head = static_cast<D*>(this);
    if (next_) {
      next_->prev_ = head;
    }
  }
  ~Registry() {
    --count;
    if (prev_) prev_->next_ = next_;
    if (next_) next_->prev_ = prev_;
    if (head == static_cast<D*>(this)) {
      head = next_;
    }
  }
  D* prev_;
  D* next_;
};

class C : public Registry<C> {
 public:
  explicit C(int i) : i_{i} {}
  friend std::ostream& operator<<(std::ostream& os, const C& rhs) {
    os << rhs.i_;
    return os;
  }

 private:
  int i_ = 0;
};

class D : public Registry<D> {
 public:
  explicit D(int i) : i_{i} {}
  friend std::ostream& operator<<(std::ostream& os, const D& rhs) {
    os << rhs.i_;
    return os;
  }

 private:
  int i_ = 0;
};

template <typename T>
void report() {
  std::cout << "Count: " << std::endl;
  for (const T* p = T::head; p; p = p->next_) {
    std::cout << " " << *p;
  }
  std::cout << std::endl;
}

int main() {
  report<C>();
  C* c4 = nullptr;
  {
    C c1{1};
    report<C>();
    c4 = new C{4};
    C c2{2};
    D d1{10};
    report<C>();
    report<D>();
  }
  report<C>();
  C c3{3};
  report<C>();
  delete c4;
  report<C>();
  report<D>();
  return 0;
}