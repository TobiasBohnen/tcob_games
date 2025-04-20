// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../Common.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

struct pickup_result {
    string                    Message {};
    std::shared_ptr<inv_item> Item {};
};

class object {
public:
    virtual ~object() = default;

    point_i Position {};

    auto virtual symbol() const -> string     = 0;
    auto virtual colors() const -> color_pair = 0;

    auto virtual is_blocking() const -> bool = 0;

    auto virtual can_interact(player& player) const -> bool = 0;
    auto virtual interact(player& player) -> string         = 0;

    auto virtual can_pickup(player& player) const -> bool = 0;
    auto virtual pickup(player& player) -> pickup_result  = 0;
};

class feature : public object {
public:
    auto can_pickup(player& player) const -> bool override { return false; }
    auto pickup(player& player) -> pickup_result override { return {}; }
};

class item : public object {
public:
    auto is_blocking() const -> bool override { return false; }
    auto can_interact(player& player) const -> bool override { return false; }
    auto interact(player& player) -> string override { return ""; }
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class door : public feature {
public:
    auto symbol() const -> string override;
    auto colors() const -> color_pair override;

    auto is_blocking() const -> bool override;

    auto can_interact(player& player) const -> bool override;
    auto interact(player& player) -> string override;

private:
    bool _open {false};
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class gold : public item {
public:
    explicit gold(i32 amount);

    auto symbol() const -> string override;
    auto colors() const -> color_pair override;

    auto can_pickup(player& player) const -> bool override;
    auto pickup(player& player) -> pickup_result override;

private:
    i32 _amount;
};

////////////////////////////////////////////////////////////

enum class quality {
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
    wand(quality quality);

    auto symbol() const -> string override;
    auto colors() const -> color_pair override;

    auto can_pickup(player& player) const -> bool override;
    auto pickup(player& player) -> pickup_result override;
};

////////////////////////////////////////////////////////////

class rod : public item {
public:
    rod(quality quality);

    auto symbol() const -> string override;
    auto colors() const -> color_pair override;

    auto can_pickup(player& player) const -> bool override;
    auto pickup(player& player) -> pickup_result override;
};

////////////////////////////////////////////////////////////

class staff : public item {
public:
    staff(quality quality);

    auto symbol() const -> string override;
    auto colors() const -> color_pair override;

    auto can_pickup(player& player) const -> bool override;
    auto pickup(player& player) -> pickup_result override;
};

}
