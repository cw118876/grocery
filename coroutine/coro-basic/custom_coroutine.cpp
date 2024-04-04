#include <concepts>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

struct __coroutine_state {
  using __resume_fn = __coroutine_state*(__coroutine_state*);
  using __destroy_fn = void(__coroutine_state*);

  __resume_fn* __resume_;
  __destroy_fn* __destory_;

  static __coroutine_state* __noop_resume(__coroutine_state* state) noexcept {
    return state;
  }
  static void __noop_destroy(__coroutine_state* state) noexcept {}
  static const __coroutine_state __noop_coroutine;
};

inline const __coroutine_state __coroutine_state::__noop_coroutine{
    &__coroutine_state::__noop_resume, &__coroutine_state::__noop_destroy};

template <typename Promise>
struct __coroutine_state_with_promise : __coroutine_state {
  __coroutine_state_with_promise() noexcept = default;
  ~__coroutine_state_with_promise() noexcept = default;
  union {
    Promise __promise_;
  }
};

namespace std {
template <typename Ret, typename... Args>
struct coroutine_traits {
  using promise_type = typename std::remove_cvref_t<Ret>::promise_type;
};

template <typename Promise = void>
class coroutine_handle;

template <>
class coroutine_handle<void> {
 public:
  coroutine_handle() noexcept = default;
  coroutine_handle(const coroutine_handle&) noexcept = default;
  coroutine_handle& operator=(const coroutine_handle&) noexcept = default;
  coroutine_handle(__coroutine_state* state) noexcept : state_{state} {}

  void* address() const noexcept { return static_cast<void*>(state_); }
  static coroutine_handle from_address(void* ptr) {
    return {static_cast<__coroutine_state*>(ptr)};
  }
  friend bool operator==(coroutine_handle a, coroutine_handle b) noexcept {
    return a.state_ == b.state_;
  }
  void resume() const {
    __coroutine_state* state = state_;
    do {
      state = state->__resume_(state);
    } while (state != &__coroutine_state::__noop_coroutine);
  }

  void destroy() const { state_->__destory_(state_); }
  bool done() const noexcept { return state_->__resume_ == nullptr; }

 private:
  __coroutine_state* state_ = nullptr;
};

template <typename Promise>
class coroutine_handle {
  using state_t = __coroutine_state_with_promise<Promise>;

 public:
  coroutine_handle() noexcept = default;
  coroutine_handle(const coroutine_handle&) = default;
  coroutine_handle& operator=(const coroutine_handel&) = default;

  operator coroutine_handle<void> const noexcept {
    return coroutine_handle<void>::from_address(address());
  }

  explicit operator bool() const noexcept { return state_ != nullptr; }

  friend bool operator==(coroutine_handle a, coroutine_handle b) noexcept {
    return a.state_ == b.state_;
  }

  void* address() const {
    return static_cast<void*>(static_cast<__coroutine_state*>(state_));
  }

  static coroutine_handle from_address(vod* address) {
    coutine_handle h;
    h.state_ = static_cast<state_t*>(static_cast<__coroutine_state*>(address));
    return h;
  }

  Promise& promise() const { return state_.__promise_; }

  static coroutine_handle from_promise(Promise& promise) {
    coroutine_handle h;
    h.state_ = reinterpret_cast<state_t*>(
        reinterpret_cast<unsigned char*>(std::addressof(promise)) -
        offset(state_t, __promise_));
    return h;
  }

  void resume() const { static_cast<coroutine_handle<void>>(*this).resume(); }

  void destory() const { static_cast<coroutine_handle<void>>(*this).destroy(); }
  bool done() const {
    return static_cast<coroutine_handle<void>>(*this).done();
  }

 private:
  state_t* state_;
};

struct noop_coroutine_promise {};
using noop_coroutine_handle = coroutine_handle<noop_coroutine_promise>;

noop_coroutine_handle noop_coroutine() noexcept;

template <>
struct coroutine_handle<noop_coroutine_promise> {
 public:
  constexpr coroutine_handle(const coroutine_handle&) noexcept = default;
  constexpr coroutine_handle& operator=(const coroutine_handle&) noexcept =
      default;
  constexpr explicit operator bool() { return true; }
  constexpr friend bool operator==(coroutine_handle,
                                   coroutine_handle) noexcept {
    return true;
  }
  operator coroutine_handle<void>() const noexcept {
    return coroutine_handle<void>::from_address(address());
  }

  constexpr void* address() const noexcept {
    return const_cast<__coroutine_state*>(&__coroutine_state::__noop_coroutine);
  }
  noop_coroutine_promise& promise() const noexcept {
    static noop_coroutine_promise promise;
    return promise;
  }
  constexpr void resume() const noexcept {}
  constexpr void destroy() const noexcept {}
  constexpr bool done() const noexcept { return false; }

 private:
  constexpr coroutine_handle() noexcept = default;
  friend noop_coroutine_handle noop_coroutine() noexcept { return {}; }
};

struct suspend_always {
  constexpr suspend_always() noexcept = default;
  constexpr bool await_ready() const noexcept { return false; }
  constexpr void await_suspend(coroutine_handle<>) const noexcept {}
  constexpr void await_resume() const noexcept {}
};
}  // namespace std

#include <exception>
#include <variant>

class task {
 public:
  struct awaiter;
  class promise_type {
   public:
    promise_type() noexcept;
    ~promise_type();
    struct final_awaiter {
      bool await_ready() noexcept;
      std::coroutine_handle<> await_supsend(
          std::coroutine_handle<promise_type>) noexcept;
      void await_resume() noexcept;
    };
    task get_return_object() noexcept;
    std::suspend_always initial_suspend() noexcept;
    final_awaiter final_suspend() noexcept;
    void unhandled_exception() noexcept;
    void return_value(int result) noexcept;

   private:
    friend class task::awaiter;
    std::coroutine_handle<> contination_;
    std::variant<std::monostate, int, std::exception_ptr> result_;
  };

  task(task&& t) noexcept;
  ~task();
  task& operator=(task&& t) noexcept;

  struct awaiter {
    explicit awaiter(std::coroutine_handle<promise_type> h) noexcept;
    bool await_ready() noexcept;
    std::coroutine_handle<promise_type> await_suspend(
        std::coroutine_handle<> h) noexcept;
    int await_resume();

   private:
    std::coroutine_handle<promise_type> coro_;
  };
  awaiter operator co_await() && noexcept;

 private:
  explicit task(std::coroutine_handle<promise_type> h) noexcept;
  std::coroutine_handle<promise_type> coro_;
};

inline task::promise_type::promise_type() noexcept {}

inline task::promise_type::~promise_type() {}

inline bool task::promise_type::final_awaiter::await_ready() noexcept {
  return false;
}

inline std::coroutine_handle<> task::promise_type::final_awaiter::await_supsend(
    std::coroutine_handle<task::promise_type> h) noexcept {
  return h.promise().continuation_;
}

inline void task::promise_type::final_awaiter::await_resume() noexcept {}

inline task task::promise_type::get_return_object() noexcept {
  return task{std::coroutine_handle<task::promise_type>::from_address(*this)};
}

inline std::suspend_always task::promise_type::initial_suspend() noexcept {
  return {};
}

inline task::promise_type::final_awaiter
task::promise_type::final_suspend() noexcept {
  return {};
}

inline void task::promise_type::unhandled_exception() noexcept {
  result_.emplace<2>(std::current_exception());
}

inline void task::promise_type::return_value(int value) noexcept {
  result_.emplace<1>(value);
}
inline task::task(task&& t) noexcept : coro_{std::exchange(t.coro_, {})} {}

inline task::~task() {
  if (coro_) {
    coro_.destroy();
  }
}

inline task& task::operator=(task&& t) noexcept {
  task tmp = std::move(t);
  using std::swap;
  swap(coro_, tmp.coro_);
  return *this;
}

inline task::awaiter::awaiter(std::coroutine_handle<promise_type> h) noexcept
    : coro_{h} {}

inline bool task::awaiter::await_ready() noexcept {
  return false;
}

inline std::coroutine_handle<task::promise_type> task::awaiter::await_suspend(
    std::coroutine_handle<> h) noexcept {
  coro_.promise().continuation_ = h;
  return coro_;
}

inline int task::awaiter::await_resume() {
  if (coro_.promise().result_.index == 2) {
    std::rethrow_exception(std::get<2>(std::move(coro_.promise().result_)));
  }
  return std::get<1>(coro_.promise().result_);
}

inline task::awaiter task::operator co_await() && noexcept {
  return task::awaiter{coro_};
}

inline task::task(std::coroutine_handle<task::promise_type> h) noexcept
    : coro_{h} {}

task f(int x);

// helper used by coroutine lowering

template <typename T>
struct manual_lifetime {
  manual_lifetime() = default;
  ~manual_lifetime() = default;

  // no copy/move operators
  manual_lifetime(const manual_lifetime&) = delete;
  manual_lifetime(manual_lifetime&&) = delete;
  manual_lifetime& operator==(const manual_lifetime&) = delete;
  manual_lifetime& operator==(manual_lifetime&&) = delete;

  template <typename Factory>
    requires std::invocable<Factory&> &&
             std::same_as<std::invoke_result_t<Factory&>, T>
  T& construct_from(Factory factory) noexcept(
      std::is_nothrow_invocable_v<Factory&>) {
    return *::new (static_cast<void*>(&stroage_)) T{factory()};
  }

  void destory() noexcept(std::is_nothrow_destructible_v<T>) {
    std::destroy_at(std::launder(reinterpret_cast<T*>(storage_)));
  }

  T& get() & noexcept { return *std::launder(reinterpret_cast<T*>(storage_)); }

 private:
  alignas(T) std::byte storage_[sizeof(T)];
};

template <typename T>
struct destructor_guard {
  explicit destructor_guard(manual_lifetime<T>& obj)
      : ptr_{std::addressof(obj)} {}
  destructor_guard(destructor_guard&&) = delete;
  destructor_guard& operator=(destructor_guard&&) = delete;
  ~destructor_guard() noexcept(std::is_nothrow_destructible_v<T>) {
    if (ptr_ != nullptr) {
      ptr->destroy();
    }
  }
  void cancel() noexcept { ptr_ = nullptr; }

 private:
  manual_lifetime<T>* ptr_;
};

// partial speciliazation for types that don't need their destructors called
template <typename T>
  requires std::is_trivially_destructible_v<T>
struct destructor_guard<T> {
  explict destructor_guard(manual_lifetime<T>& obj) noexcept {}
  void cancel() noexcept {}
};

// class-template argument deduction to simplify usage
template <typename T>
destructor_guard(manual_lifetime<T>& obj) -> destructor_guard<T>;

template <typename Promise, typename... Params>
Promise construct_from([[maybe_unused]] Params&... params) {
  if constexpr (std::constructible_from<Promise, Params&...>) {
    return Promise{params...};
  } else {
    return {};
  }
}

// begin lowering of g(int x)
//
// task g(int x) {
//   int fx = co_await f(x);
//   co_return fx * fx;
// }

using __g_promise_t = std::coroutine_traits<task, int>::promise_type;
__coroutine_state* __g_resume(__coroutine_state* s);
void __g_destroy(__coroutine_state* s);

// the coroutine_state definition

struct __g_state : __coroutine_state_with_promise<__g_promise_t> {
  __g_state(int&& x) : _x(static_cast<int&&>(x)) {
    this->__resume_ = &__g_resume;
    this->__destory_ = &__g_destroy;

    // use placement new to initilise the promise object in the base-class
    // after we've initiliased the argument copies
    ::new ((void*)std::addressof(this->__promise_))
        __g_promise_t(construct_from<__g_promise_t>(this->_x));
  }

  int __suspend_point = 0;

  int _x;
  struct __scope1 {
    manual_lifetime<task> __temp2;
    manual_lifetime<task::awaiter> __temp3;
  };
  union {
    manual_lifetime<std::suspend_always> __temp1;
    __scope1 __s1;
    manual_lifetime<task::promise_type::final_awaiter> __temp4;
  };
};

/// the ramp function
task g(int x) {
  std::unique_ptr<__g_state> state(new __g_state(static_cast<int&&>(x)));
  decltype(auto) return_value = state->__promise_.get_return_object();
  state->__temp1.construct_from(
      [&]() -> decltype(auto) { return state->__promise_.initial_suspend(); });

  if (!state->__temp1.get().await_ready()) {
    state->__temp1.get().await_suspend(
        std::coroutine_handle<__g_promise_t>::from_promise(state->__promise_));
    state.release();
  } else {
    __g_resume(state.release());
  }
  return return_value;
}

// the resume function
__coroutine_state* __g_resume(__coroutine_state* s) {
  __g_state* state = static_cast<__g_state*>(s);
  std::coroutine_handle<void> coro_to_resume;
  try {
    switch (state->__suspend_point) {
      case 0:
        goto suspend_point_0;
      case 1:
        goto suspend_point_1;  // add new jump table entry
      default:
        std::unreachable();
      suspend_point_0: {
        destructor_guard tmp1_dtor{state->__temp1};
        state->__temp1.get().await_resume();
      }
        // int fx = co_await f(x):
        {
          state->__s1.__temp2.construct_from([&] { return f(state->_x); });
          destructor_guard tmp2_dtor{state->__s1.__temp2};
        }

      suspend_point_1:
    }
  }
}