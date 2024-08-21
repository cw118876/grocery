#include "function.hpp"


namespace tmp {

 const char* bad_function_call::what() const noexcept {
    return "tmp::bad_function_call";
 }

}  // namespace tmp