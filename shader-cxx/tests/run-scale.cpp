// Compile with the pinned Vulkan-Headers include directory and libvulkan.so.1.
#include "reflection.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>
#include <vulkan/vulkan.h>

static void check(VkResult result, const char *operation) {
  if (result != VK_SUCCESS) {
    std::fprintf(stderr, "%s failed: %d\n", operation, result);
    std::exit(1);
  }
}
int main(int argc, char **argv) {
  if (argc != 2 && argc != 4)
    return 1;
  const char *entry = argc == 4 ? argv[2] : "scale";
  const char *scenario = argc == 4 ? argv[3] : "scale";
  const bool math = std::strcmp(scenario, "math") == 0;
  const bool typedSpecialization =
      std::strcmp(scenario, "specialization_types") == 0;
  const bool operations = std::strcmp(scenario, "operations") == 0;
  const bool vectorMath = std::strcmp(scenario, "vector_matrix") == 0;
  const bool specialization = std::strncmp(scenario, "specialization", 14) == 0;
  const bool pointers = std::strcmp(scenario, "pointers") == 0;
  unsigned auditSize = 4;
  if (const char *text = std::getenv("SHADER_TEST_EXPECTED"))
    auditSize = 1 + std::count(text, text + std::strlen(text), ',');
  std::vector<float> input(operations   ? 35
                           : vectorMath ? 78
                           : math       ? 14
                           : pointers   ? 8
                                        : auditSize);
  for (unsigned i = 0; i < input.size(); ++i)
    input[i] = static_cast<float>(i + 1);
  if (math)
    input[0] = 4;
  std::vector<float> expected = input;
  if (typedSpecialization) {
    expected = {8,
                -1,
                255,
                -1234,
                60000,
                static_cast<float>(-9000000000LL),
                static_cast<float>(0xffffffffffffffffULL),
                1.25f,
                1.5f};
  } else if (operations) {
    const uint32_t values[35] = {4, 4, 6, 4, 2, 2, 2, 2, 2, 0, 1, 1,
                                 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                 0, 0, 0, 0, 0, 0, 1, 1, 0, 7, 7};
    std::memcpy(expected.data(), values, sizeof(values));
  } else if (vectorMath) {
    expected = {32,
                std::sqrt(14.0f),
                std::sqrt(27.0f),
                1 / std::sqrt(14.0f),
                -3,
                -1,
                -1,
                -1,
                5,
                -3,
                4,
                .25f,
                1,
                4,
                1,
                2.5f,
                1,
                1,
                1,
                1,
                std::sin(1.0f),
                std::cos(1.0f),
                std::exp(1.0f),
                0,
                1,
                1 / std::sqrt(14.0f),
                1,
                7,
                3,
                -2,
                -2,
                58,
                154,
                1,
                1,
                1,
                6,
                10,
                3,
                5,
                6,
                4,
                2,
                7,
                3,
                4,
                2,
                2,
                2,
                3,
                0,
                1,
                1,
                0,
                8,
                -2,
                7,
                1.5f,
                -2,
                1,
                2,
                -1,
                -3,
                -2,
                -2,
                1,
                5 / std::sqrt(155.0f),
                -3,
                10,
                2,
                -.5f,
                4,
                9,
                12,
                9,
                7,
                18,
                2};
  } else if (specialization) {
    expected = {8, 4, -7, static_cast<float>(9000000000ULL)};
    if (std::strcmp(scenario, "specialization_override") == 0) {
      expected[0] = 21;
      expected[1] = 7;
    } else if (std::strcmp(scenario, "specialization_off") == 0) {
      expected[0] = -1;
      expected[1] = 7;
    }
  } else if (math) {
    const float x = input[0], y = input[1];
    expected = {std::sqrt(x),    std::sin(x),         std::cos(x),
                std::exp(x),     std::exp2(x),        std::log(x),
                std::log2(x),    std::fabs(x),        std::floor(x),
                std::ceil(x),    std::trunc(x),       std::fmin(x, y),
                std::fmax(x, y), std::fma(x, y, 1.0f)};
  } else if (pointers) {
    expected[0] = 6;
    uint32_t firstTag = 12, secondTag = 99;
    std::memcpy(&expected[1], &firstTag, sizeof(firstTag));
    std::memcpy(&expected[3], &secondTag, sizeof(secondTag));
  } else if (std::strcmp(scenario, "invocation_state") == 0) {
    expected = {1, 2, 3, 3};
  } else if (std::strcmp(scenario, "align") == 0) {
    expected[0] *= 2;
    expected[1] *= 2;
  } else if (std::strcmp(scenario, "synchronization") == 0) {
    uint32_t count = 4;
    std::memcpy(&expected[0], &count, sizeof(count));
  } else if (std::strcmp(scenario, "shared") == 0) {
    expected[0] += 1;
  } else if (std::strcmp(scenario, "linked") == 0) {
    expected[0] *= 2;
  } else {
    for (auto &x : expected)
      x *= 2;
  }
  if (const char *text = std::getenv("SHADER_TEST_EXPECTED")) {
    for (unsigned i = 0; i < expected.size(); ++i) {
      char *end = nullptr;
      expected[i] = std::strtof(text, &end);
      if (end == text || (i + 1 < expected.size() && *end != ','))
        return 1;
      text = end + (*end == ',');
    }
  }
  VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};
  app.apiVersion = VK_API_VERSION_1_3;
  VkInstanceCreateInfo ici{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
  ici.pApplicationInfo = &app;
  VkInstance instance;
  check(vkCreateInstance(&ici, nullptr, &instance), "create instance");
  uint32_t count = 0;
  check(vkEnumeratePhysicalDevices(instance, &count, nullptr),
        "enumerate devices");
  if (!count)
    return 77;
  std::vector<VkPhysicalDevice> devices(count);
  check(vkEnumeratePhysicalDevices(instance, &count, devices.data()),
        "enumerate devices");
  VkPhysicalDevice physical = devices[0];
  VkPhysicalDeviceVulkan12Features f12{
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
  VkPhysicalDeviceVulkan13Features f13{
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
  f12.pNext = &f13;
  VkPhysicalDeviceVulkan11Features f11{
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES};
  f11.pNext = &f12;
  VkPhysicalDeviceFeatures2 features{
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
  features.pNext = &f11;
  vkGetPhysicalDeviceFeatures2(physical, &features);
  VkPhysicalDeviceProperties deviceProperties{};
  vkGetPhysicalDeviceProperties(physical, &deviceProperties);
  uint32_t requiredPushBytes = 0;
  if (!checkRequirements(argv[1], features.features, f11, f12, f13,
                         deviceProperties.limits, &requiredPushBytes)) {
    vkDestroyInstance(instance, nullptr);
    return 77;
  }
  const bool sharedMemory = operations ||
                            std::strcmp(scenario, "shared") == 0 ||
                            std::strcmp(scenario, "synchronization") == 0;
  if (((vectorMath || typedSpecialization) &&
       !features.features.shaderFloat64) ||
      (typedSpecialization &&
       (!f12.shaderFloat16 || !features.features.shaderInt16)) ||
      (sharedMemory && !f13.shaderZeroInitializeWorkgroupMemory) ||
      (operations && !f12.vulkanMemoryModelDeviceScope)) {
    std::printf("SKIP: device lacks a feature required by %s\n", entry);
    vkDestroyInstance(instance, nullptr);
    return 77;
  }
  if (operations) {
    VkPhysicalDeviceSubgroupProperties subgroup{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES};
    VkPhysicalDeviceProperties2 properties{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
    properties.pNext = &subgroup;
    vkGetPhysicalDeviceProperties2(physical, &properties);
    const VkSubgroupFeatureFlags required =
        VK_SUBGROUP_FEATURE_BASIC_BIT | VK_SUBGROUP_FEATURE_VOTE_BIT |
        VK_SUBGROUP_FEATURE_ARITHMETIC_BIT | VK_SUBGROUP_FEATURE_BALLOT_BIT |
        VK_SUBGROUP_FEATURE_SHUFFLE_BIT |
        VK_SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT;
    if (!(subgroup.supportedStages & VK_SHADER_STAGE_COMPUTE_BIT) ||
        (subgroup.supportedOperations & required) != required) {
      std::puts("SKIP: device lacks required compute subgroup operations");
      vkDestroyInstance(instance, nullptr);
      return 77;
    }
  }
  vkGetPhysicalDeviceQueueFamilyProperties(physical, &count, nullptr);
  std::vector<VkQueueFamilyProperties> families(count);
  vkGetPhysicalDeviceQueueFamilyProperties(physical, &count, families.data());
  uint32_t family = 0;
  while (family < count &&
         !(families[family].queueFlags & VK_QUEUE_COMPUTE_BIT))
    ++family;
  if (family == count)
    return 77;
  float priority = 1;
  VkDeviceQueueCreateInfo qci{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
  qci.queueFamilyIndex = family;
  qci.queueCount = 1;
  qci.pQueuePriorities = &priority;
  VkPhysicalDeviceVulkan12Features enabled{
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
  VkPhysicalDeviceVulkan13Features enabled13{
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
  enabled13.shaderZeroInitializeWorkgroupMemory =
      f13.shaderZeroInitializeWorkgroupMemory;
  enabled.pNext = &enabled13;
  VkPhysicalDeviceVulkan11Features enabled11{
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES};
  enabled11.pNext = &enabled;
  enabled11.storageBuffer16BitAccess = f11.storageBuffer16BitAccess;
  enabled11.uniformAndStorageBuffer16BitAccess =
      f11.uniformAndStorageBuffer16BitAccess;
  enabled11.storagePushConstant16 = f11.storagePushConstant16;
  enabled11.storageInputOutput16 = f11.storageInputOutput16;
  enabled.vulkanMemoryModelDeviceScope = f12.vulkanMemoryModelDeviceScope;
  enabled.bufferDeviceAddress = VK_TRUE;
  enabled.vulkanMemoryModel = VK_TRUE;
  enabled.shaderInt8 = f12.shaderInt8;
  enabled.storageBuffer8BitAccess = f12.storageBuffer8BitAccess;
  enabled.uniformAndStorageBuffer8BitAccess =
      f12.uniformAndStorageBuffer8BitAccess;
  enabled.storagePushConstant8 = f12.storagePushConstant8;
  enabled.shaderBufferInt64Atomics = f12.shaderBufferInt64Atomics;
  enabled.shaderSharedInt64Atomics = f12.shaderSharedInt64Atomics;
  enabled.shaderFloat16 = f12.shaderFloat16;
  enabled.scalarBlockLayout = VK_TRUE;
  enabled.descriptorIndexing = f12.descriptorIndexing;
  VkPhysicalDeviceFeatures core{};
  core.shaderInt64 = features.features.shaderInt64;
  core.shaderStorageImageReadWithoutFormat =
      features.features.shaderStorageImageReadWithoutFormat;
  core.shaderStorageImageWriteWithoutFormat =
      features.features.shaderStorageImageWriteWithoutFormat;
  core.shaderStorageImageExtendedFormats =
      features.features.shaderStorageImageExtendedFormats;
  core.shaderFloat64 = features.features.shaderFloat64;
  core.shaderInt16 = features.features.shaderInt16;
  VkDeviceCreateInfo dci{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
  dci.pNext = &enabled11;
  dci.pEnabledFeatures = &core;
  dci.queueCreateInfoCount = 1;
  dci.pQueueCreateInfos = &qci;
  VkDevice device;
  check(vkCreateDevice(physical, &dci, nullptr, &device), "create device");
  VkQueue queue;
  vkGetDeviceQueue(device, family, 0, &queue);
  VkBufferCreateInfo bci{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  bci.size = input.size() * sizeof(float);
  bci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
              VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
  VkBuffer buffer;
  check(vkCreateBuffer(device, &bci, nullptr, &buffer), "create buffer");
  VkMemoryRequirements req;
  vkGetBufferMemoryRequirements(device, buffer, &req);
  VkPhysicalDeviceMemoryProperties memory;
  vkGetPhysicalDeviceMemoryProperties(physical, &memory);
  uint32_t memoryType = 0;
  const auto flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  while (memoryType < memory.memoryTypeCount &&
         (!(req.memoryTypeBits & (1u << memoryType)) ||
          (memory.memoryTypes[memoryType].propertyFlags & flags) != flags))
    ++memoryType;
  if (memoryType == memory.memoryTypeCount)
    return 77;
  VkMemoryAllocateFlagsInfo allocationFlags{
      VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO};
  allocationFlags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
  VkMemoryAllocateInfo mai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  mai.pNext = &allocationFlags;
  mai.allocationSize = req.size;
  mai.memoryTypeIndex = memoryType;
  VkDeviceMemory allocation;
  check(vkAllocateMemory(device, &mai, nullptr, &allocation),
        "allocate memory");
  check(vkBindBufferMemory(device, buffer, allocation, 0), "bind buffer");
  void *mapped;
  check(vkMapMemory(device, allocation, 0, req.size, 0, &mapped), "map memory");
  std::memcpy(mapped, input.data(), input.size() * sizeof(float));
  VkBufferDeviceAddressInfo addressInfo{
      VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO};
  addressInfo.buffer = buffer;
  struct Params {
    uint64_t data;
    uint32_t count;
    uint32_t padding;
  };
  static_assert(sizeof(Params) == 16);
  Params params{vkGetBufferDeviceAddress(device, &addressInfo), 4, 0};
  std::vector<uint8_t> pushData(
      std::max<size_t>(sizeof(params), requiredPushBytes));
  std::memcpy(pushData.data(), &params, sizeof(params));
  if (const char *layout = std::getenv("SHADER_TEST_PUSH")) {
    std::istringstream parts(layout);
    std::string part;
    while (std::getline(parts, part, ',')) {
      unsigned offset;
      char kind[16];
      double number;
      if (std::sscanf(part.c_str(), "%u:%15[^:]:%lf", &offset, kind, &number) !=
          3)
        return 1;
      if (std::strcmp(kind, "address") == 0) {
        uint64_t address =
            params.data + sizeof(float) * static_cast<int64_t>(number);
        if (offset + sizeof(address) > pushData.size())
          return 1;
        std::memcpy(pushData.data() + offset, &address, sizeof(address));
      } else if (std::strcmp(kind, "float") == 0) {
        float value = number;
        if (offset + sizeof(value) > pushData.size())
          return 1;
        std::memcpy(pushData.data() + offset, &value, sizeof(value));
      } else {
        uint32_t value = number;
        if (offset + sizeof(value) > pushData.size())
          return 1;
        std::memcpy(pushData.data() + offset, &value, sizeof(value));
      }
    }
  }
  std::ifstream stream(argv[1], std::ios::binary | std::ios::ate);
  if (!stream || stream.tellg() <= 0 || stream.tellg() % 4)
    return 1;
  std::vector<uint32_t> code(static_cast<size_t>(stream.tellg()) / 4);
  stream.seekg(0);
  stream.read(reinterpret_cast<char *>(code.data()), code.size() * 4);
  VkShaderModuleCreateInfo smci{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
  smci.codeSize = code.size() * 4;
  smci.pCode = code.data();
  VkShaderModule module;
  check(vkCreateShaderModule(device, &smci, nullptr, &module),
        "create shader module");
  VkPushConstantRange range{VK_SHADER_STAGE_COMPUTE_BIT, 0,
                            static_cast<uint32_t>(pushData.size())};
  VkPipelineLayoutCreateInfo plci{
      VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
  plci.pushConstantRangeCount = 1;
  plci.pPushConstantRanges = &range;
  VkPipelineLayout layout;
  check(vkCreatePipelineLayout(device, &plci, nullptr, &layout),
        "create pipeline layout");
  VkComputePipelineCreateInfo cpci{
      VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
  cpci.layout = layout;
  cpci.stage = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
  cpci.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
  cpci.stage.module = module;
  cpci.stage.pName = entry;
  struct Overrides {
    unsigned count;
    float scale;
    VkBool32 gate;
  } overrides{7, 3.0f, std::strcmp(scenario, "specialization_off") != 0};
  const VkSpecializationMapEntry entries[] = {{0, 0, 4}, {1, 4, 4}, {2, 8, 4}};
  VkSpecializationInfo specializationInfo{3, entries, sizeof(overrides),
                                          &overrides};
  if (std::strcmp(scenario, "specialization_override") == 0 ||
      std::strcmp(scenario, "specialization_off") == 0)
    cpci.stage.pSpecializationInfo = &specializationInfo;
  VkPipeline pipeline;
  check(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &cpci, nullptr,
                                 &pipeline),
        "create compute pipeline");
  VkCommandPoolCreateInfo poolInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
  poolInfo.queueFamilyIndex = family;
  VkCommandPool pool;
  check(vkCreateCommandPool(device, &poolInfo, nullptr, &pool),
        "create command pool");
  VkCommandBufferAllocateInfo cai{
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
  cai.commandPool = pool;
  cai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  cai.commandBufferCount = 1;
  VkCommandBuffer command;
  check(vkAllocateCommandBuffers(device, &cai, &command),
        "allocate command buffer");
  VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  check(vkBeginCommandBuffer(command, &begin), "begin command buffer");
  vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
  vkCmdPushConstants(command, layout, VK_SHADER_STAGE_COMPUTE_BIT, 0,
                     pushData.size(), pushData.data());
  vkCmdDispatch(command, 1, 1, 1);
  if (std::strcmp(scenario, "invocation_state") == 0) {
    VkMemoryBarrier again{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
    again.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    again.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    vkCmdPipelineBarrier(command, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, &again, 0,
                         nullptr, 0, nullptr);
    vkCmdDispatch(command, 1, 1, 1);
  }
  VkMemoryBarrier barrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
  barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
  vkCmdPipelineBarrier(command, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                       VK_PIPELINE_STAGE_HOST_BIT, 0, 1, &barrier, 0, nullptr,
                       0, nullptr);
  check(vkEndCommandBuffer(command), "end command buffer");
  VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
  submit.commandBufferCount = 1;
  submit.pCommandBuffers = &command;
  check(vkQueueSubmit(queue, 1, &submit, VK_NULL_HANDLE), "submit");
  check(vkQueueWaitIdle(queue), "wait");
  auto *result = static_cast<float *>(mapped);
  const char *toleranceText = std::getenv("SHADER_TEST_TOLERANCE");
  float tolerance = 0.00001f;
  if (toleranceText) {
    tolerance = std::strtof(toleranceText, nullptr);
    if (!std::isfinite(tolerance) || tolerance <= 0)
      return 1;
  }
  bool pass = true;
  for (unsigned i = 0; i < input.size(); ++i) {
    if (math || vectorMath || specialization || toleranceText)
      pass &= std::isfinite(result[i]) &&
              std::fabs(result[i] - expected[i]) <=
                  tolerance * std::max(1.0f, std::fabs(expected[i]));
    else
      pass &= std::memcmp(&result[i], &expected[i], sizeof(float)) == 0;
    if (!pass)
      std::fprintf(stderr, "%s element %u: got %g, expected %g\n", entry, i,
                   result[i], expected[i]);
  }
  std::printf("%s runtime: %s\n", entry, pass ? "PASS" : "FAIL");
  vkDestroyCommandPool(device, pool, nullptr);
  vkDestroyPipeline(device, pipeline, nullptr);
  vkDestroyPipelineLayout(device, layout, nullptr);
  vkDestroyShaderModule(device, module, nullptr);
  vkUnmapMemory(device, allocation);
  vkDestroyBuffer(device, buffer, nullptr);
  vkFreeMemory(device, allocation, nullptr);
  vkDestroyDevice(device, nullptr);
  vkDestroyInstance(instance, nullptr);
  return pass ? 0 : 1;
}
