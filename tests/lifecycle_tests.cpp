#include "CabbageHardware.h"
#include "HardwareWrapperVulkan/ResourceRegistry.h"
#include "drop_oldest_queue.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>

namespace
{
void require(bool condition, const char *message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

struct TestTag
{
};

struct TestPayload
{
    int value{0};
};

struct TestTraits
{
    static void destroy(TestPayload &payload)
    {
        ++destroyed;
        payload.value = 0;
    }

    static std::atomic<int> destroyed;
};

std::atomic<int> TestTraits::destroyed{0};

using TestRegistry = CabbageHardwareInternal::ResourceRegistry<TestPayload, TestTag, TestTraits>;
using TestHandle = CabbageHardwareInternal::ResourceHandle<TestTag>;

void test_drop_oldest_capacity_and_latest()
{
    DropOldestQueue<int> queue(3);
    for (int i = 0; i < 5; ++i)
    {
        require(queue.push(i), "push should succeed before close");
    }

    require(queue.size() == 3, "queue should retain capacity items");
    require(queue.dropped_count() == 2, "queue should count discarded oldest items");

    auto latest = queue.try_pop_all_latest();
    require(latest.has_value() && *latest == 4, "try_pop_all_latest should return newest item");
    require(queue.size() == 0, "try_pop_all_latest should drain backlog");
}

void test_drop_oldest_close_wakes_waiter()
{
    DropOldestQueue<int> queue(2);
    std::atomic<bool> woke{false};
    std::thread consumer([&] {
        auto value = queue.pop_wait();
        require(!value.has_value(), "closed empty queue should return nullopt");
        woke.store(true, std::memory_order_release);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    queue.close();
    consumer.join();
    require(woke.load(std::memory_order_acquire), "close should wake blocked consumer");
    require(!queue.push(42), "push should fail after close");
}

void test_drop_oldest_mpsc()
{
    constexpr int producerCount = 4;
    constexpr int pushesPerProducer = 1000;
    constexpr int totalPushes = producerCount * pushesPerProducer;

    DropOldestQueue<int> queue(64);
    std::atomic<int> consumed{0};

    std::thread consumer([&] {
        while (auto value = queue.pop_wait())
        {
            (void)*value;
            consumed.fetch_add(1, std::memory_order_relaxed);
        }
    });

    std::vector<std::thread> producers;
    producers.reserve(producerCount);
    for (int producer = 0; producer < producerCount; ++producer)
    {
        producers.emplace_back([&, producer] {
            for (int i = 0; i < pushesPerProducer; ++i)
            {
                require(queue.push(producer * pushesPerProducer + i), "producer push should succeed");
            }
        });
    }

    for (auto &producer : producers)
    {
        producer.join();
    }
    queue.close();
    consumer.join();

    const auto accounted = static_cast<uint64_t>(consumed.load(std::memory_order_relaxed)) + queue.dropped_count();
    require(accounted == totalPushes, "every pushed item should be consumed or dropped");
}

void test_resource_handle_copy_move_and_destroy_once()
{
    TestTraits::destroyed.store(0, std::memory_order_release);
    TestRegistry registry("test");

    auto root = registry.allocate_handle("root");
    require(root.valid(), "fresh handle should be valid");

    {
        TestHandle copy = root;
        require(copy.valid(), "copied handle should be valid");

        TestHandle moved = std::move(copy);
        require(moved.valid(), "moved handle should keep ownership");
        require(!copy.valid(), "moved-from handle should be empty");
    }

    require(TestTraits::destroyed.load(std::memory_order_acquire) == 0, "payload should survive while root is alive");
    root.reset();
    require(TestTraits::destroyed.load(std::memory_order_acquire) == 1, "payload should be destroyed once");
    require(registry.live_count() == 0, "registry should be empty after final release");
}

void test_resource_handle_concurrent_retain_release()
{
    TestTraits::destroyed.store(0, std::memory_order_release);
    TestRegistry registry("test");
    auto root = registry.allocate_handle("root");

    constexpr int threadCount = 8;
    constexpr int iterations = 10000;
    std::vector<std::thread> threads;
    threads.reserve(threadCount);

    for (int threadIndex = 0; threadIndex < threadCount; ++threadIndex)
    {
        threads.emplace_back([&] {
            for (int i = 0; i < iterations; ++i)
            {
                TestHandle local = root;
                require(local.valid(), "concurrent copy should stay valid while root lives");
                TestHandle moved = std::move(local);
                require(moved.valid(), "concurrent moved copy should stay valid");
            }
        });
    }

    for (auto &thread : threads)
    {
        thread.join();
    }

    require(TestTraits::destroyed.load(std::memory_order_acquire) == 0, "concurrent copies should not destroy root");
    root.reset();
    require(TestTraits::destroyed.load(std::memory_order_acquire) == 1, "final reset should destroy once");
}

void test_resource_handle_stale_generation()
{
    TestTraits::destroyed.store(0, std::memory_order_release);
    TestRegistry registry("test");

    auto first = registry.allocate_handle("first");
    const auto firstId = first.id();
    const auto firstGeneration = first.generation();
    first.reset();

    auto stale = TestHandle::adopt(&registry, firstId, firstGeneration);
    require(!stale.valid(), "adopted stale id/generation should not become live");
    stale.reset();

    auto second = registry.allocate_handle("second");
    if (second.id() == firstId)
    {
        require(second.generation() != firstGeneration, "reused slot should advance generation");
    }
    second.reset();

    require(TestTraits::destroyed.load(std::memory_order_acquire) == 2, "both live allocations should destroy exactly once");
}

void test_push_constant_subview_keeps_parent_alive()
{
    HardwarePushConstant whole(16, 0);
    auto *wholeData = whole.getData();
    require(wholeData != nullptr, "whole push constant should allocate data");
    for (uint8_t i = 0; i < 16; ++i)
    {
        wholeData[i] = static_cast<uint8_t>(i);
    }

    HardwarePushConstant sub(4, 4, &whole);
    whole = HardwarePushConstant();

    require(sub.getSize() == 4, "sub push constant should keep its size");
    auto *subData = sub.getData();
    require(subData != nullptr, "sub push constant should keep parent data alive");
    require(subData[0] == 4 && subData[3] == 7, "sub view should still point at parent bytes");
}

void run_all()
{
    test_drop_oldest_capacity_and_latest();
    test_drop_oldest_close_wakes_waiter();
    test_drop_oldest_mpsc();
    test_resource_handle_copy_move_and_destroy_once();
    test_resource_handle_concurrent_retain_release();
    test_resource_handle_stale_generation();
    test_push_constant_subview_keeps_parent_alive();
}
} // namespace

int main()
{
    try
    {
        run_all();
    }
    catch (const std::exception &error)
    {
        std::cerr << "CabbageHardwareLifecycleTests failed: " << error.what() << '\n';
        return 1;
    }

    std::cout << "CabbageHardwareLifecycleTests passed\n";
    return 0;
}
