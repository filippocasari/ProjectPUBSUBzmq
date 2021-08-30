//
// Created by Filippo Casari on 30/08/21.
//

#ifndef PROJECTPUBSUBZMQ_BLOCKINGQUEUE_H
#define PROJECTPUBSUBZMQ_BLOCKINGQUEUE_H


template <typename T>
class BlockingQueue
        {
        private:
            std::mutex              d_mutex;
            std::condition_variable d_condition;
            std::deque<T>           d_queue;
        public:
            void push(T const& value) {
                {
                    std::unique_lock<std::mutex> lock(this->d_mutex);
                    d_queue.push_front(value);
                }
                this->d_condition.notify_one();
            }
            T pop() {
                std::unique_lock<std::mutex> lock(this->d_mutex);
                this->d_condition.wait(lock, [=]{ return !this->d_queue.empty(); });
                T rc(std::move(this->d_queue.back()));
                this->d_queue.pop_front();
                return rc;
            }
        };


#endif //PROJECTPUBSUBZMQ_BLOCKINGQUEUE_H