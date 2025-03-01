// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "B2DebugDraw.hpp"

namespace BrickOut {

constexpr point_f physicsWorldSize {14.4, 14.4};

B2DDebugDraw::B2DDebugDraw(gfx::font* font)
    : physics::debug_draw {}
    , _font {font}
    , _debugRenderer {_canvas}
{
}

void B2DDebugDraw::draw_polygon(std::span<point_f const> vertices, color color)
{
    _canvas.set_stroke_width(3);
    _canvas.set_stroke_style(color);

    std::vector<point_f> verts;
    for (auto const vert : vertices) {
        verts.push_back(vert * physicsWorldSize);
    }
    _canvas.stoke_poly(verts);
}

void B2DDebugDraw::draw_solid_polygon(physics::body_transform xform, std::span<point_f const> vertices, f32 radius, color color)
{
    _canvas.set_fill_style(color);

    gfx::transform xform2;
    xform2.rotate_at(xform.Angle, xform.Center);

    std::vector<point_f> verts;
    for (auto const vert : vertices) {
        verts.push_back((xform2 * (vert + xform.Center)) * physicsWorldSize);
    }
    _canvas.fill_poly(verts);
}

void B2DDebugDraw::draw_circle(point_f center, f32 radius, color color)
{
    _canvas.set_stroke_width(3);
    _canvas.set_stroke_style(color);
    _canvas.begin_path();
    _canvas.circle(center * physicsWorldSize, radius * physicsWorldSize.X);
    _canvas.stroke();
}

void B2DDebugDraw::draw_solid_circle(physics::body_transform xform, f32 radius, color color)
{
    _canvas.set_fill_style(color);
    _canvas.begin_path();

    _canvas.rotate_at(xform.Angle, xform.Center * physicsWorldSize);
    _canvas.circle(xform.Center * physicsWorldSize, radius * physicsWorldSize.X);
    _canvas.reset_transform();

    _canvas.fill();
}

void B2DDebugDraw::draw_capsule(point_f /* p1 */, point_f /* p2 */, f32 /* radius */, color /* color */)
{
}

void B2DDebugDraw::draw_solid_capsule(point_f p1, point_f p2, f32 radius, color color)
{
    _canvas.set_stroke_width(radius * physicsWorldSize.X * 2);
    _canvas.set_stroke_style(color);
    _canvas.stroke_line(p1 * physicsWorldSize, p2 * physicsWorldSize);

    _canvas.set_fill_style(color);

    _canvas.begin_path();
    _canvas.circle(p1 * physicsWorldSize, radius * physicsWorldSize.X);
    _canvas.circle(p2 * physicsWorldSize, radius * physicsWorldSize.X);
    _canvas.fill();
}

void B2DDebugDraw::draw_segment(point_f p1, point_f p2, color color)
{
    _canvas.set_stroke_width(3);
    _canvas.set_stroke_style(color);
    _canvas.stroke_line(p1 * physicsWorldSize, p2 * physicsWorldSize);
}

void B2DDebugDraw::draw_transform(physics::body_transform const& xf)
{
    _canvas.set_fill_style(colors::DodgerBlue);
    _canvas.begin_path();

    _canvas.arc(xf.Center * physicsWorldSize, 10 * physicsWorldSize.X, radian_f {0}, xf.Angle, gfx::winding::CCW);

    _canvas.fill();
}

void B2DDebugDraw::draw_point(point_f p, f32 size, color color)
{
    _canvas.set_fill_style(color);
    _canvas.begin_path();
    _canvas.circle(p * physicsWorldSize, size);
    _canvas.fill();
}

void B2DDebugDraw::draw_string(point_f p, string const& text)
{
    _canvas.set_font(_font);
    _canvas.set_fill_style(colors::Black);
    _canvas.draw_text({p * physicsWorldSize, {1000, 1000}}, text);
}

void B2DDebugDraw::draw(physics::world const& world, f32 alpha, gfx::render_target& target)
{
    rect_f const bounds {point_f::Zero, size_f {target.Size()}};

    _canvas.begin_frame(target.Size, 1);
    _canvas.set_global_alpha(alpha);
    _canvas.set_fill_style(colors::White);
    _canvas.begin_path();
    _canvas.rect(bounds);
    _canvas.fill();
    world.draw(*this);
    _canvas.end_frame();

    _debugRenderer.set_bounds(bounds);
    _debugRenderer.set_layer(0);
    _debugRenderer.render_to_target(target);
}

}
