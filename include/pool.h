#pragma once

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <new>
#include <numeric>
#include <vector>

class PoolAllocator
{
public:
    PoolAllocator(const std::size_t block_size, std::initializer_list<std::size_t> sizes)
        : m_block_size(block_size)
        , m_elements_sizes(sizes)
        , m_block_start_positions(sizes.size())
    {
        std::sort(m_elements_sizes.begin(), m_elements_sizes.end());
        std::size_t total_size = 0;
        for (std::size_t i = 0; i < sizes.size(); i++) {
            m_block_start_positions[i] = total_size;
            total_size += (block_size / m_elements_sizes[i]) * m_elements_sizes[i];
        }
        m_storage.resize(total_size);
        m_used_map.resize(total_size);
    }

    void * allocate(std::size_t n);

    void deallocate(const void * ptr);

private:
    static constexpr std::size_t invalid_pos = static_cast<std::size_t>(-1);
    const std::size_t m_block_size;
    std::vector<std::size_t> m_elements_sizes;
    std::vector<std::size_t> m_block_start_positions;
    std::vector<std::byte> m_storage;
    std::vector<bool> m_used_map;

    std::size_t find_empty_place(std::size_t n) const;
};
