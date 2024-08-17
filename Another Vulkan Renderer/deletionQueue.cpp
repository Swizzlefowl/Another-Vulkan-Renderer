#include "deletionQueue.h"

void DeletionQueue::enqueue(std::function<void()>&& fun){
    queue.emplace_back(std::forward<std::function<void()>>(fun));
}

void DeletionQueue::deqeue(){
    std::deque<std::function<void()>> delQueue = { queue.rbegin(), queue.rend() };
    while (!delQueue.empty()) {
        delQueue.front()();
        delQueue.pop_front();
    }
}

DeletionQueue::~DeletionQueue(){
    deqeue();
}
