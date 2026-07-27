// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

////////////////////////////////////////////////////////////

// PLACEHOLDER START
inline constexpr i32 sprite1Texture {999};
inline constexpr i32 door1Texture {444};
inline constexpr i32 door1FrameTexture {445};
inline constexpr i32 handTexture {666};
// PLACEHOLDER END

////////////////////////////////////////////////////////////

class texture_cache final {
public:
    auto texture(i32 idx, i32 variant) -> u8*;
    auto texture_size(i32 idx, i32 variant) const -> size_i;

    void load();

private:
    struct texture_entry {
        usize  Offset {};
        size_i Size {};
    };

    auto get_entry(i32 idx, i32 variant) const -> texture_entry const&;

    std::vector<u8> _textures;

    std::unordered_map<i32, std::unordered_map<i32, texture_entry>> _directory {};
};
