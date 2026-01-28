
#include <system_error>

enum class SuccessCondition { Success = 0 };

namespace std {
template <> struct is_error_condition_enum<SuccessCondition> : true_type {};
} // namespace std

std::error_condition make_error_condition(SuccessCondition);