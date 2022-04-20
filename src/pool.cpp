#include "pool.h"

#include <algorithm>
#include <cassert>
#include <functional>

std::size_t PoolAllocator::find_empty_place(const std::size_t n) const
{
    const auto it = std::lower_bound(m_elements_sizes.begin(), m_elements_sizes.end(), n);
    if (it == m_elements_sizes.end() || *it != n) {
        return invalid_pos;
    }
    std::size_t block_start_position = m_block_start_positions[it - m_elements_sizes.begin()];
    for (std::size_t i = 0; i < m_block_size / n; ++i) {
        if (!m_used_map[block_start_position + n * i]) {
            return block_start_position + n * i;
        }
    }
    return invalid_pos;
}

void * PoolAllocator::allocate(const std::size_t n)
{
    const auto pos = find_empty_place(n);
    if (pos != invalid_pos) {
        m_used_map[pos] = true;
        return &m_storage[pos];
    }
    throw std::bad_alloc{};
}

void PoolAllocator::deallocate(const void * ptr)
{
    const auto * b_ptr = static_cast<const std::byte *>(ptr);
    const auto * begin = &m_storage[0];
    std::less_equal<const std::byte *> cmp;
    if (cmp(begin, b_ptr) && cmp(b_ptr, &m_storage.back())) {
        const std::size_t position = b_ptr - begin;
        assert(position < m_used_map.size());
        m_used_map[position] = false;
    }
}
