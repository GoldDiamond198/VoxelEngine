// src/VulkanApp.cpp

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "VulkanApp.h"
#include <stdexcept>
#include <iostream>
#include <vector>
#include <set>
#include <fstream>
#include <cstring>
#include <string>   // for std::string in readFile


// Shader helpers
std::vector<char> VulkanApp::readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file) throw std::runtime_error("Failed to open file: " + filename);
    size_t size = (size_t)file.tellg();
    std::vector<char> buffer(size);
    file.seekg(0);
    file.read(buffer.data(), size);
    file.close();
    return buffer;
}

VkShaderModule VulkanApp::createShaderModule(VkDevice device, const std::vector<char>& code) {
    VkShaderModuleCreateInfo info{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    info.codeSize = code.size();
    info.pCode = reinterpret_cast<const uint32_t*>(code.data());
    VkShaderModule module;
    if (vkCreateShaderModule(device, &info, nullptr, &module) != VK_SUCCESS)
        throw std::runtime_error("Failed to create shader module");
    return module;
}

// Queue‐family finder
struct QueueFamilyIndices {
    int graphicsFamily = -1, presentFamily = -1;
    bool isComplete() const { return graphicsFamily >= 0 && presentFamily >= 0; }
};

static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice dev, VkSurfaceKHR surf) {
    QueueFamilyIndices indices;
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &count, nullptr);
    std::vector<VkQueueFamilyProperties> props(count);
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &count, props.data());
    for (uint32_t i = 0; i < props.size(); i++) {
        if (props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.graphicsFamily = i;
        VkBool32 present = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, surf, &present);
        if (present) indices.presentFamily = i;
        if (indices.isComplete()) break;
    }
    return indices;
}

// 1. Instance
void VulkanApp::createInstance() {
    VkApplicationInfo appInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
    appInfo.pApplicationName = "VoxelEngine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo ci{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    ci.pApplicationInfo = &appInfo;
    uint32_t extCount = 0;
    const char** exts = glfwGetRequiredInstanceExtensions(&extCount);
    ci.enabledExtensionCount = extCount;
    ci.ppEnabledExtensionNames = exts;
    ci.enabledLayerCount = 0;

    if (vkCreateInstance(&ci, nullptr, &instance) != VK_SUCCESS)
        throw std::runtime_error("Failed to create Vulkan instance");
}

// 2. Surface
void VulkanApp::createSurface() {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
        throw std::runtime_error("Failed to create window surface");
}

// 3. Pick GPU
void VulkanApp::pickPhysicalDevice() {
    uint32_t devCount = 0;
    vkEnumeratePhysicalDevices(instance, &devCount, nullptr);
    if (devCount == 0) throw std::runtime_error("No Vulkan GPUs found");

    std::vector<VkPhysicalDevice> devices(devCount);
    vkEnumeratePhysicalDevices(instance, &devCount, devices.data());
    for (auto& dev : devices) {
        auto idx = findQueueFamilies(dev, surface);
        if (idx.isComplete()) {
            physicalDevice = dev;
            graphicsQueueFamilyIndex = idx.graphicsFamily;
            presentQueueFamilyIndex = idx.presentFamily;
            break;
        }
    }
    if (physicalDevice == VK_NULL_HANDLE)
        throw std::runtime_error("Failed to find a suitable GPU");
}

// 4. Logical device & queues
void VulkanApp::createLogicalDevice() {
    std::set<uint32_t> qfs = { graphicsQueueFamilyIndex, presentQueueFamilyIndex };
    float prio = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> qis;
    for (auto qf : qfs) {
        VkDeviceQueueCreateInfo qi{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
        qi.queueFamilyIndex = qf; qi.queueCount = 1; qi.pQueuePriorities = &prio;
        qis.push_back(qi);
    }
    VkPhysicalDeviceFeatures feats{};
    VkDeviceCreateInfo di{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    di.queueCreateInfoCount = (uint32_t)qis.size();
    di.pQueueCreateInfos = qis.data();
    di.pEnabledFeatures = &feats;
    const char* devExts[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    di.enabledExtensionCount = 1;
    di.ppEnabledExtensionNames = devExts;
    di.enabledLayerCount = 0;
    if (vkCreateDevice(physicalDevice, &di, nullptr, &device) != VK_SUCCESS)
        throw std::runtime_error("Failed to create logical device");
    vkGetDeviceQueue(device, graphicsQueueFamilyIndex, 0, &graphicsQueue);
    vkGetDeviceQueue(device, presentQueueFamilyIndex, 0, &presentQueue);
}

// 5. Swapchain
void VulkanApp::createSwapchain() {
    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &caps);
    uint32_t imgCount = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && imgCount > caps.maxImageCount)
        imgCount = caps.maxImageCount;

    uint32_t fmtCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &fmtCount, nullptr);
    std::vector<VkSurfaceFormatKHR> fmts(fmtCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &fmtCount, fmts.data());
    VkSurfaceFormatKHR fmt = fmts[0];
    if (fmtCount == 1 && fmt.format == VK_FORMAT_UNDEFINED) {
        fmt.format = VK_FORMAT_B8G8R8A8_SRGB;
        fmt.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    }

    uint32_t pmCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &pmCount, nullptr);
    std::vector<VkPresentModeKHR> pms(pmCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &pmCount, pms.data());
    VkPresentModeKHR pm = VK_PRESENT_MODE_FIFO_KHR;
    for (auto m : pms) if (m == VK_PRESENT_MODE_MAILBOX_KHR) { pm = m; break; }

    if (caps.currentExtent.width != UINT32_MAX) {
        swapchainExtent = caps.currentExtent;
    }
    else {
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        swapchainExtent = { (uint32_t)w, (uint32_t)h };
    }

    VkSwapchainCreateInfoKHR sci{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
    sci.surface = surface;
    sci.minImageCount = imgCount;
    sci.imageFormat = fmt.format;
    sci.imageColorSpace = fmt.colorSpace;
    sci.imageExtent = swapchainExtent;
    sci.imageArrayLayers = 1;
    sci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    uint32_t qArr[] = { graphicsQueueFamilyIndex, presentQueueFamilyIndex };
    if (graphicsQueueFamilyIndex != presentQueueFamilyIndex) {
        sci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        sci.queueFamilyIndexCount = 2;
        sci.pQueueFamilyIndices = qArr;
    }
    else {
        sci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    sci.preTransform = caps.currentTransform;
    sci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    sci.presentMode = pm;
    sci.clipped = VK_TRUE;
    sci.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &sci, nullptr, &swapchain) != VK_SUCCESS)
        throw std::runtime_error("Failed to create swapchain");

    vkGetSwapchainImagesKHR(device, swapchain, &imgCount, nullptr);
    swapchainImages.resize(imgCount);
    vkGetSwapchainImagesKHR(device, swapchain, &imgCount, swapchainImages.data());
    swapchainImageFormat = fmt.format;
}

// 6. Image Views
void VulkanApp::createImageViews() {
    swapchainImageViews.resize(swapchainImages.size());
    for (size_t i = 0; i < swapchainImages.size(); i++) {
        VkImageViewCreateInfo vi{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        vi.image = swapchainImages[i];
        vi.viewType = VK_IMAGE_VIEW_TYPE_2D;
        vi.format = swapchainImageFormat;
        vi.components = {
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY
        };
        vi.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        if (vkCreateImageView(device, &vi, nullptr, &swapchainImageViews[i]) != VK_SUCCESS)
            throw std::runtime_error("Failed to create image view");
    }
}

// 7. Render pass
void VulkanApp::createRenderPass() {
    VkAttachmentDescription ca{};
    ca.format = swapchainImageFormat;
    ca.samples = VK_SAMPLE_COUNT_1_BIT;
    ca.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    ca.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    ca.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    ca.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    ca.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ca.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference cr{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

    VkSubpassDescription sp{};
    sp.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sp.colorAttachmentCount = 1;
    sp.pColorAttachments = &cr;

    VkRenderPassCreateInfo rpci{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
    rpci.attachmentCount = 1;
    rpci.pAttachments = &ca;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &sp;

    if (vkCreateRenderPass(device, &rpci, nullptr, &renderPass) != VK_SUCCESS)
        throw std::runtime_error("Failed to create render pass");
}

// 8. Graphics pipeline
void VulkanApp::createGraphicsPipeline() {
    auto vertCode = readFile("shaders/vert.spv");
    auto fragCode = readFile("shaders/frag.spv");
    VkShaderModule vertModule = createShaderModule(device, vertCode);
    VkShaderModule fragModule = createShaderModule(device, fragCode);

    VkPipelineShaderStageCreateInfo vertStage{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
    vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertStage.module = vertModule;
    vertStage.pName = "main";
    VkPipelineShaderStageCreateInfo fragStage{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
    fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragStage.module = fragModule;
    fragStage.pName = "main";
    VkPipelineShaderStageCreateInfo stages[] = { vertStage, fragStage };

    auto bindingDesc = Vertex::getBindingDescription();
    auto attributeDesc = Vertex::getAttributeDescriptions();
    VkPipelineVertexInputStateCreateInfo viInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    viInfo.vertexBindingDescriptionCount = 1;
    viInfo.pVertexBindingDescriptions = &bindingDesc;
    viInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDesc.size());
    viInfo.pVertexAttributeDescriptions = attributeDesc.data();

    VkPipelineInputAssemblyStateCreateInfo iaInfo{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    iaInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    iaInfo.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{ 0.0f, 0.0f,
        (float)swapchainExtent.width, (float)swapchainExtent.height,
        0.0f, 1.0f
    };
    VkRect2D scissor{ {0,0}, swapchainExtent };
    VkPipelineViewportStateCreateInfo vpInfo{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
    vpInfo.viewportCount = 1; vpInfo.pViewports = &viewport;
    vpInfo.scissorCount = 1; vpInfo.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rsInfo{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    rsInfo.depthClampEnable = VK_FALSE;
    rsInfo.rasterizerDiscardEnable = VK_FALSE;
    rsInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rsInfo.lineWidth = 1.0f;
    rsInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rsInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rsInfo.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo msInfo{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    msInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState cbAtt{};
    cbAtt.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    cbAtt.blendEnable = VK_FALSE;
    VkPipelineColorBlendStateCreateInfo cbInfo{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
    cbInfo.attachmentCount = 1; cbInfo.pAttachments = &cbAtt;

    VkPipelineLayoutCreateInfo plInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    plInfo.setLayoutCount = 0; plInfo.pushConstantRangeCount = 0;
    if (vkCreatePipelineLayout(device, &plInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create pipeline layout");

    VkGraphicsPipelineCreateInfo gpInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
    gpInfo.stageCount = 2; gpInfo.pStages = stages;
    gpInfo.pVertexInputState = &viInfo;
    gpInfo.pInputAssemblyState = &iaInfo;
    gpInfo.pViewportState = &vpInfo;
    gpInfo.pRasterizationState = &rsInfo;
    gpInfo.pMultisampleState = &msInfo;
    gpInfo.pColorBlendState = &cbInfo;
    gpInfo.layout = pipelineLayout;
    gpInfo.renderPass = renderPass;
    gpInfo.subpass = 0;
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &gpInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
        throw std::runtime_error("Failed to create graphics pipeline");

    vkDestroyShaderModule(device, fragModule, nullptr);
    vkDestroyShaderModule(device, vertModule, nullptr);
}

// 9. Framebuffers
void VulkanApp::createFramebuffers() {
    swapchainFramebuffers.resize(swapchainImageViews.size());
    for (size_t i = 0; i < swapchainImageViews.size(); i++) {
        VkImageView atts[] = { swapchainImageViews[i] };
        VkFramebufferCreateInfo fbci{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        fbci.renderPass = renderPass;
        fbci.attachmentCount = 1;
        fbci.pAttachments = atts;
        fbci.width = swapchainExtent.width;
        fbci.height = swapchainExtent.height;
        fbci.layers = 1;
        if (vkCreateFramebuffer(device, &fbci, nullptr, &swapchainFramebuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("Failed to create framebuffer");
    }
}

// 10. Command pool
void VulkanApp::createCommandPool() {
    VkCommandPoolCreateInfo cpci{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    cpci.queueFamilyIndex = graphicsQueueFamilyIndex;
    cpci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if (vkCreateCommandPool(device, &cpci, nullptr, &commandPool) != VK_SUCCESS)
        throw std::runtime_error("Failed to create command pool");
}

// 11. Command buffers + draw triangle
void VulkanApp::createCommandBuffers() {
    commandBuffers.resize(swapchainFramebuffers.size());
    VkCommandBufferAllocateInfo cbai{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    cbai.commandPool = commandPool;
    cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cbai.commandBufferCount = (uint32_t)commandBuffers.size();
    if (vkAllocateCommandBuffers(device, &cbai, commandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate command buffers");

    for (size_t i = 0; i < commandBuffers.size(); i++) {
        VkCommandBufferBeginInfo bi{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        vkBeginCommandBuffer(commandBuffers[i], &bi);

        VkClearValue clearCol = { {{0.1f,0.1f,0.1f,1.0f}} };
        VkRenderPassBeginInfo rpbi{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
        rpbi.renderPass = renderPass;
        rpbi.framebuffer = swapchainFramebuffers[i];
        rpbi.renderArea.extent = swapchainExtent;
        rpbi.clearValueCount = 1;
        rpbi.pClearValues = &clearCol;

        vkCmdBeginRenderPass(commandBuffers[i], &rpbi, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

        VkBuffer vbs[] = { vertexBuffer };
        VkDeviceSize offs[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vbs, offs);
        vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffers[i], indexCount, 1, 0, 0, 0);
        vkCmdEndRenderPass(commandBuffers[i]);
        vkEndCommandBuffer(commandBuffers[i]);
    }
}

// 12. Synchronization objects
void VulkanApp::createSyncObjects() {
    VkSemaphoreCreateInfo si{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    if (vkCreateSemaphore(device, &si, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(device, &si, nullptr, &renderFinishedSemaphore) != VK_SUCCESS)
        throw std::runtime_error("Failed to create semaphores");
}

// 13. Upload vertex/index data
void VulkanApp::uploadMesh(const std::vector<Vertex>& vertices,
                           const std::vector<uint32_t>& indices) {
    indexCount = static_cast<uint32_t>(indices.size());

    VkDeviceSize vbSize = sizeof(Vertex) * vertices.size();
    VkDeviceSize ibSize = sizeof(uint32_t) * indices.size();

    createBuffer(vbSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
    createBuffer(ibSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

    VkBuffer stagingVB, stagingIB; VkDeviceMemory stagingVBMem, stagingIBMem;
    createBuffer(vbSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingVB, stagingVBMem);
    createBuffer(ibSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingIB, stagingIBMem);

    void* data;
    vkMapMemory(device, stagingVBMem, 0, vbSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)vbSize);
    vkUnmapMemory(device, stagingVBMem);
    vkMapMemory(device, stagingIBMem, 0, ibSize, 0, &data);
    memcpy(data, indices.data(), (size_t)ibSize);
    vkUnmapMemory(device, stagingIBMem);

    copyBuffer(stagingVB, vertexBuffer, vbSize);
    copyBuffer(stagingIB, indexBuffer, ibSize);

    vkDestroyBuffer(device, stagingVB, nullptr);
    vkFreeMemory(device, stagingVBMem, nullptr);
    vkDestroyBuffer(device, stagingIB, nullptr);
    vkFreeMemory(device, stagingIBMem, nullptr);
}

// 14. Draw frame
void VulkanApp::drawFrame() {
    uint32_t imageIndex;
    vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailableSemaphore,
                          VK_NULL_HANDLE, &imageIndex);

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo si{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
    si.waitSemaphoreCount = 1;
    si.pWaitSemaphores = &imageAvailableSemaphore;
    si.pWaitDstStageMask = &waitStage;
    si.commandBufferCount = 1;
    si.pCommandBuffers = &commandBuffers[imageIndex];
    si.signalSemaphoreCount = 1;
    si.pSignalSemaphores = &renderFinishedSemaphore;
    if (vkQueueSubmit(graphicsQueue, 1, &si, VK_NULL_HANDLE) != VK_SUCCESS)
        throw std::runtime_error("Failed to submit draw command buffer");

    VkPresentInfoKHR pi{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    pi.waitSemaphoreCount = 1;
    pi.pWaitSemaphores = &renderFinishedSemaphore;
    pi.swapchainCount = 1;
    pi.pSwapchains = &swapchain;
    pi.pImageIndices = &imageIndex;
    vkQueuePresentKHR(presentQueue, &pi);
    vkQueueWaitIdle(presentQueue);
}

// Helper: create buffer
void VulkanApp::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                             VkMemoryPropertyFlags properties, VkBuffer& buffer,
                             VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bi{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bi.size = size;
    bi.usage = usage;
    bi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(device, &bi, nullptr, &buffer) != VK_SUCCESS)
        throw std::runtime_error("Failed to create buffer");

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(device, buffer, &memReq);
    VkMemoryAllocateInfo mai{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    mai.allocationSize = memReq.size;
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);
    uint32_t type = 0;
    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
        if ((memReq.memoryTypeBits & (1 << i)) &&
            (memProps.memoryTypes[i].propertyFlags & properties) == properties) {
            type = i; break;
        }
    }
    mai.memoryTypeIndex = type;
    if (vkAllocateMemory(device, &mai, nullptr, &bufferMemory) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate buffer memory");
    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

// Helper: copy buffer using a temporary command buffer
void VulkanApp::copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size) {
    VkCommandBufferAllocateInfo ai{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    ai.commandPool = commandPool;
    ai.commandBufferCount = 1;
    VkCommandBuffer cb;
    vkAllocateCommandBuffers(device, &ai, &cb);
    VkCommandBufferBeginInfo bi{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cb, &bi);
    VkBufferCopy copy{ 0,0,size };
    vkCmdCopyBuffer(cb, src, dst, 1, &copy);
    vkEndCommandBuffer(cb);
    VkSubmitInfo si{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
    si.commandBufferCount = 1; si.pCommandBuffers = &cb;
    vkQueueSubmit(graphicsQueue, 1, &si, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);
    vkFreeCommandBuffers(device, commandPool, 1, &cb);
}

// 15. Window and event loop
void VulkanApp::initWindow(int width, int height, const char* title) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
}

void VulkanApp::initVulkan(const std::vector<Vertex>& vertices,
                           const std::vector<uint32_t>& indices) {
    createInstance();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapchain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFramebuffers();
    createCommandPool();
    uploadMesh(vertices, indices);
    createCommandBuffers();
    createSyncObjects();
}

void VulkanApp::mainLoop() {
    float last = static_cast<float>(glfwGetTime());
    while (!glfwWindowShouldClose(window)) {
        float now = static_cast<float>(glfwGetTime());
        float dt = now - last;
        last = now;
        glfwPollEvents();
        if (updateCallback) updateCallback(dt);
        drawFrame();
    }
    vkDeviceWaitIdle(device);
}

void VulkanApp::cleanup() {
    vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
    vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);
    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);
    for (auto fb : swapchainFramebuffers) vkDestroyFramebuffer(device, fb, nullptr);
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);
    for (auto iv : swapchainImageViews) vkDestroyImageView(device, iv, nullptr);
    vkDestroySwapchainKHR(device, swapchain, nullptr);
    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}