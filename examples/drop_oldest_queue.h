#pragma once

#include <condition_variable>
#include <cstdint>
#include <cstddef>
#include <deque>
#include <mutex>
#include <optional>

// 主要作用：
// - 限制队列容量，避免生产者过快时内存无限增长。
// - 队列满时丢弃最旧数据，保留最新帧，减少显示滞后（更适合调 shader）。
// - pop_wait() 让消费者可阻塞等待，线程协作简单。
// - try_pop_all_latest() 让显示线程一次拿到最新结果并清空积压。
// - close() 可唤醒阻塞线程，退出流程不容易死锁。
// - dropped_count() 可统计背压和丢帧情况，方便调优。

template <typename T>
class DropOldestQueue
{
  public:
    // 构造队列，capacity 传 0 时自动回退为 1。
    explicit DropOldestQueue(std::size_t capacity = 10) : capacity_(capacity == 0 ? 1 : capacity){}

    // 入队：若队列已满则丢弃最旧元素，再写入新元素。
    // 返回 false 表示队列已关闭，写入失败。
    bool push(T value)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (closed_)
        {
            return false;
        }

        if (queue_.size() >= capacity_)
        {
            queue_.pop_front();
            ++dropped_count_;
        }
        queue_.push_back(std::move(value));
        condition_variable_.notify_one();
        return true;
    }

    // 阻塞出队：有数据时返回一项；若队列关闭且为空则返回 nullopt。
    std::optional<T> pop_wait()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_variable_.wait(lock, [&] {
            return closed_ || !queue_.empty();
        });

        if (queue_.empty())
        {
            return std::nullopt;
        }

        T value = std::move(queue_.front());
        queue_.pop_front();
        return value;
    }

    // 非阻塞获取“最新项”：清空积压，只返回最后一个元素。
    std::optional<T> try_pop_all_latest()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty())
        {
            return std::nullopt;
        }

        T latest = std::move(queue_.back());
        queue_.clear();
        return latest;
    }

    // 关闭队列并唤醒所有等待线程。
    void close()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        closed_ = true;
        condition_variable_.notify_all();
    }

    // 查询“已关闭且为空”，常用于线程退出条件。
    [[nodiscard]] bool is_closed_and_empty() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return closed_ && queue_.empty();
    }

    // 返回累计被丢弃的旧项数量。
    [[nodiscard]] uint64_t dropped_count() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return dropped_count_;
    }

    // 返回当前队列长度。
    [[nodiscard]] std::size_t size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    // 返回队列容量。
    [[nodiscard]] std::size_t capacity() const
    {
        return capacity_;
    }

  private:
    std::size_t capacity_{10};

    mutable std::mutex mutex_;
    std::condition_variable condition_variable_;
    std::deque<T> queue_;
    uint64_t dropped_count_{0};
    bool closed_{false};
};
