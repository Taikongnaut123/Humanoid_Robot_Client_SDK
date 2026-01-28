#include "robot/common/success_condition.h"

namespace { // anonymous namespace

struct SuccessConditionCategory : public std::error_category {
public:
  const char *name() const noexcept override;
  std::string message(int value) const override;
};

const char *SuccessConditionCategory::name() const noexcept {
  return "SuccessCondition";
}

std::string SuccessConditionCategory::message(int value) const {
  switch (static_cast<SuccessCondition>(value)) {
  case SuccessCondition::Success:
    return "Success";
  default:
    return "Fail";
  }
}

const SuccessConditionCategory SuccessConditionCategory_category{};

} // anonymous namespace

std::error_condition make_error_condition(SuccessCondition value) {
  return {static_cast<int>(value), SuccessConditionCategory_category};
}