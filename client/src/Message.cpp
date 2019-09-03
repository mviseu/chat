#include "Message.h"
#include <iomanip>
#include <optional>
#include <sstream>

namespace client {

namespace {
auto CheckMaxSize(std::size_t size) -> void {
  if (size > maxSize) {
    throw std::runtime_error("Message is larger than allowable size");
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

auto EncodeHeader(const std::string &message) -> std::string {
  const auto size = message.size();
  CheckMaxSize(size);
  std::ostringstream os;
  os << EncodeHeaderSize(size);
  os << message;
  return os.str();
}

} // namespace client