#pragma once
#include <iostream>
#include <vector>
using Vector2D = std::vector<std::vector<int>>;

class Rectangle {
 public:
  explicit Rectangle(double width, double length)
      : width_{width}, length_{length} {}

 private:
  double width_ = 0;
  double length_ = 0;
};

void transfer(Rectangle&, const Vector2D&) {
  std::cout << "Rectangle: transfer\n";
}

void rotate(Rectangle&, double const&) {
  std::cout << "Rectangle: rotate\n";
}

void draw(const Rectangle&) {
  std::cout << "Rectangle: draw\n";
}
