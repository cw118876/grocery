// #include "circle.hpp"
// #include "square.hpp"
// #include "rectangle.hpp"

#include <iostream>
#include <vector>
#include <memory>
#include <utility>
#include <array>

using Vector2D = std::vector<std::vector<int>>;

class Square {
 public:
 explicit Square(double side): side_{side}  {}

 private:
   double side_  = 0;
};


void transfer(const Square&, const Vector2D&) {
  std::cout << "Square: transfer\n";
}

void rotate(const Square&, double const&) {
  std::cout << "Square: rotate\n";
}

void draw(const Square&) {
  std::cout << "Square: draw\n";
}

class Circle {
 public:
 explicit Circle(double radius): radius_ {radius}  {}

 private:
   double radius_  = 0;
};


void transfer(const Circle&, const Vector2D&) {
  std::cout << "Circle: transfer\n";
}

void rotate(const Circle&, double const&) {
  std::cout << "Circle: rotate\n";
}

void draw(const Circle&) {
  std::cout << "Circle: draw\n";
}

class Rectangle {
 public:
 explicit Rectangle(double width, double length): width_{width}, length_{length}  {}

 private:
   double width_  = 0;
   double length_ = 0;
};


void transfer(const Rectangle&, const Vector2D&) {
  std::cout << "Rectangle: transfer\n";
}

void rotate(const Rectangle&, double const&) {
  std::cout << "Rectangle: rotate\n";
}

void draw(const Rectangle&) {
  std::cout << "Rectangle: draw\n";
}



class ShapeConstRef {
 public: 
  template <typename ShapeT>
  ShapeConstRef(const ShapeT& shape): shape_{std::addressof(shape)}, 
                                      draw_ {[](void const* shape) {
                                        draw(*static_cast<const ShapeT*>(shape));
                                      }},
                                      rotate_ {[](void const* shape, const double& d) {
                                        rotate(*static_cast<const ShapeT*>(shape), d);
                                      }} {}
  friend void draw(const ShapeConstRef& shape) {
    shape.draw_(shape.shape_);
  }

  friend void rotate(const ShapeConstRef& shape, const double& d) {
    shape.rotate_(shape.shape_, d);
  }
  

 private:
   using DrawOperation = void(void const *);
   using RotateOperation = void(void const*, const double &);
   const void* shape_ {nullptr};
   DrawOperation* draw_ {nullptr};
   RotateOperation* rotate_ {nullptr};
};


void drawAll(std::vector<ShapeConstRef>& vec) {
    for (auto& v: vec) {
        draw(v);
    }
}

void rotateAll(std::vector<ShapeConstRef>& vec) {
    for (auto& v: vec) {
        rotate(v, 10);
    }
}


int main(int argc, const char* argv[]) {
    std::vector<ShapeConstRef> vec;
    vec.reserve(10);
    auto c = Circle {10.0};
    vec.emplace_back(c);
    auto s = Square {10.1};
    vec.emplace_back(s);
    auto r = Rectangle {10.1, 10.111};
    vec.emplace_back(r);
    drawAll(vec);
    rotateAll(vec);
    return 0;
}