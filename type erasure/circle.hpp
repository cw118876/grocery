#pragma once
#include <iostream>
#include <vector>
using Vector2D = std::vector<std::vector<int>>;

class Circle {
 public:
  explicit Circle(double radius) : radius_{radius} {}

 private:
  double radius_ = 0;
};

void transfer(Circle&, const Vector2D&) {
  std::cout << "Circle: transfer\n";
}

void rotate(Circle&, double const&) {
  std::cout << "Circle: rotate\n";
}

void draw(const Circle&) {
  std::cout << "Circle: draw\n";
}
