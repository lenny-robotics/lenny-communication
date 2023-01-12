#pragma once

#include <string>

namespace lenny::communication {

class Utils {
private:  //Purely static class
    Utils() = default;
    ~Utils() = default;

public:
    inline static constexpr int TCP_BUFFER_SIZE = 4096;
    inline static bool TCP_PRINT_MESSAGES = false;
    inline static const std::string TCP_CLIENT_CLOSE_MESSAGE = "TCP_CLIENT_CLOSE";

    inline static constexpr int UDP_BUFFER_SIZE = 0XFFFF;
    inline static bool UDP_PRINT_MESSAGES = false;
};

}  // namespace lenny::communication