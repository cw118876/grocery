#ifndef UTILITY_FIFO2_HPP_
#define UTILITY_FIFO2_HPP_

#include <atomic>
#include <cassert>
#include <memory>

// NO-thread safe, cicular FIFO, has data races
template <class Tp, class Alloc = std::allocator<Tp>>
class Fifo2 : private Alloc {
 public:
  using value_type = Tp;
  using pointer_type = Tp*;
  using allocator_traits = std::allocator_traits<Alloc>;
  using size_type = typename allocator_traits::size_type;
  explicit Fifo2(size_type sz, const Alloc& alloc = Alloc{})
      : Alloc(alloc),
        capacity_(sz),
        ring_(allocator_traits::allocate(*this, capacity_)) {}
  Fifo2(const Fifo2&) = delete;
  Fifo2& operator=(const Fifo2&) = delete;

  Fifo2(Fifo2&&) = delete;
  Fifo2& operator=(Fifo2&&) = delete;

  ~Fifo2() {
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
  using cursor_type = std::atomic<size_type>;
  static_assert(cursor_type::is_always_lock_free, "size_type should lock-free");
  size_type capacity_;
  pointer_type ring_;
  cursor_type pushCursor_{};
  cursor_type popCursor_{};
};

#endif  // UTILITY_FIFO2_HPP_