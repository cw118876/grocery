#include "circle.hpp"
#include "rectangle.hpp"
#include "square.hpp"

#include <iostream>
#include <memory>
#include <utility>
#include <vector>

class Shape {
 public:
  class ShapeConcept {
   public:
    virtual void do_transfer(Vector2D&) = 0;
    virtual void do_rotate(double const&) = 0;
    virtual void do_draw() = 0;
    virtual std::unique_ptr<ShapeConcept> clone() = 0;
    virtual ~ShapeConcept(){};
  };

  template <typename T>
  class ShapeDetail : public ShapeConcept {
   public:
    ShapeDetail(T t) : value_{t} {}

    ShapeDetail(const ShapeDetail& rhs) : value_{rhs.value_} {}
    ShapeDetail& operator=(const ShapeDetail& rhs) {
      value_ = rhs.value_;
      return *this;
    }

    ShapeDetail(ShapeDetail&& rhs) : value_{std::move(rhs.value_)} {}
    ShapeDetail& operator=(ShapeDetail&& rhs) {
      value_ = std::move(rhs.value_);
      return *this;
    }

    void do_transfer(Vector2D& v) final { transfer(value_, v); }
    void do_rotate(double const& d) final { rotate(value_, d); }
    void do_draw() final { draw(value_); }
    std::unique_ptr<ShapeConcept> clone() {
      return std::make_unique<ShapeDetail>(*this);
    }

    ~ShapeDetail() = default;

   private:
    T value_;
  };

  friend void draw(const Shape& shape) { shape.pimp_->do_draw(); }
  friend void rotate(const Shape& shape, const double& d) {
    shape.pimp_->do_rotate(d);
  }
  friend void transfer(const Shape& shape, Vector2D& v) {
    shape.pimp_->do_transfer(v);
  }

  template <typename ShapeT>
  explicit Shape(ShapeT shape)
      : pimp_{std::make_unique<ShapeDetail<ShapeT>>(std::move(shape))} {}
  Shape(const Shape& rhs) : pimp_{rhs.pimp_->clone()} {}
  Shape& operator=(const Shape& rhs) {
    pimp_.reset();
    pimp_ = rhs.pimp_->clone();
    return *this;
  }

  Shape(Shape&& rhs) : pimp_{std::exchange(rhs.pimp_, {})} {}

  Shape& operator=(Shape&& rhs) {
    pimp_ = std::exchange(rhs.pimp_, {});
    return *this;
  }

 private:
  std::unique_ptr<ShapeConcept> pimp_;
};

void drawAll(std::vector<Shape>& vec) {
  for (auto& v : vec) {
    draw(v);
  }
}

void rotateAll(std::vector<Shape>& vec) {
  for (auto& v : vec) {
    rotate(v, 10);
  }
}

int main(int argc, const char* argv[]) {
  std::vector<Shape> vec;
  vec.reserve(10);
  vec.emplace_back(Circle{10.0});
  vec.emplace_back(Square{10.1});
  vec.emplace_back(Rectangle{10.1, 10.111});
  drawAll(vec);
  rotateAll(vec);
  return 0;
}