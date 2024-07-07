//
// Created by Cristian Bosin  on 06/07/24.
//

#ifndef SIMPLEDB_PAGE_HPP
#define SIMPLEDB_PAGE_HPP

#include <array>
#include <cstddef>
#include <iostream>
#include <span>
#include <sstream>

template<class T>
concept Streamable = requires(std::ostream &os, T a) {
    os << a;
};


template<size_t PageSize = 16384>
class page {
public:
    constexpr static size_t page_size = PageSize;
    using page_type = page<page_size>;
    using value_type = uint8_t;
    using const_value = const value_type;
    using view_type = std::span<value_type>;
    using const_view = std::span<const_value>;

    using intra_offset_t = std::conditional_t<page_size <= 16384, uint16_t, uint32_t>;
    using inter_offset_t = uint32_t;

    struct header {
        intra_offset_t free_region_start;
        intra_offset_t free_region_end;
    };

    constexpr static size_t data_size = PageSize - sizeof(header);

    page() {
        m_header.free_region_start = sizeof(header);
        m_header.free_region_end = data_size;
    }

    [[nodiscard]] bool empty() const {
        return size() == 0;
    }

    [[nodiscard]] bool contains(size_t index) const {
        return index < size();
    }

    [[nodiscard]] const_view at(size_t index) const {
        if (not contains(index)) {
            throw std::out_of_range("Index out of range");
        }
        auto [size, offset] = size_and_offset_of(index);
        const auto *first = reinterpret_cast<const_value *>(offset);
        const auto *last = std::next(first, size);
        return {first, last};
    }

    view_type at(size_t index) {
        auto view = const_cast<const page *>(this)->at(index);
        return {const_cast<value_type *>(view.data()), view.size()};
    }


    template<Streamable T>
    // require non-string type
        requires(not std::same_as<T, std::string> and not std::same_as<T, std::string_view> and not std::same_as<T, const char *>)
    size_t insert(T data) {
        std::ostringstream os;
        os << data;
        return insert(os.str());
    }

    size_t insert(std::string_view data) {
        if (not fits(data.size() + sizeof(intra_offset_t) + 2)) {
            throw std::bad_alloc();
        }
        auto start = m_header.free_region_end - data.size() - sizeof(intra_offset_t);
        auto *offset = reinterpret_cast<intra_offset_t *>(&m_data[start]);
        *offset = static_cast<intra_offset_t>(data.size());
        std::copy(data.begin(), data.end(), m_data.begin() + start + sizeof(intra_offset_t));
        auto *offsets = reinterpret_cast<intra_offset_t *>(m_data.data());
        auto current_entry = size();
        offsets[current_entry] = start;
        m_header.free_region_start += sizeof(intra_offset_t);
        m_header.free_region_end = start;
        return current_entry;
    }

    [[nodiscard]] bool fits(size_t size) const {
        return free_space() >= size;
    }

    [[nodiscard]] size_t size() const {
        return (m_header.free_region_start - sizeof(header)) / sizeof(intra_offset_t);
    }

    void clear() {
        m_header.free_region_start = sizeof(header);
        m_header.free_region_end = PageSize;
    }

    [[nodiscard]] size_t free_space() const {
        return m_header.free_region_end - m_header.free_region_start;
    }

    friend std::ostream &operator<<(std::ostream &os, const page &page) {
        os.write(reinterpret_cast<const char *>(&page.m_header), sizeof(header));
        os.write(reinterpret_cast<const char *>(page.m_data.data()), page.m_data.size());
        return os;
    }

    friend std::istream &operator>>(std::istream &is, page &page) {
        is.read(reinterpret_cast<char *>(&page.m_header), sizeof(header));
        is.read(reinterpret_cast<char *>(page.m_data.data()), page.m_data.size());
        return is;
    }

private:
    std::pair<intra_offset_t, const intra_offset_t *> size_and_offset_of(size_t index) const {
        const auto *offsets = reinterpret_cast<const intra_offset_t *>(m_data.data());
        const intra_offset_t data_offset = offsets[index];
        const auto *data_ptr = reinterpret_cast<const intra_offset_t *>(&m_data[data_offset]);
        return {*data_ptr, data_ptr + 1};
    }


    header m_header;
    std::array<uint8_t, data_size> m_data;
};


#endif//SIMPLEDB_PAGE_HPP
