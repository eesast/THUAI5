#pragma once

#ifndef CONCURRENCY_CONCURRENT_QUEUE_HPP__

#define CONCURRENCY_CONCURRENT_QUEUE_HPP__

#include <queue>
#include <mutex>
#include <utility>
#include <optional>

#define THUAI_CONCURRENCY_NAMESPACE_BEGIN namespace thuai::concurrency {
#define THUAI_CONCURRENCY_NAMESPACE_END }

THUAI_CONCURRENCY_NAMESPACE_BEGIN

template <typename Elem>
class concurrent_queue
{
private:
    using queue_type = ::std::queue<Elem>;

public:
    using size_type = typename queue_type::size_type;
    using value_type = typename queue_type::value_type;
    using reference = typename queue_type::reference;
    using const_reference = typename queue_type::const_reference;
    using container_type = typename queue_type::container_type;

    concurrent_queue() = default;
    concurrent_queue(const concurrent_queue&) = delete;
    ~concurrent_queue() noexcept = default;
    concurrent_queue& operator=(const concurrent_queue&) = delete;

    void clear()
    {
        ::std::scoped_lock<::std::mutex> lk(mtx);
        while (!q.empty()) q.pop();
    }

    [[nodiscard]] bool empty() const
    {
        ::std::scoped_lock<::std::mutex> lk(mtx);
        return q.empty();
    }

    [[nodiscard]] auto size() const
    {
        ::std::scoped_lock<::std::mutex> lk(mtx);
        return q.size();
    }

    template <typename... Ts>
    void emplace(Ts&&... args)
    {
        ::std::scoped_lock<::std::mutex> lk(mtx);
        q.emplace(::std::forward<Ts>(args)...);
    }

    [[nodiscard]] ::std::optional<value_type> try_pop()
    {
        ::std::scoped_lock<::std::mutex> lk(mtx);
        if (q.empty()) return ::std::nullopt;
        auto out = ::std::make_optional<value_type>(::std::move(q.front()));
        q.pop();
        return out;
    }

private:
    queue_type q;
    mutable ::std::mutex mtx;
};

THUAI_CONCURRENCY_NAMESPACE_END

#undef THUAI_CONCURRENCY_NAMESPACE_END
#undef THUAI_CONCURRENCY_NAMESPACE_BEGIN

#endif //!CONCURRENCY_CONCURRENT_QUEUE_HPP
