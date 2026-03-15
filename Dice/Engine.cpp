// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Engine.hpp"

#include "Die.hpp"
#include "Socket.hpp"
#include "SpriteManager.hpp"

using namespace scripting;

engine::engine(init init)
    : _init {init}
    , _hudProxy {_init.UIState.HUD, PALETTE[0]}
    , _fgProxy {_init.SpriteMgr.Foreground, colors::Transparent}
    , _bgProxy {_init.SpriteMgr.Background, PALETTE[0]}
    , _texProxy {_init.SpriteMgr.Sprites, colors::Transparent}
{
    _script.open_libraries(library::Table, library::String, library::Math);
    _script.open_addons();
    _script.Warning.connect([](script::warning_event const& ev) {
        logger::Warning(ev.Message);
    });

    create_env();
    create_wrappers();

    _init.Events.SpriteCollision.connect([this](auto const& ev) {
        if (_gameStatus != game_status::Running) { return; }
        call(_callbacks.OnCollision, ev.A, ev.B);
    });

    _init.Events.DieInsert.connect([this]([[maybe_unused]] socket const* socket) {
        if (_gameStatus == game_status::Waiting) {
            _updateHUD = true;
        }
    });
    _init.Events.DieRemove.connect([this]([[maybe_unused]] socket const* socket) {
        if (_gameStatus == game_status::Waiting) {
            _updateHUD = true;
        }
    });
    _init.Events.DieMotion.connect([this]([[maybe_unused]] die const* die) {
        if (_gameStatus == game_status::Waiting) {
            _updateHUD = true;
        }
    });

    _init.Events.TurnStart.connect([this]() { start_turn(); });
}

engine::~engine()
{
    _script.remove_hook();
}

template <typename R>
inline auto engine::call(callback<R> const& func, auto&&... args) -> R
{
    if (func) {
        auto& fn {*func};
        auto  result {fn.protected_call(_table, this, args...)};
        if (result) { return result.value(); }

        logger::Error("error calling function");
    }

    return R {};
}

void engine::run(string const& file)
{
    auto env {**_script.Environment};

    // require
    auto const path {io::get_parent_folder(file)};

    auto func {make_unique_closure(std::function {[this, path, &env](char const* module) {
        if (env.has("package", "loaded", module)) { return env["package"]["loaded"][module].as<table>(); }

        if (auto pkg {_script.run_file<table>(std::format("{}/{}.lua", path, module))}) {
            auto& val {pkg.value()};
            env["package"]["loaded"][module] = val;
            return val;
        }

        _script.view().error("module %s not found", module);
        return _script.create_table();
    }})};
    env["require"] = func.get();
    _funcs.push_back(std::move(func));

    _table = *_script.run_file<table>(file);

    _table.try_get(_callbacks.OnSetup, "on_setup");
    _table.try_get(_callbacks.OnTurnStart, "on_turn_start");
    _table.try_get(_callbacks.OnTurnUpdate, "on_turn_update");
    _table.try_get(_callbacks.OnTurnFinish, "on_turn_finish");
    _table.try_get(_callbacks.OnDrawHUD, "on_draw_hud");
    _table.try_get(_callbacks.OnCollision, "on_collision");

    call(_callbacks.OnSetup);
}

auto engine::update(milliseconds deltaTime) -> bool
{
    update_hud();

    _init.UIState.CanStartTurn = _gameStatus == game_status::Waiting;

    if (_gameStatus != game_status::Running) { return false; }

    for (auto& channel : _soundChannels) {
        channel.update();
    }

    auto const dt {deltaTime.count()};
    _turnTime += dt;
    _gameStatus = static_cast<game_status>(call(_callbacks.OnTurnUpdate, dt, _turnTime));

    switch (_gameStatus) {
    case game_status::Running: return true;
    case game_status::Waiting: {
        call(_callbacks.OnTurnFinish);
        _turnTime = 0;
        _init.Sockets.reset();
        _init.Events.TurnFinish();
        return false;
    }
    case game_status::GameOver: {
        _updateHUD = true;
        _init.Events.GameOver();
        return false;
    }
    }

    return true;
}

void engine::enable_debug(ui::form<ui::dock_layout>& form)
{
    using namespace tcob::ui;
    auto& overlay {form.create_container<panel>(dock_style::Fill, "DebugOverlay")};
    auto& layout {overlay.create_layout<dock_layout>()};
    auto& tv {layout.create_widget<tree_view>(dock_style::Fill, "DebugTree")};

    _script.set_hook([&tv, last = clock::now()](debug const& dbg) mutable {
        auto const now {clock::now()};
        if (now - last < 3ms) { return; }
        last = now;

        auto const info {dbg.get_info()};
        string     source {info.ShortSource};
        auto&      vw {dbg.view()};

        std::function<tree_view::node(table const&, string const&)> make_table_node {
            [&](table const& t, string const& label) -> tree_view::node {
                tree_view::node tableNode {.Item = {.Text = label + " = {table}"}, .Expanded = false};
                for (auto const& key : t.get_keys<std::variant<i32, string>>()) {
                    string const keyStr {std::visit([](auto const& k) { return std::format("{}", k); }, key)};
                    if (t.is<table>(key)) {
                        auto const inner {t[key].template as<table>()};
                        tableNode.Children.push_back(make_table_node(inner, keyStr));
                    } else if (sprite* sprite {nullptr}; t.try_get(sprite, key)) {
                        rect_f const                       bounds {sprite->Bounds};
                        std::vector<tree_view::node> const spriteProp {
                            {.Item = {.Text = std::format("x = {:.2f}", bounds.left())}},
                            {.Item = {.Text = std::format("y = {:.2f}", bounds.top())}},
                            {.Item = {.Text = std::format("w = {:.2f}", bounds.width())}},
                            {.Item = {.Text = std::format("h = {:.2f}", bounds.height())}},
                        };
                        tableNode.Children.push_back({.Item = {.Text = std::format("{} = {{sprite}}", keyStr)}, .Children = spriteProp});
                    } else if (socket* socket {nullptr}; t.try_get(socket, key)) {
                        std::vector<tree_view::node> const socketProp {
                            {.Item = {.Text = std::format("die value = {}", socket->is_empty() ? 0 : socket->current_die()->face().Value)}},
                        };
                        tableNode.Children.push_back({.Item = {.Text = std::format("{} = {{socket}}", keyStr)}, .Children = socketProp});
                    } else if (f64 dVal {0}; t.try_get(dVal, key)) {
                        tableNode.Children.push_back({.Item = {.Text = std::format("{} = {:.4f}", keyStr, dVal)}});
                    } else if (string strVal; t.try_get(strVal, key)) {
                        tableNode.Children.push_back({.Item = {.Text = std::format("{} = {}", keyStr, strVal)}});
                    }
                }
                return tableNode;
            }};

        std::function<void(tree_view::node&, tree_view::node const&)> preserve_expanded {
            [&](tree_view::node& dst, tree_view::node const& src) {
                dst.Expanded = src.Expanded;
                for (auto& dstChild : dst.Children) {
                    auto const cit {std::ranges::find_if(src.Children, [&](auto const& c) { return c.Item.Text == dstChild.Item.Text; })};
                    if (cit != src.Children.end()) { preserve_expanded(dstChild, *cit); }
                }
            }};

        tree_view::node sourceNode {.Item = {.Text = std::format("{}:{}", source, info.CurrentLine)}, .Expanded = true};

        for (i32 i {1};; ++i) {
            auto name {dbg.get_local(i)};
            if (name.empty()) { break; }
            if (name[0] == '(') {
                vw.pop(1);
                continue;
            }

            auto const t {vw.get_type(-1)};
            if (t == type::Table) {
                table tab {table::Acquire(vw, -1)};
                sourceNode.Children.push_back(make_table_node(tab, name));
            } else if (t == type::Userdata || t == type::LightUserdata) {
                // ignore for now
            } else if (t == type::Function) {
                // ignore
            } else if (t == type::Number) {
                f64 value {};
                i32 idx {-1};
                if (base_converter<f64>::From(vw, idx, value)) {
                    sourceNode.Children.push_back({.Item = {.Text = std::format("{} = {:.4f}", name, value)}});
                }
            } else {
                string value;
                i32    idx {-1};
                if (!base_converter<string>::From(vw, idx, value)) {
                    value = "<" + string {vw.type_name(-1)} + ">";
                }
                sourceNode.Children.push_back({.Item = {.Text = std::format("{} = {}", name, value)}});
            }
            vw.pop(1);
        }

        tv.Nodes.mutate([&](auto& nodes) {
            auto it {std::ranges::find_if(nodes, [&](auto const& n) {
                return n.Item.Text.starts_with(source);
            })};
            if (it != nodes.end()) {
                preserve_expanded(sourceNode, *it);
                *it = sourceNode;
            } else {
                nodes.push_back(sourceNode);
            }
            return true;
        });
    },
                     scripting::debug_mask {.Call = false, .Return = false, .Line = true, .Count = false});
}

auto engine::start_turn() -> bool
{
    if (_gameStatus != game_status::Waiting) { return false; }

    call(_callbacks.OnTurnStart);
    _gameStatus = game_status::Running;
    return true;
}

void engine::create_env()
{
    auto& global {_script.global_table()};

    // create environment
    table env {_script.create_table()};
    env["table"]              = global["table"];
    env["string"]             = global["string"];
    env["math"]               = global["math"];
    env["math"]["random"]     = nullptr;
    env["math"]["randomseed"] = nullptr;
    env["assert"]             = global["assert"];
    env["pairs"]              = global["pairs"];
    env["ipairs"]             = global["ipairs"];
    env["print"]              = global["print"];
    env["type"]               = global["type"];
    env["tonumber"]           = global["tonumber"];
    env["tostring"]           = global["tostring"];

    for (auto const& color : PALETTE_MAP) {
        env["Palette"][color.first] = color.second;
    }

    env["ScreenSize"] = VIRTUAL_SCREEN_SIZE;

    env["GameStatus"]["Running"]  = static_cast<i32>(game_status::Running);
    env["GameStatus"]["Waiting"]  = static_cast<i32>(game_status::Waiting);
    env["GameStatus"]["GameOver"] = static_cast<i32>(game_status::GameOver);

    env["Rot"]["R0"]   = 0;
    env["Rot"]["R90"]  = 90;
    env["Rot"]["R180"] = 180;
    env["Rot"]["R270"] = 270;

    _script.Environment = env;
}

void engine::create_wrappers()
{
    create_sprite_wrapper();
    create_socket_wrapper();
    create_engine_wrapper();
    create_tex_wrapper();
}

void engine::create_sprite_wrapper()
{
    auto env {**_script.Environment};

    auto newFunc {make_unique_closure(std::function {
        [this](table const& spriteOwner) -> sprite* {
            sprite_def const def {spriteOwner["spriteInit"].get<sprite_def>().value_or(sprite_def {})};

            texture*   tex {nullptr};
            auto const texID {def.TexID};
            if (texID && _textures.contains(*texID)) { tex = &_textures[*texID]; }

            return _init.SpriteMgr.add({.Def = def, .Texture = tex, .Owner = spriteOwner});
        }})};
    env["sprite"]["new"] = newFunc.get();
    _funcs.push_back(std::move(newFunc));

    auto sortFunc {make_unique_closure(std::function {[this]() { _init.SpriteMgr.sort_by_y(); }})};
    env["sprite"]["y_sort"] = sortFunc.get();
    _funcs.push_back(std::move(sortFunc));

    auto& spriteWrapper {*_script.create_wrapper<sprite>("sprite")};
    spriteWrapper["position"] = property {
        [](sprite* sprite) -> point_f { return sprite->Bounds.Position; },
        [](sprite* sprite, point_f p) { sprite->set_bounds(p, sprite->Bounds.Size); }};
    spriteWrapper["size"] = getter {
        [](sprite* sprite) -> size_f { return sprite->Bounds.Size; }};
    spriteWrapper["center"] = getter {
        [](sprite* sprite) -> point_f { return sprite->Bounds.center(); }};
    spriteWrapper["owner"] = getter {
        [](sprite* sprite) -> table const& { return sprite->owner(); }};
    spriteWrapper["texture"] = property {
        [](sprite* sprite) -> u32 { return sprite->get_texture()->ID; },
        [this](sprite* sprite, u32 texID) {
            if (!_textures.contains(texID)) {
                logger::Error("Invalid texture id: {}", texID);
                return;
            }
            sprite->set_texture(&_textures[texID]);
        }};
    spriteWrapper["remove"] = [this](sprite* sprite) { _init.SpriteMgr.remove(sprite); };
}

void engine::create_socket_wrapper()
{
    auto env {**_script.Environment};

    auto func {make_unique_closure(std::function {
        [this](table const& socketInit) -> socket* { return _init.Sockets.add(socketInit.get<socket_face>().value_or(socket_face {})); }})};
    env["socket"]["new"] = func.get();
    _funcs.push_back(std::move(func));
    static auto const convert_sockets {[](std::unordered_map<std::variant<i32, string>, socket*> const& socketMap) {
        std::vector<socket*> sockets;
        sockets.reserve(socketMap.size());
        for (auto const& key : socketMap) { sockets.push_back(key.second); }
        return sockets;
    }};
    env["socket"]["get_hand"] = +[](std::unordered_map<std::variant<i32, string>, socket*> const& sockets) -> hand {
        return get_hand(convert_sockets(sockets));
    };
    env["socket"]["get_value"] = +[](std::unordered_map<std::variant<i32, string>, socket*> const& sockets) -> std::pair<u32, u32> {
        auto socketVec {convert_sockets(sockets)};

        auto const sum {get_sum(socketVec)};

        i32 hand {0};
        switch (get_hand(socketVec).Value) {
        case value_category::None:         hand = 1; break;
        case value_category::OnePair:      hand = 2; break;
        case value_category::TwoPair:      hand = 3; break;
        case value_category::ThreeOfAKind: hand = 4; break;
        case value_category::Straight:     hand = 5; break;
        case value_category::FullHouse:    hand = 6; break;
        case value_category::FourOfAKind:  hand = 8; break;
        case value_category::FiveOfAKind:  hand = 10; break;
        }

        return {sum, hand};
    };

    auto& socketWrapper {*_script.create_wrapper<socket>("socket")};
    socketWrapper["is_empty"] = getter {
        [](socket* socket) -> bool { return socket->is_empty(); }};
    socketWrapper["die_value"] = getter {
        [](socket* socket) -> u8 { return socket->is_empty() ? 0 : socket->current_die()->face().Value; }};
    socketWrapper["die_color"] = getter {
        [](socket* socket) -> std::optional<u8> { return socket->is_empty() ? std::nullopt : std::optional<u8> {socket->current_die()->face().Color}; }};
    socketWrapper["position"] = property {
        [](socket* socket) -> point_i { return socket->HUDPosition; },
        [this](socket* socket, point_f pos) {
            socket->HUDPosition = point_i {pos};
            auto const&   rect {_init.UIState.HUDBounds};
            point_f const spos {rect.left() + (rect.width() * (pos.X / static_cast<f32>(HUD_SIZE.Width))),
                                rect.top() + (rect.height() * (pos.Y / static_cast<f32>(HUD_SIZE.Height)))};
            socket->move_to(spos);
        }};
    socketWrapper["remove"] = [this](socket* socket) { _init.Sockets.remove(socket); };
}

void engine::create_engine_wrapper()
{
    auto& engineWrapper {*_script.create_wrapper<engine>("engine")};
    // gfx
    engineWrapper["define_texture"] = [](engine* engine, u32 id, rect_f const& uv) { engine->define_texture(id, rect_i {uv}); };

    // sfx
    engineWrapper["define_sound"] = [](engine* engine, u32 id, audio::sound_wave soundWave) {
        engine->_soundBank.emplace(id, audio::sound_generator {}.create_buffer(soundWave));
    };
    engineWrapper["play_sound"] = [](engine* engine, u32 soundID, std::optional<u8> channelID, std::optional<bool> now) {
        if (channelID >= engine->_soundChannels.size()) {
            logger::Error("Invalid channel id: {}", *channelID);
            return;
        }

        auto     sound {std::make_unique<audio::sound>(engine->_soundBank[soundID])};
        u8 const channel {channelID.value_or(0)};
        if (now.value_or(true)) {
            engine->_soundChannels[channel].play_now(std::move(sound));
        } else {
            engine->_soundChannels[channel].play_queued(std::move(sound));
        }
    };

    // functions
    engineWrapper["rnd"]  = [](engine* engine, f32 min, f32 max) { return engine->_init.Rng(min, max); };
    engineWrapper["irnd"] = [](engine* engine, i32 min, i32 max) { return engine->_init.Rng(min, max); };
    engineWrapper["log"]  = [](engine*, string const& str) { logger::Info(str); };

    engineWrapper["give_score"] = [](engine* engine, i32 score) { engine->_init.UIState.Score += score; };
    engineWrapper["update_hud"] = [](engine* engine) { engine->_updateHUD = true; };

    // properties
    engineWrapper["fg"]  = getter {[](engine* engine) -> tex_proxy* { return &engine->_fgProxy; }};
    engineWrapper["bg"]  = getter {[](engine* engine) -> tex_proxy* { return &engine->_bgProxy; }};
    engineWrapper["spr"] = getter {[](engine* engine) -> tex_proxy* { return &engine->_texProxy; }};
    engineWrapper["ssd"] = property {
        [](engine* engine) -> string { return *engine->_init.UIState.SSD; },
        [](engine* engine, string const& val) { engine->_init.UIState.SSD = val; }};
    engineWrapper["is_game_over"] = getter {[](engine* engine) -> bool { return engine->_gameStatus == game_status::GameOver; }};
}

void engine::create_tex_wrapper()
{
    auto& texWrapper {*_script.create_wrapper<tex_proxy>("tex")};
    texWrapper["bounds"] = getter {[](tex_proxy* tex) -> rect_i { return tex->bounds(); }};
    texWrapper["size"]   = getter {[](tex_proxy* tex) -> size_i { return tex->bounds().Size; }};

    texWrapper["clear"] = [](tex_proxy* tex, std::optional<rect_f> const& rect) {
        std::optional<rect_i> r {std::nullopt};
        if (rect) { r = rect_i {*rect}; }
        tex->clear(r);
    };

    texWrapper["pixel"]  = [](tex_proxy* tex, point_f pos, u8 color) { tex->pixel(point_i {pos}, color); };
    texWrapper["line"]   = [](tex_proxy* tex, point_f start, point_f end, u8 color) { tex->line(point_i {start}, point_i {end}, color); };
    texWrapper["circle"] = [](tex_proxy* tex, point_f center, f32 radius, u8 color, bool fill) { tex->circle(point_i {center}, static_cast<i32>(radius), color, fill); };
    texWrapper["rect"]   = [](tex_proxy* tex, rect_f const& rect, u8 color, bool fill) { tex->rect(rect_i {rect}, color, fill); };

    texWrapper["blit"] = [](tex_proxy* tex, rect_f const& rect, string const& dotStr, std::optional<blit_settings> settings) {
        tex->blit(rect_i {rect}, dotStr, settings ? *settings : blit_settings {});
    };
    texWrapper["print"] = [](tex_proxy* tex, point_f pos, string_view text, u8 color, std::optional<font_type> fontType) {
        tex->print(point_i {pos}, text, color, fontType ? *fontType : font_type::Font5x5);
    };
    texWrapper["socket"] = [](tex_proxy* tex, socket* socket) { tex->socket(socket); };
}

void engine::define_texture(u32 id, rect_i const& uv)
{
    auto& tex {_textures[id]};
    tex.ID     = id;
    tex.Size   = size_f {uv.Size};
    tex.Region = std::to_string(id);
    tex.Alpha  = grid<u8> {uv.Size};

    _init.SpriteMgr.define_texture_region(tex.Region, uv);

    for (i32 y {0}; y < uv.height(); ++y) {
        for (i32 x {0}; x < uv.width(); ++x) {
            color const col {_init.SpriteMgr.Sprites->get_pixel(point_i {x, y} + uv.top_left())};
            tex.Alpha[x, y] = col.A;
        }
    }
}

void engine::update_hud()
{
    if (_updateHUD) {
        call(_callbacks.OnDrawHUD, &_hudProxy);
        _updateHUD = false;
    }
}
