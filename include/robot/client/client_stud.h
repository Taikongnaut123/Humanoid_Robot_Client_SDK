#ifndef CLIENT_STUD_H
#define CLIENT_STUD_H

#include <string>
#include "robot/common/status.h"
using namespace humanoid_robot::common;
namespace humanoid_robot
{
    namespace robot
    {
        class ClientStud
        {
        public:
            ClientStud();
            ~ClientStud();

            void Init(const std::string &server_address, int port);

            Status send(const std::string &req, int64_t timeout_ms = 1000);
        };
    }

}

#endif // CLIENT_STUD_H