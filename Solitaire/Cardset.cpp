#include "Cardset.hpp"

#include <ranges>

namespace solitaire {

static constexpr color  CardsetBackColor {colors::SeaShell};
static string const     CardsetFolder {"/cardsets/cards1/"};
static constexpr isize  CardsetCardCount {68};
static constexpr size_f CardsetTexSize {240, 360};

cardset::cardset(std::string const& matName, assets::group& resGrp)
{
    if (!load()) { create(resGrp, CardsetTexSize); }

    auto mat {resGrp.get_bucket<gfx::material>()->create_or_get(matName, nullptr)};
    mat->Texture = _texture;
}

auto cardset::get_card_size() -> size_f
{
    return CardsetTexSize;
}

auto cardset::load() const -> bool
{
    auto files {io::enumerate(CardsetFolder, {"*card*.png"}, false)};
    if (files.size() < CardsetCardCount) { return false; }

    auto const info {gfx::image::LoadInfo(*files.begin())};
    if (!info) { return false; }
    auto const texSize {gfx::image::LoadInfo(*files.begin())->Size};

    _texture->create(texSize, static_cast<u32>(files.size()), gfx::texture::format::RGBA8);
    _texture->Filtering = gfx::texture::filtering::Linear;

    u32  level {0};
    auto tempImg {gfx::image::CreateEmpty(texSize, gfx::image::format::RGBA)};
    for (auto const& file : files) {
        if (tempImg.load(file) != load_status::Ok) { return false; }
        _texture->update_data(tempImg.get_data(), level);
        _texture->add_region(io::get_stem(file), {{0, 0, 1, 1}, level++});
    }

    return true;
}

void cardset::create(assets::group& resGrp, size_f texSize)
{
    i32 const columns {10};
    i32 const rows {8};
    static_assert(columns * rows >= CardsetCardCount);
    size_f const canvasSize {texSize.Width * columns, texSize.Height * rows};

    gfx::canvas canvas;
    canvas.begin_frame(size_i {canvasSize}, 1.0f);

    auto        fontFamily {resGrp.get<gfx::font_family>("Poppins")};
    fonts const fonts {.NormalFont = fontFamily->get_font({false, gfx::font::weight::Normal}, static_cast<u32>(texSize.Height / 7.5f)).get_obj(),
                       .LargeFont  = fontFamily->get_font({false, gfx::font::weight::Bold}, static_cast<u32>(texSize.Height / 2)).get_obj()};

    canvas.set_text_halign(gfx::horizontal_alignment::Centered);
    canvas.set_text_valign(gfx::vertical_alignment::Middle);

    auto tempTex {canvas.get_texture()};
    tempTex->Filtering = gfx::texture::filtering::Linear;

    i32  i {0};
    auto nextRect {[&]() {
        f32 const x {static_cast<f32>(i % columns * texSize.Width)};
        f32 const y {static_cast<f32>(i / columns * texSize.Height)};
        ++i;
        return rect_f {{x, y}, texSize};
    }};

    auto addRegion {[&](std::string const& texName, rect_f const& rect) {
        tempTex->add_region(texName, {rect, 0});
    }};

    // draw cards
    for (u8 cs {static_cast<u8>(suit::Hearts)}; cs <= static_cast<u8>(suit::Spades); ++cs) {
        for (u8 cr {static_cast<u8>(rank::Ace)}; cr <= static_cast<u8>(rank::King); ++cr) {
            rect_f const rect {nextRect()};
            card const   c {static_cast<suit>(cs), static_cast<rank>(cr), 0, false};
            draw_card(canvas, fonts, c.get_suit(), c.get_rank(), rect);
            addRegion(c.get_texture_name(), rect);
        }
    }

    // draw back
    {
        rect_f const rect {nextRect()};
        draw_back(canvas, rect);
        addRegion("card_back", rect);
    }

    // draw foundation/tableau marker
    {
        rect_f const rect {nextRect()};
        draw_shape(canvas, pad_rect(rect), CardsetBackColor, colors::Black);

        canvas.set_fill_style(colors::Green);
        canvas.begin_path();
        canvas.star(rect.get_center(), rect.Width / 2, rect.Width / 4, 5);
        canvas.fill();
        addRegion("card_base_gen", rect);
    }

    {
        for (u8 cr {static_cast<u8>(rank::Ace)}; cr <= static_cast<u8>(rank::King); ++cr) {
            rect_f const rect {nextRect()};
            draw_marker(canvas, fonts, static_cast<rank>(cr), rect);
            addRegion("card_base_" + helper::to_lower(get_rank_name(static_cast<rank>(cr))), rect);
        }
    }

    // draw empty
    {
        rect_f const rect {nextRect()};
        draw_shape(canvas, pad_rect(rect), CardsetBackColor, colors::Black);
        addRegion("card_empty", rect);
    }

    canvas.end_frame();
    auto const& regions {tempTex->get_regions()};
    _texture->create(size_i {texSize}, static_cast<u32>(regions.size()), gfx::texture::format::RGBA8);
    _texture->Filtering = gfx::texture::filtering::Linear;

    auto tempImg {tempTex->copy_to_image(0)};
    tempImg.flip_vertically();
    u32 level {0};
    io::create_folder(CardsetFolder);
    for (auto const& [k, v] : regions) {
        if (k == "default") { continue; }

        auto const data {tempImg.get_data(rect_i {v.UVRect})};
        gfx::image cardimg {gfx::image::Create(size_i {texSize}, gfx::image::format::RGBA, data)};
        (void)cardimg.save(CardsetFolder + k + ".png");
        _texture->update_data(data, level);
        _texture->add_region(k, gfx::texture_region {{0, 0, 1, 1}, level++});
    }
}

void cardset::draw_card(gfx::canvas& canvas, fonts const& fonts, suit s, rank r, rect_f const& rect)
{
    canvas.save();

    size_f const  texSize {rect.get_size()};
    rect_f const  cardRect {pad_rect(rect)};
    point_f const cardCenter {cardRect.get_center()};

    draw_shape(canvas, cardRect, CardsetBackColor, colors::Black);

    {
        point_f offset {rect.X + 6, rect.Y + 1};

        set_suit_color(canvas, s);
        canvas.set_font(fonts.NormalFont);
        std::string const rankSymbol {get_rank_symbol(r)};
        auto const        rankSize {canvas.measure_text(rect.Height, rankSymbol)};
        rect_f const      rankRect {offset, rankSize};

        f32 const suitSize {texSize.Width / 8};
        canvas.draw_textbox(rankRect, rankSymbol);
        draw_suit(canvas, s, {offset.X + suitSize / 2, offset.Y + fonts.NormalFont->get_info().LineHeight + suitSize / 2}, suitSize);
        canvas.rotate_at({180}, cardCenter);
        canvas.draw_textbox(rankRect, rankSymbol);
        draw_suit(canvas, s, {offset.X + suitSize / 2, offset.Y + fonts.NormalFont->get_info().LineHeight + suitSize / 2}, suitSize);
        canvas.reset_transform();
    }
    {
        f32 const suitSize {texSize.Width / 6};
        f32 const centerX {cardCenter.X};
        f32 const centerY {cardCenter.Y};
        f32 const left {centerX - cardRect.Width / 4};
        f32 const right {centerX + cardRect.Width / 4};
        f32 const top {cardRect.Y + suitSize * 2};
        f32 const centerTop {top + suitSize};

        f32 const nineCenterY {top + suitSize * 1.5f};
        f32 const tenCenterY {(top + nineCenterY) / 2};

        auto drawTwo {[&]() {
            draw_suit(canvas, s, {centerX, top}, suitSize);
            canvas.rotate_at({180}, cardCenter);
            draw_suit(canvas, s, {centerX, top}, suitSize);
            canvas.reset_transform();
        }};

        auto drawFour {[&]() {
            draw_suit(canvas, s, {left, top}, suitSize);
            draw_suit(canvas, s, {right, top}, suitSize);
            canvas.rotate_at({180}, cardCenter);
            draw_suit(canvas, s, {left, top}, suitSize);
            draw_suit(canvas, s, {right, top}, suitSize);
            canvas.reset_transform();
        }};

        auto drawSix {[&]() {
            drawFour();
            draw_suit(canvas, s, {left, centerY}, suitSize);
            draw_suit(canvas, s, {right, centerY}, suitSize);
        }};

        switch (r) {
        case rank::Ace: {
            draw_suit(canvas, s, cardCenter, suitSize * 2);
        } break;
        case rank::Two: {
            drawTwo();
        } break;
        case rank::Three: {
            drawTwo();
            draw_suit(canvas, s, cardCenter, suitSize);
        } break;
        case rank::Four: {
            drawFour();
        } break;
        case rank::Five: {
            drawFour();
            draw_suit(canvas, s, cardCenter, suitSize);
        } break;
        case rank::Six: {
            drawSix();
        } break;
        case rank::Seven: {
            drawSix();
            draw_suit(canvas, s, {centerX, centerTop}, suitSize);
        } break;
        case rank::Eight: {
            drawSix();
            draw_suit(canvas, s, {centerX, centerTop}, suitSize);
            canvas.rotate_at({180}, cardCenter);
            draw_suit(canvas, s, {centerX, centerTop}, suitSize);
            canvas.reset_transform();
        } break;
        case rank::Nine: {
            drawFour();
            draw_suit(canvas, s, {left, nineCenterY}, suitSize);
            draw_suit(canvas, s, {right, nineCenterY}, suitSize);
            draw_suit(canvas, s, cardCenter, suitSize);
            canvas.rotate_at({180}, cardCenter);
            draw_suit(canvas, s, {left, nineCenterY}, suitSize);
            draw_suit(canvas, s, {right, nineCenterY}, suitSize);
            canvas.reset_transform();
        } break;
        case rank::Ten: {
            drawFour();
            draw_suit(canvas, s, {left, nineCenterY}, suitSize);
            draw_suit(canvas, s, {right, nineCenterY}, suitSize);
            draw_suit(canvas, s, {centerX, tenCenterY}, suitSize);
            canvas.rotate_at({180}, cardCenter);
            draw_suit(canvas, s, {left, nineCenterY}, suitSize);
            draw_suit(canvas, s, {right, nineCenterY}, suitSize);
            draw_suit(canvas, s, {centerX, tenCenterY}, suitSize);
            canvas.reset_transform();
        } break;

        case rank::Jack:
        case rank::Queen:
        case rank::King: {
            set_suit_color(canvas, s);
            canvas.set_font(fonts.LargeFont);
            std::string const rankSymbol {get_rank_symbol(r)};
            canvas.draw_textbox(pad_rect(rect), rankSymbol);
        } break;
        }
    }

    canvas.restore();
}

void cardset::draw_marker(gfx::canvas& canvas, fonts const& fonts, rank r, rect_f const& rect)
{
    canvas.save();

    draw_shape(canvas, pad_rect(rect), CardsetBackColor, colors::Black);

    canvas.set_fill_style(colors::Green);
    canvas.set_font(fonts.LargeFont);
    std::string const rankSymbol {get_rank_symbol(r)};
    canvas.draw_textbox(pad_rect(rect), rankSymbol);

    canvas.restore();
}

void cardset::draw_back(gfx::canvas& canvas, rect_f const& rect)
{
    canvas.save();
    draw_shape(canvas, pad_rect(rect), colors::LightSteelBlue, colors::White);

    rect_f const backRect {rect.as_padded(rect.get_size() / 50 * 4)};

    canvas.set_scissor(backRect);

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

            canvas.set_fill_style(color);
            canvas.begin_path();
            canvas.move_to({x1, y1});
            canvas.line_to({x2, y2});
            canvas.line_to({x3, y3});
            canvas.line_to({x4, y4});
            canvas.close_path();
            canvas.fill();
            canvas.set_stroke_style(CardsetBackColor);
            canvas.set_stroke_width(3);
            canvas.stroke();
        }
    }

    canvas.set_stroke_style(colors::White);
    canvas.set_stroke_width(3);
    canvas.stroke_rect(backRect);

    canvas.restore();
}

void cardset::set_suit_color(gfx::canvas& canvas, suit s)
{
    switch (s) {
    case suit::Diamonds:
    case suit::Hearts:
        canvas.set_fill_style(colors::Red);
        break;
    case suit::Clubs:
    case suit::Spades:
        canvas.set_fill_style(colors::Black);
        break;
    }
}

auto cardset::pad_rect(rect_f const& rect) -> rect_f
{
    auto const cardPad {rect.get_size() / 50};
    return rect.as_padded(cardPad);
}

void cardset::draw_suit(gfx::canvas& canvas, suit s, point_f pos, f32 size)
{
    auto draw {[&](std::vector<std::vector<point_f>> const& halfPath) {
        canvas.save();
        canvas.translate(pos);
        canvas.scale({size, size});
        canvas.begin_path();

        canvas.move_to(halfPath[0][0]);
        for (auto const& path : halfPath) {
            canvas.cubic_bezier_to(path[1], path[2], path[3]);
        }
        for (auto const& path : halfPath | std::views::reverse) {
            canvas.cubic_bezier_to({-path[2].X, path[2].Y}, {-path[1].X, path[1].Y}, {-path[0].X, path[0].Y});
        }

        set_suit_color(canvas, s);
        canvas.fill();

        canvas.set_stroke_style(colors::DarkGray);
        canvas.set_stroke_width(0.04f);
        canvas.stroke();

        canvas.restore();
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

void cardset::draw_shape(gfx::canvas& canvas, rect_f const& bounds, color fill, color stroke)
{
    canvas.set_fill_style(fill);
    canvas.set_stroke_style(stroke);
    canvas.set_stroke_width(3);
    canvas.begin_path();
    canvas.rounded_rect(bounds, 15);
    canvas.fill();
    canvas.stroke();
}

} // namespace solitaire
