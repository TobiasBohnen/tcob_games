// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

////////////////////////////////////////////////////////////

inline constexpr i32 sprite1Texture {11};
inline constexpr i32 door1Texture {12};
inline constexpr i32 door1FrameTexture {13};

////////////////////////////////////////////////////////////

class texture_cache final {
public:
    auto texture(i32 idx, i32 facing) -> u8*;
    auto texture_size(i32 idx, i32 facing) const -> size_i;

    void load();

private:
    struct texture_entry {
        usize  Offset {};
        size_i Size {};
    };

    auto get_entry(i32 idx, i32 facing) const -> texture_entry const&;

    std::vector<u8> _textures;

    std::unordered_map<i32, std::unordered_map<i32, texture_entry>> _directory {};
};
