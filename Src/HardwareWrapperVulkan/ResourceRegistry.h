#pragma once

#include "CabbageHardware.h"
#include "corona/kernel/utils/storage.h"

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>

namespace CabbageHardwareInternal
{

template <typename Payload, typename Tag, typename Traits>
class ResourceRegistry final : public ResourceRegistryBase
{
  public:
    using StorageType = Corona::Kernel::Utils::Storage<Payload>;
    using ObjectId = typename StorageType::ObjectId;
    using ReadHandle = typename StorageType::ReadHandle;
    using WriteHandle = typename StorageType::WriteHandle;
    using Handle = ResourceHandle<Tag>;

    explicit ResourceRegistry(const char *registryName) : registryName_(registryName)
    {
    }

    ResourceRegistry(const ResourceRegistry &) = delete;
    ResourceRegistry &operator=(const ResourceRegistry &) = delete;

    [[nodiscard]] Handle allocate_handle(std::string debugName = {})
    {
        const ObjectId id = storage_.allocate();
        std::uint64_t generation = 0;
        {
            std::lock_guard<std::mutex> lock(controlMutex_);
            generation = ++generations_[id];
            controls_[id] = std::make_unique<Control>(generation, std::move(debugName));
        }
        return Handle::adopt(this, id, generation);
    }

    [[nodiscard]] ObjectId allocate() = delete;

    void deallocate(ObjectId id) = delete;

    [[nodiscard]] ReadHandle acquire_read(ObjectId id)
    {
        return storage_.acquire_read(id);
    }

    [[nodiscard]] WriteHandle acquire_write(ObjectId id)
    {
        return storage_.acquire_write(id);
    }

    [[nodiscard]] ReadHandle try_acquire_read(ObjectId id)
    {
        return storage_.try_acquire_read(id);
    }

    [[nodiscard]] WriteHandle try_acquire_write(ObjectId id)
    {
        return storage_.try_acquire_write(id);
    }

    [[nodiscard]] std::int64_t seq_id(ObjectId id) const
    {
        return storage_.seq_id(id);
    }

    [[nodiscard]] std::int64_t seq_id(const ReadHandle &handle) const
    {
        return storage_.seq_id(handle);
    }

    [[nodiscard]] std::int64_t seq_id(const WriteHandle &handle) const
    {
        return storage_.seq_id(handle);
    }

    [[nodiscard]] bool contains(ObjectId id) const
    {
        return alive(id, generation_of(id));
    }

    [[nodiscard]] std::size_t live_count() const
    {
        std::lock_guard<std::mutex> lock(controlMutex_);
        return controls_.size();
    }

    void retain(std::uintptr_t id, std::uint64_t generation) noexcept override
    {
        std::lock_guard<std::mutex> lock(controlMutex_);
        auto it = controls_.find(id);
        if (it == controls_.end())
        {
            return;
        }
        Control &control = *it->second;
        if (!control.alive || control.generation != generation)
        {
            return;
        }
        control.strongRefs.fetch_add(1, std::memory_order_relaxed);
    }

    void release(std::uintptr_t id, std::uint64_t generation) noexcept override
    {
        bool shouldDestroy = false;
        {
            std::lock_guard<std::mutex> lock(controlMutex_);
            auto it = controls_.find(id);
            if (it == controls_.end())
            {
                return;
            }

            Control &control = *it->second;
            if (!control.alive || control.generation != generation)
            {
                return;
            }

            const std::uint64_t oldRefs = control.strongRefs.fetch_sub(1, std::memory_order_acq_rel);
            if (oldRefs > 1)
            {
                return;
            }

            control.alive = false;
            shouldDestroy = true;
        }

        if (!shouldDestroy)
        {
            return;
        }

        try
        {
            {
                auto payload = storage_.acquire_write(id);
                Traits::destroy(*payload);
                *payload = Payload{};
            }

            {
                std::lock_guard<std::mutex> lock(controlMutex_);
                controls_.erase(id);
            }

            storage_.deallocate(id);
        }
        catch (...)
        {
            std::lock_guard<std::mutex> lock(controlMutex_);
            controls_.erase(id);
        }
    }

    [[nodiscard]] bool alive(std::uintptr_t id, std::uint64_t generation) const noexcept override
    {
        std::lock_guard<std::mutex> lock(controlMutex_);
        auto it = controls_.find(id);
        if (it == controls_.end())
        {
            return false;
        }

        const Control &control = *it->second;
        return control.alive && control.generation == generation &&
               control.strongRefs.load(std::memory_order_acquire) > 0;
    }

    [[nodiscard]] const char *name() const noexcept override
    {
        return registryName_;
    }

  private:
    struct Control
    {
        Control(std::uint64_t generationValue, std::string debugNameValue)
            : generation(generationValue), debugName(std::move(debugNameValue))
        {
        }

        std::uint64_t generation{0};
        std::atomic<std::uint64_t> strongRefs{1};
        bool alive{true};
        std::string debugName;
    };

    [[nodiscard]] std::uint64_t generation_of(ObjectId id) const noexcept
    {
        std::lock_guard<std::mutex> lock(controlMutex_);
        auto it = controls_.find(id);
        return it == controls_.end() ? 0 : it->second->generation;
    }

    const char *registryName_{""};
    StorageType storage_{};
    mutable std::mutex controlMutex_;
    std::unordered_map<ObjectId, std::unique_ptr<Control>> controls_;
    std::unordered_map<ObjectId, std::uint64_t> generations_;
};

} // namespace CabbageHardwareInternal
