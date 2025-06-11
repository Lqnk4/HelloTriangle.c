#include "cglm/types.h"
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <assert.h>
#include <cglm/cglm.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

#define optional_type(type)                                                    \
    struct {                                                                   \
        bool is_present;                                                       \
        type value;                                                            \
    }

typedef struct {
    optional_type(uint32_t) graphicsFamily;
    optional_type(uint32_t) presentFamily;
} QueueFamilyIndices;

bool isComplete(QueueFamilyIndices indices) {
    return indices.graphicsFamily.is_present &&
           indices.presentFamily.is_present;
}

typedef struct {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR* formats;
    size_t formatCount;
    VkPresentModeKHR* presentModes;
    size_t presentModeCount;
} SwapChainSupportDetails;

struct Vertex {
    vec2 pos;
    vec3 color;
};

static VkVertexInputBindingDescription getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription = {
        .binding = 0,
        .stride = sizeof(struct Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };
    return bindingDescription;
}

static VkVertexInputAttributeDescription getPositionAttributeDescription() {
    VkVertexInputAttributeDescription positionAttributeDescription = {
        .binding = 0,
        .location = 0,
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = offsetof(struct Vertex, pos),
    };
    return positionAttributeDescription;
}

static VkVertexInputAttributeDescription getColorAttributeDescription() {
    VkVertexInputAttributeDescription colorAttributeDescription = {
        .binding = 0,
        .location = 1,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(struct Vertex, color),
    };
    return colorAttributeDescription;
}

#define SWAPCHAIN_LENGTH 64
#define MAX_FRAMES_IN_FLIGHT 2

GLFWwindow* window;
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 800;
// const int MAX_FRAMES_IN_FLIGHT = 2;

const char* const validationLayers[] = {"VK_LAYER_KHRONOS_validation"};

const char* const deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

const struct Vertex vertices[4] = {{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                                   {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
                                   {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
                                   {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}};
const uint16_t indices[6] = {0, 1, 2, 2, 3, 0};

VkInstance instance;
VkSurfaceKHR surface;
VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
VkDevice device;
VkQueue graphicsQueue;
VkQueue presentQueue;
VkSwapchainKHR swapChain;
uint32_t swapChainImageCount;
VkImage swapChainImages[SWAPCHAIN_LENGTH]; // excess arbitrary length
VkImageView swapChainImageViews[SWAPCHAIN_LENGTH];
VkFramebuffer swapChainFrameBuffers[SWAPCHAIN_LENGTH];
VkFormat swapChainImageFormat;
VkExtent2D swapChainExtent;
VkRenderPass renderPass;
VkPipelineLayout pipelineLayout;
VkPipeline graphicsPipeline;
VkCommandPool commandPool;
VkBuffer vertexBuffer;
VkDeviceMemory vertexBufferMemory;
VkBuffer indexBuffer;
VkDeviceMemory indexBufferMemory;
VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];
VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];
uint32_t currentFrame;
bool framebufferResized;

int run();
int initWindow();
static void framebufferResizeCallback(GLFWwindow* window, int width,
                                      int height);
int initVulkan();
int createInstance();
int createSurface();
const char** getRequiredExtensions();
bool checkRequiredGLFWExtensions(const uint32_t glfwExtensionCount,
                                 const char* const* glfwExtensions);
bool checkValidationLayerSupport();
int pickPhysicalDevice();
int rateDeviceSuitability(VkPhysicalDevice device);
bool checkDeviceExtensionSupport(VkPhysicalDevice device);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
VkSurfaceFormatKHR chooseSwapSurfaceFormat(VkSurfaceFormatKHR* availableFormats,
                                           size_t formatCount);
VkPresentModeKHR chooseSwapPresentMode(VkPresentModeKHR* availablePresentModes,
                                       size_t presentModeCount);
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR* capabilities);
uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
int createLogicalDevice();
int createSwapChain();
int recreateSwapChain();
int createImageViews();
int createRenderPass();
int createGraphicsPipeline();
VkShaderModule createShaderModule(const char* code, size_t codeSize);
int createFrameBuffers();
int createCommandPool();
int createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                 VkMemoryPropertyFlags properties, VkBuffer* buffer,
                 VkDeviceMemory* bufferMemory);
int createVertexBuffer();
void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
int createIndexBuffer();
int createCommandBuffers();
int recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
int createSyncObjects();
int mainloop();
int drawFrame();
void cleanup();
static char* readFile(const char* fileName, size_t* fileSize);
int compare_uint32_t(const void* a, const void* b);
uint32_t removeDup(uint32_t arr[], size_t n);

int main() {

    int err = run();
    if (err != 0) {
        return err;
    }

    return 0;
}

int run() {
    if (initWindow() != 0) {
        exit(1);
    }
    if (initVulkan() != 0) {
        exit(1);
    };
    mainloop();
    cleanup();
    return 0;
}

int initWindow() {
    if (glfwInit() == GLFW_FALSE) {
        fprintf(stderr, "WARNING: Failed to initialize libglfw\n");
        return -1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", NULL, NULL);
    glfwSetWindowSizeCallback(window, framebufferResizeCallback);

    return 0;
}

static void framebufferResizeCallback(__attribute__((unused))
                                      GLFWwindow* window,
                                      __attribute__((unused)) int width,
                                      __attribute__((unused)) int height) {
    framebufferResized = true;
}

int initVulkan() {

    if (enableValidationLayers && !checkValidationLayerSupport()) {
        fprintf(stderr,
                "ERROR: validation layers requested, but not available!\n");
        return -1;
    }

    if (createInstance() != 0) {
        fprintf(stderr, "ERROR: failed to create vulkan instance\n");
        return -1;
    }

    if (createSurface() != 0) {
        fprintf(stderr, "ERROR: failed to create vulkan surface\n");
        return -1;
    }

    if (pickPhysicalDevice() != 0) {
        fprintf(stderr, "ERROR: failed to pick physical device\n");
        return -1;
    }

    if (createLogicalDevice() != 0) {
        fprintf(stderr, "ERROR: failed to create logical device\n");
        return -1;
    }

    if (createSwapChain() != 0) {
        fprintf(stderr, "ERROR: failed to create swap chain\n");
        return -1;
    }

    if (createImageViews() != 0) {
        fprintf(stderr, "ERROR: failed to create image views\n");
        return -1;
    }

    if (createRenderPass() != 0) {
        fprintf(stderr, "ERROR: failed to create render pass\n");
        return -1;
    }

    if (createGraphicsPipeline() != 0) {
        fprintf(stderr, "ERROR: failed to create graphics pipeline\n");
        return -1;
    }

    if (createFrameBuffers() != 0) {
        fprintf(stderr, "ERROR: failed to create frame buffers\n");
        return -1;
    }

    if (createCommandPool() != 0) {
        fprintf(stderr, "ERROR: failed to create command pool\n");
        return -1;
    }

    if (createVertexBuffer() != 0) {
        fprintf(stderr, "ERROR: failed to create vertex buffers\n");
        return -1;
    }

    if (createIndexBuffer() != 0) {
        fprintf(stderr, "ERROR: failed to create index buffers\n");
        return -1;
    }

    if (createCommandBuffers() != 0) {
        fprintf(stderr, "ERROR: failed to create command buffer\n");
        return -1;
    }

    if (createSyncObjects() != 0) {
        fprintf(stderr, "ERROR: failed to create sync objects\n");
        return -1;
    }

    return 0;
}

int createInstance() {

    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Hello Triangle",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0,
    };

    uint32_t glfwExtensionCount = 0;
    const char* const* glfwExtensions =
        glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = glfwExtensionCount,
        .ppEnabledExtensionNames = glfwExtensions,
    };
    if (enableValidationLayers) {
        // for(size_t i = 0; i <
        // sizeof(validationLayers)/sizeof(validationLayers[0]); ++i) {
        //     printf("\tValidationLayer: %s\n", validationLayers[i]);
        // }
        createInfo.enabledLayerCount =
            sizeof(validationLayers) / sizeof(validationLayers[0]);
        createInfo.ppEnabledLayerNames = validationLayers;
    }

    if (vkCreateInstance(&createInfo, NULL, &instance) != VK_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to create vulkan instance\n");
        return -1;
    }

    if (!checkRequiredGLFWExtensions(glfwExtensionCount, glfwExtensions)) {
        fprintf(stderr, "ERROR: Failed to find required extensions\n");
        return -1;
    }

    return 0;
}

int createSurface() {
    if (glfwCreateWindowSurface(instance, window, NULL, &surface) !=
        VK_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to create window surface\n");
        return -1;
    }

    return 0;
}

bool checkRequiredGLFWExtensions(const uint32_t glfwExtensionCount,
                                 const char* const* glfwExtensions) {

    // list extensions for gflw and vulkan
    uint32_t vkExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &vkExtensionCount, NULL);
    VkExtensionProperties vkExtensionProperties[vkExtensionCount];
    vkEnumerateInstanceExtensionProperties(NULL, &vkExtensionCount,
                                           vkExtensionProperties);

    // printf("GLFW Required Extension List:\n");
    for (size_t i = 0; i < glfwExtensionCount; ++i) {
        // printf("\t Extension: %s\n", glfwExtensions[i]);
    }

    // printf("Vulkan Extension List:\n");
    for (size_t i = 0; i < vkExtensionCount; ++i) {
        // printf("\t Extension: %s, Version: %d\n",
        //        vkExtensionProperties[i].extensionName,
        //        vkExtensionProperties[i].specVersion);
    }

    // emit warnings if extensions are missing
    for (size_t i = 0; i < glfwExtensionCount; ++i) {
        bool extensionFound = false;
        for (size_t j = 0; j < vkExtensionCount; ++j) {
            if (strcmp(glfwExtensions[i],
                       vkExtensionProperties[j].extensionName) == 0) {
                extensionFound = true;
                break;
            };
        }
        if (!extensionFound) {
            printf("ERROR: required GLFW extension \'%s\' was not found\n",
                   glfwExtensions[i]);
            return false;
        }
    }
    return true;
}

bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    VkLayerProperties availableLayers[layerCount];
    if (!(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers) ==
          VK_SUCCESS)) {
        fprintf(stderr,
                "ERROR: Failed to enumerate instance layer properties\n");
        return false;
    }

    for (size_t i = 0;
         i < (sizeof(validationLayers) / sizeof(validationLayers[0])); ++i) {
        const char* layerName = validationLayers[i];
        bool layerFound = false;
        for (size_t j = 0; j < layerCount; ++j) {
            const VkLayerProperties layerProperties = availableLayers[j];
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) {
            return false;
        }
    }

    return true;
}

int pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
    if (deviceCount == 0) {
        fprintf(stderr, "ERROR: Found 0 physical devices\n");
        return -1;
    }
    VkPhysicalDevice devices[deviceCount];
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

    VkPhysicalDevice bestDevice = VK_NULL_HANDLE;
    int bestDeviceRating = 0;
    for (size_t i = 0; i < deviceCount; ++i) {
        int deviceRating = rateDeviceSuitability(devices[i]);
        if (deviceRating > bestDeviceRating) {
            bestDevice = devices[i];
            bestDeviceRating = deviceRating;
        }
    }

    if (bestDeviceRating <= 0) {
        fprintf(stderr, "ERROR: Failed to find a suitable device\n");
        return -1;
    }

    physicalDevice = bestDevice;

    if (physicalDevice == VK_NULL_HANDLE) {
        fprintf(stderr, "ERROR: Failed to find a suitable device\n");
        return -1;
    }

    return 0;
}

int rateDeviceSuitability(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    int score = 0;

    if (!deviceFeatures.geometryShader) {
        return 0;
    }

    QueueFamilyIndices indices = findQueueFamilies(device);
    if (!isComplete(indices)) {
        return 0;
    }

    if (!checkDeviceExtensionSupport(device)) {
        return 0;
    }

    // only query swap chain support after checking extensions
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
    if (swapChainSupport.formats == NULL ||
        swapChainSupport.presentModes == NULL) {
        return 0;
    }

    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    }

    score += deviceProperties.limits.maxImageDimension2D;

    return score;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);
    VkExtensionProperties availableExtensions[extensionCount];
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount,
                                         availableExtensions);

    size_t deviceExtensionsLength =
        sizeof(deviceExtensions) / sizeof(deviceExtensions[0]);
    for (size_t i = 0; i < deviceExtensionsLength; ++i) {
        bool extensionFound = false;
        for (size_t j = 0; j < extensionCount; ++j) {
            if (strcmp(deviceExtensions[i],
                       availableExtensions[j].extensionName) == 0) {
                extensionFound = true;
                break;
            }
        }
        if (!extensionFound) {
            return false;
        }
    }

    return true;
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices = {
        .graphicsFamily = {false, 0},
        .presentFamily = {false, 0},
    };

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);
    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             queueFamilies);

    for (size_t i = 0; i < queueFamilyCount; ++i) {
        VkQueueFamilyProperties queueFamily = queueFamilies[i];
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily.is_present = true;
            indices.graphicsFamily.value = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface,
                                             &presentSupport);
        if (presentSupport) {
            indices.presentFamily.is_present = true;
            indices.presentFamily.value = i;
        }

        if (isComplete(indices)) {
            break;
        }
    }

    return indices;
}

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                              &surfaceCapabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, NULL);
    VkSurfaceFormatKHR surfaceFormats[formatCount];
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
                                         surfaceFormats);

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
                                              &presentModeCount, NULL);
    VkPresentModeKHR presentModes[presentModeCount];
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
                                              &presentModeCount, presentModes);

    SwapChainSupportDetails details = {
        .capabilities = surfaceCapabilities,
        .formats = surfaceFormats,
        .formatCount = formatCount,
        .presentModes = presentModes,
        .presentModeCount = presentModeCount,
    };

    return details;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(VkSurfaceFormatKHR* availableFormats,
                                           size_t formatCount) {
    for (size_t i = 0; i < formatCount; ++i) {
        VkSurfaceFormatKHR availableFormat = availableFormats[i];
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(VkPresentModeKHR* availablePresentModes,
                                       size_t presentModeCount) {
    for (size_t i = 0; i < presentModeCount; ++i) {
        if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentModes[i];
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR* capabilities) {
    if (capabilities->currentExtent.width != UINT32_MAX) {
        return capabilities->currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {(uint32_t)width, (uint32_t)height};

        if (actualExtent.width < capabilities->minImageExtent.width)
            actualExtent.width = capabilities->minImageExtent.width;
        if (actualExtent.width > capabilities->maxImageExtent.width)
            actualExtent.width = capabilities->maxImageExtent.width;
        if (actualExtent.height < capabilities->minImageExtent.height)
            actualExtent.height = capabilities->minImageExtent.height;
        if (actualExtent.height > capabilities->maxImageExtent.height)
            actualExtent.height = capabilities->maxImageExtent.height;

        return actualExtent;
    }
}

int createLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    assert(indices.graphicsFamily.is_present);
    assert(indices.presentFamily.is_present);

    uint32_t queueFamilies[2] = {indices.graphicsFamily.value,
                                 indices.presentFamily.value};
    uint32_t queueFamiliesLength =
        sizeof(queueFamilies) / sizeof(queueFamilies[0]);

    // remove duplicates
    qsort(queueFamilies, queueFamiliesLength, sizeof(queueFamilies[0]),
          compare_uint32_t);
    uint32_t uniqueQueueFamiliesLength =
        removeDup(queueFamilies, queueFamiliesLength);

    VkDeviceQueueCreateInfo uniqueQueueFamilies[uniqueQueueFamiliesLength] = {};

    float queuePriority = 1.0f;
    for (size_t i = 0; i < uniqueQueueFamiliesLength; ++i) {
        uint32_t queueFamily = queueFamilies[i];

        VkDeviceQueueCreateInfo queueCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = queueFamily,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority,
        };
        uniqueQueueFamilies[i] = queueCreateInfo;
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = uniqueQueueFamilies,
        .queueCreateInfoCount = 1,
        .pEnabledFeatures = &deviceFeatures,
        .enabledExtensionCount =
            sizeof(deviceExtensions) / sizeof(deviceExtensions[0]),
        .ppEnabledExtensionNames = deviceExtensions};

    if (vkCreateDevice(physicalDevice, &createInfo, NULL, &device) !=
        VK_SUCCESS) {
        fprintf(stderr, "ERROR: failed to create logical device\n");
        return -1;
    }

    vkGetDeviceQueue(device, indices.graphicsFamily.value, 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value, 0, &presentQueue);

    return 0;
}

int createSwapChain() {
    SwapChainSupportDetails swapChainSupport =
        querySwapChainSupport(physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(
        swapChainSupport.formats, swapChainSupport.formatCount);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(
        swapChainSupport.presentModes, swapChainSupport.presentModeCount);
    VkExtent2D extent = chooseSwapExtent(&swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = swapChainSupport.capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    assert(indices.graphicsFamily.is_present);
    assert(indices.presentFamily.is_present);

    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value,
                                     indices.presentFamily.value};

    if (indices.graphicsFamily.value != indices.presentFamily.value) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = NULL;
    }

    if (vkCreateSwapchainKHR(device, &createInfo, NULL, &swapChain) !=
        VK_SUCCESS) {
        fprintf(stderr, "ERROR: failed to create swapchain\n");
        return -1;
    }

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;

    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, NULL);
    swapChainImageCount = imageCount;
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages);

    return 0;
}

void cleanupSwapChain() {
    for (size_t i = 0; i < swapChainImageCount; ++i) {
        vkDestroyFramebuffer(device, swapChainFrameBuffers[i], NULL);
        vkDestroyImageView(device, swapChainImageViews[i], NULL);
    }
    vkDestroySwapchainKHR(device, swapChain, NULL);
}

int recreateSwapChain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(device);
    cleanupSwapChain();
    createSwapChain();
    createImageViews();
    createFrameBuffers();
    return 0;
}

int createImageViews() {

    for (size_t i = 0; i < swapChainImageCount; ++i) {
        VkImageViewCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = swapChainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = swapChainImageFormat,
            .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
            .subresourceRange =
                {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
        };

        if (vkCreateImageView(device, &createInfo, NULL,
                              &swapChainImageViews[i]) != VK_SUCCESS) {
            fprintf(stderr, "ERROR: failed to create image view %lu\n", i);
            return -1;
        }
    }

    return 0;
}

int createRenderPass() {
    VkAttachmentDescription colorAttachment = {
        .format = swapChainImageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkAttachmentReference colorAttatchmentRef = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription subpass = {.pipelineBindPoint =
                                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    .colorAttachmentCount = 1,
                                    .pColorAttachments = &colorAttatchmentRef};

    VkSubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    };

    VkRenderPassCreateInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency};

    if (vkCreateRenderPass(device, &renderPassInfo, NULL, &renderPass) !=
        VK_SUCCESS) {
        fprintf(stderr, "ERROR: failed to create render pass\n");
        return -1;
    }

    return 0;
}

int createGraphicsPipeline() {
    size_t vertShaderSize;
    size_t fragShaderSize;
    char* vertShaderCode = readFile("src/shaders/vert.spv", &vertShaderSize);
    char* fragShaderCode = readFile("src/shaders/frag.spv", &fragShaderSize);

    if (vertShaderCode == NULL) {
        fprintf(stderr, "ERROR: failed to read vertex shader\n");
        return -1;
    }
    if (fragShaderCode == NULL) {
        fprintf(stderr, "ERROR: failed to read fragment shader\n");
        return -1;
    }

    VkShaderModule vertShaderModule =
        createShaderModule(vertShaderCode, vertShaderSize);
    VkShaderModule fragShaderModule =
        createShaderModule(fragShaderCode, fragShaderSize);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertShaderModule,
        .pName = "main",
        .pSpecializationInfo = NULL,
    };
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragShaderModule,
        .pName = "main",
        .pSpecializationInfo = NULL,
    };

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                      fragShaderStageInfo};

    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    VkPipelineDynamicStateCreateInfo dynamicState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = sizeof(dynamicStates) / sizeof(dynamicStates[0]),
        .pDynamicStates = dynamicStates,
    };

    VkVertexInputBindingDescription bindingDescriptions[1] = {
        getBindingDescription()};
    VkVertexInputAttributeDescription attributeDescriptions[2] = {
        getPositionAttributeDescription(), getColorAttributeDescription()};
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pVertexBindingDescriptions = bindingDescriptions,
        .vertexBindingDescriptionCount =
            sizeof(bindingDescriptions) / sizeof(bindingDescriptions[0]),
        .pVertexAttributeDescriptions = attributeDescriptions,
        .vertexAttributeDescriptionCount =
            sizeof(attributeDescriptions) / sizeof(attributeDescriptions[0]),
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    // VkViewport viewport = {
    //     .x = 0.0f,
    //     .y = 0.0f,
    //     .width = (float)swapChainExtent.width,
    //     .height = (float)swapChainExtent.height,
    //     .minDepth = 0.0f,
    //     .maxDepth = 1.0f,
    // };
    //
    // VkRect2D scissor = {
    //     .offset = {0, 0},
    //     .extent = swapChainExtent,
    // };

    VkPipelineViewportStateCreateInfo viewportState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
    };

    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
    };

    VkPipelineMultisampleStateCreateInfo multiSampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .minSampleShading = 1.0f,
        .pSampleMask = NULL,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
    };

    VkPipelineColorBlendStateCreateInfo colorBlending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY, // Optional
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants[0] = 0.0f, // Optional
        .blendConstants[1] = 0.0f, // Optional
        .blendConstants[2] = 0.0f, // Optional
        .blendConstants[3] = 0.0f, // Optional
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,
        .pSetLayouts = NULL,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = NULL,
    };

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL,
                               &pipelineLayout) != VK_SUCCESS) {
        fprintf(stderr, "ERROR: failed to create pipeline layout\n");
        return -1;
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multiSampling,
        .pDepthStencilState = NULL,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = pipelineLayout,
        .renderPass = renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1,
    };

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                  NULL, &graphicsPipeline) != VK_SUCCESS) {
        fprintf(stderr, "ERROR: failed to create graphics pipeline\n");
        return -1;
    }

    free(vertShaderCode);
    free(fragShaderCode);

    vkDestroyShaderModule(device, vertShaderModule, NULL);
    vkDestroyShaderModule(device, fragShaderModule, NULL);

    return 0;
}

VkShaderModule createShaderModule(const char* code, size_t codeSize) {
    VkShaderModuleCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = codeSize,
        .pCode = (uint32_t*)code,
    };

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, NULL, &shaderModule) !=
        VK_SUCCESS) {
        fprintf(stderr, "ERROR: failed to create shader module\n");
        return NULL;
    }

    return shaderModule;
}

int createFrameBuffers() {
    for (size_t i = 0; i < swapChainImageCount; ++i) {
        VkImageView attachments[] = {swapChainImageViews[i]};

        VkFramebufferCreateInfo framebufferInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = renderPass,
            .attachmentCount = 1,
            .pAttachments = attachments,
            .width = swapChainExtent.width,
            .height = swapChainExtent.height,
            .layers = 1,
        };

        if (vkCreateFramebuffer(device, &framebufferInfo, NULL,
                                &swapChainFrameBuffers[i]) != VK_SUCCESS) {
            fprintf(stderr, "ERROR: failed to create framebuffer %lu\n", i);
            return -1;
        }
    }

    return 0;
}

int createCommandPool() {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);
    assert(queueFamilyIndices.graphicsFamily.is_present);

    VkCommandPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queueFamilyIndices.graphicsFamily.value,
    };

    if (vkCreateCommandPool(device, &poolInfo, NULL, &commandPool) !=
        VK_SUCCESS) {
        fprintf(stderr, "ERROR: failed to create command pool\n");
        return -1;
    }

    return 0;
}

int createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                 VkMemoryPropertyFlags properties, VkBuffer* buffer,
                 VkDeviceMemory* bufferMemory) {
    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    if (vkCreateBuffer(device, &bufferInfo, NULL, buffer) != VK_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to create vertex buffer");
        return -1;
    }

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device, *buffer, &memoryRequirements);
    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex =
            findMemoryType(memoryRequirements.memoryTypeBits, properties),
    };
    if (vkAllocateMemory(device, &allocInfo, NULL, bufferMemory) !=
        VK_SUCCESS) {
        fprintf(stderr, "ERROR: failed to allcate vertexBufferMemory\n");
        return -1;
    }

    vkBindBufferMemory(device, *buffer, *bufferMemory, 0);
    return 0;
}

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
        if ((typeFilter & (1 << i)) &&
            (memoryProperties.memoryTypes[i].propertyFlags & properties) ==
                properties) {
            return i;
        }
    }

    fprintf(stderr, "ERROR: Unable to find suitable memory type\n");
    exit(1);
}

int createVertexBuffer() {
    VkDeviceSize bufferSize = sizeof(vertices);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 &stagingBuffer, &stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices, (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vertexBuffer,
                 &vertexBufferMemory);
    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);

    return 0;
}

void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = commandPool,
        .commandBufferCount = 1,
    };
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion = {
        .size = size,
        .srcOffset = 0,
        .dstOffset = 0,
    };
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
    };
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

int createIndexBuffer() {
    VkDeviceSize bufferSize = sizeof(indices);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 &stagingBuffer, &stagingBufferMemory);
    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices, (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &indexBuffer, &indexBufferMemory);
    copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);
    return 0;
}

int createCommandBuffers() {

    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = MAX_FRAMES_IN_FLIGHT,
    };

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers) !=
        VK_SUCCESS) {
        return -1;
    }

    return 0;
}

int recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0,
        .pInheritanceInfo = NULL,
    };

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        fprintf(stderr, "ERROR: failed to begin recording command buffer\n");
        return -1;
    }

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    VkRenderPassBeginInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass,
        .framebuffer = swapChainFrameBuffers[imageIndex],
        .renderArea.offset = {0, 0},
        .renderArea.extent = swapChainExtent,
        .clearValueCount = 1,
        .pClearValues = &clearColor,
    };

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      graphicsPipeline);

    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)(swapChainExtent.width),
        .height = (float)(swapChainExtent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = swapChainExtent,
    };
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdDrawIndexed(commandBuffer, sizeof(indices)/sizeof(indices[0]), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        fprintf(stderr, "ERROR: failed to record command buffer\n");
        return -1;
    }

    return 0;
}

int createSyncObjects() {
    VkSemaphoreCreateInfo semaphoreInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        if (vkCreateSemaphore(device, &semaphoreInfo, NULL,
                              &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, NULL,
                              &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, NULL, &inFlightFences[i]) !=
                VK_SUCCESS) {
            fprintf(stderr,
                    "ERROR: failed to create synchronization objects for a "
                    "frame\n");
            return -1;
        }
    }

    return 0;
}

int mainloop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        drawFrame();
    }

    vkDeviceWaitIdle(device);

    return 0;
}

int drawFrame() {
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE,
                    UINT64_MAX);

    uint32_t imageIndex;

    VkResult result = vkAcquireNextImageKHR(
        device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame],
        VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return 0;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        fprintf(stderr, "ERROR: Failed to acquire swapchain image\n");
        return -1;
    }
    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    vkResetCommandBuffer(commandBuffers[currentFrame], 0);

    recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffers[currentFrame],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores,
    };

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo,
                      inFlightFences[currentFrame]) != VK_SUCCESS) {
        fprintf(stderr, "ERROR: failed to submit draw command buffer\n");
        return -1;
    }

    VkSwapchainKHR swapchains[] = {swapChain};
    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,
        .swapchainCount = 1,
        .pSwapchains = swapchains,
        .pImageIndices = &imageIndex,
        .pResults = NULL,
    };

    result = vkQueuePresentKHR(presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        framebufferResized) {
        framebufferResized = false;
        recreateSwapChain();
    } else if (result != VK_SUCCESS) {
        fprintf(stderr, "ERROR: failed to present swapchain image\n");
        return -1;
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    return 0;
}

void cleanup() {
    cleanupSwapChain();

    vkDestroyBuffer(device, indexBuffer, NULL);
    vkFreeMemory(device, indexBufferMemory, NULL);

    vkDestroyBuffer(device, vertexBuffer, NULL);
    vkFreeMemory(device, vertexBufferMemory, NULL);

    vkDestroyPipeline(device, graphicsPipeline, NULL);
    vkDestroyPipelineLayout(device, pipelineLayout, NULL);
    vkDestroyRenderPass(device, renderPass, NULL);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        vkDestroySemaphore(device, imageAvailableSemaphores[i], NULL);
        vkDestroySemaphore(device, renderFinishedSemaphores[i], NULL);
        vkDestroyFence(device, inFlightFences[i], NULL);
    }

    vkDestroyCommandPool(device, commandPool, NULL);

    vkDestroyDevice(device, NULL);
    vkDestroySurfaceKHR(instance, surface, NULL);
    vkDestroyInstance(instance, NULL);

    glfwDestroyWindow(window);
    glfwTerminate();
}

// Returns NULL on failure
static char* readFile(const char* fileName, size_t* fileSize) {
    FILE* fp = fopen(fileName, "rb");
    if (fp == NULL) {
        fprintf(stderr, "ERROR: couldn't open file %s\n", fileName);
        return NULL;
    }

    size_t size;

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    rewind(fp);

    char* dict = malloc(size + 1); // shader files do not need to be null
                                   // terminated but its here for safety
    if (dict == NULL) {
        fprintf(stderr,
                "ERROR: failed to allocate memory while reading file %s\n",
                fileName);
        fclose(fp);
        return NULL;
    }

    size_t nread = fread(dict, 1, size, fp);
    if (nread != size) {
        fprintf(stderr, "ERROR: only read %lu/%lu bytes\n", nread, size);
    }
    dict[nread] = '\0';

    (void)fclose(fp);

    *fileSize = size;

    return dict;
}

int compare_uint32_t(const void* a, const void* b) {
    return (*(uint32_t*)a - *(uint32_t*)b);
}

uint32_t removeDup(uint32_t arr[], size_t n) {
    if (n == 0)
        return 0;

    int j = 0;
    for (size_t i = 1; i < n - 1; i++) {

        // If a unique element is found, place
        // it at arr[j + 1]
        if (arr[i] != arr[j])
            arr[++j] = arr[i];
    }

    // Return the new ending of arr that only
    // contains unique elements
    return j + 1;
}
