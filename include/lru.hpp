//
// Created by Cristian Bosin  on 07/07/24.
//

#ifndef SIMPLEDB_LRU_HPP
#define SIMPLEDB_LRU_HPP

#include <list>
#include <map>
#include <memory>


template<class Key, class Value>
class lru {
public:
    using key_type = Key;
    using value_type = Value;
    using pointer = std::shared_ptr<value_type>;
    using weak_pointer = std::weak_ptr<value_type>;

    explicit lru(size_t size) : m_size(size) {}

    void insert(key_type key, pointer ptr) {
        if (m_list.size() >= m_size) {
            m_list.pop_back();
        }
        m_list.push_front(ptr);
        m_map[key] = ptr;
        m_iter_map[key] = m_list.begin();
    }

    void insert(key_type key, value_type &&value) {
        insert(key, std::make_shared<value_type>(std::move(value)));
    }

    void insert(std::pair<key_type, value_type> &&pair) {
        insert(pair.first, std::move(pair.second));
    }

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
    std::list<pointer> m_list;
    std::map<key_type, weak_pointer> m_map;
    std::map<key_type, list_iterator> m_iter_map;
    size_t m_size;
};


#endif//SIMPLEDB_LRU_HPP
