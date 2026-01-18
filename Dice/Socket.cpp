// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Socket.hpp"

#include <algorithm>
#include <utility>

#include "Die.hpp"

////////////////////////////////////////////////////////////

socket::socket(socket_face face)
    : _face {std::move(face)}
{
}

auto socket::is_empty() const -> bool
{
    return _die == nullptr;
}

auto socket::current_die() const -> die*
{
    return _die;
}

auto socket::can_insert_die(die_face dieFace) const -> bool
{
    if (_die) { return false; }

    if (_face.Values && !_face.Values->empty()) {
        if (!_face.Values->contains(dieFace.Value)) { return false; }
    }

    if (_face.Colors && !_face.Colors->empty()) {
        if (!_face.Colors->contains(dieFace.Color)) { return false; }
    }
    return true;
}

void socket::insert_die(die* die)
{
    _die = die;
    if (_die) {
        _die->lock();
        _die->set_bounds(_bounds);
    }
}

auto socket::can_remove_die(die* die) const -> bool
{
    return _die && die == _die;
}

void socket::remove_die()
{
    if (_die) { _die->unlock(); }
    _die = nullptr;
}

void socket::move_to(point_f pos)
{
    _bounds = {pos, _bounds.Size};
    if (_die) {
        _die->set_bounds(_bounds);
    }
}

auto socket::state() const -> socket_state { return _state; }
auto socket::face() const -> socket_face const& { return _face; }

////////////////////////////////////////////////////////////

sockets::sockets(size_f scale)
    : _scale {scale}
{
}

void sockets::lock() { _locked = true; }

void sockets::unlock() { _locked = false; }

auto sockets::add_socket(socket_face const& face) -> socket*
{
    auto& retValue {_sockets.emplace_back(std::make_unique<socket>(face))};
    retValue->_bounds = {point_f::Zero, DICE_SIZE * _scale};
    return retValue.get();
}

void sockets::remove_socket(socket* socket)
{
    if (auto* die {socket->current_die()}) {
        socket->remove_die();
        die->mark_for_reroll();
    }
    helper::erase_first(_sockets, [&](auto const& s) { return s.get() == socket; });
}

auto sockets::try_insert_die(die* hoverDie) -> socket*
{
    if (_locked || !hoverDie || !_hoverSocket || !_hoverSocket->can_insert_die(hoverDie->current_face())) { return nullptr; }

    _hoverSocket->insert_die(hoverDie);
    _hoverSocket->_state = socket_state::Idle;
    return _hoverSocket;
}

auto sockets::try_remove_die(die* die) -> socket*
{
    if (_locked || !die) { return nullptr; }

    for (auto& socket : _sockets) {
        if (socket->can_remove_die(die)) {
            socket->remove_die();
            socket->_state = socket_state::Idle;
            return socket.get();
        }
    }

    return nullptr;
}

void sockets::hover(rect_f const& rect)
{
    if (_locked) {
        _hoverSocket = nullptr;
        return;
    }

    auto const find {[&]() -> socket* {
        socket* bestSocket {nullptr};
        f32     maxArea {0.0f};

        for (auto& s : _sockets) {
            if (!s->is_empty()) { continue; }

            auto const& socketRect {s->_bounds};
            auto const  interSect {rect.as_intersection_with(socketRect)};
            if (interSect.Size.area() > maxArea) {
                maxArea    = interSect.Size.area();
                bestSocket = s.get();
            }
        }

        return bestSocket;
    }};

    auto* socket {find()};
    if (_hoverSocket != socket) {
        if (_hoverSocket) {
            _hoverSocket->_state = socket_state::Idle;
        }
        _hoverSocket = socket;
    }
}

void sockets::on_drag(die* draggedDie)
{
    if (!draggedDie) { return; }
    hover(draggedDie->get_bounds());
    if (_hoverSocket) {
        _hoverSocket->_state = _hoverSocket->can_insert_die(draggedDie->current_face()) ? socket_state::Accept : socket_state::Reject;
    }
}

auto sockets::count() const -> usize
{
    return _sockets.size();
}

void sockets::reset()
{
    unlock();

    for (auto& socket : _sockets) {
        if (auto* die {socket->current_die()}) {
            socket->remove_die();
            die->mark_for_reroll();
        }
    }
}

auto get_hand(std::span<socket* const> sockets) -> hand
{
    if (sockets.size() > 5 || sockets.empty()) { return {}; }

    struct indexed_face {
        die_face Face;
        socket*  Socket;
    };

    static auto const func {[](auto const& f) { return f.Face.Value; }};

    std::vector<indexed_face> faces;
    faces.reserve(sockets.size());
    for (usize i {0}; i < sockets.size(); ++i) {
        if (sockets[i]->is_empty()) { continue; }
        faces.push_back({.Face = sockets[i]->current_die()->current_face(), .Socket = sockets[i]});
    }
    std::ranges::stable_sort(faces, {}, func);

    hand result {};

    // value
    if (sockets.size() >= 5 && (faces.back().Face.Value - faces.front().Face.Value == faces.size() - 1)
        && (std::ranges::adjacent_find(faces, {}, func) == faces.end())) {
        result.Value = value_category::Straight;
        for (auto const& f : faces) { result.Sockets.push_back(f.Socket); }
    } else {
        std::array<i8, 6> counts {};
        std::array<i8, 4> groups {};
        for (auto const& f : faces) {
            i8 const count {++counts[f.Face.Value - 1]};
            if (count >= 2 && count <= 5) {
                ++groups[count - 2];
                if (count > 2) { --groups[count - 3]; }
            }
        }

        auto const collect {[&](u8 targetValue) {
            for (auto const& f : faces) {
                if (f.Face.Value == targetValue) { result.Sockets.push_back(f.Socket); }
            }
        }};
        auto const collectAll {[&](u8 targetCount) {
            for (u8 v {0}; v < 6; ++v) {
                if (counts[v] == targetCount) { collect(v + 1); }
            }
        }};

        if (groups[3] == 1) {
            result.Value = value_category::FiveOfAKind; // FiveOfAKind
            collect(faces[0].Face.Value);
        } else if (groups[2] == 1) {
            result.Value = value_category::FourOfAKind; // FourOfAKind
            collectAll(4);
        } else if (groups[0] == 1 && groups[1] == 1) {
            result.Value = value_category::FullHouse;   // FullHouse
            collect(faces.front().Face.Value);
            collect(faces.back().Face.Value);
        } else if (groups[1] == 1) {
            result.Value = value_category::ThreeOfAKind; // ThreeOfAKind
            collectAll(3);
        } else if (groups[0] == 2) {
            result.Value = value_category::TwoPair;      // TwoPair
            collectAll(2);
        } else if (groups[0] == 1) {
            result.Value = value_category::OnePair;      // OnePair
            collectAll(2);
        }
    }

    // color
    std::unordered_set<u8> uniqueColors;
    for (auto const& f : faces) { uniqueColors.insert(f.Face.Color); }

    switch (uniqueColors.size()) {
    case 1:  result.Color = color_category::Flush; break;
    case 5:  result.Color = color_category::Rainbow; break;
    default: break;
    }

    return result;
}
auto get_sum(std::span<socket* const> sockets) -> u32
{
    u32 retValue {0};

    for (auto* const socket : sockets) {
        if (!socket->is_empty()) {
            retValue += socket->current_die()->current_face().Value;
        }
    }

    return retValue;
}