#include "Renderer.hpp"

void avr::Renderer::init(){
    ctx.createWindow(720, 640, "default name");
    ctx.initVulkanCtx();
    pEngine.createSwapchain();
}

void avr::Renderer::init(size_t height, size_t width, const std::string& title){
    ctx.createWindow(height, width, title);
    ctx.initVulkanCtx();
    pEngine.createSwapchain();
}

avr::Renderer::Renderer(){
}

avr::Renderer::~Renderer(){
}
