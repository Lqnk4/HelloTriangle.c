#include "arrays/arrays.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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

GLFWwindow* window;
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 800;

const char* const validationLayers[] = {"VK_LAYER_KHRONOS_validation"};

const char* const deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

VkInstance instance;
VkSurfaceKHR surface;
VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
VkDevice device;
VkQueue graphicsQueue;
VkQueue presentQueue;
VkSwapchainKHR swapChain;
VkImage* swapChainImages;
uint32_t swapChainImageCount;
VkImageView* swapChainImageViews;
VkFormat swapChainImageFormat;
VkExtent2D swapChainExtent;

int run();

int initWindow();

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
int createLogicalDevice();
int createSwapChain();
int createImageViews();

void mainloop();
void cleanup();

int main() {

    int err = run();
    if (err != 0) {
        return err;
    }

    return 0;
}

int run() {
    if (initWindow() != 0)
        return -1;
    initVulkan();
    mainloop();
    cleanup();
    return 0;
}

int initWindow() {
    if (glfwInit() == GLFW_FALSE) {
        perror("WARNING: Failed to initialize libglfw\n");
        return -1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", NULL, NULL);

    return 0;
}

int initVulkan() {

    if (enableValidationLayers && !checkValidationLayerSupport()) {
        perror("ERROR: validation layers requested, but not available!\n");
        return -1;
    }

    if (createInstance() != 0) {
        perror("ERROR: failed to create vulkan instance\n");
        return -1;
    }

    if (createSurface() != 0) {
        perror("ERROR: failed to create vulkan surface\n");
        return -1;
    }

    if (pickPhysicalDevice() != 0) {
        perror("ERROR: failed to pick physical device\n");
        return -1;
    }

    if (createLogicalDevice() != 0) {
        perror("ERROR: failed to create logical device\n");
        return -1;
    }

    if(createSwapChain() != 0) {
        perror("ERROR: failed to create swap chain\n");
        return -1;
    }

    if(createImageViews() != 0) {
        perror("ERROR: failed to create image views\n");
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
        perror("ERROR: Failed to create vulkan instance\n");
        return -1;
    }

    if (!checkRequiredGLFWExtensions(glfwExtensionCount, glfwExtensions)) {
        perror("ERROR: Failed to find required extensions\n");
        return -1;
    }

    return 0;
}

int createSurface() {
    if (glfwCreateWindowSurface(instance, window, NULL, &surface) !=
        VK_SUCCESS) {
        perror("ERROR: Failed to create window surface\n");
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
        perror("ERROR: Failed to enumerate instance layer properties\n");
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
        perror("ERROR: Found 0 physical devices\n");
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
        perror("ERROR: Failed to find a suitable device\n");
        return -1;
    }

    physicalDevice = bestDevice;

    if (physicalDevice == VK_NULL_HANDLE) {
        perror("ERROR: Failed to find a suitable device\n");
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

        i++;
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
        perror("ERROR: failed to create logical device\n");
        return -1;
    }

    vkGetDeviceQueue(device, indices.graphicsFamily.value, 0, &graphicsQueue);

    return 0;
}

int createSwapChain() {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

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

    if(vkCreateSwapchainKHR(device, &createInfo, NULL, &swapChain) != VK_SUCCESS) {
        perror("ERROR: failed to create swapchain\n");
        return -1;
    }

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;

    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, NULL);
    swapChainImageCount = imageCount;
    swapChainImages = malloc(sizeof(VkImage) * imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages);

    return 0;
}

int createImageViews() {
    // sufficient if we are only drawing one step
    swapChainImageViews = malloc(sizeof(VkImageView) * swapChainImageCount);

    for(size_t i = 0; i < swapChainImageCount; ++i) {
        VkImageViewCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = swapChainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = swapChainImageFormat,
            .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };

        if (vkCreateImageView(device, &createInfo, NULL, &swapChainImageViews[i]) != VK_SUCCESS) {
            fprintf(stderr, "ERROR: failed to create image view %lu\n", i);
            return -1;
        }
    }

    return 0;
}

void mainloop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

void cleanup() {
    for(size_t i = 0; i < swapChainImageCount; ++i) {
        vkDestroyImageView(device, swapChainImageViews[i], NULL);
    }

    vkDestroySwapchainKHR(device, swapChain, NULL);
    vkDestroyDevice(device, NULL);
    vkDestroySurfaceKHR(instance, surface, NULL);
    vkDestroyInstance(instance, NULL);

    glfwDestroyWindow(window);
    glfwTerminate();
}
