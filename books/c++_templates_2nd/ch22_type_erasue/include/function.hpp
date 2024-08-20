#ifndef FUNCTION_HPP_
#define FUNCTION_HPP_

#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include <exception>

namespace tmp {

template <typename Signature>
class function;

class bad_function_call: public std::exception {
public:
   bad_function_call() noexcept = default;
   bad_function_call(const bad_function_call&) noexcept = default;
   bad_function_call& operator=(const bad_function_call&) noexcept = default;

   ~bad_function_call() override = default;
   [[nodiscard]] const char* what() const noexcept override;
};

namespace detail {



template <typename Fp>
bool not_null(const Fp&) {
  return true;
}

template <typename Fp>
bool not_null(Fp* ptr) {
  return ptr;
}

template <typename Ret, typename C>
bool not_null(Ret C::*ptr) {
  return ptr;
}

template <typename Signature>
bool not_null(const function<Signature>& f) {
  return !!f;
}

template <typename Fun, typename Rp, typename... Args>
class default_alloc_fun {
  Fun f_;

 public:
  using target_type = Fun;
  target_type& target() const { return f_; }
  explicit default_alloc_fun(const target_type& f) : f_(f) {}
  explicit default_alloc_fun(target_type&& f) noexcept : f_(std::move(f)) {}

  Rp operator()(Args&&... args) {
    return std::invoke(f_, std::forward<Args>(args)...);
  }

  default_alloc_fun* clone() const {
    auto ptr = std::make_unique<default_alloc_fun>(*this);
    return ptr.release();
  }
  void destroy() noexcept { f_.~target_type(); }

  static void purge(default_alloc_fun* df) {
    df->destroy();
    ::operator delete(df);  // just delete the memory block
  }
};

template <typename Signature>
class func_base;

template <typename Rp, typename... Args>
class func_base<Rp(Args...)> {
  using return_type = Rp;
  func_base() = default;
  virtual ~func_base() = default;
  virtual func_base* clone() const = 0;
  // in place construction, caller should guarantee memory's validity
  virtual void clone(func_base* p) const = 0;

  virtual void destroy() = 0;
  virtual void purge() = 0;
  virtual Rp operator()(Args&&...) = 0;
};

template <typename Fp, typename Signature>
class func;

template <typename Fp, typename Rp, typename... Args>
class func<Fp, Rp(Args...)> : public func_base<Rp(Args...)> {
  default_alloc_fun<Fp, Rp(Args...)> f_;

 public:
  explicit func(Fp&& f) : f_(std::move(f)) {}
  explicit func(const Fp& f) : f_(f) {}
  func_base<Rp(Args...)>* clone() const override;
  void clone(func_base<Rp(Args...)>* p) const override;

  void destroy() override;
  void purge() override;
  Rp operator()(Args&&...) override;
};

template <typename Fp, typename Rp, typename... Args>
func_base<Rp(Args...)>* func<Fp, Rp(Args...)>::clone() const {
  auto ptr = std::make_unique<func>(f_.target());
  return ptr.release();
}

template <typename Fp, typename Rp, typename... Args>
void func<Fp, Rp(Args...)>::clone(func_base<Rp(Args...)>* ptr) const {
  ::new (static_cast<void*>(ptr)) func(f_.target());
}
template <typename Fp, typename Rp, typename... Args>
void func<Fp, Rp(Args...)>::destroy() {
    f_.destroy();
}
template <typename Fp, typename Rp, typename... Args>
void func<Fp, Rp(Args...)>::purge() {
    purge(&f_);
}

template <typename Fp, typename Rp, typename... Args>
Rp func<Fp, Rp(Args...)>::operator()(Args &&... args) {
    return f_(std::forward<Args>(args)...);
}

template <typename Signature>
class value_func;


template <typename Rp, typename... Args>
class value_func<Rp(Args...)> {
    static constexpr size_t kStorageLen = 3 * sizeof(void*);
    using func_base_type = func_base<Rp(Args...)>;
    std::aligned_storage_t<kStorageLen> buf_;
    func_base_type* f_;
    static func_base_type* as_base(void* ptr) { return reinterpret_cast<func_base_type*>(ptr);}
    [[nodiscard]] bool self_referential() const noexcept {
        return f_ == &buf_;
    }

    void cleanup() {
        if (f_) {
            if (self_referential()) {
                f_->destroy();
            } else {
                f_->purge();
            }
        }

    }
public:
    value_func() noexcept: f_(nullptr) {}

    template <typename Fp, typename = std::enable_if_t<std::is_same_v<value_func, std::decay_t<Fp>>>>
    explicit value_func(Fp&& f): f_(nullptr) {
        using FunType = func<Fp, Rp(Args...)>;
        if (not_null(f)) {
            if constexpr (sizeof(Fp) <= kStorageLen && std::is_nothrow_copy_constructible_v<Fp>) {
                f_ = ::new (&buf_) FunType(std::forward<Fp>(f));
            } else {
                auto ptr = std::make_unique<FunType>(std::forward<Fp>(f));
                f_ = ptr.release();
            }
        }
    }
    
    value_func(const value_func& other): f_(nullptr) {
        if (other.f_) {
            if (other.self_referential()) {
                f_ = as_base(&buf_);
               other->f_.clone(f_);
            } else {
                f_ = other.f_->clone();
            }
        }
    }
    
    value_func& operator=(const value_func& other) {
        if (this != &other) {
            value_func tmp{other};
            swap(tmp);           
        }
        return *this;

    }
    
    value_func(value_func&& other) noexcept: f_(nullptr) {
        if (other.f_) {
            if (other.self_referential()) {
                f_ = as_base(&buf_);
                other.f_->clone(f_);     // TODO(move): handle move semantics
            } else {
                f_ = std::exchange(other.f_, nullptr);
            }
        }
    }
    value_func& operator=(value_func&& other)  noexcept {
        if (this != &other) {
            value_func tmp{std::move(other)};
            swap(tmp);
        }
        return *this;
    }


    ~value_func() {
        cleanup();
    }

    value_func& operator=(std::nullptr_t) {
        cleanup();
        f_ = nullptr;
        return *this;
    }

    Rp operator()(Args&&... args) {
        if (f_ == nullptr) {
            throw bad_function_call();

        }
        return f_->operator()(std::forward<Args>(args)...);
    }
    
    void swap(value_func& other) {
        if (this != &other) {
            if (self_referential() && other.self_referential()) {
                std::aligned_storage_t<kStorageLen> tempbuf;
                func_base_type* ptr = as_base(&tempbuf);
                other.f_->clone(ptr);
                other.f_->destroy();
                other.f_ = nullptr;
                f_->clone(as_base(&other.buf_));
                f_->destroy();
                f_ = nullptr;
                other.f_ = as_base(&other.buf_);

                ptr->clone(as_base(&buf_));
                ptr->destroy();
                f_ = as_base(&buf_);
            } else if (self_referential()) {
                auto other_ptr = as_base(&other.buf_);
                f_->clone(other_ptr);
                f_->destroy();
                f_ = other.f_;
                other.f_ = other_ptr;
            } else if (other.self_referential()) {
                other.swap(*this);
            } else {
                std::swap(f_, other.f_);
            }
        }
    }

    



};




}  // namespace detail

}  //  namespace tmp

#endif  // FUNCTION_HPP_