// Test harness support. Parse the compiler's reflection with LLVM's JSON
// parser.
#ifndef SHADER_CXX_TEST_REFLECTION_H
#define SHADER_CXX_TEST_REFLECTION_H
#include "llvm/Support/Error.h"
#include "llvm/Support/JSON.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <map>
#include <string>
#include <vulkan/vulkan.h>

static bool checkRequirements(const char *path,
                              const VkPhysicalDeviceFeatures &core,
                              const VkPhysicalDeviceVulkan11Features &f11,
                              const VkPhysicalDeviceVulkan12Features &f12,
                              const VkPhysicalDeviceVulkan13Features &f13,
                              const VkPhysicalDeviceLimits &limits,
                              uint32_t *pushBytes = nullptr) {
  std::ifstream stream(std::string(path) + ".json");
  std::string text((std::istreambuf_iterator<char>(stream)), {});
  auto value = llvm::json::parse(text);
  if (!value) {
    std::fprintf(stderr, "Invalid shader reflection: %s\n",
                 llvm::toString(value.takeError()).c_str());
    std::exit(1);
  }
  auto *object = value->getAsObject();
  if (!object || !object->getArray("required_features") ||
      !object->getObject("required_limits")) {
    std::fprintf(stderr, "Shader reflection is missing requirements\n");
    std::exit(1);
  }
  const std::map<std::string, VkBool32> available{
      {"bufferDeviceAddress", f12.bufferDeviceAddress},
      {"shaderDemoteToHelperInvocation", f13.shaderDemoteToHelperInvocation},
      // This profile marker enables the descriptor-indexing family in the
      // compiler. Actual device feature requirements are listed individually.
      {"descriptorIndexing", VK_TRUE},
      {"vulkanMemoryModel", f12.vulkanMemoryModel},
      {"scalarBlockLayout", f12.scalarBlockLayout},
      {"vulkanMemoryModelDeviceScope", f12.vulkanMemoryModelDeviceScope},
      {"shaderInt8", f12.shaderInt8},
      {"shaderInt16", core.shaderInt16},
      {"shaderInt64", core.shaderInt64},
      {"shaderFloat16", f12.shaderFloat16},
      {"shaderFloat64", core.shaderFloat64},
      {"shaderBufferInt64Atomics", f12.shaderBufferInt64Atomics},
      {"shaderSharedInt64Atomics", f12.shaderSharedInt64Atomics},
      {"storageBuffer8BitAccess", f12.storageBuffer8BitAccess},
      {"uniformAndStorageBuffer8BitAccess",
       f12.uniformAndStorageBuffer8BitAccess},
      {"storagePushConstant8", f12.storagePushConstant8},
      {"storageBuffer16BitAccess", f11.storageBuffer16BitAccess},
      {"uniformAndStorageBuffer16BitAccess",
       f11.uniformAndStorageBuffer16BitAccess},
      {"storagePushConstant16", f11.storagePushConstant16},
      {"storageInputOutput16", f11.storageInputOutput16},
      {"shaderStorageImageReadWithoutFormat",
       core.shaderStorageImageReadWithoutFormat},
      {"shaderStorageImageWriteWithoutFormat",
       core.shaderStorageImageWriteWithoutFormat},
      {"shaderStorageImageExtendedFormats",
       core.shaderStorageImageExtendedFormats},
      {"shaderZeroInitializeWorkgroupMemory",
       f13.shaderZeroInitializeWorkgroupMemory},
      {"geometryShader", core.geometryShader},
      {"sampleRateShading", core.sampleRateShading},
      {"shaderDrawParameters", f11.shaderDrawParameters},
      {"multiview", f11.multiview},
      {"runtimeDescriptorArray", f12.runtimeDescriptorArray},
      {"shaderUniformBufferArrayNonUniformIndexing",
       f12.shaderUniformBufferArrayNonUniformIndexing},
      {"shaderSampledImageArrayNonUniformIndexing",
       f12.shaderSampledImageArrayNonUniformIndexing},
      {"shaderStorageBufferArrayNonUniformIndexing",
       f12.shaderStorageBufferArrayNonUniformIndexing},
      {"shaderStorageImageArrayNonUniformIndexing",
       f12.shaderStorageImageArrayNonUniformIndexing},
      {"shaderInputAttachmentArrayNonUniformIndexing",
       f12.shaderInputAttachmentArrayNonUniformIndexing},
      {"shaderUniformTexelBufferArrayNonUniformIndexing",
       f12.shaderUniformTexelBufferArrayNonUniformIndexing},
      {"shaderStorageTexelBufferArrayNonUniformIndexing",
       f12.shaderStorageTexelBufferArrayNonUniformIndexing}};
  for (auto &feature : *object->getArray("required_features")) {
    auto name = feature.getAsString();
    auto found = name ? available.find(name->str()) : available.end();
    if (found == available.end() || !found->second) {
      std::printf("SKIP: device/harness lacks required feature %s\n",
                  name ? name->str().c_str() : "<invalid>");
      return false;
    }
  }
  auto *required = object->getObject("required_limits");
  if (pushBytes)
    *pushBytes = required->getInteger("maxPushConstantsSize").value_or(0);
  for (auto [name, supported] :
       {std::pair{"maxPushConstantsSize", limits.maxPushConstantsSize},
        std::pair{"maxComputeSharedMemorySize",
                  limits.maxComputeSharedMemorySize},
        std::pair{"maxComputeWorkGroupInvocations",
                  limits.maxComputeWorkGroupInvocations}}) {
    if (auto size = required->getInteger(name);
        size && uint64_t(*size) > supported) {
      std::printf("SKIP: required %s=%llu exceeds device limit %u\n", name,
                  static_cast<unsigned long long>(*size), supported);
      return false;
    }
  }
  if (auto *size = required->getArray("maxComputeWorkGroupSize"))
    for (unsigned i = 0; i < size->size(); ++i) {
      auto dimension = (*size)[i].getAsInteger();
      if (i >= 3 || !dimension || *dimension <= 0 ||
          uint64_t(*dimension) > limits.maxComputeWorkGroupSize[i]) {
        std::puts("SKIP: workgroup dimension exceeds device limit");
        return false;
      }
    }
  return true;
}
#endif
