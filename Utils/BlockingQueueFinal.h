//
// Created by Filippo Casari on 02/09/21.
//
#include <iostream>
#include <deque>

#ifndef PROJECTPUBSUBZMQ_BLOCKINGQUEUEFINAL_H
#define PROJECTPUBSUBZMQ_BLOCKINGQUEUEFINAL_H

using namespace std;
template<typename T, typename mtx_type, typename cond_variable_type> class blocking_queue {
private:
    std::deque<T, my_allocator<T>> _deque;
    mtx_type mtx;
    cond_variable_type cv;

public:

    blocking_queue() { }
    blocking_queue(blocking_queue&& other) : _deque(move(other._deque)) { }
    blocking_queue(const blocking_queue& other) = delete;

    T get() {
        std::unique_lock<mtx_type> lock(mtx);
        while (_deque.size() == 0)
            cv.wait(lock);

        T t = move(_deque.front()); _deque.pop_front();
        return move(t);

    }

    void put(T&& t) {
        std::unique_lock<mtx_type> lock(mtx);
        _deque.push_back(move(t));
        cv.notify_one();
    }
    void put(const T& t) {
        std::unique_lock<mtx_type> lock(mtx);
        _deque.push_back(t);
        cv.notify_one();
    }

    bool try_get(char uninitialized[sizeof(T)]) {
        std::unique_lock<mtx_type> lock(mtx);

        if (_deque.size() > 0) {
            new (uninitialized) T(move(_deque.front()));
            _deque.pop_front();
            return true;
        }

        return false;
    }

    bool try_get(T& out) {
        std::unique_lock<mtx_type> lock(mtx);

        if (_deque.size() > 0) {
            out = move(_deque.front());
            _deque.pop_front();
            return true;
        }

        return false;
    }

};


#endif //PROJECTPUBSUBZMQ_BLOCKINGQUEUEFINAL_H
