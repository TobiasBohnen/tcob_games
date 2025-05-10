// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../Common.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

class object {
public:
    virtual ~object() = default;

    point_i Position {};

    auto virtual symbol() const -> string     = 0;
    auto virtual colors() const -> color_pair = 0;

    auto virtual is_blocking() const -> bool { return false; }

    auto virtual can_interact(actor& actor) const -> bool = 0;
    auto virtual interact(actor& actor) -> log_message    = 0;

    auto virtual on_enter(actor& actor) -> log_message { return {""}; }
    auto virtual on_search(actor& actor) -> log_message { return {""}; }
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class door : public object {
public:
    door(bool visible, bool open);

    auto symbol() const -> string override;
    auto colors() const -> color_pair override;

    auto is_blocking() const -> bool override;

    auto can_interact(actor& actor) const -> bool override;
    auto interact(actor& actor) -> log_message override;

    auto on_enter(actor& actor) -> log_message override;
    auto on_search(actor& actor) -> log_message override;

private:
    bool _visible;
    bool _open;
};

////////////////////////////////////////////////////////////
enum class trap_type : u8 {
    Spikes
};

class trap : public object {
public:
    trap(bool visible, trap_type type);

    auto symbol() const -> string override;
    auto colors() const -> color_pair override;

    auto can_interact(actor& actor) const -> bool override;
    auto interact(actor& actor) -> log_message override;

    auto on_enter(actor& actor) -> log_message override;
    auto on_search(actor& actor) -> log_message override;

private:
    bool      _visible;
    bool      _armed {true};
    trap_type _type;
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

enum class item_type : u8 {
    Potion,
    Weapon,
    Armor,
    Gold
};

class item : public object {
public:
    auto virtual amount() const -> i32 { return 1; }

    auto virtual can_pickup(actor& actor) const -> bool = 0;
    auto virtual pickup(actor& actor) -> log_message    = 0;

    auto virtual can_stack() const -> bool { return false; }
    auto virtual type() const -> item_type = 0;
    auto virtual name() const -> string    = 0;

private:
    auto can_interact(actor& actor) const -> bool override { return false; }
    auto interact(actor& actor) -> log_message override { return {""}; }
};

////////////////////////////////////////////////////////////

class gold : public item {
public:
    explicit gold(i32 amount);

    auto symbol() const -> string override;
    auto colors() const -> color_pair override;

    auto can_pickup(actor& actor) const -> bool override;
    auto pickup(actor& actor) -> log_message override;

    auto can_stack() const -> bool override;
    auto type() const -> item_type override;
    auto name() const -> string override;

    auto amount() const -> i32 override;

private:
    i32 _amount;
};

////////////////////////////////////////////////////////////

enum class quality : u8 {
    Poor,
    Common,
    Rare,
    Epic,
    Legendary
};

////////////////////////////////////////////////////////////
/* .   A floor space
^   A trap (known)
;   A glyph of warding
'   An open door
<   A staircase up
>   A staircase down
#   A wall
+   A closed door
%   A mineral vein
*   A mineral vein with treasure
:   A pile of rubble
!   A potion (or flask)
?   A scroll (or book)
,   A mushroom (or food)
-   A wand or rod
_   A staff
=   A ring
"   An amulet
$   Gold or gems
~   Lights, Tools, Chests, etc
&   Multiple items
/   A pole-arm
|   An edged weapon
\   A hafted weapon
// }   A sling, bow, or x-bow
{   A shot, arrow, or bolt
(   Soft armour
[   Hard armour
]   Misc. armour
)   A shield */
class wand : public item {
public:
    explicit wand(quality quality);

    auto symbol() const -> string override;
    auto colors() const -> color_pair override;

    auto can_pickup(actor& actor) const -> bool override;
    auto pickup(actor& actor) -> log_message override;

    auto type() const -> item_type override;
    auto name() const -> string override;
};

////////////////////////////////////////////////////////////

class rod : public item {
public:
    explicit rod(quality quality);

    auto symbol() const -> string override;
    auto colors() const -> color_pair override;

    auto can_pickup(actor& actor) const -> bool override;
    auto pickup(actor& actor) -> log_message override;

    auto type() const -> item_type override;
    auto name() const -> string override;
};

////////////////////////////////////////////////////////////

class staff : public item {
public:
    explicit staff(quality quality);

    auto symbol() const -> string override;
    auto colors() const -> color_pair override;

    auto can_pickup(actor& actor) const -> bool override;
    auto pickup(actor& actor) -> log_message override;

    auto type() const -> item_type override;
    auto name() const -> string override;
};

}
