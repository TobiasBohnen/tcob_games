// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

////////////////////////////////////////////////////////////

class cache final {
public:
    explicit cache(size_i screenSize);

    auto screen() -> u32*;

    auto texture(i32 idx, i32 facing) -> u8*;
    auto texture_size(i32 idx, i32 facing) const -> size_i;

    void load();

    static void copy(u32* dst, u8 const* src, i32 srcIdx, f64 darken);

private:
    struct texture_entry {
        usize  Offset {};
        size_i Size {};
    };

    auto get_entry(i32 idx, i32 facing) const -> texture_entry const&;

    static void set(u32* raw, color c, f64 darken);
    static auto get(u8 const* img, usize idx) -> color;

    std::vector<u32> _screen;
    std::vector<u8>  _textures;

    std::unordered_map<i32, std::unordered_map<i32, texture_entry>> _directory {};
};
