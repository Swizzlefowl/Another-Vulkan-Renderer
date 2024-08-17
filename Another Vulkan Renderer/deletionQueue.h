#pragma once
#include <queue>
#include <deque>
#include <functional>
class DeletionQueue{
public:
    void enqueue(std::function<void()>&& fun);
    void deqeue();
    ~DeletionQueue();
private:
    std::deque<std::function<void()>> queue{};
    std::queue<std::function<void()>> delQueue{};

};

