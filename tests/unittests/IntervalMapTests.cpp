// SPDX-FileCopyrightText: Michael Popoloski
// SPDX-License-Identifier: MIT

#define CATCH_CONFIG_ENABLE_PAIR_STRINGMAKER

#include "Test.h"
#include <fmt/format.h>
#include <random>

#include "slang/util/IntervalMap.h"

TEST_CASE("IntervalMap -- empty map") {
    struct Foo {};
    IntervalMap<int32_t, Foo*> map;

    CHECK(map.empty());
    CHECK(map.begin() == map.begin());
    CHECK(map.end() == map.begin());
    CHECK(map.end() == map.end());
    CHECK(!map.begin().valid());

    CHECK(std::cbegin(map) == map.begin());
    CHECK(std::cend(map) == map.end());
}

TEST_CASE("IntervalMap -- small num elems in root leaf") {
    IntervalMap<int32_t, int32_t> map;
    BumpAllocator ba;
    IntervalMap<int32_t, int32_t>::Allocator alloc(ba);

    map.insert(1, 10, 1, alloc);
    map.insert(3, 7, 2, alloc);
    map.insert(2, 12, 3, alloc);
    map.insert(32, 42, 4, alloc);
    map.insert(3, 6, 5, alloc);

    auto it = map.begin();
    REQUIRE(it != map.end());
    CHECK(it.left() == 1);
    CHECK(it.right() == 10);
    CHECK(*it == 1);

    ++it;
    CHECK(it.left() == 2);
    CHECK(it.right() == 12);

    it++;
    CHECK(it.left() == 3);
    CHECK(it.right() == 6);

    it++;
    CHECK(it.left() == 3);
    CHECK(it.right() == 7);

    --it;
    CHECK(it.right() == 6);

    it--;
    CHECK(it.left() == 2);
    CHECK(*it == 3);

    CHECK(map.getBounds() == std::pair{1, 42});
}

TEST_CASE("IntervalMap -- branching inserts") {
    IntervalMap<int32_t, int32_t> map;
    BumpAllocator ba;
    IntervalMap<int32_t, int32_t>::Allocator alloc(ba);

    // Insert a bunch of elements to force branching.
    for (int32_t i = 1; i < 1000; i++) {
        map.insert(10 * i, 10 * i + 5, i, alloc);
        CHECK(map.getBounds() == std::pair{10, 10 * i + 5});
    }

    CHECK(!map.empty());
    CHECK(map.getBounds() == std::pair{10, 9995});

    auto it = map.begin();
    for (uint32_t i = 1; i < 1000; i++) {
        CHECK(it.valid());
        CHECK(it.left() == 10 * i);
        CHECK(it.right() == 10 * i + 5);
        CHECK(*it == i);
        it++;
    }

    CHECK(!it.valid());
    CHECK(it == map.end());

    for (uint32_t i = 999; i; --i) {
        --it;
        CHECK(it.valid());
        CHECK(it.left() == 10 * i);
        CHECK(it.right() == 10 * i + 5);
        CHECK(*it == i);
    }
    CHECK(it == map.begin());

    // Insert more intervals in the middle.
    for (int32_t i = 0; i < 100; i++)
        map.insert(11 * i, 11 * i + i, i, alloc);

    // Insert a bunch of psuedo-random intervals.
    std::mt19937 mt;
    std::uniform_int_distribution dist;
    using PT = std::uniform_int_distribution<int>::param_type;
    for (int32_t i = 0; i < 1000; i++) {
        int32_t left = dist(mt, PT{1, 10000});
        int32_t right = dist(mt, PT{left, 10000});
        map.insert(left, right, i, alloc);
    }

    map.verify();
}
