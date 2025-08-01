#ifndef HUMANOID_ROBOT_ERROR_CODE_H
#define HUMANOID_ROBOT_ERROR_CODE_H
#include "marco.h"
using namespace humanoid_robot::common;

namespace humanoid_robot
{
    namespace common
    {
        constexpr error_t SUCCESS = 0;

        constexpr error_t INTERFACE_BASE = -100000;
        constexpr error_t INTERFACE_NET_BASE = INTERFACE_BASE - 1000;
        constexpr error_t INTERFACE_SERVICE_BASE = INTERFACE_BASE - 2000;

        constexpr error_t INTERFACE_CONNECT_FAILED = INTERFACE_NET_BASE - 1;
        constexpr error_t INTERFACE_SEND_FAILED = INTERFACE_NET_BASE - 2;

    } // namespace common
}
#endif // HUMANOID_ROBOT_ERROR_CODE_H
