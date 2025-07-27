// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "CardSet.hpp"

#include <ranges>
#include <utility>

namespace solitaire {

static std::string const UserFolder {"/cardsets/"};
static constexpr isize   CardsetCardCount {68};

void load_card_sets(std::map<std::string, std::shared_ptr<card_set>>& cardsetMap, assets::group& resGrp)
{
    cardsetMap.clear();

    auto resFolder {resGrp.mount_point() + "cardsets/"};

    cardsetMap["gen_0"]  = std::make_shared<gen_cardset>(resGrp);
    cardsetMap["mini_h"] = std::make_shared<mini_h_cardset>(resGrp);
    cardsetMap["mini_v"] = std::make_shared<mini_v_cardset>(resGrp);

    std::unordered_map<std::string, std::unordered_set<std::string>> folders;
    folders[UserFolder] = io::get_sub_folders(UserFolder);
    folders[resFolder]  = io::get_sub_folders(resFolder);

    for (auto const& [folder, subfolders] : folders) {
        for (auto const& gi : subfolders) {
            auto const name {io::get_stem(gi)};
            if (cardsetMap.contains(name)) { continue; }
            cardsetMap[name] = std::make_shared<card_set>(folder, name);
        }
    }
}

////////////////////////////////////////////////////////////

card_set::card_set(std::string folder, std::string name)
    : _name {std::move(name)}
    , _folder {std::move(folder)}
    , _loaded {load()}
{
    _material->Texture = _texture;
}

auto card_set::get_card_size() -> size_f
{
    return size_f {_texture->info().Size};
}

auto card_set::get_material() const -> assets::asset_ptr<gfx::material>
{
    return _material;
}

auto card_set::load() const -> bool
{
    std::string const folder {get_folder()};
    data::object      json;
    json.load(folder + "cardset.json");

    auto files {io::enumerate(folder, {"*card*.png"}, false)};
    if (files.size() < CardsetCardCount) { return false; }

    auto const info {gfx::image::LoadInfo(*files.begin())};
    if (!info) { return false; }
    auto const texSize {gfx::image::LoadInfo(*files.begin())->Size};

    _texture->create(texSize, static_cast<u32>(files.size()), gfx::texture::format::RGBA8);
    _texture->Filtering = json.get<gfx::texture::filtering>("filtering").value_or(gfx::texture::filtering::Linear);

    struct image_ftr {
        u32                      Depth {};
        path                     Path {};
        gfx::image               Image {};
        std::future<load_status> Future {};
    };

    u32                    level {0};
    std::vector<image_ftr> images;
    images.reserve(files.size());
    for (auto const& file : files) {
        image_ftr& ftr {images.emplace_back()};
        ftr.Depth  = level++;
        ftr.Path   = file;
        ftr.Future = ftr.Image.load_async(file);
    }

    bool assetLoadingDone {false};
    while (!assetLoadingDone) {
        assetLoadingDone = true;
        for (auto imgIt {images.begin()}; imgIt != images.end(); ++imgIt) {
            if (auto& statusFuture {imgIt->Future}; statusFuture.valid()) {
                if (statusFuture.wait_for(0s) != std::future_status::ready) {
                    assetLoadingDone = false;
                    continue;
                }
                if (statusFuture.get() != load_status::Ok) { return false; }
                _texture->update_data(imgIt->Image, imgIt->Depth);
                string const name {json.get<std::string>("cards", io::get_filename(imgIt->Path)).value_or(io::get_stem(imgIt->Path))};
                _texture->regions()[name] = {.UVRect = {0, 0, 1, 1}, .Level = imgIt->Depth};
            }
        }
    }

    return true;
}

auto card_set::get_folder() const -> std::string
{
    return _folder + _name + "/";
}

auto card_set::get_texture() const -> gfx::texture*
{
    return _texture.ptr();
}

auto card_set::is_loaded() const -> bool
{
    return _loaded;
}

auto card_set::pad_rect(rect_f const& rect) -> rect_f
{
    auto const cardPad {rect.Size / 50};
    return rect.as_padded_by(cardPad);
}

void card_set::set_suit_color(gfx::canvas& canvas, suit s)
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

void card_set::draw_suit(gfx::canvas& canvas, suit s, point_f center, f32 size)
{
    center -= point_f {size / 2, size / 2};

    auto draw {[&](std::string_view path) {
        canvas.save();
        canvas.translate(center);
        canvas.scale({size, size});
        canvas.begin_path();

        canvas.path_2d(*gfx::path2d::Parse(path));

        set_suit_color(canvas, s);
        canvas.fill();

        canvas.set_stroke_style(colors::Silver);
        canvas.set_stroke_width(0.04f);
        canvas.stroke();

        canvas.restore();
    }};

    switch (s) {
    case suit::Hearts: {
        draw("m 0.483 0.2 c 0 -0.123 -0.095 -0.2 -0.233 -0.2 -0.138 0 -0.25 0.099 -0.25 0.222 0.001 0.011 0 0.021 0 0.031 0 0.081 0.045 0.16 0.083 0.233 0.04 0.076 0.097 0.142 0.151 0.21 0.083 0.105 0.265 0.303 0.265 0.303 0 0 0.182 -0.198 0.265 -0.303 0.054 -0.068 0.111 -0.134 0.151 -0.21 0.039 -0.074 0.083 -0.152 0.083 -0.233 -0 -0.011 -0 -0.021 0 -0.031 0 -0.123 -0.112 -0.222 -0.25 -0.222 -0.138 0 -0.233 0.077 -0.233 0.2 0 0.022 -0.033 0.022 -0.033 0 z");
    } break;
    case suit::Diamonds: {
        draw("m 0.5 0 c 0 0 0.1 0.166 0.216 0.277 0.117 0.111 0.283 0.222 0.283 0.222 0 0 -0.166 0.111 -0.283 0.222 C 0.6 0.833 0.5 0.999 0.5 0.999 c 0 0 -0.1 -0.166 -0.216 -0.277 C 0.167 0.611 0 0.5 0 0.5 c 0 0 0.166 -0.111 0.283 -0.222 C 0.4 0.167 0.5 0 0.5 0");
    } break;
    case suit::Clubs: {
        draw("m 0.583 0.622 c 0.033 0.044 0.117 0.111 0.2 0.111 0.15 0 0.216 -0.089 0.216 -0.2 0 -0.111 -0.067 -0.211 -0.216 -0.211 -0.083 0 -0.133 0.056 -0.183 0.089 -0.017 0.011 -0.033 0 -0.017 -0.011 0.05 -0.033 0.133 -0.122 0.133 -0.2 0 -0.089 -0.083 -0.2 -0.216 -0.2 -0.133 0 -0.216 0.111 -0.216 0.2 0 0.078 0.083 0.166 0.133 0.2 0.017 0.011 0 0.022 -0.017 0.011 -0.05 -0.033 -0.1 -0.089 -0.183 -0.089 -0.15 0 -0.216 0.1 -0.216 0.211 0 0.111 0.067 0.2 0.216 0.2 0.083 0 0.166 -0.067 0.2 -0.111 0.017 -0.011 0.017 0 0.017 0.011 -0.017 0.089 -0.017 0.111 -0.033 0.178 -0.017 0.067 -0.067 0.189 -0.067 0.189 0.083 -0.022 0.25 -0.022 0.333 0 0 0 -0.05 -0.122 -0.067 -0.189 -0.017 -0.067 -0.017 -0.089 -0.033 -0.178 0 -0.011 0 -0.022 0.017 -0.011 z");
    } break;
    case suit::Spades: {
        draw("m 0.583 0.622 c 0.017 0.044 0.083 0.156 0.2 0.156 C 0.933 0.778 1 0.689 1 0.578 1 0.5 0.957 0.449 0.917 0.389 0.874 0.325 0.81 0.268 0.75 0.211 0.673 0.138 0.5 0 0.5 0 c 0 0 -0.173 0.138 -0.25 0.211 C 0.19 0.268 0.126 0.325 0.083 0.389 0.043 0.449 0 0.5 0 0.578 c 0 0.111 0.067 0.2 0.217 0.2 0.117 0 0.183 -0.111 0.2 -0.156 0.017 -0.011 0.017 0 0.017 0.011 C 0.417 0.722 0.417 0.744 0.4 0.811 0.383 0.878 0.333 1 0.333 1 0.417 0.978 0.583 0.978 0.667 1 c 0 0 -0.05 -0.122 -0.067 -0.189 -0.017 -0.067 -0.017 -0.089 -0.033 -0.178 0 -0.011 0 -0.022 0.017 -0.011 z");
    } break;
    }
}

void card_set::save_textures(assets::asset_ptr<gfx::texture> const& canvasTex, size_f texSize) const
{
    auto* tex {get_texture()};

    auto const& regions {canvasTex->regions()};
    tex->create(size_i {texSize}, static_cast<u32>(regions.size()), gfx::texture::format::RGBA8);
    tex->Filtering = gfx::texture::filtering::Linear;

    auto tempImg {canvasTex->copy_to_image(0)};
    tempImg.flip_vertically();

    auto const folder {get_folder()};
    io::create_folder(folder);
    u32 level {0};

    data::object cardsetObj;
    for (auto const& [k, v] : regions) {
        if (k == "default") { continue; }

        auto const data {tempImg.data(rect_i {v.UVRect})};
        gfx::image cardimg {gfx::image::Create(size_i {texSize}, gfx::image::format::RGBA, data)};

        string const file {k + ".png"};
        cardsetObj["cards"][file] = k;
        std::ignore               = cardimg.save(folder + file);
        tex->update_data(data, level);
        tex->regions()[k] = gfx::texture_region {.UVRect = {0, 0, 1, 1}, .Level = level++};
    }
    cardsetObj["version"]   = "1.0";
    cardsetObj["filtering"] = "Linear";
    cardsetObj.save(folder + "cardset.json");
}

////////////////////////////////////////////////////////////

static constexpr color CardsetBackColorDefault {colors::SeaShell};

gen_cardset::gen_cardset(assets::group& resGrp)
    : card_set {UserFolder, "gen_0"}
{
    if (!is_loaded()) {
        create(resGrp);
    }
}

void gen_cardset::create(assets::group& resGrp)
{
    size_f const texSize {240, 360};
    i32 const    columns {10};
    i32 const    rows {8};
    static_assert(columns * rows >= CardsetCardCount);
    size_f const canvasSize {texSize.Width * columns, texSize.Height * rows};

    gfx::canvas canvas;
    canvas.begin_frame(size_i {canvasSize}, 1.0f);

    auto        fontFamily {resGrp.get<gfx::font_family>(FONT)};
    fonts const fonts {.NormalFont = fontFamily->get_font({false, gfx::font::weight::Normal}, static_cast<u32>(texSize.Height / 7.5f)).ptr(),
                       .LargeFont  = fontFamily->get_font({false, gfx::font::weight::Bold}, static_cast<u32>(texSize.Height / 2)).ptr()};

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
        tempTex->regions()[texName] = {.UVRect = rect, .Level = 0};
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
        draw_shape(canvas, pad_rect(rect), CardsetBackColorDefault, colors::Black);

        canvas.set_fill_style(colors::Green);
        canvas.begin_path();
        canvas.star(rect.center(), rect.width() / 2, rect.width() / 4, 5);
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
        draw_shape(canvas, pad_rect(rect), CardsetBackColorDefault, colors::Black);
        addRegion("card_empty", rect);
    }

    canvas.end_frame();

    save_textures(tempTex, texSize);
}

void gen_cardset::draw_card(gfx::canvas& canvas, fonts const& fonts, suit s, rank r, rect_f const& rect)
{
    canvas.save();

    size_f const  texSize {rect.Size};
    rect_f const  cardRect {pad_rect(rect)};
    point_f const cardCenter {cardRect.center()};

    draw_shape(canvas, cardRect, CardsetBackColorDefault, colors::Black);

    {
        point_f offset {rect.left() + 6, rect.top() + 1};

        set_suit_color(canvas, s);
        canvas.set_font(fonts.NormalFont);
        std::string const rankSymbol {get_rank_symbol(r)};
        auto const        rankSize {canvas.measure_text(rect.height(), rankSymbol)};
        rect_f const      rankRect {offset, rankSize};

        f32 const suitSize {texSize.Width / 8};
        canvas.draw_text(rankRect, rankSymbol);
        draw_suit(canvas, s, {offset.X + (suitSize / 2), offset.Y + fonts.NormalFont->info().LineHeight + (suitSize / 2)}, suitSize);
        canvas.rotate_at(degree_f {180}, cardCenter);
        canvas.draw_text(rankRect, rankSymbol);
        draw_suit(canvas, s, {offset.X + (suitSize / 2), offset.Y + fonts.NormalFont->info().LineHeight + (suitSize / 2)}, suitSize);
        canvas.reset_transform();
    }
    {
        f32 const suitSize {texSize.Width / 6};
        f32 const centerX {cardCenter.X};
        f32 const centerY {cardCenter.Y};
        f32 const left {centerX - (cardRect.width() / 4)};
        f32 const right {centerX + (cardRect.width() / 4)};
        f32 const top {cardRect.top() + (suitSize * 2)};
        f32 const centerTop {top + suitSize};

        f32 const nineCenterY {top + (suitSize * 1.5f)};
        f32 const tenCenterY {(top + nineCenterY) / 2};

        auto drawTwo {[&]() {
            draw_suit(canvas, s, {centerX, top}, suitSize);
            canvas.rotate_at(degree_f {180}, cardCenter);
            draw_suit(canvas, s, {centerX, top}, suitSize);
            canvas.reset_transform();
        }};

        auto drawFour {[&]() {
            draw_suit(canvas, s, {left, top}, suitSize);
            draw_suit(canvas, s, {right, top}, suitSize);
            canvas.rotate_at(degree_f {180}, cardCenter);
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
            canvas.rotate_at(degree_f {180}, cardCenter);
            draw_suit(canvas, s, {centerX, centerTop}, suitSize);
            canvas.reset_transform();
        } break;
        case rank::Nine: {
            drawFour();
            draw_suit(canvas, s, {left, nineCenterY}, suitSize);
            draw_suit(canvas, s, {right, nineCenterY}, suitSize);
            draw_suit(canvas, s, cardCenter, suitSize);
            canvas.rotate_at(degree_f {180}, cardCenter);
            draw_suit(canvas, s, {left, nineCenterY}, suitSize);
            draw_suit(canvas, s, {right, nineCenterY}, suitSize);
            canvas.reset_transform();
        } break;
        case rank::Ten: {
            drawFour();
            draw_suit(canvas, s, {left, nineCenterY}, suitSize);
            draw_suit(canvas, s, {right, nineCenterY}, suitSize);
            draw_suit(canvas, s, {centerX, tenCenterY}, suitSize);
            canvas.rotate_at(degree_f {180}, cardCenter);
            draw_suit(canvas, s, {left, nineCenterY}, suitSize);
            draw_suit(canvas, s, {right, nineCenterY}, suitSize);
            draw_suit(canvas, s, {centerX, tenCenterY}, suitSize);
            canvas.reset_transform();
        } break;

        case rank::Jack:
        case rank::Queen:
        case rank::King:  {
            set_suit_color(canvas, s);
            canvas.set_font(fonts.LargeFont);
            canvas.draw_text(pad_rect(rect), get_rank_symbol(r));
        } break;
        }
    }

    canvas.restore();
}

void gen_cardset::draw_marker(gfx::canvas& canvas, fonts const& fonts, rank r, rect_f const& rect)
{
    canvas.save();

    draw_shape(canvas, pad_rect(rect), CardsetBackColorDefault, colors::Black);

    canvas.set_fill_style(colors::Green);
    canvas.set_font(fonts.LargeFont);
    canvas.draw_text(pad_rect(rect), get_rank_symbol(r));

    canvas.restore();
}

void gen_cardset::draw_back(gfx::canvas& canvas, rect_f const& rect)
{
    canvas.save();
    draw_shape(canvas, pad_rect(rect), colors::LightSteelBlue, colors::White);

    rect_f const backRect {rect.as_padded_by(rect.Size / 50 * 4)};

    canvas.set_scissor(backRect);

    f32 const rhombusSize {backRect.width() / 16.f};
    f32 constexpr sqrt3 {std::numbers::sqrt3_v<f32>};

    for (f32 row {backRect.top()}; row < backRect.bottom(); row += rhombusSize * sqrt3) {
        for (f32 col {backRect.left()}; col < backRect.right(); col += rhombusSize * 2) {
            f32 const x1 {col};
            f32 const y1 {row + (rhombusSize * sqrt3 / 2)};
            f32 const x2 {col + rhombusSize};
            f32 const y2 {row};
            f32 const x3 {col + (rhombusSize * 2)};
            f32 const y3 {y1};
            f32 const x4 {x2};
            f32 const y4 {row + (rhombusSize * sqrt3)};

            color const color {static_cast<i32>((row / (rhombusSize * sqrt3)) + (col / (rhombusSize * 2))) % 2 == 0 ? colors::LightBlue : colors::LightGray};

            canvas.set_fill_style(color);
            canvas.begin_path();
            canvas.move_to({x1, y1});
            canvas.line_to({x2, y2});
            canvas.line_to({x3, y3});
            canvas.line_to({x4, y4});
            canvas.close_path();
            canvas.fill();
            canvas.set_stroke_style(CardsetBackColorDefault);
            canvas.set_stroke_width(3);
            canvas.stroke();
        }
    }

    canvas.set_stroke_style(colors::White);
    canvas.set_stroke_width(3);
    canvas.begin_path();
    canvas.rect(backRect);
    canvas.stroke();

    canvas.restore();
}

void gen_cardset::draw_shape(gfx::canvas& canvas, rect_f const& bounds, color fill, color stroke)
{
    canvas.set_fill_style(fill);
    canvas.set_stroke_style(stroke);
    canvas.set_stroke_width(3);
    canvas.begin_path();
    canvas.rounded_rect(bounds, 15);
    canvas.fill();
    canvas.stroke();
}

////////////////////////////////////////////////////////////

static constexpr color CardsetBackColorMini {colors::LightGray};

mini_cardset::mini_cardset(std::string folder)
    : card_set {UserFolder, std::move(folder)}
{
}

void mini_cardset::create(assets::group& resGrp, size_f texSize)
{
    i32 const columns {10};
    i32 const rows {8};
    static_assert(columns * rows >= CardsetCardCount);
    size_f const canvasSize {texSize.Width * columns, texSize.Height * rows};
    bool const   isHori {texSize.Width > texSize.Height};

    gfx::canvas canvas;
    canvas.begin_frame(size_i {canvasSize}, 1.0f);

    auto       fontFamily {resGrp.get<gfx::font_family>(FONT)};
    gfx::font* font {fontFamily->get_font({false, gfx::font::weight::Normal},
                                          static_cast<u32>((isHori ? texSize.Height : texSize.Width) * 0.9f))
                         .ptr()};

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
        tempTex->regions()[texName] = {.UVRect = rect, .Level = 0};
    }};

    // draw cards
    for (u8 cs {static_cast<u8>(suit::Hearts)}; cs <= static_cast<u8>(suit::Spades); ++cs) {
        for (u8 cr {static_cast<u8>(rank::Ace)}; cr <= static_cast<u8>(rank::King); ++cr) {
            rect_f const rect {nextRect()};
            card const   c {static_cast<suit>(cs), static_cast<rank>(cr), 0, false};
            draw_card(canvas, font, c.get_suit(), c.get_rank(), rect);
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
        draw_shape(canvas, pad_rect(rect), CardsetBackColorMini, colors::Black);

        canvas.set_fill_style(colors::Green);
        canvas.begin_path();
        canvas.star(rect.center(), rect.width() / 5, rect.width() / 8, 5);
        canvas.fill();
        addRegion("card_base_gen", rect);
    }

    {
        for (u8 cr {static_cast<u8>(rank::Ace)}; cr <= static_cast<u8>(rank::King); ++cr) {
            rect_f const rect {nextRect()};
            draw_marker(canvas, font, static_cast<rank>(cr), rect);
            addRegion("card_base_" + helper::to_lower(get_rank_name(static_cast<rank>(cr))), rect);
        }
    }

    // draw empty
    {
        rect_f const rect {nextRect()};
        draw_shape(canvas, pad_rect(rect), CardsetBackColorMini, colors::Black);
        addRegion("card_empty", rect);
    }

    canvas.end_frame();
    save_textures(tempTex, texSize);
}

void mini_cardset::draw_card(gfx::canvas& canvas, gfx::font* font, suit s, rank r, rect_f const& rect)
{
    canvas.save();

    rect_f const cardRect {pad_rect(rect)};
    f32 const    width {cardRect.width()};
    f32 const    height {cardRect.height()};

    draw_shape(canvas, cardRect, CardsetBackColorMini, colors::Black);

    set_suit_color(canvas, s);
    canvas.set_font(font);
    if (rect.width() > rect.height()) {
        canvas.draw_text({cardRect.Position, {width / 2, height}}, get_rank_symbol(r));
        draw_suit(canvas, s, {cardRect.Position + point_f {width * 0.75f, height / 2}}, width / 2.25f);
    } else {
        canvas.draw_text({cardRect.Position, {width, height / 2}}, get_rank_symbol(r));
        draw_suit(canvas, s, {cardRect.Position + point_f {width / 2, height * 0.75f}}, width / 2.25f);
    }

    canvas.restore();
}

void mini_cardset::draw_marker(gfx::canvas& canvas, gfx::font* font, rank r, rect_f const& rect)
{
    canvas.save();

    draw_shape(canvas, pad_rect(rect), CardsetBackColorMini, colors::Black);

    canvas.set_fill_style(colors::Green);
    canvas.set_font(font);
    canvas.draw_text(pad_rect(rect), get_rank_symbol(r));

    canvas.restore();
}

void mini_cardset::draw_back(gfx::canvas& canvas, rect_f const& rect)
{
    canvas.save();
    rect_f const bounds {pad_rect(rect)};
    draw_shape(canvas, bounds, colors::LightSteelBlue, colors::White);

    canvas.begin_path();
    canvas.rounded_rect(bounds.as_padded_by({3, 3}), 3);
    canvas.set_fill_style(colors::MidnightBlue);
    canvas.fill();

    canvas.restore();
}

void mini_cardset::draw_shape(gfx::canvas& canvas, rect_f const& bounds, color fill, color stroke)
{
    canvas.set_fill_style(fill);
    canvas.set_stroke_style(stroke);
    canvas.set_stroke_width(2);
    canvas.begin_path();
    canvas.rounded_rect(bounds, 3);
    canvas.fill();
    canvas.stroke();
}

mini_h_cardset::mini_h_cardset(assets::group& resGrp)
    : mini_cardset {"mini_h"}
{
    if (!is_loaded()) {
        create(resGrp, {360, 180});
    }
}

mini_v_cardset::mini_v_cardset(assets::group& resGrp)
    : mini_cardset {"mini_v"}
{
    if (!is_loaded()) {
        create(resGrp, {180, 360});
    }
}

} // namespace solitaire
