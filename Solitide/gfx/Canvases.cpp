// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Canvases.hpp"

#include "CardTable.hpp"
#include "games/Games.hpp"

namespace solitaire {

////////////////////////////////////////////////////////////

foreground_canvas::foreground_canvas(card_table& parent, assets::group& resGrp)
    : _parent {parent}
    , _resGrp {resGrp}
    , _renderer {_canvas}
{
    _hintTimer.Tick.connect([&] {
        _canvasDirty = true;
        _showHint    = false;
    });
}

void foreground_canvas::show_hint()
{
    auto const& hints {_parent.game()->get_available_hints()};
    if (hints.empty()) { return; }

    _currentHint++;
    if (_currentHint >= std::ssize(hints)) { // TODO:reset _currentHint on game change
        _currentHint = 0;
    }

    _hintTimer.start(5s, timer::mode::BusyLoop, false); // Sleep blocks
    _canvasDirty = true;
    _showHint    = true;
}

void foreground_canvas::draw(gfx::render_target& target)
{
    if (target.Size != _bounds.Size) {
        _bounds      = {point_i::Zero, target.Size};
        _canvasDirty = true;
        _renderer.set_bounds(rect_f {_bounds});
    }

    if (_canvasDirty) {
        _canvas.begin_frame(_bounds.Size, 1.0f);

        _canvas.save();
        _canvas.set_scissor(_parent.Bounds);

        draw_hint(target);
        draw_state();

        _canvas.end_frame();

        _canvasDirty = false;
    }

    _renderer.add_layer(0);
    _renderer.render_to_target(target);
}

void foreground_canvas::update(milliseconds)
{
    if (_parent.game()->Status != _lastStatus) {
        _lastStatus = _parent.game()->Status;
        mark_dirty();
    }
}

void foreground_canvas::disable_hint()
{
    _hintTimer.stop();
    mark_dirty();
}

void foreground_canvas::mark_dirty()
{
    _canvasDirty = true;
}

void foreground_canvas::draw_hint(gfx::render_target& target)
{
    if (!_showHint) { return; }

    auto const& hints {_parent.game()->get_available_hints()};
    if (hints.empty()) { return; }
    auto const& hint {hints[_currentHint]};

    rect_f srcBounds {hint.Src->Cards[hint.SrcCardIdx].Bounds};
    for (isize i {hint.SrcCardIdx + 1}; i < std::ssize(hint.Src->Cards); ++i) {
        srcBounds = srcBounds.as_union_with(hint.Src->Cards[i].Bounds);
    }

    rect_f dstBounds;
    if (hint.DstCardIdx >= 0) {
        dstBounds = hint.Dst->Cards[hint.DstCardIdx].Bounds;
    } else {
        if (hint.Dst->HasMarker && hint.Dst->Marker) {
            dstBounds = hint.Dst->Marker->Bounds;
        } else if (!hint.Dst->Cards.empty()) {
            dstBounds = hint.Dst->Cards[0].Bounds;
        }
    }

    auto& camera {target.camera()};

    // Draw bounds
    _canvas.begin_path();
    _canvas.rounded_rect(rect_f {camera.convert_world_to_screen(dstBounds)}, 10);
    _canvas.set_stroke_style(colors::Black);
    _canvas.set_stroke_width(10);
    _canvas.stroke();
    _canvas.set_stroke_style(colors::Red);
    _canvas.set_stroke_width(6);
    _canvas.stroke();

    auto const screenSrcBounds {rect_f {camera.convert_world_to_screen(srcBounds)}};

    _canvas.set_global_composite_blendfunc(gfx::blend_func::One, gfx::blend_func::Zero);
    _canvas.begin_path();
    _canvas.rounded_rect(screenSrcBounds, 10);
    _canvas.set_fill_style(colors::Transparent);
    _canvas.fill();
    _canvas.set_global_composite_blendfunc(gfx::blend_func::SrcAlpha, gfx::blend_func::OneMinusSrcAlpha);

    _canvas.begin_path();
    _canvas.rounded_rect(screenSrcBounds, 10);
    _canvas.set_stroke_style(colors::Black);
    _canvas.set_stroke_width(10);
    _canvas.stroke();
    _canvas.set_stroke_style(colors::Green);
    _canvas.set_stroke_width(6);
    _canvas.stroke();

    // Draw arrow
    auto from {point_f {camera.convert_world_to_screen(srcBounds.center())}};
    auto to {point_f {camera.convert_world_to_screen(dstBounds.center())}};

    f32 const borderWidth {3};
    f32 const arrowWidth {6};
    f32 const headLength {arrowWidth * 6};

    _canvas.set_line_cap(gfx::line_cap::Round);
    _canvas.begin_path();
    _canvas.move_to(from);
    _canvas.line_to(to);

    f32 const angle {std::atan2(to.Y - from.Y, to.X - from.X)};

    _canvas.move_to(to);
    _canvas.line_to({to.X - (headLength * std::cos(angle - (TAU_F / 12))), to.Y - (headLength * std::sin(angle - (TAU_F / 12)))});
    _canvas.move_to(to);
    _canvas.line_to({to.X - (headLength * std::cos(angle + (TAU_F / 12))), to.Y - (headLength * std::sin(angle + (TAU_F / 12)))});
    _canvas.set_stroke_style(colors::Black);
    _canvas.set_stroke_width(arrowWidth + borderWidth);
    _canvas.stroke();
    _canvas.set_stroke_style(colors::Gray);
    _canvas.set_stroke_width(arrowWidth);
    _canvas.stroke();
}

void foreground_canvas::draw_state()
{
    if (_lastStatus != game_status::Success && _lastStatus != game_status::Failure) { return; }

    auto const& pBounds {_parent.Bounds};
    f32 const   size {pBounds->width() / 5};
    rect_f      bounds {pBounds->center() - point_f {size / 2, size / 2}, {size, size}};

    _canvas.set_fill_style(colors::Silver);
    _canvas.begin_path();
    _canvas.rounded_rect(bounds, 15);
    _canvas.fill();
    _canvas.set_stroke_style(colors::Black);
    _canvas.set_stroke_width(5);
    _canvas.stroke();

    bounds = bounds.as_inset_by({bounds.Size.Width / 10, bounds.Size.Height / 10});
    if (_lastStatus == game_status::Success) {
        f32 const width {bounds.width() / 15};
        _canvas.begin_path();
        _canvas.move_to({bounds.left(), bounds.center().Y});
        _canvas.line_to({bounds.center().X, bounds.bottom()});
        _canvas.line_to(bounds.top_right());

        _canvas.set_stroke_width(width * 1.5f);
        _canvas.set_stroke_style(colors::Black);
        _canvas.set_line_cap(gfx::line_cap::Round);
        _canvas.stroke();

        _canvas.set_stroke_width(width);
        _canvas.set_stroke_style(colors::Green);
        _canvas.stroke();
    } else if (_lastStatus == game_status::Failure) {
        f32 const width {bounds.width() / 10};

        _canvas.begin_path();
        _canvas.move_to(bounds.top_left());
        _canvas.line_to(bounds.bottom_right());
        _canvas.move_to(bounds.top_right());
        _canvas.line_to(bounds.bottom_left());

        _canvas.set_stroke_width(width * 1.5f);
        _canvas.set_stroke_style(colors::Black);
        _canvas.set_line_cap(gfx::line_cap::Round);
        _canvas.stroke();

        _canvas.set_stroke_width(width);
        _canvas.set_stroke_style(colors::Red);
        _canvas.stroke();
    }
}

////////////////////////////////////////////////////////////

background_canvas::background_canvas(card_table& parent, assets::group& resGrp)
    : _parent {parent}
    , _resGrp {resGrp}
    , _renderer {_canvas}
{
}

void background_canvas::draw(gfx::render_target& target)
{
    if (target.Size != _bounds.Size) {
        _bounds      = {point_i::Zero, target.Size};
        _canvasDirty = true;
        _renderer.set_bounds(rect_f {_bounds});
    }

    if (_canvasDirty) {
        _canvas.begin_frame(_bounds.Size, 1.0f);

        auto rect {rect_f {_bounds}};
        _canvas.set_fill_style(_canvas.create_linear_gradient(
            rect.top_left() + point_f {0, _bounds.height() * 0.25f},
            rect.bottom_left() - point_f {0, _bounds.height() * 0.25f},
            {_colorA, _colorB}));
        _canvas.begin_path();
        _canvas.rect(rect);
        _canvas.fill();

        _canvas.end_frame();

        _canvasDirty = false;
    }

    _renderer.add_layer(0);
    _renderer.render_to_target(target);
}

void background_canvas::update(milliseconds /* deltaTime */)
{
}

void background_canvas::set_background_colors(color a, color b)
{
    _colorA      = a;
    _colorB      = b;
    _canvasDirty = true;
}

void background_canvas::mark_dirty()
{
    _canvasDirty = true;
}

} // namespace solitaire
