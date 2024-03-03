#include <cassert>
#include <concepts>
#include <cstddef>
#include <exception>
#include <iostream>
#include <memory>
#include <type_traits>
#include <utility>
#include <variant>

struct __coroutine_state {
  using __resume_fn = __coroutine_state*(__coroutine_state*);
  using __destroy_fn = void(__coroutine_state*);

  __resume_fn* resume_;
  __destroy_fn* destroy_;
  static __coroutine_state* __noop_resume(__coroutine_state* state) noexcept {
    return state;
  }
  static void __noop_destroy(__coroutine_state* state) noexcept { (void)state; }
  static const __coroutine_state __noop_coroutine;
};

inline const __coroutine_state __coroutine_state::__noop_coroutine = {
    __coroutine_state::__noop_resume, __coroutine_state::__noop_destroy};

template <typename Promise>
struct __coroutine_state_with_promise : __coroutine_state {
  __coroutine_state_with_promise() noexcept {}
  ~__coroutine_state_with_promise() {}
  union {
    Promise promise_;
  };
};

namespace coro {
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
  void* address() const noexcept { return static_cast<void*>(state_); }

  static coroutine_handle from_address(void* addr) {
    coroutine_handle h;
    h.state_ = static_cast<__coroutine_state*>(addr);
    return h;
  }
  explicit operator bool() noexcept { return state_ != nullptr; }
  friend bool operator==(coroutine_handle lhs, coroutine_handle rhs) noexcept {
    return lhs.state_ == rhs.state_;
  }
  bool done() const { return state_ == nullptr; }
  void destroy() const { state_->destroy_(state_); }
  void resume() const {
    __coroutine_state* s = state_;
    assert(s != nullptr);
    do {
      s = s->resume_(s);
    } while (s != &__coroutine_state::__noop_coroutine);
  }

 private:
  __coroutine_state* state_ = nullptr;
};

template <typename Promise>
class coroutine_handle {
  using state_t = __coroutine_state_with_promise<Promise>;

 public:
  coroutine_handle() noexcept = default;
  coroutine_handle(const coroutine_handle&) noexcept = default;
  coroutine_handle& operator=(const coroutine_handle&) noexcept = default;

  operator coroutine_handle<void>() const noexcept {
    return coroutine_handle<void>::from_address(static_cast<void*>(state_));
  }
  explicit operator bool() const { return state_ != nullptr; }
  friend bool operator==(coroutine_handle lhs, coroutine_handle rhs) noexcept {
    return lhs.state_ == rhs.state_;
  }
  bool done() const {
    return static_cast<coroutine_handle<void>>(*this).done();
  }
  void resume() const { static_cast<coroutine_handle<void>>(*this).resume(); }
  void destroy() const { static_cast<coroutine_handle<void>>(*this).destroy(); }
  void* address() const { return static_cast<void*>(state_); }
  static coroutine_handle from_address(void* addr) {
    coroutine_handle h;
    h.state_ = reinterpret_cast<state_t*>(addr);
    return h;
  }
  Promise& promise() noexcept { return state_->promise_; }
  static coroutine_handle from_promise(Promise& promise) {
    coroutine_handle h;
    h.state_ = reinterpret_cast<state_t*>(
        reinterpret_cast<unsigned char*>(std::addressof(promise)) -
        offsetof(state_t, promise_));
    return h;
  }

 private:
  state_t* state_ = nullptr;
};

struct noop_coroutine_promise {};
using noop_coroutine_handle = coroutine_handle<noop_coroutine_promise>;

noop_coroutine_handle noop_coroutine() noexcept;

template <>
class coroutine_handle<noop_coroutine_promise> {
 public:
  coroutine_handle(const coroutine_handle&) noexcept = default;
  coroutine_handle& operator=(const coroutine_handle&) noexcept = default;
  operator coroutine_handle<void>() const noexcept {
    return coroutine_handle<void>::from_address(address());
  }

  explicit operator bool() const noexcept { return true; }
  constexpr void* address() const noexcept {
    return static_cast<void*>(
        const_cast<__coroutine_state*>(&__coroutine_state::__noop_coroutine));
  }
  void resume() const noexcept {}
  void destory() const noexcept {}
  bool done() const noexcept { return false; }
  friend bool operator==(coroutine_handle lhs, coroutine_handle rhs) {
    return true;
  }
  noop_coroutine_promise& promise() const noexcept {
    static noop_coroutine_promise p;
    return p;
  }

 private:
  coroutine_handle() = default;
  ;
  friend noop_coroutine_handle noop_coroutine() noexcept { return {}; }
};

struct suspend_always {
  constexpr suspend_always() noexcept = default;
  constexpr bool await_ready() const noexcept { return false; }
  constexpr void await_suspend(coroutine_handle<>) const noexcept {}
  constexpr void await_resume() const noexcept {}
};

struct suspend_never {
  constexpr suspend_never() noexcept = default;
  constexpr bool await_ready() const noexcept { return true; }
  constexpr void await_suspend(coroutine_handle<>) const noexcept {}
  constexpr void await_resume() const noexcept {}
};

}  // namespace coro

class task {
 public:
  struct awaiter;
  class promise;
  using promise_type = promise;

  class promise {
   public:
    promise() noexcept;
    ~promise();
    struct final_awaiter {
      bool await_ready() noexcept;
      coro::coroutine_handle<> await_suspend(
          coro::coroutine_handle<promise_type> h) noexcept;
      void await_resume() noexcept;
    };
    task get_return_object() noexcept;
    coro::suspend_always initial_suspend() noexcept;
    final_awaiter final_suspend() noexcept;
    void unhandled_exception() noexcept;
    void return_value(int result) noexcept;
    void set_value(int result) noexcept;

   private:
    friend task::awaiter;
    coro::coroutine_handle<> continuation_;
    std::variant<std::monostate, int, std::exception_ptr> result_;
  };
  task(task&& t) noexcept;
  ~task();
  task& operator=(task&& t) noexcept;
  struct awaiter {
    explicit awaiter(coro::coroutine_handle<promise_type> h) noexcept;
    bool await_ready() noexcept;
    coro::coroutine_handle<promise_type> await_suspend(
        coro::coroutine_handle<> h) noexcept;

    int await_resume();
    coro::coroutine_handle<promise_type> coro_;
  };
  awaiter operator co_await() && noexcept;
  void resume();

 private:
  explicit task(coro::coroutine_handle<promise_type> h) noexcept;
  coro::coroutine_handle<promise_type> coro_;
};

inline task::promise::promise() noexcept {}
inline task::promise::~promise() {}

inline bool task::promise::final_awaiter::await_ready() noexcept {
  return false;
}

inline coro::coroutine_handle<> task::promise::final_awaiter::await_suspend(
    coro::coroutine_handle<task::promise_type> h) noexcept {
  if (h.promise().continuation_) {
    return h.promise().continuation_;
  }
  return coro::noop_coroutine();
}

inline void task::promise::final_awaiter::await_resume() noexcept {}
inline task task::promise::get_return_object() noexcept {
  return task{coro::coroutine_handle<task::promise_type>::from_promise(*this)};
}

inline coro::suspend_always task::promise::initial_suspend() noexcept {
  return {};
}

inline task::promise::final_awaiter task::promise::final_suspend() noexcept {
  return {};
}

inline void task::promise::unhandled_exception() noexcept {
  result_.emplace<2>(std::current_exception());
}

inline void task::promise::return_value(int value) noexcept {
  result_.emplace<1>(value);
}
inline void task::promise::set_value(int value) noexcept {
  result_.emplace<1>(value);
}

inline task::task(task&& t) noexcept : coro_(std::exchange(t.coro_, {})) {}

inline task::~task() {
  if (coro_) {
    coro_.destroy();
  }
}
inline void task::resume() {
  if (coro_) {
    coro_.resume();
  }
}

inline task& task::operator=(task&& t) noexcept {
  coro_ = std::exchange(t.coro_, {});
  return *this;
}

inline task::awaiter::awaiter(coro::coroutine_handle<promise_type> h) noexcept
    : coro_{h} {}

inline bool task::awaiter::await_ready() noexcept { return false; }

inline coro::coroutine_handle<task::promise_type> task::awaiter::await_suspend(
    coro::coroutine_handle<> h) noexcept {
  coro_.promise().continuation_ = h;
  return coro_;
}

inline int task::awaiter::await_resume() {
  // exeception case
  if (coro_.promise().result_.index() == 2) {
    std::rethrow_exception(std::get<2>(std::move(coro_.promise().result_)));
  }
  return std::get<1>(coro_.promise().result_);
}

inline task::awaiter task::operator co_await() && noexcept {
  return task::awaiter{coro_};
}

inline task::task(coro::coroutine_handle<promise_type> h) noexcept : coro_{h} {}

template <typename T>
struct manual_lifetime {
  manual_lifetime() noexcept = default;
  ~manual_lifetime() = default;
  manual_lifetime(const manual_lifetime&) = delete;
  manual_lifetime& operator=(const manual_lifetime&) = delete;
  manual_lifetime(manual_lifetime&&) = delete;
  manual_lifetime& operator=(manual_lifetime&&) = delete;

  template <typename Factory>
    requires std::invocable<Factory&> &&
             std::same_as<std::invoke_result_t<Factory&>, T>
  T& construct_from(Factory factory) noexcept(
      std::is_nothrow_invocable_v<Factory&>) {
    return *::new (static_cast<void*>(std::addressof(storage_))) T{factory()};
  }

  void destroy() {
    std::destroy_at(
        std::launder(reinterpret_cast<T*>(std::addressof(storage_))));
  }

  T& get() noexcept {
    return *std::launder(reinterpret_cast<T*>(std::addressof(storage_)));
  }

 private:
  alignas(T) std::byte storage_[sizeof(T)];
};

template <typename T>
struct destructor_gaurd {
  explicit destructor_gaurd(manual_lifetime<T>& obj) noexcept
      : ptr_{std::addressof(obj)} {}

  destructor_gaurd(destructor_gaurd&&) = delete;
  destructor_gaurd& operator=(destructor_gaurd&&) = delete;
  ~destructor_gaurd() {
    if (ptr_ != nullptr) {
      ptr_->destroy();
    }
  }
  void cancel() { ptr_ = nullptr; }

 private:
  manual_lifetime<T>* ptr_;
};

template <typename T>
  requires std::is_trivially_destructible_v<T>
struct destructor_gaurd<T> {
  explicit destructor_gaurd(manual_lifetime<T>& obj) noexcept {}
  void cancel() noexcept {}
};

template <typename T>
destructor_gaurd(manual_lifetime<T>& obj) -> destructor_gaurd<T>;

template <typename Promise, typename... Args>
Promise construct_from([[maybe_unused]] Args&&... args) {
  if constexpr (std::constructible_from<Promise, Args&&...>) {
    return Promise{std::forward<Args>(args)...};
  } else {
    return Promise{};
  }
}

__coroutine_state* __f_resume(__coroutine_state* s) {
  std::cout << "__f_resume\n";
  return const_cast<__coroutine_state*>(&__coroutine_state::__noop_coroutine);
}

void __f_destory(__coroutine_state* s) { std::cout << "__f_destroy\n"; }

// forward declaration
task f(int x) {
  static __coroutine_state_with_promise<task::promise_type> cstate{};
  cstate.resume_ = &__f_resume;
  cstate.destroy_ = &__f_destory;
  cstate.promise_.set_value(x);
  return cstate.promise_.get_return_object();
}

/// Begin lowering of g(int x)
/// task(int x) {
/// int fx = co_await g(x);
/// co_return fx * fx;
/// }

using __g_promise_t = coro::coroutine_traits<task, int>::promise_type;

__coroutine_state* __g_resume(__coroutine_state* s);
void __g_destroy(__coroutine_state* s);

struct __g_state : __coroutine_state_with_promise<__g_promise_t> {
  __g_state(int x) : x_{x} {
    this->resume_ = &__g_resume;
    this->destroy_ = &__g_destroy;
    ::new (static_cast<void*>(std::addressof(this->promise_)))
        __g_promise_t{construct_from<__g_promise_t>(x_)};
  }

  int suspend_point_;
  int x_;
  struct __scope1 {
    manual_lifetime<task> tmp2_;
    manual_lifetime<task::awaiter> tmp3_;
  };

  union {
    manual_lifetime<coro::suspend_always> tmp1_;
    __scope1 s1_;
    manual_lifetime<task::promise_type::final_awaiter> tmp4_;
  };
};

///
/// the "ramp" function

task g(int x) {
  std::unique_ptr<__g_state> state{new __g_state{x}};
  std::cout << "create coroutine state success\n";
  decltype(auto) return_value = state->promise_.get_return_object();
  state->tmp1_.construct_from(
      [&]() -> decltype(auto) { return state->promise_.initial_suspend(); });
  if (!state->tmp1_.get().await_ready()) {
    state->tmp1_.get().await_suspend(
        coro::coroutine_handle<__g_promise_t>::from_promise(state->promise_));
    std::cout << "initial suspend\n";
    state.release();
    // fall trhough to return statement below
  } else {
    // coroutine did not suspend, start executing the body immediately
    __g_resume(state.release());
  }
  return return_value;
}

/// the resume function
__coroutine_state* __g_resume(__coroutine_state* s) {
  auto* state = reinterpret_cast<__g_state*>(s);
  coro::coroutine_handle<void> coro_to_resume;
  try {
    switch (state->suspend_point_) {
      case 0:
        std::cout << "resume: suspend point 0\n";
        goto suspend_point_0;
      case 1:
        std::cout << "resume: suspend point 1\n";
        goto suspend_point_1;
      default:
        std::unreachable();
    }
  suspend_point_0: {
    destructor_gaurd tmp1_dtor{state->tmp1_};
    state->tmp1_.get().await_resume();
  }
    // int fx = co_await f(x);
    {
      state->s1_.tmp2_.construct_from([&]() { return f(state->x_); });
      destructor_gaurd tmp2_dtor{state->s1_.tmp2_};
      state->s1_.tmp3_.construct_from([&]() {
        return static_cast<task&&>(state->s1_.tmp2_.get()).operator co_await();
      });
      destructor_gaurd tmp3_dtor{state->s1_.tmp3_};
      if (!state->s1_.tmp3_.get().await_ready()) {
        state->suspend_point_ = 1;
        auto h = state->s1_.tmp3_.get().await_suspend(
            coro::coroutine_handle<__g_promise_t>::from_promise(
                state->promise_));
        // A coroutine suspends  without existing scopes - so cancel the
        // destructor-gaueds
        tmp2_dtor.cancel();
        tmp3_dtor.cancel();
        return reinterpret_cast<__coroutine_state*>(h.address());
      } else {
        // Don't exit the scope here
        // We can't goto a label that enters the scope of a variable with a
        // non-trivial destructor So we have to exit the scope of the destructor
        // gaurds here without calling the destructors and then recreate them
        // after the suspoint_point_1 label.
        tmp2_dtor.cancel();
        tmp3_dtor.cancel();
      }
    }
  suspend_point_1:
    int fx = [&]() -> decltype(auto) {
      destructor_gaurd tmp2_dtor{state->s1_.tmp2_};
      destructor_gaurd tmp3_dtor{state->s1_.tmp3_};
      return state->s1_.tmp3_.get().await_resume();
    }();
    state->promise_.return_value(fx * fx);
    goto final_suspend;
  } catch (...) {
    state->promise_.unhandled_exception();
    goto final_suspend;
  }
final_suspend:
  std::cout << "resume: final suspend\n";
  // co_await promise.final_suspend()
  {
    state->tmp4_.construct_from(
        [&]() noexcept { return state->promise_.final_suspend(); });
    destructor_gaurd tmp4_dtor{state->tmp4_};
    if (!state->tmp4_.get().await_ready()) {
      state->suspend_point_ = 2;
      state->resume_ = nullptr;
      auto h = state->tmp4_.get().await_suspend(
          coro::coroutine_handle<__g_promise_t>::from_promise(state->promise_));
      tmp4_dtor.cancel();
      return reinterpret_cast<__coroutine_state*>(h.address());
    }
    state->tmp4_.get().await_resume();
  }
  delete state;
  return reinterpret_cast<__coroutine_state*>(coro::noop_coroutine().address());
}

// "The destory" function

void __g_destroy(__coroutine_state* s) {
  auto* state = reinterpret_cast<__g_state*>(s);
  switch (state->suspend_point_) {
    case 0:
      std::cout << "destory: suspend point 0\n";
      goto suspend_point_0;
    case 1:
      std::cout << "destroy: suspend point 1\n";
      goto suspend_point_1;
    case 2:
      std::cout << "destroy: suspend point 2\n";
      goto suspend_point_2;
    default:
      std::unreachable();
  }
suspend_point_0:
  state->tmp1_.destroy();
  goto destory_state;
suspend_point_1:
  state->s1_.tmp2_.destroy();
  state->s1_.tmp3_.destroy();
  goto destory_state;
suspend_point_2:
  state->tmp4_.destroy();
  goto destory_state;
destory_state:
  std::cout << "destory: destroy state\n";
  delete state;
}

int main(int argc, char* argv[]) {
  auto t = g(100);
  t.resume();
  t.resume();

  return 0;
}