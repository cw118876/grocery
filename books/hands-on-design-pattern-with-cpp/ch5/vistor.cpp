#include <iostream>
#include <string>
#include <vector>

struct Animal {
  enum Type { CAT, DOG, RAT };
  Animal(Type t, const std::string& name) : type_{t}, name_{name} {}
  Animal(Type t, std::string&& name) : type_{t}, name_{std::move(name)} {}
  const Type type_;
  std::string name_;
};

template <typename D>
class GenericVisitor {
 public:
  template <typename Iter>
  void visit(Iter begin, Iter end) {
    for (Iter cur = begin; cur != end; ++cur) {
      visit(*cur);
    }
  }
  D& getRef() { return *static_cast<D*>(this); }
  D& getRef() const { return *static_cast<const D*>(this); }

  void visit(const Animal& animal) {
    switch (animal.type_) {
      case Animal::CAT:
        getRef().do_visit_cat(animal);
        break;
      case Animal::DOG:
        getRef().do_visit_dog(animal);
        break;
      case Animal::RAT:
        getRef().do_visit_rat(animal);
        break;
      default:
        (void)animal;
    }
  }
  void do_visit_cat(const Animal& animal) {
    std::cout << "default visit cat: " << animal.name_ << std::endl;
  }
  void do_visit_dog(const Animal& animal) {
    std::cout << "default visit dog: " << animal.name_ << std::endl;
  }
  void do_visit_rat(const Animal& animal) {
    std::cout << "default visit rat: " << animal.name_ << std::endl;
  }
};

class DefaultVisitor : public GenericVisitor<DefaultVisitor> {};
class FooVisitor : public GenericVisitor<FooVisitor> {
 public:
  void do_visit_cat(const Animal& animal) {
    std::cout << "##### foo visit cat: " << animal.name_ << std::endl;
  }
  void do_visit_dog(const Animal& animal) {
    std::cout << "##### foo visit dog: " << animal.name_ << std::endl;
  }
  void do_visit_rat(const Animal& animal) {
    std::cout << "##### foo visit rat: " << animal.name_ << std::endl;
  }
};

class BarVisitor : public GenericVisitor<BarVisitor> {
 public:
  void do_visit_dog(const Animal& animal) {
    std::cout << "$$$$$ bar visit dog: " << animal.name_ << std::endl;
  }
  void do_visit_rat(const Animal& animal) {
    std::cout << "$$$$$ bar visit rat: " << animal.name_ << std::endl;
  }
};

int main() {
  std::vector<Animal> animals{Animal{Animal::Type::CAT, "lucy"},
                              Animal{Animal::Type::DOG, "dylen"},
                              Animal{Animal::Type::RAT, "rax"}};
  DefaultVisitor dv;
  dv.visit(animals.begin(), animals.end());
  FooVisitor fv;
  fv.visit(animals.begin(), animals.end());
  BarVisitor bv;
  bv.visit(animals.begin(), animals.end());
  return 0;
}