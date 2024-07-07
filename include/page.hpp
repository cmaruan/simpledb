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

/**
 * @brief A concept that checks if a type can be streamed to an output stream.
 */
template<class T>
concept Streamable = requires(std::ostream &os, T a) {
    os << a;
};

/**
 * @class page
 * @tparam PageSize The size of the page.
 * @brief A class that represents a page in a database.
 *
 * This class represents a page in a database. It provides methods for inserting data into the page,
 * retrieving data from the page, and checking if the page contains a given index. It also provides
 * methods for checking if the page is empty, if it fits a given size, and for clearing the page.
 *
 * Each page is divided into three regions: the header region, the offset region, and the data region.
 * The header region is a struct that contains the following fields:
 * - free_region_start: The offset to the start of the free space in the data region.
 * - free_region_end: The offset to the end of the free space in the data region.
 *
 * The offset region is an array of offsets that point to the start of each data entry in the data region.
 * Each offset is a two-byte value that points to the start of a data entry in the data region.
 *
 * The data region contains the actual data entries. Each data entry is stored as a two-byte length followed by the data.
 *
 * The following diagram shows the layout of a page:
 * +-----------------+-----------------+------------------+---------------+
 * | Header Region   | Offset Region   | Unused Space     | Data Region   |
 * +-----------------+-----------------+------------------+---------------+
 *
 * By pointing to offsets in the data region, the page can store variable-length data entries efficiently, without
 * the need to move data around when inserting or deleting entries. However, this design has the drawback that
 * when data is removed, the page may become fragmented, with unused space scattered throughout the data region.
 * This can be mitigated by periodically reorganizing the data in the page to remove the fragmentation.
 */
template<size_t PageSize = 16384>
class page {
public:
    constexpr static size_t page_size = PageSize;
    using page_type = page<page_size>;
    using value_type = uint8_t;
    using const_value = const value_type;
    using view_type = std::span<value_type>;
    using const_view = std::span<const_value>;

    // Offset used for intra-page references
    using intra_offset_t = std::conditional_t<page_size <= 16384, uint16_t, uint32_t>;
    // Offset used for inter-page references
    using inter_offset_t = uint32_t;

    /**
     * @struct header
     * @brief A struct that represents the header of a page.
     */
    struct header {
        intra_offset_t free_region_start;
        intra_offset_t free_region_end;
    };

    constexpr static size_t data_size = PageSize - sizeof(header);

    /**
     * @brief Constructs a new page.
     */
    page() {
        m_header.free_region_start = sizeof(header);
        m_header.free_region_end = data_size;
    }

    /**
     * @brief Checks if the page is empty.
     * @return true if the page is empty, false otherwise.
     */
    [[nodiscard]] bool empty() const {
        return size() == 0;
    }

    /**
     * @brief Checks if the page contains a given index.
     * @param index The index to check.
     * @return true if the page contains the index, false otherwise.
     */
    [[nodiscard]] bool contains(size_t index) const {
        return index < size();
    }

    /**
     * @brief Retrieves a const view of the data at a given index.
     * @param index The index of the data.
     * @return A const view of the data.
     * @throws std::out_of_range If the index is out of range.
     */
    [[nodiscard]] const_view at(size_t index) const {
        if (not contains(index)) {
            throw std::out_of_range("Index out of range");
        }
        auto [size, offset] = size_and_offset_of(index);
        const auto *first = reinterpret_cast<const_value *>(offset);
        const auto *last = std::next(first, size);
        return {first, last};
    }

    /**
     * @brief Retrieves a view of the data at a given index.
     * @param index The index of the data.
     * @return A view of the data.
     */
    view_type at(size_t index) {
        auto view = const_cast<const page *>(this)->at(index);
        return {const_cast<value_type *>(view.data()), view.size()};
    }

    /**
     * @brief Inserts data into the page.
     * @tparam T The type of the data.
     * @param data The data to insert.
     * @return The index of the inserted data.
     * @throws std::bad_alloc If the data does not fit in the page.
     */
    template<Streamable T>
        requires(not std::same_as<T, std::string> and not std::same_as<T, std::string_view> and not std::same_as<T, const char *>)
    size_t insert(T data) {
        std::ostringstream os;
        os << data;
        return insert(os.str());
    }

    /**
     * @brief Inserts a string view into the page.
     * @param data The string view to insert.
     * @return The index of the inserted string view.
     * @throws std::bad_alloc If the string view does not fit in the page.
     */
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

    /**
     * @brief Checks if a given size fits in the page.
     * @param size The size to check.
     * @return true if the size fits in the page, false otherwise.
     */
    [[nodiscard]] bool fits(size_t size) const {
        return free_space() >= size;
    }

    /**
     * @brief Retrieves the size of the page.
     * @return The size of the page.
     */
    [[nodiscard]] size_t size() const {
        return (m_header.free_region_start - sizeof(header)) / sizeof(intra_offset_t);
    }

    /**
     * @brief Clears the page.
     */
    void clear() {
        m_header.free_region_start = sizeof(header);
        m_header.free_region_end = PageSize;
    }

    /**
     * @brief Retrieves the free space in the page.
     * @return The free space in the page.
     */
    [[nodiscard]] size_t free_space() const {
        return m_header.free_region_end - m_header.free_region_start;
    }

    /**
     * @brief Writes the page to an output stream.
     * @param os The output stream.
     * @param page The page to write.
     * @return The output stream.
     */
    friend std::ostream &operator<<(std::ostream &os, const page &page) {
        os.write(reinterpret_cast<const char *>(&page.m_header), sizeof(header));
        os.write(reinterpret_cast<const char *>(page.m_data.data()), page.m_data.size());
        return os;
    }

    /**
     * @brief Reads a page from an input stream.
     * @param is The input stream.
     * @param page The page to read into.
     * @return The input stream.
     */
    friend std::istream &operator>>(std::istream &is, page &page) {
        is.read(reinterpret_cast<char *>(&page.m_header), sizeof(header));
        is.read(reinterpret_cast<char *>(page.m_data.data()), page.m_data.size());
        return is;
    }

private:
    /**
     * @brief Retrieves the size and offset of the data at a given index.
     * @param index The index of the data.
     * @return A pair of the size and offset of the data.
     */
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