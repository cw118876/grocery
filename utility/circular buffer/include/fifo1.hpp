#ifndef UTILITY_HPP_
#define UTILITY_HPP_

#include <cassert>
#include <memory>

// NO-thread safe, cicular FIFO, has data races
template <class Tp, class Alloc = std::allocator<Tp>>
class Fifo1 : private Alloc {
 public:
  using value_type = Tp;
  using pointer_type = Tp*;
  using allocator_traits = std::allocator_traits<Alloc>;
  using size_type = typename allocator_traits::size_type;
  explicit Fifo1(size_type sz, const Alloc& alloc = Alloc{})
      : Alloc(alloc),
        capacity_(sz),
        ring_(allocator_traits::allocate(*this, capacity_)) {}
  Fifo1(const Fifo1&) = delete;
  Fifo1& operator=(const Fifo1&) = delete;

  Fifo1(Fifo1&&) = delete;
  Fifo1& operator=(Fifo1&&) = delete;

  ~Fifo1() {
    while (!empty()) {
      ring_[popCursor_ % capacity_].~Tp();
      popCursor_++;
    }
    allocator_traits::deallocate(*this, ring_, capacity_);
  }
  auto size() const noexcept {
    assert(popCursor_ <= pushCursor_);
    return pushCursor_ - popCursor_;
  }
  auto full() const noexcept { return size() == capacity_; }
  auto empty() const noexcept { return size() == 0; }

  auto push(const Tp& value) {
    if (full()) {
      return false;
    }
    ::new (&ring_[pushCursor_ % capacity_]) Tp(value);
    ++pushCursor_;
    return true;
  }
  auto pop(value_type& value) {
    if (empty()) {
      return false;
    }
    value = ring_[popCursor_ % capacity_];
    ring_[popCursor_ % capacity_].~Tp();
    ++popCursor_;
    return true;
  }

 private:
  size_type capacity_;
  pointer_type ring_;
  size_type pushCursor_{};
  size_type popCursor_{};
};

#endif  // UTILITY_HPP_