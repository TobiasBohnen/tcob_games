// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <vector>

#include "Common.hpp"

struct mote_settings {
    u32                  Count {0};         // motes per cell; 0 disables the pass
    f64                  FallSpeed {0.0};   // cell heights per second; negative rises
    f64                  SpeedJitter {0.0}; // per-mote speed variation, 0..1
    point_d              Wind {0.0, 0.0};   // constant drift in cells per second; motes wrap inside their cell
    f64                  Drift {0.0};       // wobble radius in world units
    f64                  DriftSpeed {1.0};  // wobble speed
    f64                  MinHeight {0.0};   // vertical band as a fraction of the cell height
    f64                  MaxHeight {1.0};
    f64                  Size {0.0};        // world units; 0 = single pixel
    f64                  Length {0.0};      // streak length in world units, trailing upward; 0 = dot
    f64                  Alpha {1.0};
    f64                  Twinkle {0.0};     // 0..1 depth of the brightness pulse
    f64                  TwinkleSpeed {1.0};
    f64                  Emissive {0.0};    // 0 = fully lit by the level's lights, 1 = self-lit
    bool                 Additive {false};  // glow instead of alpha blend
    std::vector<color_d> Colors {};         // each mote picks a random blend along this list; empty = white
};

class motes {
public:
    auto at(u8 idx) const -> mote_settings const&
    {
        return _presets[idx < _presets.size() ? idx : 0]; // out-of-range indices fall back to preset 0
    }

    void set(u8 idx, mote_settings const& preset)
    {
        if (idx >= _presets.size()) { _presets.resize(static_cast<usize>(idx) + 1); }
        _presets[idx] = preset;
    }

private:
    static constexpr usize INITIAL_PRESETS {16};

    std::vector<mote_settings> _presets {INITIAL_PRESETS};
};
