#include "Message.h"
#include <iomanip>
#include <optional>
#include <sstream>

namespace msg {

namespace {
auto CheckMaxSize(int size) -> void {
  if (size > maxSize) {
    throw std::runtime_error("Message is larger than allowable size");
  }
}

auto CheckMinSize(int size) -> void {
  if (size < 0) {
    throw std::runtime_error("Message is lower than allowable size");
  }
}

auto EncodeHeaderSize(std::size_t size) -> std::string {
  std::ostringstream os;
  // fill with 0s on right if need be
  const auto prev = os.fill('0');
  // setw is not sticky
  os << std::setw(nrDigitsInMsgHeader) << size;
  // reset fill to previous setting
  os.fill(prev);
  return os.str();
}
} // namespace

auto GetIsAliveMsg() -> std::string { return std::string("0000"); }

auto IsThisAPingMessage(int msgSizeInt) -> bool { return msgSizeInt == 0; }

auto CheckHeaderSize(int size) -> void {
  CheckMaxSize(size);
  CheckMinSize(size);
}

auto EncodeHeader(const std::string &message) -> std::string {
  const auto size = message.size();
  CheckHeaderSize(static_cast<int>(size));
  std::ostringstream os;
  os << EncodeHeaderSize(size);
  os << message;
  return os.str();
}

} // namespace msg