#include "Cardset.hpp"

#include <ranges>

namespace solitaire {

auto static get_rank_symbol(rank r) -> std::string
{
    switch (r) {
    case rank::Ace: return "A";
    case rank::Two: return "2";
    case rank::Three: return "3";
    case rank::Four: return "4";
    case rank::Five: return "5";
    case rank::Six: return "6";
    case rank::Seven: return "7";
    case rank::Eight: return "8";
    case rank::Nine: return "9";
    case rank::Ten: return "10";
    case rank::Jack: return "J";
    case rank::Queen: return "Q";
    case rank::King: return "K";
    }

    return "";
}

static constexpr color backColor {colors::SeaShell};

cardset::cardset(std::string const& matName, assets::group& resGrp)
{
    i32 const columns {10};
    i32 const rows {8};
    _texSize = {240, 360};

    size_f const canvasSize {_texSize.Width * columns, _texSize.Height * rows};
    _cardPad = _texSize / 50;

    auto canvasMat {resGrp.get_bucket<gfx::material>()->create_or_get(matName, nullptr)};
    _canvas = std::make_shared<gfx::canvas>();
    _canvas->begin_frame(size_i {canvasSize}, 1.0f);

    auto fontFamily {resGrp.get<gfx::font_family>("Poppins")};
    _normalFont = fontFamily->get_font({false, gfx::font::weight::Normal}, static_cast<u32>(_texSize.Height / 7.5f)).get_obj();
    _largeFont  = fontFamily->get_font({false, gfx::font::weight::Bold}, static_cast<u32>(_texSize.Height / 2)).get_obj();

    _canvas->set_text_halign(gfx::horizontal_alignment::Centered);
    _canvas->set_text_valign(gfx::vertical_alignment::Middle);

    auto tex {_canvas->get_texture()};
    tex->Filtering = gfx::texture::filtering::Linear;

    i32  i {0};
    auto nextRect {[&]() {
        f32 const x {static_cast<f32>(i % columns * _texSize.Width)};
        f32 const y {static_cast<f32>(i / columns * _texSize.Height)};
        ++i;
        return rect_f {{x, y}, _texSize};
    }};

    auto addRegion {[&](std::string const& texName, rect_f const& rect) {
        tex->add_region(texName, {{rect.X / canvasSize.Width, -rect.Y / canvasSize.Height, rect.Width / canvasSize.Width, -rect.Height / canvasSize.Height}, 0});
    }};

    // draw cards
    for (u8 cs {static_cast<u8>(suit::Hearts)}; cs <= static_cast<i32>(suit::Spades); ++cs) {
        for (u8 cr {static_cast<u8>(rank::Ace)}; cr <= static_cast<i32>(rank::King); ++cr) {
            rect_f const rect {nextRect()};
            card const   c {static_cast<suit>(cs), static_cast<rank>(cr), 0, false};
            draw_card(c.get_suit(), c.get_rank(), rect);
            addRegion(c.get_texture_name(), rect);
        }
    }

    // draw back
    {
        rect_f const rect {nextRect()};
        draw_back(rect);
        addRegion("card_back", rect);
    }

    // draw foundation/tableau marker
    {
        rect_f const rect {nextRect()};
        draw_shape(rect.as_shrunk(_cardPad), backColor, colors::Black);

        _canvas->set_fill_style(colors::Green);
        _canvas->begin_path();
        _canvas->star(rect.get_center(), rect.Width / 2, rect.Width / 4, 5);
        _canvas->fill();
        addRegion("card_base_gen", rect);
    }
    {
        rect_f const rect {nextRect()};
        draw_marker(rank::Ace, rect);
        addRegion("card_base_ace", rect);
    }
    {
        rect_f const rect {nextRect()};
        draw_marker(rank::King, rect);
        addRegion("card_base_king", rect);
    }

    // draw empty
    {
        rect_f const rect {nextRect()};
        draw_shape(rect.as_shrunk(_cardPad), backColor, colors::Black);
        addRegion("card_empty", rect);
    }

    _canvas->end_frame();
    _texture           = tex.get_ptr();
    _canvas            = nullptr;
    canvasMat->Texture = tex;
}

void cardset::draw_card(suit s, rank r, rect_f const& rect)
{
    _canvas->save();

    rect_f const  cardRect {rect.as_shrunk(_cardPad)};
    point_f const cardCenter {cardRect.get_center()};

    draw_shape(cardRect, backColor, colors::Black);

    point_f offset {rect.X + 6, rect.Y + 1};

    {
        set_suit_color(s);
        _canvas->set_font(_normalFont);
        std::string const rankSymbol {get_rank_symbol(r)};
        auto const        rankSize {_canvas->measure_text(rect.Height, rankSymbol)};
        rect_f const      rankRect {offset, rankSize};

        f32 const suitSize {_texSize.Width / 8};
        _canvas->draw_textbox(rankRect, rankSymbol);
        draw_suit(s, {offset.X + suitSize / 2, offset.Y + _normalFont->get_info().LineHeight + suitSize / 2}, suitSize);
        _canvas->rotate_at({180}, cardCenter);
        _canvas->draw_textbox(rankRect, rankSymbol);
        draw_suit(s, {offset.X + suitSize / 2, offset.Y + _normalFont->get_info().LineHeight + suitSize / 2}, suitSize);
        _canvas->reset_transform();
    }
    {
        f32 const suitSize {_texSize.Width / 6};
        f32 const centerX {cardCenter.X};
        f32 const centerY {cardCenter.Y};
        f32 const left {centerX - cardRect.Width / 4};
        f32 const right {centerX + cardRect.Width / 4};
        f32 const top {cardRect.Y + suitSize * 2};
        f32 const centerTop {top + suitSize};

        f32 const ten {top + suitSize / 2};
        f32 const nine {ten + suitSize};

        auto drawTwo {[&]() {
            draw_suit(s, {centerX, top}, suitSize);
            _canvas->rotate_at({180}, cardCenter);
            draw_suit(s, {centerX, top}, suitSize);
            _canvas->reset_transform();
        }};

        auto drawFour {[&]() {
            draw_suit(s, {left, top}, suitSize);
            draw_suit(s, {right, top}, suitSize);
            _canvas->rotate_at({180}, cardCenter);
            draw_suit(s, {left, top}, suitSize);

            draw_suit(s, {right, top}, suitSize);
            _canvas->reset_transform();
        }};

        auto drawSix {[&]() {
            drawFour();
            draw_suit(s, {left, centerY}, suitSize);
            draw_suit(s, {right, centerY}, suitSize);
        }};

        switch (r) {
        case rank::Ace: {
            draw_suit(s, cardCenter, suitSize * 2);
        } break;
        case rank::Two: {
            drawTwo();
        } break;
        case rank::Three: {
            drawTwo();
            draw_suit(s, cardCenter, suitSize);
        } break;
        case rank::Four: {
            drawFour();
        } break;
        case rank::Five: {
            drawFour();
            draw_suit(s, cardCenter, suitSize);
        } break;
        case rank::Six: {
            drawSix();
        } break;
        case rank::Seven: {
            drawSix();
            draw_suit(s, {centerX, centerTop}, suitSize);
        } break;
        case rank::Eight: {
            drawSix();
            draw_suit(s, {centerX, centerTop}, suitSize);
            _canvas->rotate_at({180}, cardCenter);
            draw_suit(s, {centerX, centerTop}, suitSize);
            _canvas->reset_transform();
        } break;
        case rank::Nine: {
            drawFour();
            draw_suit(s, {left, nine}, suitSize);
            draw_suit(s, {right, nine}, suitSize);
            draw_suit(s, cardCenter, suitSize);
            _canvas->rotate_at({180}, cardCenter);
            draw_suit(s, {left, nine}, suitSize);
            draw_suit(s, {right, nine}, suitSize);
            _canvas->reset_transform();
        } break;
        case rank::Ten: {
            drawFour();
            draw_suit(s, {left, nine}, suitSize);
            draw_suit(s, {right, nine}, suitSize);
            draw_suit(s, {centerX, ten}, suitSize);
            _canvas->rotate_at({180}, cardCenter);
            draw_suit(s, {left, nine}, suitSize);
            draw_suit(s, {right, nine}, suitSize);
            draw_suit(s, {centerX, ten}, suitSize);
            _canvas->reset_transform();
        } break;

        case rank::Jack:
        case rank::Queen:
        case rank::King: {
            set_suit_color(s);
            _canvas->set_font(_largeFont);
            std::string const rankSymbol {get_rank_symbol(r)};
            _canvas->draw_textbox(rect.as_shrunk({_cardPad.Width, _cardPad.Height + _largeFont->get_info().Descender}), rankSymbol);
        } break;
        }
    }

    _canvas->restore();
}

void cardset::draw_marker(rank r, rect_f const& rect)
{
    _canvas->save();

    draw_shape(rect.as_shrunk(_cardPad), backColor, colors::Black);

    _canvas->set_fill_style(colors::Green);
    _canvas->set_font(_largeFont);
    std::string const rankSymbol {get_rank_symbol(r)};
    _canvas->draw_textbox(rect.as_shrunk({_cardPad.Width, _cardPad.Height + _largeFont->get_info().Descender}), rankSymbol);

    _canvas->restore();
}

void cardset::draw_back(rect_f const& rect)
{
    _canvas->save();
    draw_shape(rect.as_shrunk(_cardPad), colors::LightSteelBlue, colors::White);

    rect_f const backRect {rect.as_shrunk(_cardPad * 4)};

    _canvas->set_scissor(backRect);

    f32 const rhombusSize {backRect.Width / 16.f};
    f32 constexpr sqrt3 {std::numbers::sqrt3_v<f32>};

    for (f32 row {backRect.Y}; row < backRect.bottom(); row += rhombusSize * sqrt3) {
        for (f32 col {backRect.X}; col < backRect.right(); col += rhombusSize * 2) {
            f32 const x1 {col};
            f32 const y1 {row + rhombusSize * sqrt3 / 2};
            f32 const x2 {col + rhombusSize};
            f32 const y2 {row};
            f32 const x3 {col + rhombusSize * 2};
            f32 const y3 {y1};
            f32 const x4 {x2};
            f32 const y4 {row + rhombusSize * sqrt3};

            color const color {static_cast<i32>(row / (rhombusSize * sqrt3) + col / (rhombusSize * 2)) % 2 == 0 ? colors::LightBlue : colors::LightGray};

            _canvas->set_fill_style(color);
            _canvas->begin_path();
            _canvas->move_to({x1, y1});
            _canvas->line_to({x2, y2});
            _canvas->line_to({x3, y3});
            _canvas->line_to({x4, y4});
            _canvas->close_path();
            _canvas->fill();
            _canvas->set_stroke_style(backColor);
            _canvas->set_stroke_width(3);
            _canvas->stroke();
        }
    }

    _canvas->set_stroke_style(colors::White);
    _canvas->set_stroke_width(3);
    _canvas->stroke_rect(backRect);

    _canvas->restore();
}

void cardset::set_suit_color(suit s)
{
    switch (s) {
    case suit::Diamonds:
    case suit::Hearts:
        _canvas->set_fill_style(colors::Red);
        break;
    case suit::Clubs:
    case suit::Spades:
        _canvas->set_fill_style(colors::Black);
        break;
    }
}

void cardset::draw_suit(suit s, point_f pos, f32 size)
{
    auto draw {[&](std::vector<std::vector<point_f>> const& halfPath) {
        _canvas->save();
        _canvas->translate(pos);
        _canvas->scale({size, size});
        _canvas->begin_path();

        _canvas->move_to(halfPath[0][0]);
        for (auto const& path : halfPath) {
            _canvas->cubic_bezier_to(path[1], path[2], path[3]);
        }
        for (auto const& path : halfPath | std::views::reverse) {
            _canvas->cubic_bezier_to({-path[2].X, path[2].Y}, {-path[1].X, path[1].Y}, {-path[0].X, path[0].Y});
        }

        set_suit_color(s);
        _canvas->fill();

        _canvas->set_stroke_style(colors::DarkGray);
        _canvas->set_stroke_width(0.04f);
        _canvas->stroke();

        _canvas->restore();
    }};

    f32 const n {0.5f};
    switch (s) {
    case suit::Hearts: {
        draw({{{0, n}, {0, n}, {n, 0}, {n, -n / 2}},
              {{n, -n / 2}, {n, -n / 2}, {n, -n}, {n / 2, -n}},
              {{n / 2, -n}, {n / 2, -n}, {0, -n}, {0, -n / 2}}});
    } break;
    case suit::Diamonds: {
        draw({{{0, n}, {0, n}, {3 * n / 4, 0}, {3 * n / 4, 0}},
              {{3 * n / 4, 0}, {3 * n / 4, 0}, {0, -n}, {0, -n}}});
    } break;
    case suit::Clubs: {
        draw({{{0, -n}, {0, -n}, {n / 2, -n}, {n / 2, -n / 2}},
              {{n / 2, -n / 2}, {n / 2, -n / 2}, {n, -n / 2}, {n, 0}},
              {{n, 0}, {n, 0}, {n, n / 2}, {n / 2, n / 2}},
              {{n / 2, n / 2}, {n / 2, n / 2}, {n / 8, n / 2}, {n / 8, n / 8}},
              {{n / 8, n / 8}, {n / 8, n / 2}, {n / 2, n}, {n / 2, n}},
              {{n / 2, n}, {n / 2, n}, {0, n}, {0, n}}});
    } break;
    case suit::Spades: {
        draw({{{0, -n}, {0, -n}, {n, -n / 2}, {n, 0}},
              {{n, 0}, {n, 0}, {n, n / 2}, {n / 2, n / 2}},
              {{n / 2, n / 2}, {n / 2, n / 2}, {n / 8, n / 2}, {n / 8, n / 8}},
              {{n / 8, n / 8}, {n / 8, n / 2}, {n / 2, n}, {n / 2, n}},
              {{n / 2, n}, {n / 2, n}, {0, n}, {0, n}}});
    } break;
    }
}

void cardset::draw_shape(rect_f const& bounds, color fill, color stroke)
{
    _canvas->set_fill_style(fill);
    _canvas->set_stroke_style(stroke);
    _canvas->set_stroke_width(3);
    _canvas->begin_path();
    _canvas->rounded_rect(bounds, 15);
    _canvas->fill();
    _canvas->stroke();
}

} // namespace solitaire
