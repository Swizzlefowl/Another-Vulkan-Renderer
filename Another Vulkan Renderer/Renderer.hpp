#pragma once
#include "context.h"
#include "presentEngine.hpp"
namespace avr {
    class Renderer {
    public:
        void init();
        void init(size_t height, size_t width, const std::string& title);
        Renderer();
        ~Renderer();
        Context ctx{};
        PresentEngine pEngine{ctx};
    };
}

