#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdio.h>

#ifdef DEBUG
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
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

GLFWwindow* window;
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 800;

const char* const validationLayers[] = {"VK_LAYER_KHRONOS_validation"};

VkInstance instance;
VkSurfaceKHR surface;
VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
VkDevice device;
VkQueue graphicsQueue;
VkQueue presentQueue;

int run();

int initWindow();

int initVulkan();
int createInstance();
int createSurface();
const char** getRequiredExtensions();
bool checkRequiredExtensions(const uint32_t glfwExtensionCount,
                             const char* const* glfwExtensions);
bool checkValidationLayerSupport();
int pickPhysicalDevice();
int rateDeviceSuitability(VkPhysicalDevice device);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
int createLogicalDevice();

void mainloop();
void cleanup();

int main(int argc, char** argv) {
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

    if (!checkRequiredExtensions(glfwExtensionCount, glfwExtensions)) {
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

// Emits a warning to stdin whenever a required glfw extension is not found
bool checkRequiredExtensions(const uint32_t glfwExtensionCount,
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

    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    }

    score += deviceProperties.limits.maxImageDimension2D;

    return score;
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

int createLogicalDevice() {
    // indices should implicitly be complete after creation of physicalDevice
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    uint32_t queueFamilies[2] = {indices.graphicsFamily.value,
                                 indices.presentFamily.value};
    uint32_t queueFamiliesLength =
        sizeof(queueFamilies) / sizeof(queueFamilies[0]);

    //  FIX: make this not create duplicate queues if queueFamilies are the same
    VkDeviceQueueCreateInfo uniqueQueueFamilies[queueFamiliesLength] = {};

    float queuePriority = 1.0f;
    for (size_t i = 0; i < queueFamiliesLength; ++i) {
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
        .enabledExtensionCount = 0, // leave this for now
    };

    if (vkCreateDevice(physicalDevice, &createInfo, NULL, &device) !=
        VK_SUCCESS) {
        perror("ERROR: failed to create logical device\n");
        return -1;
    }

    vkGetDeviceQueue(device, indices.graphicsFamily.value, 0, &graphicsQueue);

    return 0;
}

void mainloop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

void cleanup() {
    vkDestroyDevice(device, NULL);
    vkDestroySurfaceKHR(instance, surface, NULL);
    vkDestroyInstance(instance, NULL);

    glfwDestroyWindow(window);
    glfwTerminate();
}
