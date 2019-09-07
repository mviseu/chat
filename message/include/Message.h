#pragma once
#include <string>

namespace client {
constexpr int nrDigitsInMsgHeader = 4;
constexpr int32_t maxSize = 9999;

auto EncodeHeader(const std::string &message) -> std::string;
} // namespace client
