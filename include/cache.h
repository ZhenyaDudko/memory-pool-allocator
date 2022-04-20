#pragma once

#include <algorithm>
#include <cstddef>
#include <list>
#include <new>
#include <ostream>

template <class Key, class KeyProvider, class Allocator>
class Cache
{
public:
    template <class... AllocArgs>
    Cache(const std::size_t cache_size, AllocArgs &&... alloc_args)
        : m_max_top_size(cache_size)
        , m_max_low_size(cache_size)
        , m_alloc(std::forward<AllocArgs>(alloc_args)...)
    {
    }

    std::size_t size() const
    {
        return m_queue_top.size() + m_queue_low.size();
    }

    bool empty() const
    {
        return size() == 0;
    }

    template <class T>
    T & get(const Key & key);

    std::ostream & print(std::ostream & strm) const;

    friend std::ostream & operator<<(std::ostream & strm, const Cache & cache)
    {
        return cache.print(strm);
    }

private:
    const std::size_t m_max_top_size;
    const std::size_t m_max_low_size;
    Allocator m_alloc;
    std::list<KeyProvider *> m_queue_top;
    std::list<KeyProvider *> m_queue_low;

    template <class T>
    void queue_top_check()
    {
        if (m_max_top_size < m_queue_top.size()) {
            m_queue_low.push_front(m_queue_top.back());
            m_queue_top.pop_back();
            queue_low_check();
        }
    }

    void queue_low_check()
    {
        if (m_max_low_size < m_queue_low.size()) {
            m_alloc.template destroy<KeyProvider>(m_queue_low.back());
            m_queue_low.pop_back();
        }
    }
};

template <class Key, class KeyProvider, class Allocator>
template <class T>
inline T & Cache<Key, KeyProvider, Allocator>::get(const Key & key)
{
    static_assert(std::is_constructible_v<T, const Key &>, "T has to be constructible from Key");
    static_assert(std::is_base_of_v<KeyProvider, T>, "T has to be a child of KeyProvider");

    auto finder = [&key](const KeyProvider * ptr) {
        return *ptr == key;
    };

    auto it = std::find_if(m_queue_top.begin(), m_queue_top.end(), finder);
    if (it != m_queue_top.end()) {
        m_queue_top.splice(m_queue_top.begin(), m_queue_top, it);
    }
    else {
        it = std::find_if(m_queue_low.begin(), m_queue_low.end(), finder);
        if (it != m_queue_low.end()) {
            m_queue_top.splice(m_queue_top.begin(), m_queue_low, it);
            queue_top_check<T>();
        }
        else {
            m_queue_low.push_front(m_alloc.template create<T>(key));
            queue_low_check();
            return *static_cast<T *>(m_queue_low.front());
        }
    }
    return *static_cast<T *>(m_queue_top.front());
}

template <class Key, class KeyProvider, class Allocator>
inline std::ostream & Cache<Key, KeyProvider, Allocator>::print(std::ostream & strm) const
{
    strm << "Priority:";
    for (const auto ptr : m_queue_top) {
        strm << " " << *ptr;
    }
    strm << '\n'
         << "Regular:";
    for (const auto ptr : m_queue_low) {
        strm << " " << *ptr;
    }
    return strm << '\n';
}
