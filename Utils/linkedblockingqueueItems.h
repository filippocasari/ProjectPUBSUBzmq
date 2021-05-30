#include <condition_variable>
#include <queue>
#include <cstring>

template <typename T> class BlockingQueue {
    std::condition_variable _cvCanPop;
    std::mutex _sync;
    std::queue<T> _qu;
    bool _bShutdown = false;
    size_t _capacity;

public:
    inline explicit BlockingQueue(size_t capacity) : _capacity(capacity) {
            // empty
    }
    void Push(const T& item)
    {

        std::unique_lock<std::mutex> lock(_sync);
        while (_qu.size() >= _capacity){
            _cvCanPop.wait(lock);
        }
        _qu.push(item);
        _cvCanPop.notify_one();
    }

    void RequestShutdown() {
        {
            std::unique_lock<std::mutex> lock(_sync);
            _bShutdown = true;
        }
        _cvCanPop.notify_all();
    }

    bool Pop(T &item) {
        std::unique_lock<std::mutex> lock(_sync);
        for (;;) {
            if (_qu.empty()) {
                if (_bShutdown) {
                    return false;
                }
            }else {
                break;
            }
            _cvCanPop.wait(lock);
        }
        item = std::move(_qu.front());
        _qu.pop();
        return true;
    }
};