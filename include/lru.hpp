//
// Created by Cristian Bosin  on 07/07/24.
//

#ifndef SIMPLEDB_LRU_HPP
#define SIMPLEDB_LRU_HPP

#include <list>
#include <map>
#include <memory>

/**
 * @file lru.hpp
 * @brief Defines a Least Recently Used (LRU) cache.
 */

/**
 * @class lru
 * @tparam Key The type of the keys in the cache.
 * @tparam Value The type of the values in the cache.
 * @brief A Least Recently Used (LRU) cache.
 *
 * This class implements a Least Recently Used (LRU) cache. The cache has a fixed size,
 * specified at construction time. When the cache is full and a new item is inserted,
 * the least recently used item is removed from the cache.
 */
template<class Key, class Value>
class lru {
public:
    using key_type = Key; ///< The type of the keys in the cache.
    using value_type = Value; ///< The type of the values in the cache.
    using pointer = std::shared_ptr<value_type>; ///< A shared pointer to a value.
    using weak_pointer = std::weak_ptr<value_type>; ///< A weak pointer to a value.

    /**
     * @brief Constructs a new LRU cache.
     * @param size The size of the cache.
     */
    explicit lru(size_t size) : m_size(size) {}

    /**
     * @brief Inserts a new item into the cache.
     * @param key The key of the item.
     * @param ptr A shared pointer to the item.
     */
    void insert(key_type key, pointer ptr) {
        if (m_list.size() >= m_size) {
            m_list.pop_back();
        }
        m_list.push_front(ptr);
        m_map[key] = ptr;
        m_iter_map[key] = m_list.begin();
    }

    /**
     * @brief Inserts a new item into the cache.
     * @param key The key of the item.
     * @param value The value of the item.
     */
    void insert(key_type key, value_type &&value) {
        insert(key, std::make_shared<value_type>(std::move(value)));
    }

    /**
     * @brief Inserts a new item into the cache.
     * @param pair The key-value pair of the item.
     */
    void insert(std::pair<key_type, value_type> &&pair) {
        insert(pair.first, std::move(pair.second));
    }

    /**
     * @brief Retrieves an item from the cache.
     * @param key The key of the item.
     * @return A shared pointer to the item if it exists in the cache, otherwise nullptr.
     */
    pointer get(key_type key) {
        auto it = m_map.find(key);
        if (it == m_map.end()) {
            return nullptr;
        }

        pointer ptr = it->second.lock();
        if (!ptr) {
            m_map.erase(it);
            m_iter_map.erase(key);
            return nullptr;
        }

        auto list_iter = m_iter_map[key];
        m_list.splice(m_list.begin(), m_list, list_iter);
        return ptr;
    }

private:
    using list_iterator = typename std::list<pointer>::iterator;
    std::list<pointer> m_list; ///< A list of pointers to the items in the cache.
    std::map<key_type, weak_pointer> m_map; ///< A map from keys to weak pointers to the items in the cache.
    std::map<key_type, list_iterator> m_iter_map; ///< A map from keys to iterators to the items in the cache.
    size_t m_size; ///< The size of the cache.
};


#endif//SIMPLEDB_LRU_HPP