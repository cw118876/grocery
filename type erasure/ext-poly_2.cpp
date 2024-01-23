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



class Shape {
 public:
  class ShapeConcept {
    public: 
     virtual void do_transfer(Vector2D&) const = 0;
     virtual void do_rotate(double const&) const = 0;
     virtual void do_draw() const = 0;
     virtual void clone(ShapeConcept *) const = 0;
     virtual void move(ShapeConcept*) const = 0;
     virtual ~ShapeConcept() {};
  }; 

  template <typename T>
  class ShapeDetail: public ShapeConcept {
    public:
      ShapeDetail(T t): value_{t} {}

      ShapeDetail(const ShapeDetail& rhs): value_{rhs.value_} {}
      ShapeDetail &operator=(const ShapeDetail& rhs) {
        value_ = rhs.value_;
        return *this;
      }

      ShapeDetail(ShapeDetail&& rhs): value_{std::move(rhs.value_)} {}
      ShapeDetail &operator=(ShapeDetail&& rhs) {
        value_ = std::move(rhs.value_);
        return *this;
      }


      void do_transfer(Vector2D& v) const final {
        transfer(value_, v);
      }
      void do_rotate(double const& d) const final {
        rotate(value_, d);
      }
      void do_draw() const final {
        draw(value_);
      }
      void clone(ShapeConcept *ptr) const final{
        new (ptr) ShapeDetail {*this};
      }
      void move(ShapeConcept* ptr) const final {
        new (ptr) ShapeDetail {std::move(*this)};
      }

      ~ShapeDetail() = default;

    private:
      T value_;
  };

  friend void draw(const Shape& shape) {
    auto p = shape.pimp_();
    p->do_draw();
  }
  friend void rotate(const Shape& shape, const double& d) {
    shape.pimp_()->do_rotate(d);
  }
  friend void transfer(const Shape& shape, Vector2D& v) {
    shape.pimp_()->do_transfer(v);
  }

  template <typename ShapeT>
  explicit Shape(const ShapeT& shape) {
    ::new (pimp_()) ShapeDetail<ShapeT>{shape};
  }
  Shape(const Shape& rhs) {
    rhs.pimp_()->clone(pimp_());
  }
  Shape &operator=(const Shape& rhs) {
    Shape tmp{rhs};
    buffer_.swap(tmp.buffer_);
    return *this;
  }

  Shape(Shape&& rhs) {
    rhs.pimp_()->move(pimp_());
  }
  
  Shape &operator=(Shape&& rhs) {
    Shape tmp{std::move(rhs)};
    buffer_.swap(tmp.buffer_);
    return *this;
  }

  ~Shape() {
    pimp_()->~ShapeConcept();
  }
  ShapeConcept *pimp_() {
    return reinterpret_cast<ShapeConcept*>(buffer_.data());
  }
  const ShapeConcept *pimp_() const {
    return reinterpret_cast<const ShapeConcept*>(buffer_.data());
  }
  
  private:
    static constexpr size_t alignLen = 8;
    static constexpr size_t bufferLen = 256;
    alignas(alignLen) std::array<std::byte, bufferLen> buffer_;
};

void drawAll(std::vector<Shape>& vec) {
    for (auto& v: vec) {
        draw(v);
    }
}

void rotateAll(std::vector<Shape>& vec) {
    for (auto& v: vec) {
        rotate(v, 10);
    }
}


int main(int argc, const char* argv[]) {
    std::vector<Shape> vec;
    vec.reserve(10);
    vec.emplace_back(Circle {10.0});
    vec.emplace_back(Square {10.1});
    vec.emplace_back(Rectangle {10.1, 10.111});
    drawAll(vec);
    rotateAll(vec);
    return 0;
}