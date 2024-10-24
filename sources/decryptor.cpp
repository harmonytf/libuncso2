#include "decryptor.hpp"

#include <assert.h>
#include <cstdint>
#include <stdexcept>
#include <string>

#include "ciphers/aescipher.hpp"
#include "ciphers/basecipher.hpp"
#include "ciphers/blowfishcipher.hpp"
#include "ciphers/descipher.hpp"

namespace uc2
{
std::unique_ptr<IBaseCipher> CreateIndexCipher(int cipher)
{
    switch (cipher)
    {
        case CIPHER_DES:
            return std::make_unique<CDesCipher>();
            break;
        case CIPHER_AES:
            return std::make_unique<CAesCipher>();
            break;
        case CIPHER_BLOWFISH:
            return std::make_unique<CBlowfishCipher>();
            break;
        default:
            throw std::invalid_argument("libuncso2: Invalid cipher used");
    }
}

CDecryptor::CDecryptor(IBaseCipher* cipher, std::string_view key,
                       bool paddingEnabled)
    : m_pCipher(cipher)
{
    this->Initialize(key, {}, paddingEnabled);
}

CDecryptor::CDecryptor(IBaseCipher* cipher, std::string_view key,
                       std::string_view iv /*= {}*/,
                       bool paddingEnabled /*= true*/)
    : m_pCipher(cipher)
{
    this->Initialize(key, iv, paddingEnabled);
}

CDecryptor::CDecryptor(IBaseCipher* cipher,
                       const std::vector<std::uint8_t>& key,
                       const std::vector<std::uint8_t>& iv /*= {}*/,
                       bool paddingEnabled /*= true*/)
    : m_pCipher(cipher)
{
    std::string_view szvKey(reinterpret_cast<const char*>(key.data()),
                            key.size());
    std::string_view szvIV(reinterpret_cast<const char*>(iv.data()), iv.size());
    this->Initialize(szvKey, szvIV, paddingEnabled);
}

void CDecryptor::Initialize(std::string_view key, std::string_view iv,
                            bool paddingEnabled)
{
    if (iv.empty() == true)
    {
        constexpr std::string_view szvNullIv = {
            "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 16
        };

        iv = szvNullIv;
    }

    this->m_pCipher->Initialize(key, iv, paddingEnabled);
}

std::vector<std::uint8_t> CDecryptor::Decrypt(
    std::vector<std::uint8_t>& data) const
{
    return this->Decrypt(data.data(), data.size());
}

std::size_t CDecryptor::Decrypt(const void* pStart, void* pOutBuffer,
                                const std::size_t iLength) const
{
    std::span<const std::uint8_t> inData(
        static_cast<const std::uint8_t*>(pStart), iLength);
    std::span<std::uint8_t> outData(static_cast<std::uint8_t*>(pOutBuffer),
                                    iLength);
    return this->m_pCipher->Decrypt(inData, outData);
}

std::size_t CDecryptor::DecryptInBuffer(void* pBuffer,
                                        const std::size_t iLength) const
{
    std::span<const std::uint8_t> inData(
        static_cast<const std::uint8_t*>(pBuffer), iLength);
    std::span<std::uint8_t> outData(static_cast<std::uint8_t*>(pBuffer),
                                    iLength);
    return this->m_pCipher->Decrypt(inData, outData);
}

std::vector<std::uint8_t> CDecryptor::Decrypt(const void* pStart,
                                              const std::size_t iLength) const
{
    std::vector<std::uint8_t> vOutData(iLength);

    std::span<const std::uint8_t> inData(
        static_cast<const std::uint8_t*>(pStart), iLength);

    this->m_pCipher->Decrypt(inData, vOutData);

    return vOutData;
}
}  // namespace uc2
