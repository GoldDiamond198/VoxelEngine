// src/VulkanApp.h

#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstddef>   // for offsetof
#include <string>    // for std::string
#include <vector>
#include <array>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription binding{};
        binding.binding = 0;
        binding.stride = sizeof(Vertex);
        binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return binding;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attrs{};
        // position
        attrs[0].binding = 0;
        attrs[0].location = 0;
        attrs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attrs[0].offset = offsetof(Vertex, pos);
        // normal
        attrs[1].binding = 0;
        attrs[1].location = 1;
        attrs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attrs[1].offset = offsetof(Vertex, normal);
        // uv
        attrs[2].binding = 0;
        attrs[2].location = 2;
        attrs[2].format = VK_FORMAT_R32G32_SFLOAT;
        attrs[2].offset = offsetof(Vertex, uv);
        return attrs;
    }
};

class VulkanApp {
public:
    void initWindow(int width, int height, const char* title);
    void initVulkan();
    void mainLoop();
    void cleanup();

private:
    GLFWwindow* window = nullptr;

    VkInstance               instance;
    VkPhysicalDevice         physicalDevice = VK_NULL_HANDLE;
    VkDevice                 device;
    VkQueue                  graphicsQueue;
    VkQueue                  presentQueue;
    uint32_t                 graphicsQueueFamilyIndex = 0;
    uint32_t                 presentQueueFamilyIndex = 0;

    VkSurfaceKHR             surface;
    VkSwapchainKHR           swapchain;
    VkFormat                 swapchainImageFormat;
    VkExtent2D               swapchainExtent;
    std::vector<VkImage>     swapchainImages;
    std::vector<VkImageView> swapchainImageViews;

    VkRenderPass                renderPass;
    VkPipelineLayout            pipelineLayout;
    VkPipeline                  graphicsPipeline;
    std::vector<VkFramebuffer>  swapchainFramebuffers;

    VkCommandPool               commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    VkSemaphore                 imageAvailableSemaphore;
    VkSemaphore                 renderFinishedSemaphore;

    // Test‐triangle buffers
    VkBuffer       vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
    VkBuffer       indexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;
    uint32_t       indexCount = 0;

    // Setup steps
    void createInstance();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapchain();
    void createImageViews();
    void createRenderPass();
    void createGraphicsPipeline();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();
    void drawFrame();

    // GPU buffer helpers
    void createBuffer(VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer& buffer,
        VkDeviceMemory& bufferMemory);

    void copyBuffer(VkBuffer src,
        VkBuffer dst,
        VkDeviceSize size);

    // Shader loader helpers
    static std::vector<char> readFile(const std::string& filename);
    static VkShaderModule     createShaderModule(VkDevice device, const std::vector<char>& code);
};
