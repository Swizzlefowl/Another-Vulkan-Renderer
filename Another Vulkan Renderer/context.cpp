#include "context.h"
#include <vector>
#include <algorithm>
#include <fmt/core.h>
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
        if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
            return true;

        return true;
    }
    vk::PhysicalDevice Context::getPdevice(){
        const auto devices{ instance.enumeratePhysicalDevices() };
        for (const auto& device : devices) {
            if (isDeviceSuitable(device))
                return device;
        }
        return nullptr;
    }
    void Context::createLogicalDevice(){
        physicalDevice = getPdevice();
        if (!physicalDevice)
            throw std::runtime_error("failed to find any suitable device");

    }
}

