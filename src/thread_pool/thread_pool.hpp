#pragma once

#include <functional>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace tp {
    struct TaskQueue {
        std::queue<std::function<void()>> tasks;
        std::mutex mutex;
        std::condition_variable condition;
        std::atomic<bool> stop;
        std::atomic<uint32_t> incomplete_tasks;

        TaskQueue()
            : stop{false}
            , incomplete_tasks{0}
        {}

        template<typename TaskCallback>
        void enqueueTask(TaskCallback&& callback) {
            {
                std::lock_guard<std::mutex> lock_guard{mutex};
                tasks.push(std::forward<TaskCallback>(callback));
                incomplete_tasks++;
            }
            condition.notify_one();
        }

        bool dequeueTask(std::function<void()>& target_callback) {
            std::unique_lock<std::mutex> lock{mutex};
            condition.wait(lock, [this]{
                return !tasks.empty() || stop;
            });
            if (tasks.empty()) return false;
            target_callback = std::move(tasks.front());
            tasks.pop();
            return true;
        }

        void completeAllTasks() {
            std::unique_lock<std::mutex> lock{mutex};
            condition.wait(lock, [this] {
                return !incomplete_tasks;
            });
        }

        void finishTask() {
            {
                std::lock_guard<std::mutex> lock{mutex};
                incomplete_tasks--;
            }
            condition.notify_all();
        }

        void shutdownTasks() {
            {
                std::lock_guard<std::mutex> lock{mutex};
                stop = true;
            }
            condition.notify_all();
        }
    };

    struct Worker {
        uint32_t id;
        std::thread thread;
        TaskQueue *queue;
        bool thread_active = true;
        std::function<void()> task;

        Worker() = default;

        Worker(TaskQueue &queue, uint32_t id)
            : queue{&queue}
            , id{id}
        {
            thread = std::thread([this](){ run(); });
        }

        void run() {
            while (thread_active) {
                if (queue->dequeueTask(task)) {
                    task();
                    queue->finishTask();
                }
                task = nullptr;
            }
        }

        void deactivateThread() {
            thread_active = false;
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
            for (uint32_t i=0; i<thread_count; i++) {
                workers.emplace_back(queue, i);
            }
        }

        ~ThreadPool() {
            queue.shutdownTasks();
            for (Worker &worker : workers) {
                worker.deactivateThread();
            }
        }

        template<typename TaskCallback>
        void enqueueTask(TaskCallback&& callback) {
            queue.enqueueTask(std::forward<TaskCallback>(callback));
        }

        void completeAllTasks() {
            queue.completeAllTasks();
        }

        template<typename TaskCallback>
        void dispatch(uint32_t element_count, TaskCallback&& callback) {
            const uint32_t batch_size = element_count / thread_count;
            for (uint32_t i=0; i<thread_count; i++) {
                const uint32_t start = batch_size * i;
                const uint32_t end = start + batch_size;
                enqueueTask([start, end, &callback](){ callback(start, end); });
            }
            if (batch_size * thread_count < element_count) {
                const uint32_t start = batch_size * thread_count;
                callback(start, element_count);
            }
            completeAllTasks();
        }
    };
}
