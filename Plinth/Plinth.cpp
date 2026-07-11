// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Plinth.hpp"

#include "Common.hpp"
#include "Level.hpp"
#include "MapGenerator.hpp"
#include "MapRenderer.hpp"
#include "Raycaster.hpp"

constexpr size_i screenSize {640, 360};

// ASCII-art prefab authoring helper
//
// Legend:
//   '#'  normal_wall
//   '.'  floor_cell (open interior)
//   'D'  door_wall, orientation auto-picked from which edge it sits on
//   'P'  round_pillar
//   'o'  connector (open floor cell; corridors may attach here)
//   any other character -> floor_cell
//
// All rows must be the same length. Row 0 is the top (smallest Y).
static auto build_prefab_from_ascii(std::vector<std::string_view> const& rows, i32 wallTexture, i32 floorTexture, i32 ceilingTexture) -> map_prefab
{
    assert(!rows.empty());
    i32 const height {static_cast<i32>(rows.size())};
    i32 const width {static_cast<i32>(rows[0].size())};

    map_prefab prefab {};
    prefab.Size = {width, height};
    prefab.Cells.resize(static_cast<size_t>(width) * height);

    for (i32 y {0}; y < height; ++y) {
        assert(static_cast<i32>(rows[y].size()) == width && "all prefab rows must be the same length");

        for (i32 x {0}; x < width; ++x) {
            char const   symbol {rows[y][x]};
            size_t const idx {static_cast<size_t>(x) + (static_cast<size_t>(y) * width)};

            switch (symbol) {
            case '#': {
                normal_wall w {};
                w.Texture         = wallTexture;
                prefab.Cells[idx] = w;
                break;
            }
            case 'D': {
                door_wall d {};
                d.Texture         = wallTexture;
                d.FrameTexture    = wallTexture;
                d.FloorTexture    = floorTexture;
                d.CeilingTexture  = ceilingTexture;
                // slab orientation follows whichever edge the door sits on:
                // top/bottom edge -> slab runs east-west -> blocks north-south passage
                // left/right edge -> slab runs north-south -> blocks east-west passage
                d.Orientation     = (y == 0 || y == height - 1) ? door_wall::orientation::BlocksNorthSouth
                                                                : door_wall::orientation::BlocksEastWest;
                prefab.Cells[idx] = d;
                prefab.Connectors.push_back({x, y});
                break;
            }
            case 'o': {
                floor_cell f {};
                f.FloorTexture    = floorTexture;
                f.CeilingTexture  = ceilingTexture;
                prefab.Cells[idx] = f;
                if (x == 0 || y == 0 || x == width - 1 || y == height - 1) { prefab.Connectors.push_back({x, y}); }
                break;
            }
            case 'P': {
                round_pillar p {};
                p.Radius          = 0.3;
                p.Texture         = wallTexture;
                p.FloorTexture    = floorTexture;
                p.CeilingTexture  = ceilingTexture;
                prefab.Cells[idx] = p;
                break;
            }
            case '.':
            default:  {
                floor_cell f {};
                f.FloorTexture    = floorTexture;
                f.CeilingTexture  = ceilingTexture;
                prefab.Cells[idx] = f;
                break;
            }
            }
        }
    }

    return prefab;
}

////////////////////////////////////////////////////////////
// Example prefab library
//
// A small starter set of hand-authored rooms.
// Texture indices below are placeholders (0/1/2) — swap for real indices from your texture_cache.
static auto make_example_prefab_library() -> std::vector<map_prefab>
{
    std::vector<map_prefab> library;

    // --- Plain 7x5 room, one connector centered on each edge ---
    {
        std::vector<std::string_view> const rows {
            "#..o..#",
            "#.....#",
            "o.....o",
            "#.....#",
            "#..o..#",
        };
        library.push_back(build_prefab_from_ascii(rows, 1, 1, 1));
    }

    // --- 7x7 room with two pillars ---
    {
        std::vector<std::string_view> const rows {
            "#..o..#",
            "#.....#",
            "#..P..#",
            "o.....o",
            "#..P..#",
            "#.....#",
            "#..o..#",
        };
        library.push_back(build_prefab_from_ascii(rows, /*wallTex*/ 1, /*floorTex*/ 1, /*ceilingTex*/ 1));
    }

    // --- L-shaped room (7x7 bounding box, top-right corner walled off) ---
    {
        std::vector<std::string_view> const rows {
            "###....",
            "###....",
            "###....",
            "o......",
            "#.....o",
            "#......",
            "#..o...",
        };
        library.push_back(build_prefab_from_ascii(rows, 1, 1, 1));
    }

    // --- Small room with a real door (guards the only entrance) ---
    {
        std::vector<std::string_view> const rows {
            "#####",
            "#...#",
            "#...#",
            "#...#",
            "##D##",
        };
        library.push_back(build_prefab_from_ascii(rows, /*wallTex*/ 2, /*floorTex*/ 2, /*ceilingTex*/ 2));
    }

    return library;
}

Plinth::Plinth(game& game)
    : scene {game}
    , _cache {std::make_unique<texture_cache>()}

{

    map_generator gen {make_example_prefab_library()};
    auto const    map {gen.generate({})};
    _level = std::make_unique<level>(map);

    _material->first_pass().Texture = _texture;

    _texture->resize(screenSize, 1, gfx::texture::format::RGBA8);
    _texture->Filtering = gfx::texture::filtering::NearestNeighbor;

    auto const find_empty {[&]() {
        for (i32 x {0}; x < MAP_WIDTH; ++x) {
            for (i32 y {0}; y < MAP_HEIGHT; ++y) {
                if (map[x, y].index() == 0) {
                    return point_d {static_cast<f64>(x) + 0.5f, static_cast<f64>(y) + 0.5f};
                }
            }
        }
        return point_d {0, 0};
    }};

    _player.Position = find_empty();
    degree_d const angle {90};
    radian_d const rad {angle - degree_d {90}};
    _player.Direction = point_d::FromDirection(angle);
    f64 const fov {FOV * TAU / 360.0};
    _player.Plane = {-rad.sin() * std::tan(fov / 2.0), rad.cos() * std::tan(fov / 2.0)};

    _raycaster   = std::make_unique<raycaster>(*_cache, screenSize, (screenSize.Width / 2.0) / std::tan(fov / 2.0));
    _mapRenderer = std::make_unique<map_renderer>(*_cache, screenSize);
}

Plinth::~Plinth() = default;

void Plinth::on_start()
{
    _cache->load();
}

void Plinth::on_draw_to(gfx::render_target& target, transform const& xform)
{
    if (_draw) {
        _draw = false;
        _texture->update_data(_raycaster->draw(*_level, _player), 0);
    }
    if (_drawMap) {
        _texture->update_data(_mapRenderer->draw(*_level, _player), 0);
    }

    // aspect ratio correction
    size_i const size {*target.Size};

    f32 const srcAspect {static_cast<f32>(screenSize.Width) / static_cast<f32>(screenSize.Height)};
    f32 const dstAspect {static_cast<f32>(size.Width) / static_cast<f32>(size.Height)};

    f32 destWidth {static_cast<f32>(size.Width)};
    f32 destHeight {static_cast<f32>(size.Height)};

    if (dstAspect > srcAspect) {
        destWidth = destHeight * srcAspect;
    } else {
        destHeight = destWidth / srcAspect;
    }

    gfx::quad q {};
    gfx::geometry::set_color(q, colors::White);
    gfx::geometry::set_position(q, {0, 0, destWidth, destHeight});
    gfx::geometry::set_texcoords(q, {.UVRect = {0, 0, 1, 1}, .Level = 0});
    _renderer.set_geometry({.Vertices = q, .Indices = gfx::QuadIndicies, .Type = gfx::primitive_type::Triangles}, &_material->first_pass());
    _renderer.render_to_target(target, transform::Identity);
}

void Plinth::on_fixed_update(milliseconds deltaTime)
{
    auto const& stats {locate_service<gfx::render_system>().statistics()};
    auto const& mouse {locate_service<input::system>().mouse().get_position()};
    window().Title = std::format("Plinth | FPS avg:{:.2f} best:{:.2f} worst:{:.2f} | x:{} y:{} ",
                                 stats.average_FPS(), stats.best_FPS(), stats.worst_FPS(),
                                 mouse.X, mouse.Y);
}

void Plinth::on_update(milliseconds deltaTime)
{
    _draw = move_player(deltaTime) || _draw;
    _draw = _player.bob(deltaTime) || _draw;
    _draw = _level->update(deltaTime) || _draw;
}

auto Plinth::move_player(milliseconds deltaTime) -> bool
{
    auto const& input {locate_service<input::system>()};

    f64 const moveSpeed {deltaTime.count() / 1000 * _player.MoveSpeed};  // squares/second
    f64 const rotSpeed {deltaTime.count() / 1000 * _player.RotateSpeed}; // radians/second

    f64 forwardAmount {0};
    f64 strafeAmount {0};
    f64 rotateAmount {0};

    if (input.InputMode == input::mode::KeyboardMouse) {
        auto const& keyboard {input.keyboard()}; // TODO: remap in ui

        if (keyboard.is_key_down(input::scan_code::W) || keyboard.is_key_down(input::scan_code::UP)) { forwardAmount += moveSpeed; }
        if (keyboard.is_key_down(input::scan_code::S) || keyboard.is_key_down(input::scan_code::DOWN)) { forwardAmount -= moveSpeed; }

        if (keyboard.is_key_down(input::scan_code::D)) { strafeAmount -= moveSpeed; }
        if (keyboard.is_key_down(input::scan_code::A)) { strafeAmount += moveSpeed; }

        if (keyboard.is_key_down(input::scan_code::LEFT)) { rotateAmount -= rotSpeed; }
        if (keyboard.is_key_down(input::scan_code::RIGHT)) { rotateAmount += rotSpeed; }
    } else {
        auto const& controller {input.first_controller()}; // TODO: controller select in ui

        if (controller.is_button_pressed(input::controller::button::DPadUp)) { forwardAmount += moveSpeed; }
        if (controller.is_button_pressed(input::controller::button::DPadDown)) { forwardAmount -= moveSpeed; }

        if (controller.is_button_pressed(input::controller::button::DPadLeft)) { strafeAmount += moveSpeed; }
        if (controller.is_button_pressed(input::controller::button::DPadRight)) { strafeAmount -= moveSpeed; }

        constexpr f64 deadzone {5000.0}; // TODO: controller deadzone in ui
        constexpr f64 maxAxisValue {32768.0};

        auto const leftY {controller.get_axis_value(input::controller::axis::LeftY)};
        if (std::abs(leftY) > deadzone) {
            f64 const normalizedY {leftY / maxAxisValue};
            forwardAmount += -normalizedY * moveSpeed;
        }

        auto const leftX {controller.get_axis_value(input::controller::axis::LeftX)};
        if (std::abs(leftX) > deadzone) {
            f64 const normalizedX {leftX / maxAxisValue};
            strafeAmount += -normalizedX * moveSpeed;
        }

        auto const rightX {controller.get_axis_value(input::controller::axis::RightX)};
        if (std::abs(rightX) > deadzone) {
            f64 const normalizedRightX {rightX / maxAxisValue};
            rotateAmount += normalizedRightX * rotSpeed;
        }
    }

    return _player.move(*_level, forwardAmount, strafeAmount, rotateAmount);
}

void Plinth::on_key_down(input::keyboard::event const& ev)
{
    if (ev.Repeat) { return; }

    switch (ev.ScanCode) { // NOLINT
    case input::scan_code::BACKSPACE:
        parent().pop_current_scene();
        break;
    case input::scan_code::Q: {
        auto& spr {_level->Sprites[0]};
        spr.Facing   = spr.Position.angle_to(_player.Position);
        spr.Position = spr.Position.moved_along(degree_d {spr.Facing.Value}, 0.1);
        _draw        = true;
    } break;
    case input::scan_code::R: {
        locate_service<gfx::render_system>().statistics().reset();
    } break;
    case input::scan_code::F: {
        auto const fileName {[]() {
            for (i32 i {0};; ++i) {
                auto const name {std::format("screen{:02}.png", i)};
                if (!io::exists(name)) { return name; }
            }
        }()};
        std::ignore = window().copy_to_image().save(fileName);
    } break;
    case input::scan_code::SPACE: {
        _level->toggle_wall(point_i {_player.Position + _player.Direction});
    } break;
    case input::scan_code::TAB: {
        _drawMap = !_drawMap;
        _draw    = true;
    } break;
    default:

        break;
    }
}

void Plinth::on_controller_button_down(input::controller::button_event const& ev)
{
    switch (ev.Button) {
    case tcob::input::controller::button::A:
        _level->toggle_wall(point_i {_player.Position + _player.Direction});
        break;
    default: break;
    }
}
