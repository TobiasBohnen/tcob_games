// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "BrickOut.hpp" // IWYU pragma: keep"

////////////////////////////////////////////////////////////

namespace BrickOut {

enum class debug_mode {
    Off,
    Transparent,
    On
};

class B2DDebugDraw : public physics::debug_draw {
public:
    B2DDebugDraw(gfx::font* font);

    /// Draw a closed polygon provided in CCW order.
    void draw_polygon(std::span<point_f const> vertices, color color) override;

    /// Draw a solid closed polygon provided in CCW order.
    void draw_solid_polygon(physics::body_transform xform, std::span<point_f const> vertices, f32 radius, color color) override;

    /// Draw a circle.
    void draw_circle(point_f center, f32 radius, color color) override;

    /// Draw a solid circle.
    void draw_solid_circle(physics::body_transform xform, f32 radius, color color) override;

    void draw_solid_capsule(point_f p1, point_f p2, f32 radius, color color) override;

    /// Draw a line segment.
    void draw_segment(point_f p1, point_f p2, color color) override;

    /// Draw a transform. Choose your own length scale.
    /// @param xf a transform.
    void draw_transform(physics::body_transform const& xf) override;

    /// Draw a point.
    void draw_point(point_f p, f32 size, color color) override;

    void draw_string(point_f p, string const& text, color color) override;

    void draw(physics::world const& world, f32 alpha, gfx::render_target& target);

private:
    gfx::font*           _font;
    gfx::canvas          _canvas;
    gfx::canvas_renderer _debugRenderer;
};

}
