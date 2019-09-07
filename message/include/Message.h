#pragma once
#include <string>

namespace msg {

constexpr int nrDigitsInMsgHeader = 4;
constexpr int32_t maxSize = 9999;

auto CheckHeaderSize(int size) -> void;

auto EncodeHeader(const std::string &message) -> std::string;

} // namespace msg