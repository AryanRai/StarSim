#pragma once
#include <string>
#include <cstdint>

class IPlatform {
public:
    virtual ~IPlatform() = default;
    virtual uint64_t getMillis() = 0;
    virtual void log(const std::string& msg) = 0;
};
