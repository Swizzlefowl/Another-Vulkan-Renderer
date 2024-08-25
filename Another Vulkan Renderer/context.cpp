#include "context.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include <fmt/core.h>
#include "engineUtils.h"
namespace avr {
    Context::Window::Window() {}

    Context::Window::Window(size_t height, size_t width, const std::string& title) {
        this->height = height;
        this->width = width;
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        this->window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    }

    Context::Window::Window(Window&& win) {
        height = win.height;
        width = win.width;
        window = win.window;
        win.window = nullptr;
    }

    Context::Window& Context::Window::operator=(Window&& win) {
        height = win.height;
        width = win.width;
        window = win.window;
        win.window = nullptr;
        return *this;
    }

    Context::Window::~Window() {
        if (!window) {
            return;
        }
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    Context::Context() {}

    void Context::createWindow(size_t height, size_t width, const std::string& title) {
        window = Window{ height, width, title };
        return;
    }

    GLFWwindow* Context::getWindow() const {
        return window.window;
    }

    void Context::initVulkanCtx() {
        createVkInstance();
        createSurface();
        createLogicalDevice();
        createCommandPool();
        createCommandBuffer();
        createAllocator();
        return;
    }

    bool Context::checkForRequiredExtension(const std::string& name, Levels level, vk::PhysicalDevice physicalDevice) const {
        if (level == Levels::Instance) {
            bool found{ false };
            const auto supportedExtensions = vk::enumerateInstanceExtensionProperties();
            for (const auto& exten : supportedExtensions) {
                if (exten.extensionName == name)
                    found = true;
            }
            return found;
        }
        else if (level == Levels::Device) {
            bool found{ false };
            const auto supportedExtensions = physicalDevice.enumerateDeviceExtensionProperties();
            for (const auto& exten : supportedExtensions) {
                if (exten.extensionName == name)
                    found = true;
            }
            return found;
        }
        else
            return false;
    }

    void Context::createVkInstance() {
        if (!checkForRequiredExtension(VK_KHR_SURFACE_EXTENSION_NAME, Levels::Instance))
            throw std::runtime_error("required extension not supported");
        vk::ApplicationInfo appInfo{};
        appInfo.pApplicationName = "another triangle";
        appInfo.applicationVersion = 1;
        appInfo.pEngineName = "no Engine";
        appInfo.engineVersion = 1;
        appInfo.apiVersion = VK_API_VERSION_1_3;

        vk::InstanceCreateInfo createInfo{};
        createInfo.pApplicationInfo = &appInfo;
        auto enabledExtensions{ []() -> std::vector<const char*> {
            uint32_t extenCount{};
            auto requiredexten{ glfwGetRequiredInstanceExtensions(&extenCount) };
            std::vector<const char*> extensions{};

            for (size_t index{}; index < extenCount; index++) {
                extensions.emplace_back(requiredexten[index]);
            }
            return extensions;
            }() };
       
        createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
        createInfo.ppEnabledExtensionNames = enabledExtensions.data();
        if (vk::createInstance(&createInfo, nullptr, &instance) != vk::Result::eSuccess)
            throw std::runtime_error("failed to create instance!");
        fmt::println("created vulkan instance");
        deleteQueue.enqueue([&]() {
            instance.destroy();
            fmt::println("instance destroyed");
            });
        return;
    }
    void Context::createSurface(){
        VkSurfaceKHR tempSurface;
        if (glfwCreateWindowSurface(instance, window.window, nullptr, &tempSurface) != VK_SUCCESS)
            throw std::runtime_error("failed to create window surface!");
        
        surface = vk::SurfaceKHR{ tempSurface };
        fmt::println("created surface");
        deleteQueue.enqueue([&]() {
            instance.destroySurfaceKHR(surface);
            fmt::println("destroyed surface");
            });
        return;
    }

    bool Context::isDeviceSuitable(const vk::PhysicalDevice& dev){
        if (!checkForRequiredExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME, Levels::Device, dev)) {
            return false;
        }
        vk::PhysicalDeviceProperties deviceProperties{ dev.getProperties() };
        //vk::PhysicalDeviceFeatures deviceFeatures{ dev.getFeatures() }; might use in the future
        if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
            std::cout << deviceProperties.deviceName << std::endl;
            return true;
        }
        
        return false;
    }
    vk::PhysicalDevice Context::getPdevice(){
        const auto devices{ instance.enumeratePhysicalDevices() };
        for (const auto& device : devices) {
            if (isDeviceSuitable(device))
                return device;
        }
        return nullptr;
    }
    std::pair<uint32_t, QueueTypes> Context::getQueueIndex(const vk::PhysicalDevice& dev, QueueTypes type){
        auto queueProp{ dev.getQueueFamilyProperties2() };
        uint32_t indice{};
        std::pair<uint32_t, QueueTypes> queueIndex{0, QueueTypes::Invalid};

        switch (type){
        case avr::QueueTypes::Graphics: {
            for (const auto& queue : queueProp) {
                if (queue.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics) {
                    queueIndex.first = indice;
                    queueIndex.second = QueueTypes::Graphics;
                    break;
                }
                indice++;
            }
            
            if (!dev.getSurfaceSupportKHR(indice, surface))
                fmt::println("queue does not support presenting to surface");
        }
            break;
        case avr::QueueTypes::Compute: {
            for (const auto& queue : queueProp) {
                if (queue.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eCompute) {
                    if (queue.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics) {
                        indice++;
                        continue;
                    }
                    queueIndex.first = indice;
                    queueIndex.second = QueueTypes::Compute;
                    break;
                }
            }
        }
            break;
        case avr::QueueTypes::Transfer: {
            for (const auto& queue : queueProp) {
                if (queue.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eTransfer) {
                    if (queue.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics
                        || queue.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eCompute) {
                        indice++;
                        continue;
                    }
                    queueIndex.first = indice;
                    queueIndex.second = QueueTypes::Transfer;
                    break;
                }
            }
        }
            break;
        case avr::QueueTypes::Invalid:
            break;
        default:
            break;
        }
        if (queueIndex.second == QueueTypes::Invalid) {
            queueIndex.first = std::numeric_limits<uint32_t>::max();
        }
        return queueIndex;
    }

    void Context::createLogicalDevice(){
        physicalDevice = getPdevice();
        if (!physicalDevice)
            throw std::runtime_error("failed to find any suitable device");

        const auto queueIndex{ getQueueIndex(physicalDevice, QueueTypes::Graphics) };
        if (queueIndex.second == QueueTypes::Invalid)
            throw std::runtime_error("no suitable queue family");

         float queuePriority = 1.0f;
         vk::DeviceQueueCreateInfo queueCreateInfo{};
         queueCreateInfo.queueFamilyIndex = queueIndex.first;
         queueCreateInfo.queueCount = 1;
         queueCreateInfo.pQueuePriorities = &queuePriority;

         vk::PhysicalDeviceFeatures2 features2{};
         vk::PhysicalDeviceVulkan12Features feature12{};
         vk::PhysicalDeviceVulkan13Features feature13{};
         feature12.bufferDeviceAddress = true;
         feature12.descriptorIndexing = true;
         feature12.scalarBlockLayout = true;
         feature13.synchronization2 = true;
         feature13.dynamicRendering = true;
         features2.features.samplerAnisotropy = true;
         features2.pNext = &feature12;
         feature12.pNext = &feature13;

         vk::DeviceCreateInfo createInfo{};
         createInfo.pNext = &features2;
         createInfo.queueCreateInfoCount = 1;
         createInfo.pQueueCreateInfos = &queueCreateInfo;
         std::vector<const char*> deviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
         createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
         createInfo.ppEnabledExtensionNames = deviceExtensions.data();
         if (physicalDevice.createDevice(&createInfo, nullptr, &device) != vk::Result::eSuccess)
             throw std::runtime_error("failed to create device");
         queue = device.getQueue(queueIndex.first, 0);
         fmt::println("created device");
         deleteQueue.enqueue([&]() {
             device.destroy();
             fmt::println("destroyed device");
             });
    }
    void Context::createCommandPool(){
        commandPool = avr::createCommandPool(*this);
        fmt::println("created command pool");
        deleteQueue.enqueue([&]() {
            device.destroyCommandPool(commandPool);
            fmt::println("command pool destroyed");
            });
    }
    void Context::createCommandBuffer(){
        commandBuffer = avr::createCommandBuffer(*this, commandPool, 2);
    }

    void Context::createAllocator(){
        VmaAllocatorCreateInfo info{};
        info.vulkanApiVersion = VK_API_VERSION_1_3;
        info.instance = instance;
        info.physicalDevice = physicalDevice;
        info.device = device;
        info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
        vmaCreateAllocator(&info, &allocator);
        fmt::println("allocator created");

        deleteQueue.enqueue([&]() {
            vmaDestroyAllocator(allocator);
            fmt::println("allocator destroyed");
            });
    }
}

