#include "horizon/core/storage.h"
#include "horizon/core/stack_trace.h"

#include <array>
#include <iostream>
#include <type_traits>

static_assert(std::is_same_v<horizon::core::Storage<int>, Corona::Kernel::Utils::Storage<int>>);

int main()
{
    horizon::core::Storage<int, 2, 1> storage;
    std::array<horizon::core::Storage<int, 2, 1>::ObjectId, 3> ids {};
    for (int i = 0; i < 3; ++i)
    {
        ids[i] = storage.allocate();
        auto write = storage.acquire_write(ids[i]);
        *write = 10 + i;
    }
    for (int i = 0; i < 3; ++i)
    {
        auto read = storage.acquire_read(ids[i]);
        if (*read != 10 + i)
        {
            std::cerr << "Storage lost an object while growing\n";
            return 1;
        }
    }
    storage.deallocate(ids[0]);
    if (storage.try_acquire_read(ids[0]))
    {
        std::cerr << "A freed storage slot remains readable\n";
        return 1;
    }
    for (int i = 1; i < 3; ++i)
    {
        storage.deallocate(ids[i]);
    }
    if (horizon::core::capture_stack_trace_light(0, 4).empty())
    {
        std::cerr << "Stack trace diagnostics returned no result\n";
        return 1;
    }
    return 0;
}
