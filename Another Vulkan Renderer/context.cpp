#include "context.h"
#include <vector>
#include <algorithm>
#include <fmt/core.h>
Context::Window::Window(){}

Context::Window::Window(size_t height, size_t width, const std::string& title){
    this->height = height;
    this->width = width;
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    this->window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
}

Context::Window::Window(Window&& win){
    height = win.height;
    width = win.width;
    window = win.window;
    win.window = nullptr;
}

Context::Window& Context::Window::operator=(Window&& win){
    height = win.height;
    width = win.width;
    window = win.window;
    win.window = nullptr;
    return *this;
}

Context::Window::~Window(){
    if (!window) {
        return;
    }
    glfwDestroyWindow(window);
    glfwTerminate();
}

Context::Context(){}

void Context::createWindow(size_t height, size_t width, const std::string& title){
    window = Window{ height, width, title };
    return;
}

GLFWwindow* Context::getWindow() const{
    return window.window;
}

void Context::initVulkanCtx(){
    createVkInstance();
    return;
}

bool Context::checkForRequiredExtension(const std::string& name, const std::string& level) const {
    if (level == "instance") {
        bool found{ false };
        const auto supportedExtensions = vk::enumerateInstanceExtensionProperties();
        for (const auto& exten : supportedExtensions) {
            if (exten.extensionName == name)
                found = true;
        }
        return found;
    }
    /*else if (level == "device") {
        bool found{ false };
        const auto supportedExtensions = //TODO;
        for (const auto& exten : supportedExtensions) {
            if (exten.extensionName == name)
                found = true;
        }
        return found;
    }*/
    else
        return false;
}

void Context::createVkInstance(){
    if (!checkForRequiredExtension(VK_KHR_SURFACE_EXTENSION_NAME, "instance"))
        throw std::runtime_error("required extension not supported");
    vk::ApplicationInfo appInfo{};
    appInfo.pApplicationName = "another triangle";
    appInfo.applicationVersion = 1;
    appInfo.pEngineName = "no Engine";
    appInfo.engineVersion = 1;
    appInfo.apiVersion = VK_API_VERSION_1_3;

    vk::InstanceCreateInfo createInfo{};
    createInfo.pApplicationInfo = &appInfo;
    auto enabledExtensions = []() -> std::vector<const char*> {
        uint32_t extenCount{};
        auto requiredexten{ glfwGetRequiredInstanceExtensions(&extenCount) };
        std::vector<const char*> extensions{};
 
        for (size_t index{}; index < extenCount; index++) {
            extensions.emplace_back(requiredexten[index]);
        }
        return extensions;
        }();

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

