#ifndef UTILITY_FIFO4_HPP_
#define UTILITY_FIFO4_HPP_

#include <atomic>
#include <cassert>
#include <memory>

// NO-thread safe, cicular FIFO, has data races
template <class Tp, class Alloc = std::allocator<Tp>>
class Fifo4 : private Alloc {
 public:
  using value_type = Tp;
  using pointer_type = Tp*;
  using allocator_traits = std::allocator_traits<Alloc>;
  using size_type = typename allocator_traits::size_type;
  explicit Fifo4(size_type sz, const Alloc& alloc = Alloc{}) : Alloc(alloc) {
    capacity_ = alignedSize(sz);
    ring_ = allocator_traits::allocate(*this, capacity_);
  }
  Fifo4(const Fifo4&) = delete;
  Fifo4& operator=(const Fifo4&) = delete;

  Fifo4(Fifo4&&) = delete;
  Fifo4& operator=(Fifo4&&) = delete;

  ~Fifo4() {
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
    auto pushIdx = pushCursor_.load(std::memory_order_relaxed);
    if (full(cachedPopCursor_, pushIdx)) {
      cachedPopCursor_ = popCursor_.load(std::memory_order_acquire);
      if (full(cachedPopCursor_, pushIdx)) {
        return false;
      }
    }
    ::new (&ring_[pushIdx % capacity_]) Tp(value);
    pushCursor_.store(pushIdx + 1, std::memory_order_release);
    return true;
  }
  auto pop(value_type& value) {
    auto popIdx = popCursor_.load(std::memory_order_relaxed);
    if (empty(popIdx, cachedPushCursor_)) {
      cachedPushCursor_ = pushCursor_.load(std::memory_order_acquire);
      if (empty(popIdx, cachedPushCursor_)) {
        return false;
      }
    }

    value = ring_[popIdx % capacity_];
    ring_[popIdx % capacity_].~Tp();
    popCursor_.store(popIdx + 1, std::memory_order_release);
    return true;
  }

 private:
  auto full(size_type popIdx, size_type pushIdx) {
    // assert(popIdx <= pushIdx);
    return (pushIdx - popIdx) == capacity_;
  }
  auto empty(size_type popIdx, size_type pushIdx) {
    // assert(popIdx <= pushIdx);
    return (pushIdx - popIdx) == 0;
  }
  static constexpr size_t hardware_destructive_interference_size = 64;
  size_type alignedSize(size_type sz) {
    auto cacheSz = hardware_destructive_interference_size;
    return sz < cacheSz ? cacheSz : (sz - cacheSz) / cacheSz + cacheSz;
  }
  using cursor_type = std::atomic<size_type>;
  static_assert(cursor_type::is_always_lock_free, "size_type should lock-free");
  size_type capacity_;
  pointer_type ring_;
  alignas(hardware_destructive_interference_size) cursor_type pushCursor_{};
  alignas(hardware_destructive_interference_size) size_type cachedPushCursor_{};
  alignas(hardware_destructive_interference_size) cursor_type popCursor_{};
  alignas(hardware_destructive_interference_size) size_type cachedPopCursor_{};
};

#endif  // UTILITY_FIFO4_HPP_