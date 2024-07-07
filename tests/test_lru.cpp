#include <doctest/doctest.h>
#include "lru.hpp"

TEST_CASE("LRUCache InsertAndRetrieveValue") {
    lru<int, std::string> cache(2);
    cache.insert(1, "one");
    auto value = cache.get(1);
    CHECK(value != nullptr);
    CHECK(*value == "one");
}

TEST_CASE("LRUCache InsertBeyondCapacity") {
    lru<int, std::string> cache(2);
    cache.insert(1, "one");
    cache.insert(2, "two");
    cache.insert(3, "three");
    CHECK(cache.get(1) == nullptr);
    CHECK(*cache.get(2) == "two");
    CHECK(*cache.get(3) == "three");
}

TEST_CASE("LRUCache InsertPair") {
    lru<int, std::string> cache(2);
    cache.insert({1, "one"});
    auto value = cache.get(1);
    CHECK(value != nullptr);
    CHECK(*value == "one");
}

TEST_CASE("LRUCache RetrieveNonExistentValue") {
    lru<int, std::string> cache(2);
    auto value = cache.get(1);
    CHECK(value == nullptr);
}

TEST_CASE("LRUCache UpdateLRUOrder") {
    lru<int, std::string> cache(2);
    cache.insert(1, "one");     // lru: [(1, one)]
    cache.insert(2, "two");     // lru: [(2, two), (1, one)]
    cache.get(1);               // lru: [(1, one), (2, two)]
    cache.insert(3, "three");   // lru: [(3, three), (1, one)]
    CHECK(cache.get(2) == nullptr);
    CHECK(*cache.get(1) == "one");
    CHECK(*cache.get(3) == "three");
}