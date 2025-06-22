#ifndef BASE64_H
#define BASE64_H

#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>

namespace base64 {

namespace detail {
    const char kEncodeLookup[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const char kPadCharacter = '=';
}

inline std::string encode(const std::vector<uint8_t>& input) {
    std::string encoded;
    encoded.reserve(((input.size() / 3) + (input.size() % 3 > 0)) * 4);

    uint32_t temp = 0;
    auto cursor = input.begin();

    for (size_t i = 0; i < input.size() / 3; ++i) {
        temp  = static_cast<uint32_t>(*cursor++) << 16;
        temp += static_cast<uint32_t>(*cursor++) << 8;
        temp += static_cast<uint32_t>(*cursor++);
        encoded.append(1, detail::kEncodeLookup[(temp & 0x00FC0000) >> 18]);
        encoded.append(1, detail::kEncodeLookup[(temp & 0x0003F000) >> 12]);
        encoded.append(1, detail::kEncodeLookup[(temp & 0x00000FC0) >> 6]);
        encoded.append(1, detail::kEncodeLookup[(temp & 0x0000003F)]);
    }

    switch (input.size() % 3) {
        case 1:
            temp = static_cast<uint32_t>(*cursor++) << 16;
            encoded.append(1, detail::kEncodeLookup[(temp & 0x00FC0000) >> 18]);
            encoded.append(1, detail::kEncodeLookup[(temp & 0x0003F000) >> 12]);
            encoded.append(2, detail::kPadCharacter);
            break;
        case 2:
            temp  = static_cast<uint32_t>(*cursor++) << 16;
            temp += static_cast<uint32_t>(*cursor++) << 8;
            encoded.append(1, detail::kEncodeLookup[(temp & 0x00FC0000) >> 18]);
            encoded.append(1, detail::kEncodeLookup[(temp & 0x0003F000) >> 12]);
            encoded.append(1, detail::kEncodeLookup[(temp & 0x00000FC0) >> 6]);
            encoded.append(1, detail::kPadCharacter);
            break;
    }
    return encoded;
}


inline std::vector<uint8_t> decode(const std::string& input) {
    if (input.length() % 4 != 0) {
        throw std::runtime_error("Invalid base64 length!: " + input);
    }

    std::vector<uint8_t> decoded;
    decoded.reserve(((input.length() / 4) * 3));

    uint32_t temp = 0;
    auto cursor = input.begin();

    while (cursor < input.end()) {
        for (size_t i = 0; i < 4; ++i) {
            temp <<= 6;
            if (*cursor >= 0x41 && *cursor <= 0x5A) temp |= *cursor - 0x41;
            else if (*cursor >= 0x61 && *cursor <= 0x7A) temp |= *cursor - 0x61 + 26;
            else if (*cursor >= 0x30 && *cursor <= 0x39) temp |= *cursor - 0x30 + 52;
            else if (*cursor == '+') temp |= 62;
            else if (*cursor == '/') temp |= 63;
            else if (*cursor == detail::kPadCharacter) {
                switch (input.end() - cursor) {
                    case 1:
                        decoded.push_back(static_cast<uint8_t>((temp >> 16) & 0x000000FF));
                        decoded.push_back(static_cast<uint8_t>((temp >> 8) & 0x000000FF));
                        return decoded;
                    case 2:
                        decoded.push_back(static_cast<uint8_t>((temp >> 10) & 0x000000FF));
                        return decoded;
                    default:
                        throw std::runtime_error("Invalid padding character!");
                }
            } else {
                 throw std::runtime_error("Invalid base64 character!");
            }
            cursor++;
        }
        decoded.push_back(static_cast<uint8_t>((temp >> 16) & 0x000000FF));
        decoded.push_back(static_cast<uint8_t>((temp >> 8) & 0x000000FF));
        decoded.push_back(static_cast<uint8_t>(temp & 0x000000FF));
    }
    return decoded;
}

} // namespace base64

#endif // BASE64_H
