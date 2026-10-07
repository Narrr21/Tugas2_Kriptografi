#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace kripto {

std::vector<std::uint8_t> padPKCS7(const std::vector<std::uint8_t>& data);
std::vector<std::uint8_t> unpadPKCS7(const std::vector<std::uint8_t>& data);

} // namespace kripto
