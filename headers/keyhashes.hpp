#include <cstdint>
#include <array>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace uc2
{
std::vector<std::uint8_t> GeneratePkgIndexKey(
    int iKey, std::string_view szPkgName,
    std::span<const std::uint8_t[4][16]> keyCollectionView);

std::string GeneratePkgFileKey(std::string_view szvPkgName,
                               std::string_view szKey);
}  // namespace uc2