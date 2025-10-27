#include "FeaturesChain.h"


DeviceFeaturesChain::DeviceFeaturesChain()
{
    //rayQueryFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
    //rayQueryFeatures.pNext = nullptr;

    //rayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
    //rayTracingPipelineFeatures.pNext = &rayQueryFeatures;

    //accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
    //accelerationStructureFeatures.pNext = &rayTracingPipelineFeatures;

    //deviceFeatures13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    //deviceFeatures13.pNext = &accelerationStructureFeatures;

    deviceFeatures14.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES;
    deviceFeatures14.pNext = nullptr;
    
    deviceFeatures13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    deviceFeatures13.pNext = &deviceFeatures14;

    deviceFeatures12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    deviceFeatures12.pNext = &deviceFeatures13;

    deviceFeatures11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    deviceFeatures11.pNext = &deviceFeatures12;

    deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    deviceFeatures2.pNext = &deviceFeatures11;
}


VkPhysicalDeviceFeatures2* DeviceFeaturesChain::getChainHead()
{
    return &deviceFeatures2;
}


DeviceFeaturesChain DeviceFeaturesChain::operator&(DeviceFeaturesChain features)
{
    return (*this & features.deviceFeatures2 & features.deviceFeatures11 & features.deviceFeatures12 & features.deviceFeatures13 & features.deviceFeatures14);
}


DeviceFeaturesChain DeviceFeaturesChain::operator|(DeviceFeaturesChain features)
{
    return (*this | features.deviceFeatures2 | features.deviceFeatures11 | features.deviceFeatures12 | features.deviceFeatures13 | features.deviceFeatures14);
}


DeviceFeaturesChain DeviceFeaturesChain::operator&(VkPhysicalDeviceFeatures2 features)
{
    return (*this & features.features);
}


DeviceFeaturesChain DeviceFeaturesChain::operator|(VkPhysicalDeviceFeatures2 features)
{
    return (*this | features.features);
}


DeviceFeaturesChain DeviceFeaturesChain::operator&(VkPhysicalDeviceVulkan11Features features)
{
    DeviceFeaturesChain resultFeaturesChain = *this;
    resultFeaturesChain.deviceFeatures11.storageBuffer16BitAccess &= features.storageBuffer16BitAccess;
    resultFeaturesChain.deviceFeatures11.uniformAndStorageBuffer16BitAccess &= features.uniformAndStorageBuffer16BitAccess;
    resultFeaturesChain.deviceFeatures11.storagePushConstant16 &= features.storagePushConstant16;
    resultFeaturesChain.deviceFeatures11.storageInputOutput16 &= features.storageInputOutput16;
    resultFeaturesChain.deviceFeatures11.multiview &= features.multiview;
    resultFeaturesChain.deviceFeatures11.multiviewGeometryShader &= features.multiviewGeometryShader;
    resultFeaturesChain.deviceFeatures11.multiviewTessellationShader &= features.multiviewTessellationShader;
    resultFeaturesChain.deviceFeatures11.variablePointersStorageBuffer &= features.variablePointersStorageBuffer;
    resultFeaturesChain.deviceFeatures11.variablePointers &= features.variablePointers;
    resultFeaturesChain.deviceFeatures11.protectedMemory &= features.protectedMemory;
    resultFeaturesChain.deviceFeatures11.samplerYcbcrConversion &= features.samplerYcbcrConversion;
    resultFeaturesChain.deviceFeatures11.shaderDrawParameters &= features.shaderDrawParameters;
    return resultFeaturesChain;
}


DeviceFeaturesChain DeviceFeaturesChain::operator&(VkPhysicalDeviceVulkan12Features features)
{
    DeviceFeaturesChain resultFeaturesChain = *this;
    resultFeaturesChain.deviceFeatures12.samplerMirrorClampToEdge &= features.samplerMirrorClampToEdge;
    resultFeaturesChain.deviceFeatures12.drawIndirectCount &= features.drawIndirectCount;
    resultFeaturesChain.deviceFeatures12.storageBuffer8BitAccess &= features.storageBuffer8BitAccess;
    resultFeaturesChain.deviceFeatures12.uniformAndStorageBuffer8BitAccess &= features.uniformAndStorageBuffer8BitAccess;
    resultFeaturesChain.deviceFeatures12.storagePushConstant8 &= features.storagePushConstant8;
    resultFeaturesChain.deviceFeatures12.shaderBufferInt64Atomics &= features.shaderBufferInt64Atomics;
    resultFeaturesChain.deviceFeatures12.shaderSharedInt64Atomics &= features.shaderSharedInt64Atomics;
    resultFeaturesChain.deviceFeatures12.shaderFloat16 &= features.shaderFloat16;
    resultFeaturesChain.deviceFeatures12.shaderInt8 &= features.shaderInt8;
    resultFeaturesChain.deviceFeatures12.descriptorIndexing &= features.descriptorIndexing;
    resultFeaturesChain.deviceFeatures12.shaderInputAttachmentArrayDynamicIndexing &= features.shaderInputAttachmentArrayDynamicIndexing;
    resultFeaturesChain.deviceFeatures12.shaderUniformTexelBufferArrayDynamicIndexing &= features.shaderUniformTexelBufferArrayDynamicIndexing;
    resultFeaturesChain.deviceFeatures12.shaderStorageTexelBufferArrayDynamicIndexing &= features.shaderStorageTexelBufferArrayDynamicIndexing;
    resultFeaturesChain.deviceFeatures12.shaderUniformBufferArrayNonUniformIndexing &= features.shaderUniformBufferArrayNonUniformIndexing;
    resultFeaturesChain.deviceFeatures12.shaderSampledImageArrayNonUniformIndexing &= features.shaderSampledImageArrayNonUniformIndexing;
    resultFeaturesChain.deviceFeatures12.shaderStorageBufferArrayNonUniformIndexing &= features.shaderStorageBufferArrayNonUniformIndexing;
    resultFeaturesChain.deviceFeatures12.shaderStorageImageArrayNonUniformIndexing &= features.shaderStorageImageArrayNonUniformIndexing;
    resultFeaturesChain.deviceFeatures12.shaderInputAttachmentArrayNonUniformIndexing &= features.shaderInputAttachmentArrayNonUniformIndexing;
    resultFeaturesChain.deviceFeatures12.shaderUniformTexelBufferArrayNonUniformIndexing &= features.shaderUniformTexelBufferArrayNonUniformIndexing;
    resultFeaturesChain.deviceFeatures12.shaderStorageTexelBufferArrayNonUniformIndexing &= features.shaderStorageTexelBufferArrayNonUniformIndexing;
    resultFeaturesChain.deviceFeatures12.descriptorBindingUniformBufferUpdateAfterBind &= features.descriptorBindingUniformBufferUpdateAfterBind;
    resultFeaturesChain.deviceFeatures12.descriptorBindingSampledImageUpdateAfterBind &= features.descriptorBindingSampledImageUpdateAfterBind;
    resultFeaturesChain.deviceFeatures12.descriptorBindingStorageImageUpdateAfterBind &= features.descriptorBindingStorageImageUpdateAfterBind;
    resultFeaturesChain.deviceFeatures12.descriptorBindingStorageBufferUpdateAfterBind &= features.descriptorBindingStorageBufferUpdateAfterBind;
    resultFeaturesChain.deviceFeatures12.descriptorBindingUniformTexelBufferUpdateAfterBind &= features.descriptorBindingUniformTexelBufferUpdateAfterBind;
    resultFeaturesChain.deviceFeatures12.descriptorBindingStorageTexelBufferUpdateAfterBind &= features.descriptorBindingStorageTexelBufferUpdateAfterBind;
    resultFeaturesChain.deviceFeatures12.descriptorBindingUpdateUnusedWhilePending &= features.descriptorBindingUpdateUnusedWhilePending;
    resultFeaturesChain.deviceFeatures12.descriptorBindingPartiallyBound &= features.descriptorBindingPartiallyBound;
    resultFeaturesChain.deviceFeatures12.descriptorBindingVariableDescriptorCount &= features.descriptorBindingVariableDescriptorCount;
    resultFeaturesChain.deviceFeatures12.runtimeDescriptorArray &= features.runtimeDescriptorArray;
    resultFeaturesChain.deviceFeatures12.samplerFilterMinmax &= features.samplerFilterMinmax;
    resultFeaturesChain.deviceFeatures12.scalarBlockLayout &= features.scalarBlockLayout;
    resultFeaturesChain.deviceFeatures12.imagelessFramebuffer &= features.imagelessFramebuffer;
    resultFeaturesChain.deviceFeatures12.uniformBufferStandardLayout &= features.uniformBufferStandardLayout;
    resultFeaturesChain.deviceFeatures12.shaderSubgroupExtendedTypes &= features.shaderSubgroupExtendedTypes;
    resultFeaturesChain.deviceFeatures12.separateDepthStencilLayouts &= features.separateDepthStencilLayouts;
    resultFeaturesChain.deviceFeatures12.hostQueryReset &= features.hostQueryReset;
    resultFeaturesChain.deviceFeatures12.timelineSemaphore &= features.timelineSemaphore;
    resultFeaturesChain.deviceFeatures12.bufferDeviceAddress &= features.bufferDeviceAddress;
    resultFeaturesChain.deviceFeatures12.bufferDeviceAddressCaptureReplay &= features.bufferDeviceAddressCaptureReplay;
    resultFeaturesChain.deviceFeatures12.bufferDeviceAddressMultiDevice &= features.bufferDeviceAddressMultiDevice;
    resultFeaturesChain.deviceFeatures12.vulkanMemoryModel &= features.vulkanMemoryModel;
    resultFeaturesChain.deviceFeatures12.vulkanMemoryModelDeviceScope &= features.vulkanMemoryModelDeviceScope;
    resultFeaturesChain.deviceFeatures12.vulkanMemoryModelAvailabilityVisibilityChains &= features.vulkanMemoryModelAvailabilityVisibilityChains;
    resultFeaturesChain.deviceFeatures12.shaderOutputViewportIndex &= features.shaderOutputViewportIndex;
    resultFeaturesChain.deviceFeatures12.shaderOutputLayer &= features.shaderOutputLayer;
    resultFeaturesChain.deviceFeatures12.subgroupBroadcastDynamicId &= features.subgroupBroadcastDynamicId;
    return resultFeaturesChain;
}


DeviceFeaturesChain DeviceFeaturesChain::operator&(VkPhysicalDeviceVulkan13Features features)
{
    DeviceFeaturesChain resultFeaturesChain = *this;
    resultFeaturesChain.deviceFeatures13.robustImageAccess &= features.robustImageAccess;
    resultFeaturesChain.deviceFeatures13.inlineUniformBlock &= features.inlineUniformBlock;
    resultFeaturesChain.deviceFeatures13.descriptorBindingInlineUniformBlockUpdateAfterBind &= features.descriptorBindingInlineUniformBlockUpdateAfterBind;
    resultFeaturesChain.deviceFeatures13.pipelineCreationCacheControl &= features.pipelineCreationCacheControl;
    resultFeaturesChain.deviceFeatures13.privateData &= features.privateData;
    resultFeaturesChain.deviceFeatures13.shaderDemoteToHelperInvocation &= features.shaderDemoteToHelperInvocation;
    resultFeaturesChain.deviceFeatures13.shaderTerminateInvocation &= features.shaderTerminateInvocation;
    resultFeaturesChain.deviceFeatures13.subgroupSizeControl &= features.subgroupSizeControl;
    resultFeaturesChain.deviceFeatures13.computeFullSubgroups &= features.computeFullSubgroups;
    resultFeaturesChain.deviceFeatures13.synchronization2 &= features.synchronization2;
    resultFeaturesChain.deviceFeatures13.textureCompressionASTC_HDR &= features.textureCompressionASTC_HDR;
    resultFeaturesChain.deviceFeatures13.shaderZeroInitializeWorkgroupMemory &= features.shaderZeroInitializeWorkgroupMemory;
    resultFeaturesChain.deviceFeatures13.dynamicRendering &= features.dynamicRendering;
    resultFeaturesChain.deviceFeatures13.shaderIntegerDotProduct &= features.shaderIntegerDotProduct;
    resultFeaturesChain.deviceFeatures13.maintenance4 &= features.maintenance4;
    return resultFeaturesChain;
}


DeviceFeaturesChain DeviceFeaturesChain::operator&(VkPhysicalDeviceVulkan14Features features)
{
    DeviceFeaturesChain resultFeaturesChain = *this;
    resultFeaturesChain.deviceFeatures14.globalPriorityQuery &= features.globalPriorityQuery;
    resultFeaturesChain.deviceFeatures14.shaderSubgroupRotate &= features.shaderSubgroupRotate;
    resultFeaturesChain.deviceFeatures14.shaderSubgroupRotateClustered &= features.shaderSubgroupRotateClustered;
    resultFeaturesChain.deviceFeatures14.shaderFloatControls2 &= features.shaderFloatControls2;
    resultFeaturesChain.deviceFeatures14.shaderExpectAssume &= features.shaderExpectAssume;
    resultFeaturesChain.deviceFeatures14.rectangularLines &= features.rectangularLines;
    resultFeaturesChain.deviceFeatures14.bresenhamLines &= features.bresenhamLines;
    resultFeaturesChain.deviceFeatures14.smoothLines &= features.smoothLines;
    resultFeaturesChain.deviceFeatures14.stippledRectangularLines &= features.stippledRectangularLines;
    resultFeaturesChain.deviceFeatures14.stippledBresenhamLines &= features.stippledBresenhamLines;
    resultFeaturesChain.deviceFeatures14.stippledSmoothLines &= features.stippledSmoothLines;
    resultFeaturesChain.deviceFeatures14.vertexAttributeInstanceRateDivisor &= features.vertexAttributeInstanceRateDivisor;
    resultFeaturesChain.deviceFeatures14.vertexAttributeInstanceRateZeroDivisor &= features.vertexAttributeInstanceRateZeroDivisor;
    resultFeaturesChain.deviceFeatures14.indexTypeUint8 &= features.indexTypeUint8;
    resultFeaturesChain.deviceFeatures14.dynamicRenderingLocalRead &= features.dynamicRenderingLocalRead;
    resultFeaturesChain.deviceFeatures14.maintenance5 &= features.maintenance5;
    resultFeaturesChain.deviceFeatures14.maintenance6 &= features.maintenance6;
    resultFeaturesChain.deviceFeatures14.pipelineProtectedAccess &= features.pipelineProtectedAccess;
    resultFeaturesChain.deviceFeatures14.pipelineRobustness &= features.pipelineRobustness;
    resultFeaturesChain.deviceFeatures14.hostImageCopy &= features.hostImageCopy;
    resultFeaturesChain.deviceFeatures14.pushDescriptor &= features.pushDescriptor;
    return resultFeaturesChain;
}


DeviceFeaturesChain DeviceFeaturesChain::operator&(VkPhysicalDeviceFeatures features)
{
    DeviceFeaturesChain resultFeaturesChain = *this;
    resultFeaturesChain.deviceFeatures2.features.robustBufferAccess &= features.robustBufferAccess;
    resultFeaturesChain.deviceFeatures2.features.fullDrawIndexUint32 &= features.fullDrawIndexUint32;
    resultFeaturesChain.deviceFeatures2.features.imageCubeArray &= features.imageCubeArray;
    resultFeaturesChain.deviceFeatures2.features.independentBlend &= features.independentBlend;
    resultFeaturesChain.deviceFeatures2.features.geometryShader &= features.geometryShader;
    resultFeaturesChain.deviceFeatures2.features.tessellationShader &= features.tessellationShader;
    resultFeaturesChain.deviceFeatures2.features.sampleRateShading &= features.sampleRateShading;
    resultFeaturesChain.deviceFeatures2.features.dualSrcBlend &= features.dualSrcBlend;
    resultFeaturesChain.deviceFeatures2.features.logicOp &= features.logicOp;
    resultFeaturesChain.deviceFeatures2.features.multiDrawIndirect &= features.multiDrawIndirect;
    resultFeaturesChain.deviceFeatures2.features.drawIndirectFirstInstance &= features.drawIndirectFirstInstance;
    resultFeaturesChain.deviceFeatures2.features.depthClamp &= features.depthClamp;
    resultFeaturesChain.deviceFeatures2.features.depthBiasClamp &= features.depthBiasClamp;
    resultFeaturesChain.deviceFeatures2.features.fillModeNonSolid &= features.fillModeNonSolid;
    resultFeaturesChain.deviceFeatures2.features.depthBounds &= features.depthBounds;
    resultFeaturesChain.deviceFeatures2.features.wideLines &= features.wideLines;
    resultFeaturesChain.deviceFeatures2.features.largePoints &= features.largePoints;
    resultFeaturesChain.deviceFeatures2.features.alphaToOne &= features.alphaToOne;
    resultFeaturesChain.deviceFeatures2.features.multiViewport &= features.multiViewport;
    resultFeaturesChain.deviceFeatures2.features.samplerAnisotropy &= features.samplerAnisotropy;
    resultFeaturesChain.deviceFeatures2.features.textureCompressionETC2 &= features.textureCompressionETC2;
    resultFeaturesChain.deviceFeatures2.features.textureCompressionASTC_LDR &= features.textureCompressionASTC_LDR;
    resultFeaturesChain.deviceFeatures2.features.textureCompressionBC &= features.textureCompressionBC;
    resultFeaturesChain.deviceFeatures2.features.occlusionQueryPrecise &= features.occlusionQueryPrecise;
    resultFeaturesChain.deviceFeatures2.features.pipelineStatisticsQuery &= features.pipelineStatisticsQuery;
    resultFeaturesChain.deviceFeatures2.features.vertexPipelineStoresAndAtomics &= features.vertexPipelineStoresAndAtomics;
    resultFeaturesChain.deviceFeatures2.features.fragmentStoresAndAtomics &= features.fragmentStoresAndAtomics;
    resultFeaturesChain.deviceFeatures2.features.shaderTessellationAndGeometryPointSize &= features.shaderTessellationAndGeometryPointSize;
    resultFeaturesChain.deviceFeatures2.features.shaderImageGatherExtended &= features.shaderImageGatherExtended;
    resultFeaturesChain.deviceFeatures2.features.shaderStorageImageExtendedFormats &= features.shaderStorageImageExtendedFormats;
    resultFeaturesChain.deviceFeatures2.features.shaderStorageImageMultisample &= features.shaderStorageImageMultisample;
    resultFeaturesChain.deviceFeatures2.features.shaderStorageImageReadWithoutFormat &= features.shaderStorageImageReadWithoutFormat;
    resultFeaturesChain.deviceFeatures2.features.shaderStorageImageWriteWithoutFormat &= features.shaderStorageImageWriteWithoutFormat;
    resultFeaturesChain.deviceFeatures2.features.shaderUniformBufferArrayDynamicIndexing &= features.shaderUniformBufferArrayDynamicIndexing;
    resultFeaturesChain.deviceFeatures2.features.shaderSampledImageArrayDynamicIndexing &= features.shaderSampledImageArrayDynamicIndexing;
    resultFeaturesChain.deviceFeatures2.features.shaderStorageBufferArrayDynamicIndexing &= features.shaderStorageBufferArrayDynamicIndexing;
    resultFeaturesChain.deviceFeatures2.features.shaderStorageImageArrayDynamicIndexing &= features.shaderStorageImageArrayDynamicIndexing;
    resultFeaturesChain.deviceFeatures2.features.shaderClipDistance &= features.shaderClipDistance;
    resultFeaturesChain.deviceFeatures2.features.shaderCullDistance &= features.shaderCullDistance;
    resultFeaturesChain.deviceFeatures2.features.shaderFloat64 &= features.shaderFloat64;
    resultFeaturesChain.deviceFeatures2.features.shaderInt64 &= features.shaderInt64;
    resultFeaturesChain.deviceFeatures2.features.shaderInt16 &= features.shaderInt16;
    resultFeaturesChain.deviceFeatures2.features.shaderResourceResidency &= features.shaderResourceResidency;
    resultFeaturesChain.deviceFeatures2.features.shaderResourceMinLod &= features.shaderResourceMinLod;
    resultFeaturesChain.deviceFeatures2.features.sparseBinding &= features.sparseBinding;
    resultFeaturesChain.deviceFeatures2.features.sparseResidencyBuffer &= features.sparseResidencyBuffer;
    resultFeaturesChain.deviceFeatures2.features.sparseResidencyImage2D &= features.sparseResidencyImage2D;
    resultFeaturesChain.deviceFeatures2.features.sparseResidencyImage3D &= features.sparseResidencyImage3D;
    resultFeaturesChain.deviceFeatures2.features.sparseResidency2Samples &= features.sparseResidency2Samples;
    resultFeaturesChain.deviceFeatures2.features.sparseResidency4Samples &= features.sparseResidency4Samples;
    resultFeaturesChain.deviceFeatures2.features.sparseResidency8Samples &= features.sparseResidency8Samples;
    resultFeaturesChain.deviceFeatures2.features.sparseResidency16Samples &= features.sparseResidency16Samples;
    resultFeaturesChain.deviceFeatures2.features.sparseResidencyAliased &= features.sparseResidencyAliased;
    resultFeaturesChain.deviceFeatures2.features.variableMultisampleRate &= features.variableMultisampleRate;
    resultFeaturesChain.deviceFeatures2.features.inheritedQueries &= features.inheritedQueries;
    return resultFeaturesChain;
}

DeviceFeaturesChain DeviceFeaturesChain::operator&(VkPhysicalDeviceAccelerationStructureFeaturesKHR features)
{
    DeviceFeaturesChain resultFeaturesChain = *this;
    resultFeaturesChain.accelerationStructureFeatures.accelerationStructure &= features.accelerationStructure;
    resultFeaturesChain.accelerationStructureFeatures.accelerationStructureCaptureReplay &= features.accelerationStructureCaptureReplay;
    resultFeaturesChain.accelerationStructureFeatures.accelerationStructureIndirectBuild &= features.accelerationStructureIndirectBuild;
    resultFeaturesChain.accelerationStructureFeatures.accelerationStructureHostCommands &= features.accelerationStructureHostCommands;
    resultFeaturesChain.accelerationStructureFeatures.descriptorBindingAccelerationStructureUpdateAfterBind &= features.descriptorBindingAccelerationStructureUpdateAfterBind;
    return resultFeaturesChain;
}

DeviceFeaturesChain DeviceFeaturesChain::operator&(VkPhysicalDeviceRayTracingPipelineFeaturesKHR features)
{
    DeviceFeaturesChain resultFeaturesChain = *this;
    resultFeaturesChain.rayTracingPipelineFeatures.rayTracingPipeline &= features.rayTracingPipeline;
    resultFeaturesChain.rayTracingPipelineFeatures.rayTracingPipelineShaderGroupHandleCaptureReplay &= features.rayTracingPipelineShaderGroupHandleCaptureReplay;
    resultFeaturesChain.rayTracingPipelineFeatures.rayTracingPipelineShaderGroupHandleCaptureReplayMixed &= features.rayTracingPipelineShaderGroupHandleCaptureReplayMixed;
    resultFeaturesChain.rayTracingPipelineFeatures.rayTracingPipelineTraceRaysIndirect &= features.rayTracingPipelineTraceRaysIndirect;
    resultFeaturesChain.rayTracingPipelineFeatures.rayTraversalPrimitiveCulling &= features.rayTraversalPrimitiveCulling;
    return resultFeaturesChain;
}

DeviceFeaturesChain DeviceFeaturesChain::operator&(VkPhysicalDeviceRayQueryFeaturesKHR features)
{
    DeviceFeaturesChain resultFeaturesChain = *this;
    resultFeaturesChain.rayQueryFeatures.rayQuery &= features.rayQuery;
    return resultFeaturesChain;
}



DeviceFeaturesChain DeviceFeaturesChain::operator|(VkPhysicalDeviceVulkan11Features features)
{
    DeviceFeaturesChain resultFeaturesChain = *this;
    resultFeaturesChain.deviceFeatures11.storageBuffer16BitAccess |= features.storageBuffer16BitAccess;
    resultFeaturesChain.deviceFeatures11.uniformAndStorageBuffer16BitAccess |= features.uniformAndStorageBuffer16BitAccess;
    resultFeaturesChain.deviceFeatures11.storagePushConstant16 |= features.storagePushConstant16;
    resultFeaturesChain.deviceFeatures11.storageInputOutput16 |= features.storageInputOutput16;
    resultFeaturesChain.deviceFeatures11.multiview |= features.multiview;
    resultFeaturesChain.deviceFeatures11.multiviewGeometryShader |= features.multiviewGeometryShader;
    resultFeaturesChain.deviceFeatures11.multiviewTessellationShader |= features.multiviewTessellationShader;
    resultFeaturesChain.deviceFeatures11.variablePointersStorageBuffer |= features.variablePointersStorageBuffer;
    resultFeaturesChain.deviceFeatures11.variablePointers |= features.variablePointers;
    resultFeaturesChain.deviceFeatures11.protectedMemory |= features.protectedMemory;
    resultFeaturesChain.deviceFeatures11.samplerYcbcrConversion |= features.samplerYcbcrConversion;
    resultFeaturesChain.deviceFeatures11.shaderDrawParameters |= features.shaderDrawParameters;
    return resultFeaturesChain;
}


DeviceFeaturesChain DeviceFeaturesChain::operator|(VkPhysicalDeviceVulkan12Features features)
{
    DeviceFeaturesChain resultFeaturesChain = *this;
    resultFeaturesChain.deviceFeatures12.samplerMirrorClampToEdge |= features.samplerMirrorClampToEdge;
    resultFeaturesChain.deviceFeatures12.drawIndirectCount |= features.drawIndirectCount;
    resultFeaturesChain.deviceFeatures12.storageBuffer8BitAccess |= features.storageBuffer8BitAccess;
    resultFeaturesChain.deviceFeatures12.uniformAndStorageBuffer8BitAccess |= features.uniformAndStorageBuffer8BitAccess;
    resultFeaturesChain.deviceFeatures12.storagePushConstant8 |= features.storagePushConstant8;
    resultFeaturesChain.deviceFeatures12.shaderBufferInt64Atomics |= features.shaderBufferInt64Atomics;
    resultFeaturesChain.deviceFeatures12.shaderSharedInt64Atomics |= features.shaderSharedInt64Atomics;
    resultFeaturesChain.deviceFeatures12.shaderFloat16 |= features.shaderFloat16;
    resultFeaturesChain.deviceFeatures12.shaderInt8 |= features.shaderInt8;
    resultFeaturesChain.deviceFeatures12.descriptorIndexing |= features.descriptorIndexing;
    resultFeaturesChain.deviceFeatures12.shaderInputAttachmentArrayDynamicIndexing |= features.shaderInputAttachmentArrayDynamicIndexing;
    resultFeaturesChain.deviceFeatures12.shaderUniformTexelBufferArrayDynamicIndexing |= features.shaderUniformTexelBufferArrayDynamicIndexing;
    resultFeaturesChain.deviceFeatures12.shaderStorageTexelBufferArrayDynamicIndexing |= features.shaderStorageTexelBufferArrayDynamicIndexing;
    resultFeaturesChain.deviceFeatures12.shaderUniformBufferArrayNonUniformIndexing |= features.shaderUniformBufferArrayNonUniformIndexing;
    resultFeaturesChain.deviceFeatures12.shaderSampledImageArrayNonUniformIndexing |= features.shaderSampledImageArrayNonUniformIndexing;
    resultFeaturesChain.deviceFeatures12.shaderStorageBufferArrayNonUniformIndexing |= features.shaderStorageBufferArrayNonUniformIndexing;
    resultFeaturesChain.deviceFeatures12.shaderStorageImageArrayNonUniformIndexing |= features.shaderStorageImageArrayNonUniformIndexing;
    resultFeaturesChain.deviceFeatures12.shaderInputAttachmentArrayNonUniformIndexing |= features.shaderInputAttachmentArrayNonUniformIndexing;
    resultFeaturesChain.deviceFeatures12.shaderUniformTexelBufferArrayNonUniformIndexing |= features.shaderUniformTexelBufferArrayNonUniformIndexing;
    resultFeaturesChain.deviceFeatures12.shaderStorageTexelBufferArrayNonUniformIndexing |= features.shaderStorageTexelBufferArrayNonUniformIndexing;
    resultFeaturesChain.deviceFeatures12.descriptorBindingUniformBufferUpdateAfterBind |= features.descriptorBindingUniformBufferUpdateAfterBind;
    resultFeaturesChain.deviceFeatures12.descriptorBindingSampledImageUpdateAfterBind |= features.descriptorBindingSampledImageUpdateAfterBind;
    resultFeaturesChain.deviceFeatures12.descriptorBindingStorageImageUpdateAfterBind |= features.descriptorBindingStorageImageUpdateAfterBind;
    resultFeaturesChain.deviceFeatures12.descriptorBindingStorageBufferUpdateAfterBind |= features.descriptorBindingStorageBufferUpdateAfterBind;
    resultFeaturesChain.deviceFeatures12.descriptorBindingUniformTexelBufferUpdateAfterBind |= features.descriptorBindingUniformTexelBufferUpdateAfterBind;
    resultFeaturesChain.deviceFeatures12.descriptorBindingStorageTexelBufferUpdateAfterBind |= features.descriptorBindingStorageTexelBufferUpdateAfterBind;
    resultFeaturesChain.deviceFeatures12.descriptorBindingUpdateUnusedWhilePending |= features.descriptorBindingUpdateUnusedWhilePending;
    resultFeaturesChain.deviceFeatures12.descriptorBindingPartiallyBound |= features.descriptorBindingPartiallyBound;
    resultFeaturesChain.deviceFeatures12.descriptorBindingVariableDescriptorCount |= features.descriptorBindingVariableDescriptorCount;
    resultFeaturesChain.deviceFeatures12.runtimeDescriptorArray |= features.runtimeDescriptorArray;
    resultFeaturesChain.deviceFeatures12.samplerFilterMinmax |= features.samplerFilterMinmax;
    resultFeaturesChain.deviceFeatures12.scalarBlockLayout |= features.scalarBlockLayout;
    resultFeaturesChain.deviceFeatures12.imagelessFramebuffer |= features.imagelessFramebuffer;
    resultFeaturesChain.deviceFeatures12.uniformBufferStandardLayout |= features.uniformBufferStandardLayout;
    resultFeaturesChain.deviceFeatures12.shaderSubgroupExtendedTypes |= features.shaderSubgroupExtendedTypes;
    resultFeaturesChain.deviceFeatures12.separateDepthStencilLayouts |= features.separateDepthStencilLayouts;
    resultFeaturesChain.deviceFeatures12.hostQueryReset |= features.hostQueryReset;
    resultFeaturesChain.deviceFeatures12.timelineSemaphore |= features.timelineSemaphore;
    resultFeaturesChain.deviceFeatures12.bufferDeviceAddress |= features.bufferDeviceAddress;
    resultFeaturesChain.deviceFeatures12.bufferDeviceAddressCaptureReplay |= features.bufferDeviceAddressCaptureReplay;
    resultFeaturesChain.deviceFeatures12.bufferDeviceAddressMultiDevice |= features.bufferDeviceAddressMultiDevice;
    resultFeaturesChain.deviceFeatures12.vulkanMemoryModel |= features.vulkanMemoryModel;
    resultFeaturesChain.deviceFeatures12.vulkanMemoryModelDeviceScope |= features.vulkanMemoryModelDeviceScope;
    resultFeaturesChain.deviceFeatures12.vulkanMemoryModelAvailabilityVisibilityChains |= features.vulkanMemoryModelAvailabilityVisibilityChains;
    resultFeaturesChain.deviceFeatures12.shaderOutputViewportIndex |= features.shaderOutputViewportIndex;
    resultFeaturesChain.deviceFeatures12.shaderOutputLayer |= features.shaderOutputLayer;
    resultFeaturesChain.deviceFeatures12.subgroupBroadcastDynamicId |= features.subgroupBroadcastDynamicId;
    return resultFeaturesChain;
}


DeviceFeaturesChain DeviceFeaturesChain::operator|(VkPhysicalDeviceVulkan13Features features)
{
    DeviceFeaturesChain resultFeaturesChain = *this;
    resultFeaturesChain.deviceFeatures13.robustImageAccess |= features.robustImageAccess;
    resultFeaturesChain.deviceFeatures13.inlineUniformBlock |= features.inlineUniformBlock;
    resultFeaturesChain.deviceFeatures13.descriptorBindingInlineUniformBlockUpdateAfterBind |= features.descriptorBindingInlineUniformBlockUpdateAfterBind;
    resultFeaturesChain.deviceFeatures13.pipelineCreationCacheControl |= features.pipelineCreationCacheControl;
    resultFeaturesChain.deviceFeatures13.privateData |= features.privateData;
    resultFeaturesChain.deviceFeatures13.shaderDemoteToHelperInvocation |= features.shaderDemoteToHelperInvocation;
    resultFeaturesChain.deviceFeatures13.shaderTerminateInvocation |= features.shaderTerminateInvocation;
    resultFeaturesChain.deviceFeatures13.subgroupSizeControl |= features.subgroupSizeControl;
    resultFeaturesChain.deviceFeatures13.computeFullSubgroups |= features.computeFullSubgroups;
    resultFeaturesChain.deviceFeatures13.synchronization2 |= features.synchronization2;
    resultFeaturesChain.deviceFeatures13.textureCompressionASTC_HDR |= features.textureCompressionASTC_HDR;
    resultFeaturesChain.deviceFeatures13.shaderZeroInitializeWorkgroupMemory |= features.shaderZeroInitializeWorkgroupMemory;
    resultFeaturesChain.deviceFeatures13.dynamicRendering |= features.dynamicRendering;
    resultFeaturesChain.deviceFeatures13.shaderIntegerDotProduct |= features.shaderIntegerDotProduct;
    resultFeaturesChain.deviceFeatures13.maintenance4 |= features.maintenance4;
    return resultFeaturesChain;
}


DeviceFeaturesChain DeviceFeaturesChain::operator|(VkPhysicalDeviceVulkan14Features features)
{
    DeviceFeaturesChain resultFeaturesChain = *this;
    resultFeaturesChain.deviceFeatures14.globalPriorityQuery |= features.globalPriorityQuery;
    resultFeaturesChain.deviceFeatures14.shaderSubgroupRotate |= features.shaderSubgroupRotate;
    resultFeaturesChain.deviceFeatures14.shaderSubgroupRotateClustered |= features.shaderSubgroupRotateClustered;
    resultFeaturesChain.deviceFeatures14.shaderFloatControls2 |= features.shaderFloatControls2;
    resultFeaturesChain.deviceFeatures14.shaderExpectAssume |= features.shaderExpectAssume;
    resultFeaturesChain.deviceFeatures14.rectangularLines |= features.rectangularLines;
    resultFeaturesChain.deviceFeatures14.bresenhamLines |= features.bresenhamLines;
    resultFeaturesChain.deviceFeatures14.smoothLines |= features.smoothLines;
    resultFeaturesChain.deviceFeatures14.stippledRectangularLines |= features.stippledRectangularLines;
    resultFeaturesChain.deviceFeatures14.stippledBresenhamLines |= features.stippledBresenhamLines;
    resultFeaturesChain.deviceFeatures14.stippledSmoothLines |= features.stippledSmoothLines;
    resultFeaturesChain.deviceFeatures14.vertexAttributeInstanceRateDivisor |= features.vertexAttributeInstanceRateDivisor;
    resultFeaturesChain.deviceFeatures14.vertexAttributeInstanceRateZeroDivisor |= features.vertexAttributeInstanceRateZeroDivisor;
    resultFeaturesChain.deviceFeatures14.indexTypeUint8 |= features.indexTypeUint8;
    resultFeaturesChain.deviceFeatures14.dynamicRenderingLocalRead |= features.dynamicRenderingLocalRead;
    resultFeaturesChain.deviceFeatures14.maintenance5 |= features.maintenance5;
    resultFeaturesChain.deviceFeatures14.maintenance6 |= features.maintenance6;
    resultFeaturesChain.deviceFeatures14.pipelineProtectedAccess |= features.pipelineProtectedAccess;
    resultFeaturesChain.deviceFeatures14.pipelineRobustness |= features.pipelineRobustness;
    resultFeaturesChain.deviceFeatures14.hostImageCopy |= features.hostImageCopy;
    resultFeaturesChain.deviceFeatures14.pushDescriptor |= features.pushDescriptor;
    return resultFeaturesChain;
}


DeviceFeaturesChain DeviceFeaturesChain::operator|(VkPhysicalDeviceFeatures features)
{
    DeviceFeaturesChain resultFeaturesChain = *this;
    resultFeaturesChain.deviceFeatures2.features.robustBufferAccess |= features.robustBufferAccess;
    resultFeaturesChain.deviceFeatures2.features.fullDrawIndexUint32 |= features.fullDrawIndexUint32;
    resultFeaturesChain.deviceFeatures2.features.imageCubeArray |= features.imageCubeArray;
    resultFeaturesChain.deviceFeatures2.features.independentBlend |= features.independentBlend;
    resultFeaturesChain.deviceFeatures2.features.geometryShader |= features.geometryShader;
    resultFeaturesChain.deviceFeatures2.features.tessellationShader |= features.tessellationShader;
    resultFeaturesChain.deviceFeatures2.features.sampleRateShading |= features.sampleRateShading;
    resultFeaturesChain.deviceFeatures2.features.dualSrcBlend |= features.dualSrcBlend;
    resultFeaturesChain.deviceFeatures2.features.logicOp |= features.logicOp;
    resultFeaturesChain.deviceFeatures2.features.multiDrawIndirect |= features.multiDrawIndirect;
    resultFeaturesChain.deviceFeatures2.features.drawIndirectFirstInstance |= features.drawIndirectFirstInstance;
    resultFeaturesChain.deviceFeatures2.features.depthClamp |= features.depthClamp;
    resultFeaturesChain.deviceFeatures2.features.depthBiasClamp |= features.depthBiasClamp;
    resultFeaturesChain.deviceFeatures2.features.fillModeNonSolid |= features.fillModeNonSolid;
    resultFeaturesChain.deviceFeatures2.features.depthBounds |= features.depthBounds;
    resultFeaturesChain.deviceFeatures2.features.wideLines |= features.wideLines;
    resultFeaturesChain.deviceFeatures2.features.largePoints |= features.largePoints;
    resultFeaturesChain.deviceFeatures2.features.alphaToOne |= features.alphaToOne;
    resultFeaturesChain.deviceFeatures2.features.multiViewport |= features.multiViewport;
    resultFeaturesChain.deviceFeatures2.features.samplerAnisotropy |= features.samplerAnisotropy;
    resultFeaturesChain.deviceFeatures2.features.textureCompressionETC2 |= features.textureCompressionETC2;
    resultFeaturesChain.deviceFeatures2.features.textureCompressionASTC_LDR |= features.textureCompressionASTC_LDR;
    resultFeaturesChain.deviceFeatures2.features.textureCompressionBC |= features.textureCompressionBC;
    resultFeaturesChain.deviceFeatures2.features.occlusionQueryPrecise |= features.occlusionQueryPrecise;
    resultFeaturesChain.deviceFeatures2.features.pipelineStatisticsQuery |= features.pipelineStatisticsQuery;
    resultFeaturesChain.deviceFeatures2.features.vertexPipelineStoresAndAtomics |= features.vertexPipelineStoresAndAtomics;
    resultFeaturesChain.deviceFeatures2.features.fragmentStoresAndAtomics |= features.fragmentStoresAndAtomics;
    resultFeaturesChain.deviceFeatures2.features.shaderTessellationAndGeometryPointSize |= features.shaderTessellationAndGeometryPointSize;
    resultFeaturesChain.deviceFeatures2.features.shaderImageGatherExtended |= features.shaderImageGatherExtended;
    resultFeaturesChain.deviceFeatures2.features.shaderStorageImageExtendedFormats |= features.shaderStorageImageExtendedFormats;
    resultFeaturesChain.deviceFeatures2.features.shaderStorageImageMultisample |= features.shaderStorageImageMultisample;
    resultFeaturesChain.deviceFeatures2.features.shaderStorageImageReadWithoutFormat |= features.shaderStorageImageReadWithoutFormat;
    resultFeaturesChain.deviceFeatures2.features.shaderStorageImageWriteWithoutFormat |= features.shaderStorageImageWriteWithoutFormat;
    resultFeaturesChain.deviceFeatures2.features.shaderUniformBufferArrayDynamicIndexing |= features.shaderUniformBufferArrayDynamicIndexing;
    resultFeaturesChain.deviceFeatures2.features.shaderSampledImageArrayDynamicIndexing |= features.shaderSampledImageArrayDynamicIndexing;
    resultFeaturesChain.deviceFeatures2.features.shaderStorageBufferArrayDynamicIndexing |= features.shaderStorageBufferArrayDynamicIndexing;
    resultFeaturesChain.deviceFeatures2.features.shaderStorageImageArrayDynamicIndexing |= features.shaderStorageImageArrayDynamicIndexing;
    resultFeaturesChain.deviceFeatures2.features.shaderClipDistance |= features.shaderClipDistance;
    resultFeaturesChain.deviceFeatures2.features.shaderCullDistance |= features.shaderCullDistance;
    resultFeaturesChain.deviceFeatures2.features.shaderFloat64 |= features.shaderFloat64;
    resultFeaturesChain.deviceFeatures2.features.shaderInt64 |= features.shaderInt64;
    resultFeaturesChain.deviceFeatures2.features.shaderInt16 |= features.shaderInt16;
    resultFeaturesChain.deviceFeatures2.features.shaderResourceResidency |= features.shaderResourceResidency;
    resultFeaturesChain.deviceFeatures2.features.shaderResourceMinLod |= features.shaderResourceMinLod;
    resultFeaturesChain.deviceFeatures2.features.sparseBinding |= features.sparseBinding;
    resultFeaturesChain.deviceFeatures2.features.sparseResidencyBuffer |= features.sparseResidencyBuffer;
    resultFeaturesChain.deviceFeatures2.features.sparseResidencyImage2D |= features.sparseResidencyImage2D;
    resultFeaturesChain.deviceFeatures2.features.sparseResidencyImage3D |= features.sparseResidencyImage3D;
    resultFeaturesChain.deviceFeatures2.features.sparseResidency2Samples |= features.sparseResidency2Samples;
    resultFeaturesChain.deviceFeatures2.features.sparseResidency4Samples |= features.sparseResidency4Samples;
    resultFeaturesChain.deviceFeatures2.features.sparseResidency8Samples |= features.sparseResidency8Samples;
    resultFeaturesChain.deviceFeatures2.features.sparseResidency16Samples |= features.sparseResidency16Samples;
    resultFeaturesChain.deviceFeatures2.features.sparseResidencyAliased |= features.sparseResidencyAliased;
    resultFeaturesChain.deviceFeatures2.features.variableMultisampleRate |= features.variableMultisampleRate;
    resultFeaturesChain.deviceFeatures2.features.inheritedQueries |= features.inheritedQueries;
    return resultFeaturesChain;
}


DeviceFeaturesChain DeviceFeaturesChain::operator|(VkPhysicalDeviceAccelerationStructureFeaturesKHR features)
{
    DeviceFeaturesChain resultFeaturesChain = *this;
    resultFeaturesChain.accelerationStructureFeatures.accelerationStructure |= features.accelerationStructure;
    resultFeaturesChain.accelerationStructureFeatures.accelerationStructureCaptureReplay|= features.accelerationStructureCaptureReplay;
    resultFeaturesChain.accelerationStructureFeatures.accelerationStructureIndirectBuild |= features.accelerationStructureIndirectBuild;
    resultFeaturesChain.accelerationStructureFeatures.accelerationStructureHostCommands |= features.accelerationStructureHostCommands;
    resultFeaturesChain.accelerationStructureFeatures.descriptorBindingAccelerationStructureUpdateAfterBind |= features.descriptorBindingAccelerationStructureUpdateAfterBind;
    return resultFeaturesChain;
}

DeviceFeaturesChain DeviceFeaturesChain::operator|(VkPhysicalDeviceRayTracingPipelineFeaturesKHR features)
{
    DeviceFeaturesChain resultFeaturesChain = *this;
    resultFeaturesChain.rayTracingPipelineFeatures.rayTracingPipeline |= features.rayTracingPipeline;
    resultFeaturesChain.rayTracingPipelineFeatures.rayTracingPipelineShaderGroupHandleCaptureReplay |= features.rayTracingPipelineShaderGroupHandleCaptureReplay;
    resultFeaturesChain.rayTracingPipelineFeatures.rayTracingPipelineShaderGroupHandleCaptureReplayMixed |= features.rayTracingPipelineShaderGroupHandleCaptureReplayMixed;
    resultFeaturesChain.rayTracingPipelineFeatures.rayTracingPipelineTraceRaysIndirect |= features.rayTracingPipelineTraceRaysIndirect;
    resultFeaturesChain.rayTracingPipelineFeatures.rayTraversalPrimitiveCulling |= features.rayTraversalPrimitiveCulling;
    return resultFeaturesChain;
}

DeviceFeaturesChain DeviceFeaturesChain::operator|(VkPhysicalDeviceRayQueryFeaturesKHR features)
{
    DeviceFeaturesChain resultFeaturesChain = *this;
    resultFeaturesChain.rayQueryFeatures.rayQuery |= features.rayQuery;
    return resultFeaturesChain;
}