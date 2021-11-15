#pragma once

#ifndef CONCURRENCY_CONCURRENT_QUEUE_HPP

#define CONCURRENCY_CONCURRENT_QUEUE_HPP

#include <queue>
#include <mutex>
#include <utility>

#define CONCURRENT_NAMESPACE_BEGIN namespace concurrency {
#define CONCURRENT_NAMESPACE_END }

CONCURRENT_NAMESPACE_BEGIN

// 加了一个互斥锁的队列??
template <typename Elem>
class concurrent_queue
{
public:

    void clear()
    {
        ::std::lock_guard<::std::mutex> lg(mtx);
        while (!q.empty()) q.pop();
    }

    [[nodiscard]] bool empty() const
    {
        ::std::lock_guard<::std::mutex> lg(mtx);
        bool ret = q.empty();
        return ret;
    }

    template <typename... Ts>
    void push(Ts&&... args)
    {
        ::std::lock_guard<::std::mutex> lg(mtx);
        q.emplace(::std::forward<Ts>(args)...);
    }

    bool try_pop(Elem& out)
    {
        ::std::lock_guard<::std::mutex> lg(mtx);
        if (q.empty()) return false;
        out = ::std::move(q.front());
        q.pop();
        return true;
    }

private:

    ::std::queue<Elem> q;
    mutable ::std::mutex mtx;
};

CONCURRENT_NAMESPACE_END

#undef CONCURRENT_NAMESPACE_END
#undef CONCURRENT_NAMESPACE_BEGIN

#endif //!CONCURRENCY_CONCURRENT_QUEUE_HPP
