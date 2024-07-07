#include "page.hpp"
#include <doctest/doctest.h>

// Make a small page type for testing
using page_t = page<128>;

TEST_CASE("Can create a page") {
    page_t p;
    CHECK(p.size() == 0);
    CHECK(p.empty() == true);
}


TEST_CASE("Can insert data into a page") {
    page_t p;
    p.insert("Hello");
    p.insert("World");
    CHECK(p.size() == 2);
    CHECK(p.empty() == false);
}


TEST_CASE("Can retrieve data from a page") {
    page_t p;
    p.insert("Hello");
    p.insert("World");
    auto first_entry = p.at(0);
    auto second_entry = p.at(1);
    CHECK(std::equal(first_entry.begin(), first_entry.end(), "Hello"));
    CHECK(std::equal(second_entry.begin(), second_entry.end(), "World"));
}


TEST_CASE("Cannot insert data that is too large") {
    page_t p;
    CHECK_THROWS_AS(p.insert(std::string(128, 'a')), std::bad_alloc);
}


TEST_CASE("Check if fittable data fits") {
    page_t p;
    CHECK(p.fits(50) == true);
}


TEST_CASE("Check if non-fittable data fits") {
    page_t p;
    CHECK(p.fits(200) == false);
}


TEST_CASE("Cannot retrieve data that is out of bounds") {
    page_t p;
    CHECK_THROWS_AS(p.at(0), std::out_of_range);
}


TEST_CASE("Contains returns true for valid indices") {
    page_t p;
    p.insert("Hello");
    CHECK(p.contains(0) == true);
}


TEST_CASE("Contains returns false for invalid indices") {
    page_t p;
    CHECK(p.contains(0) == false);
}


TEST_CASE("Retrieved buffer may be modified") {
    page_t p;
    p.insert("Hello");
    auto entry = p.at(0);
    entry[0] = 'J';
    auto modified_entry = p.at(0);
    CHECK(std::equal(modified_entry.begin(), modified_entry.end(), "Jello"));
}


TEST_CASE("Clear page empties it") {
    page_t p;
    p.insert("Hello");
    p.clear();
    CHECK(p.size() == 0);
    CHECK(p.empty() == true);
    CHECK(p.contains(0) == false);
}


TEST_CASE("Serialize and deserialize a page") {
    page_t p;
    p.insert("Hello");
    p.insert("World");
    std::stringstream ss;
    ss << p;
    page_t p2;
    ss >> p2;
    CHECK(p2.size() == 2);
    CHECK(p2.empty() == false);
    auto first_entry = p2.at(0);
    auto second_entry = p2.at(1);
    CHECK(std::equal(first_entry.begin(), first_entry.end(), "Hello"));
    CHECK(std::equal(second_entry.begin(), second_entry.end(), "World"));
}
