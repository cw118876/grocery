#pragma once
#include <iostream>
#include <vector>

using Vector2D = std::vector<std::vector<int>>;

class Square {
 public:
  explicit Square(double side) : side_{side} {}

 private:
  double side_ = 0;
};

void transfer(Square&, const Vector2D&) {
  std::cout << "Square: transfer\n";
}

void rotate(Square&, double const&) {
  std::cout << "Square: rotate\n";
}

void draw(const Square&) {
  std::cout << "Square: draw\n";
}
