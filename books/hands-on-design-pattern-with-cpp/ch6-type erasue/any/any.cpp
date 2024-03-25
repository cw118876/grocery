#include "any.hpp"

namespace mystd {


const char* bad_any_cast::what noexcept  {
    return "bad any cast";
}

} // mystd
