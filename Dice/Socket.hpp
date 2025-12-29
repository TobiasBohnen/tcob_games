// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

////////////////////////////////////////////////////////////

enum class value_category : u8 {
    None,
    OnePair,
    TwoPair,
    ThreeOfAKind,
    FullHouse,
    Straight,
    FourOfAKind,
    FiveOfAKind,
};

enum class color_category : u8 {
    None,
    Flush,
    Rainbow,
};

////////////////////////////////////////////////////////////

struct hand {
    value_category Value {value_category::None};
    color_category Color {color_category::None};

    std::vector<socket*> Sockets;

    static auto constexpr Members()
    {
        return std::tuple {
            member<&hand::Value> {"value"},
            member<&hand::Color> {"color"},
            member<&hand::Sockets> {"sockets"}};
    }
};

////////////////////////////////////////////////////////////

enum class socket_state : u8 {
    Idle   = 0,
    Hover  = 1,
    Accept = 2,
    Reject = 3
};

struct socket_face {
    std::optional<std::unordered_set<u8>> Values;
    std::optional<std::unordered_set<u8>> Colors;

    auto operator==(socket_face const& other) const -> bool = default;

    static auto constexpr Members()
    {
        return std::tuple {member<&socket_face::Values, std::nullopt> {"values"},
                           member<&socket_face::Colors, std::nullopt> {"colors"}};
    }
};

////////////////////////////////////////////////////////////

class socket {
    friend class sockets;

public:
    explicit socket(socket_face face);

    auto is_empty() const -> bool;
    auto current_die() const -> die*;

    auto can_insert_die(die_face dieFace) const -> bool;
    void insert_die(die* die);
    auto can_remove_die(die* die) const -> bool;
    void remove_die();

    auto bounds() const -> rect_f const&;
    void move_to(point_f pos);

    auto state() const -> socket_state;
    auto face() const -> socket_face const&;

private:
    rect_f _bounds {};
    die*   _die {nullptr};

    socket_face _face;

    socket_state _state {socket_state::Idle};
};

////////////////////////////////////////////////////////////

class sockets {
public:
    explicit sockets(size_f scale);

    void lock();
    void unlock();

    auto add_socket(socket_face const& face) -> socket*;
    void remove_socket(socket* socket);
    auto count() const -> usize;

    auto try_insert_die(die* die) -> socket*;
    auto try_remove_die(die* die) -> socket*;

    void reset();

    void on_hover(point_f mp);
    void on_drag(die* draggedDie);

private:
    void hover(rect_f const& rect);
    auto are_filled() const -> bool;

    std::vector<std::unique_ptr<socket>> _sockets {};

    size_f _scale;

    bool _locked {false};

    socket* _hoverSocket {nullptr};
};

auto get_hand(std::span<socket* const> sockets) -> hand;