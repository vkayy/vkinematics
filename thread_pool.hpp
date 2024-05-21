#pragma once

#include <functional>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>

namespace tp {
    struct TaskQueue {
        std::queue<std::function<void()>> tasks;
        std::mutex mutex;
        std::atomic<uint32_t> remaining_tasks = 0;

        template<typename TCallback>
        void addTask(TCallback&& callback) {
            std::lock_guard<std::mutex> lock_guard{mutex};
            tasks.push(std::forward<TCallback>(callback));
            remaining_tasks++;
        }

        void getTask(std::function<void()>& target_callback) {
            std::lock_guard<std::mutex> lock_guard{mutex};
            if (tasks.empty()) return;
            target_callback = std::move(tasks.front());
            tasks.pop();
        }

        static void wait() {
            std::this_thread::yield();
        }

        void waitForCompletion() const {
            while (remaining_tasks > 0) {
                wait();
            }
        }

        void workDone() {
            remaining_tasks--;
        }
    };

    struct Worker {
        uint32_t id;
        std::thread thread;
        std::function<void()> task;
        bool running = true;
        TaskQueue* queue;

        Worker() = default;

        Worker(TaskQueue& queue, uint32_t id)
            : queue{&queue}
            , id{id}
        {
            thread = std::thread([this](){
                run();
            });
        }

        void run() {
            while (running) {
                queue->getTask(task);
                if (!task) {
                    TaskQueue::wait();
                } else {
                    task();
                    queue->workDone();
                    task = nullptr;
                }
            }
        }

        void stop() {
            running = false;
            thread.join();
        }
    };

    struct ThreadPool {
        uint32_t thread_count;
        TaskQueue queue;
        std::vector<Worker> workers;

        explicit
        ThreadPool(uint32_t thread_count)
            : thread_count{thread_count}
        {
            workers.reserve(thread_count);
            for (uint32_t i=thread_count; i>0; i--) {
                workers.emplace_back(queue, static_cast<uint32_t>(workers.size()));
            }
        }

        virtual ~ThreadPool() {
            for (Worker &worker : workers) {
                worker.stop();
            }
        }

        template<typename TCallback>
        void addTask(TCallback&& callback) {
            queue.addTask(std::forward<TCallback>(callback));
        }

        void waitForCompletion() const {
            queue.waitForCompletion();
        }

        template<typename TCallback>
        void dispatch(uint32_t element_count, TCallback&& callback) {
            const uint32_t batch_size = element_count / thread_count;
            for (uint32_t i=0; i<thread_count; ++i) {
                const uint32_t start = batch_size * i;
                const uint32_t end = start + batch_size;
                addTask([start, end, &callback](){ callback(start, end); });
            }
            if (batch_size * thread_count < element_count) {
                const uint32_t start = batch_size * thread_count;
                callback(start, element_count);
            }
            waitForCompletion();
        }
    };
}
