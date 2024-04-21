// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Canvases.hpp"

#include "CardTable.hpp"
#include "Games.hpp"

namespace solitaire {

////////////////////////////////////////////////////////////

foreground_canvas::foreground_canvas(card_table& parent, gfx::ui::canvas_widget* canvas, assets::group& resGrp)
    : _parent {parent}
    , _canvas {canvas}
    , _resGrp {resGrp}
{
    _hintTimer.Tick.connect([&](auto&&) {
        _canvasDirty = true;
        _showHint    = false;
        _hintTimer.stop();
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

    _hintTimer.start(5s, timer::mode::BusyLoop);
    _canvasDirty = true;
    _showHint    = true;
}

void foreground_canvas::draw(gfx::render_target& target)
{
    if (_canvasDirty) {
        _canvas->clear();

        _canvas->save();
        _canvas->translate(-_parent.Bounds->get_position());
        _canvas->set_scissor(_parent.Bounds);

        draw_hint(target);
        draw_state();

        _canvas->restore();
        _canvasDirty = false;
    }
}

void foreground_canvas::update(milliseconds)
{
    if (_parent.game()->State != _lastState) {
        _lastState = _parent.game()->State;
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
        srcBounds = srcBounds.as_merged(hint.Src->Cards[i].Bounds);
    }

    rect_f dstBounds;
    if (hint.DstCardIdx >= 0) {
        dstBounds = hint.Dst->Cards[hint.DstCardIdx].Bounds;
    } else if (hint.Dst->Marker) {
        dstBounds = hint.Dst->Marker->Bounds();
    }

    auto& camera {*target.Camera};

    // Draw bounds
    _canvas->begin_path();
    _canvas->rounded_rect(rect_f {camera.convert_world_to_screen(dstBounds)}, 10);
    _canvas->set_stroke_style(colors::Red);
    _canvas->set_stroke_width(10);
    _canvas->stroke();

    auto const screenSrcBounds {rect_f {camera.convert_world_to_screen(srcBounds)}};

    _canvas->set_global_composite_blendfunc(gfx::blend_func::One, gfx::blend_func::Zero);
    _canvas->begin_path();
    _canvas->rounded_rect(screenSrcBounds, 10);
    _canvas->set_fill_style(colors::Transparent);
    _canvas->fill();
    _canvas->set_global_composite_blendfunc(gfx::blend_func::SrcAlpha, gfx::blend_func::OneMinusSrcAlpha);

    _canvas->begin_path();
    _canvas->rounded_rect(screenSrcBounds, 10);
    _canvas->set_stroke_style(colors::Green);
    _canvas->set_stroke_width(10);
    _canvas->stroke();

    // Draw arrow
    auto from {point_f {camera.convert_world_to_screen(srcBounds.get_center())}};
    auto to {point_f {camera.convert_world_to_screen(dstBounds.get_center())}};

    f32 const borderWidth {3};
    f32 const arrowWidth {6};
    f32 const headLength {arrowWidth * 6};

    _canvas->set_line_cap(gfx::line_cap::Round);
    _canvas->begin_path();
    _canvas->move_to(from);
    _canvas->line_to(to);

    f32 const angle {std::atan2(to.Y - from.Y, to.X - from.X)};

    _canvas->move_to(to);
    _canvas->line_to({to.X - headLength * std::cos(angle - TAU_F / 12), to.Y - headLength * std::sin(angle - TAU_F / 12)});
    _canvas->move_to(to);
    _canvas->line_to({to.X - headLength * std::cos(angle + TAU_F / 12), to.Y - headLength * std::sin(angle + TAU_F / 12)});
    _canvas->set_stroke_style(colors::Black);
    _canvas->set_stroke_width(arrowWidth + borderWidth);
    _canvas->stroke();
    _canvas->set_stroke_style(colors::Gray);
    _canvas->set_stroke_width(arrowWidth);
    _canvas->stroke();
}

void foreground_canvas::draw_state()
{
    _canvas->set_font(_resGrp.get<gfx::font_family>("Poppins")->get_font({}, 256).get_obj()); // TODO: measure
    _canvas->set_fill_style(colors::IndianRed);
    _canvas->set_text_halign(gfx::horizontal_alignment::Centered);
    _canvas->set_text_valign(gfx::vertical_alignment::Middle);

    if (_lastState == game_state::Success) {
        _canvas->draw_textbox(_parent.Bounds, "Success!");
    } else if (_lastState == game_state::Failure) {
        _canvas->draw_textbox(_parent.Bounds, "Failure!");
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
    if (target.Size != _bounds.get_size()) {
        _bounds      = {point_i::Zero, target.Size()};
        _canvasDirty = true;
        _renderer.set_bounds(rect_f {_bounds});
    }

    if (_canvasDirty) {
        _canvas.begin_frame(_bounds.get_size(), 1.0f);

        _canvas.set_fill_style(colors::Gray);
        _canvas.fill_rect(rect_f {_bounds});

        static constexpr std::array<pile_type, 6> drawOrder {pile_type::Tableau, pile_type::Foundation, pile_type::Reserve, pile_type::FreeCell, pile_type::Waste, pile_type::Stock};
        auto const&                               gpiles {_parent.game()->piles()};
        for (auto const pileType : drawOrder) {
            if (!gpiles.contains(pileType)) { continue; }

            rect_f      pileBounds;
            auto const& piles {gpiles.at(pileType)};
            for (auto* pile : piles) {
                pileBounds = pileBounds == rect_f::Zero ? pile->Bounds : pileBounds.as_merged(pile->Bounds);
                if (pile->HasMarker) {
                    pileBounds = pileBounds == rect_f::Zero ? pile->Marker->Bounds : pileBounds.as_merged(pile->Marker->Bounds);
                }
            }
            auto srcpileBounds {rect_f {(*target.Camera).convert_world_to_screen(pileBounds)}};
            // srcpileBounds = srcpileBounds.as_padded({-5.f, -5.f});

            _canvas.begin_path();
            _canvas.rounded_rect(srcpileBounds, 5);

            switch (pileType) {
            case pile_type::Stock: _canvas.set_fill_style(colors::Red); break;
            case pile_type::Waste: _canvas.set_fill_style(colors::Brown); break;
            case pile_type::Foundation: _canvas.set_fill_style(colors::White); break;
            case pile_type::Tableau: _canvas.set_fill_style(colors::Green); break;
            case pile_type::Reserve: _canvas.set_fill_style(colors::LightGray); break;
            case pile_type::FreeCell: _canvas.set_fill_style(colors::Blue); break;
            }
            _canvas.fill();

            _canvas.set_stroke_style(colors::Black);
            _canvas.set_stroke_width(2);
            _canvas.stroke();
        }

        _canvas.end_frame();

        _canvasDirty = false;
    }

    _renderer.set_layer(0);
    _renderer.render_to_target(target);
}

void background_canvas::update(milliseconds /* deltaTime */)
{
}

void background_canvas::mark_dirty()
{
    _canvasDirty = true;
}

} // namespace solitaire
