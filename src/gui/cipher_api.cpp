#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <string>
#include <vector>
#include "../my_cipher.cpp"

#define KRIPTO_API extern "C" __declspec(dllexport)

namespace {

thread_local std::string lastError;

}

KRIPTO_API const char* kc_last_error() {
	return lastError.c_str();
}

KRIPTO_API int kc_process(
	int encrypt,
	const std::uint8_t* in, std::size_t n,
	const std::uint8_t* key,
	int mode,
	const std::uint8_t* iv,
	std::uint8_t* out, std::size_t outCapacity, std::size_t* outLen
) {
	lastError.clear();
	try {
		if (mode < 0 || mode > 4) {
			lastError = "Mode tidak dikenal";
			return -1;
		}
		const std::vector<std::uint8_t> data(in, in + n);
		const kripto::Block128 k = kripto::bytesToBlock(key);
		const kripto::Block128 v = kripto::bytesToBlock(iv);
		const auto cipherMode = static_cast<kripto::CipherMode>(mode);
		const std::vector<std::uint8_t> result = encrypt
			? kripto::encryptBuffer(data, k, cipherMode, v)
			: kripto::decryptBuffer(data, k, cipherMode, v);
		if (result.size() > outCapacity) {
			lastError = "Buffer output terlalu kecil";
			return -1;
		}
		std::copy(result.begin(), result.end(), out);
		*outLen = result.size();
		return 0;
	} catch (const std::exception& e) {
		lastError = e.what();
		return -1;
	}
}
