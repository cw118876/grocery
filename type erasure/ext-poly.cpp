#include "circle.hpp"
#include "rectangle.hpp"
#include "square.hpp"

#include <iostream>
#include <memory>
#include <vector>

class Shape {
 public:
  class ShapeConcept {
   public:
    virtual void do_transfer(Vector2D&) = 0;
    virtual void do_rotate(double const&) = 0;
    virtual void do_draw() = 0;
    virtual ~ShapeConcept(){};
  };
  template <typename T>
  class ShapeDetail : public ShapeConcept {
   public:
    template <typename... Args>
    ShapeDetail(Args&&... args) : value_{std::forward<Args>(args)...} {}
    void do_transfer(Vector2D& v) final { transfer(value_, v); }
    void do_rotate(double const& d) final { rotate(value_, d); }
    void do_draw() final { draw(value_); }
    ~ShapeDetail() = default;

   private:
    T value_;
  };
};

void draw(std::vector<std::unique_ptr<Shape::ShapeConcept>>& vec) {
  for (auto& v : vec) {
    v->do_draw();
  }
}

void rotate(std::vector<std::unique_ptr<Shape::ShapeConcept>>& vec) {
  for (auto& v : vec) {
    v->do_rotate(10);
  }
}

int main(int argc, const char* argv[]) {
  using Shape_t = std::unique_ptr<Shape::ShapeConcept>;
  std::vector<Shape_t> vec;
  vec.reserve(10);
  vec.emplace_back(new Shape::ShapeDetail<Circle>(10.0));
  vec.emplace_back(new Shape::ShapeDetail<Square>(10.1));
  vec.emplace_back(new Shape::ShapeDetail<Rectangle>(10.1, 20.2));
  draw(vec);
  rotate(vec);
  return 0;
}