// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../_common/Common.hpp"
#include "Textures.hpp"

////////////////////////////////////////////////////////////

struct texture_entry {
    usize  Offset {};
    size_i Size {};
};

class cache final {
public:
    explicit cache(size_i screenSize);

    auto screen() -> u32*;

    auto texture(i32 idx) -> u8*;
    auto texture_size(i32 idx) const -> size_i;

    void load();

    static void copy(u32* dst, u8 const* src, i32 srcIdx, bool darken);

private:
    static void set(u32* raw, color c, bool darken);
    static auto get(u8 const* img, usize idx) -> color;

    std::vector<u32> _screen;

    std::vector<u8>                         _textures;  // contiguous storage for all texture pixel data
    std::array<texture_entry, textureCount> _directory; // per-texture {offset, size} into _textures
};
