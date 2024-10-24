#pragma once

#include "basecipher.hpp"

namespace uc2
{
class CDesCipher : public IBaseCipher
{
public:
    CDesCipher();
    virtual ~CDesCipher();

    virtual void Initialize(std::string_view key, std::string_view iv,
                            bool paddingEnabled = false);
    virtual std::uint64_t Decrypt(std::span<const std::uint8_t> inData,
                                  std::span<std::uint8_t> outBuffer);
};
}  // namespace uc2
