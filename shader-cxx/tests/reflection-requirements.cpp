// Exercise rejection paths independently of the capabilities of the test GPU.
#include "reflection.h"
#include <vector>

int main(int argc, char **argv) {
  if (argc != 2)
    return 1;
  const std::string path = argv[1];
  VkPhysicalDeviceFeatures core{};
  VkPhysicalDeviceVulkan11Features f11{};
  VkPhysicalDeviceVulkan12Features f12{};
  VkPhysicalDeviceVulkan13Features f13{};
  VkPhysicalDeviceLimits limits{};
  limits.maxPushConstantsSize = 128;
  limits.maxComputeSharedMemorySize = 32768;
  limits.maxComputeWorkGroupInvocations = 256;
  for (auto &dimension : limits.maxComputeWorkGroupSize)
    dimension = 256;
  auto check = [&](const char *json, bool expected, uint32_t expectedPush = 0) {
    std::ofstream(path + ".json") << json;
    uint32_t push = 0;
    bool accepted =
        checkRequirements(path.c_str(), core, f11, f12, f13, limits, &push);
    if (accepted != expected || (accepted && push != expectedPush)) {
      std::fprintf(stderr, "Requirement check failed: %s\n", json);
      std::exit(1);
    }
  };
  check(
      R"({"required_features":[],"required_limits":{"maxPushConstantsSize":128,"maxComputeSharedMemorySize":32768,"maxComputeWorkGroupInvocations":256,"maxComputeWorkGroupSize":[256,1,1]}})",
      true, 128);
  check(
      R"({"required_features":[],"required_limits":{"maxPushConstantsSize":129}})",
      false);
  check(
      R"({"required_features":[],"required_limits":{"maxComputeSharedMemorySize":32769}})",
      false);
  check(
      R"({"required_features":[],"required_limits":{"maxComputeWorkGroupInvocations":257}})",
      false);
  for (unsigned i = 0; i != 3; ++i) {
    std::string json =
        R"({"required_features":[],"required_limits":{"maxComputeWorkGroupSize":[)";
    for (unsigned j = 0; j != 3; ++j)
      json += (j ? "," : "") + std::to_string(j == i ? 257 : 1);
    json += "]}}";
    check(json.c_str(), false);
  }
  const char *formatless =
      R"({"required_features":["shaderStorageImageReadWithoutFormat","shaderStorageImageWriteWithoutFormat"],"required_limits":{}})";
  check(formatless, false);
  core.shaderStorageImageReadWithoutFormat = VK_TRUE;
  check(formatless, false);
  core.shaderStorageImageWriteWithoutFormat = VK_TRUE;
  check(formatless, true);
  const char *storage8 =
      R"({"required_features":["storagePushConstant8"],"required_limits":{}})";
  f12.shaderInt8 =
      VK_TRUE; // Arithmetic support does not imply storage support.
  check(storage8, false);
  f12.storagePushConstant8 = VK_TRUE;
  check(storage8, true);
  const char *storage16 =
      R"({"required_features":["storagePushConstant16"],"required_limits":{}})";
  core.shaderInt16 = VK_TRUE;
  check(storage16, false);
  f11.storagePushConstant16 = VK_TRUE;
  check(storage16, true);
  const char *atomic64 =
      R"({"required_features":["shaderSharedInt64Atomics"],"required_limits":{}})";
  f12.shaderBufferInt64Atomics = VK_TRUE;
  check(atomic64, false);
  f12.shaderSharedInt64Atomics = VK_TRUE;
  check(atomic64, true);
  check(R"({"required_features":["unrecognizedFeature"],"required_limits":{}})",
        false);
  std::puts("PASS: reflected feature and limit acceptance/rejection");
}
