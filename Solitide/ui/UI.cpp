// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "UI.hpp"

#include <utility>

#include "Cardset.hpp" // IWYU pragma: keep
#include "GameInfo.hpp"
#include "Themes.hpp"  // IWYU pragma: keep

namespace solitaire {

using namespace tcob::literals;

auto static translate(std::string_view name) -> std::string // NOLINT
{
    // TODO: translation
    if (name == "btnMenu") { return "Menu"; }
    if (name == "btnNewGame") { return "New Game"; }
    if (name == "btnWizard") { return "Solitaire Wizard"; }
    if (name == "btnHint") { return "Hint"; }
    if (name == "btnCollect") { return "Collect All"; }
    if (name == "btnUndo") { return "Undo"; }
    if (name == "btnQuit") { return "Quit"; }
    if (name == "btnFilter") { return "Clear Filter"; }
    if (name == "btnStartGame") { return "Start Game"; }
    if (name == "btnApplyVideoSettings") { return "Apply"; }
    if (name == "btnBack") { return "Back"; }
    if (name == "lblResolution") { return "Resolution"; }
    if (name == "lblFullScreen") { return "Fullscreen"; }
    if (name == "lblVSync") { return "VSync"; }
    if (name == "lblHintMovable") { return "Highlight valid moves"; }
    if (name == "lblHintDrops") { return "Highlight valid targets"; }
    if (name == "btnApplyCardset") { return "Apply"; }
    return "";
}

auto static make_tooltip(form* form) -> std::shared_ptr<tooltip>
{
    auto retValue {form->create_tooltip<tooltip>("tooltip")};
    auto tooltipLayout {retValue->create_layout<dock_layout>()};
    auto tooltipLabel {tooltipLayout->create_widget<label>(dock_style::Fill, "TTLabel0")};
    tooltipLabel->Class = "tooltip-label";
    retValue->Popup.connect([lbl = tooltipLabel.get(), tt = retValue.get()](auto const& event) {
        auto const widget {event.Widget};
        lbl->Label = translate(widget->get_name());

        auto const  wBounds {widget->Bounds()};
        auto const& lStyle {lbl->get_style<label::style>()->Text};
        auto* const font {lStyle.Font->get_font(lStyle.Style, lStyle.calc_font_size({0, 0, wBounds.Width * 1.5f, wBounds.Height * 0.75f})).get_obj()};
        tt->Bounds = {point_f::Zero, gfx::text_formatter::measure(lbl->Label(), *font, -1, true)};
    });
    return retValue;
}

form_controls::form_controls(gfx::window* window, assets::group& resGrp)
    : form {"Controls", window}
{
    (*Controls).ActivateKey = input::key_code::UNKNOWN;

    // tooltip
    auto tooltip0 {make_tooltip(this)};

    auto mainPanel {create_container<glass>(dock_style::Fill, "main")};
    auto mainPanelLayout {mainPanel->create_layout<dock_layout>()};

    // menu
    {
        auto menuPanel {mainPanelLayout->create_widget<panel>(dock_style::Top, "menu")};
        menuPanel->Flex = {100_pct, 5_pct};
        auto menuPanelLayout {menuPanel->create_layout<grid_layout>(size_i {20, 1})};

        auto const create {[&](rect_i const& bounds, std::string const& name, std::string const& tex) {
            auto retValue {menuPanelLayout->create_widget<button>(bounds, name)};
            retValue->Icon    = resGrp.get<gfx::texture>(tex);
            retValue->Tooltip = tooltip0;
            return retValue;
        }};

        BtnMenu    = create({0, 0, 1, 1}, "btnMenu", "burger");
        BtnWizard  = create({2, 0, 1, 1}, "btnWizard", "wand");
        BtnNewGame = create({12, 0, 1, 1}, "btnNewGame", "newgame");
        BtnHint    = create({14, 0, 1, 1}, "btnHint", "hint");
        BtnCollect = create({15, 0, 1, 1}, "btnCollect", "collect");
        BtnUndo    = create({16, 0, 1, 1}, "btnUndo", "undo");
        BtnQuit    = create({19, 0, 1, 1}, "btnQuit", "exit");
    }

    // status
    {
        auto statusPanel {mainPanelLayout->create_widget<panel>(dock_style::Bottom, "status")};
        statusPanel->Flex = {100_pct, 10_pct};
        auto statusPanelLayout {statusPanel->create_layout<grid_layout>(size_i {20, 6})};

        i32        i {0};
        auto const create {[&](rect_i const& rect, std::string const& text = "") {
            auto l {statusPanelLayout->create_widget<label>(rect, "lblInfo" + std::to_string(i))};
            l->Class = "label-small";
            l->Label = text;
            return l;
        }};

        _lblGameName = create({0, 0, 2, 6});

        _lblPile           = create({2, 3, 2, 3});
        _lblPileLabel      = create({2, 0, 2, 3}, "Pile");  // translate
        _lblCardCount      = create({4, 3, 1, 3});
        _lblCardCountLabel = create({4, 0, 1, 3}, "Cards"); // translate

        _lblBase             = create({6, 3, 3, 3});
        _lblBaseLabel        = create({6, 0, 3, 3});
        _lblDescription      = create({9, 3, 3, 3});
        _lblDescriptionLabel = create({9, 0, 3, 3});
        _lblMove             = create({12, 3, 3, 3});
        _lblMoveLabel        = create({12, 0, 3, 3});

        _lblTurns      = create({17, 3, 1, 3});
        _lblTurnsLabel = create({17, 0, 1, 3}, "Turns"); // translate

        _lblScore      = create({18, 3, 1, 3});
        _lblScoreLabel = create({18, 0, 1, 3}, "Score"); // translate

        _lblTime      = create({19, 3, 1, 3});
        _lblTimeLabel = create({19, 0, 1, 3}, "Time");   // translate
    }

    auto overlayPanel {mainPanelLayout->create_widget<glass>(dock_style::Fill, "overlay")};
    overlayPanel->disable();
    auto overlayPanelLayout {overlayPanel->create_layout<dock_layout>()};

    Canvas = overlayPanelLayout->create_widget<canvas_widget>(dock_style::Fill, "canvas");
}

void form_controls::set_pile_labels(pile_description const& str)
{
    _lblPile->Label      = str.Pile;
    _lblCardCount->Label = str.CardCount;

    _lblDescription->Label      = str.Description;
    _lblDescriptionLabel->Label = str.DescriptionLabel;
    _lblMove->Label             = str.Move;
    _lblMoveLabel->Label        = str.MoveLabel;
    _lblBase->Label             = str.Base;
    _lblBaseLabel->Label        = str.BaseLabel;
}

void form_controls::set_game_labels(base_game* game)
{
    auto const& info {game->info()};
    _lblGameName->Label = info.Name;
    auto const& state {game->state()};
    _lblTurns->Label = std::to_string(state.Turns);
    _lblScore->Label = std::to_string(state.Score);
    _lblTime->Label  = std::format("{:%M:%S}", seconds {state.Time.count() / 1000});
}

////////////////////////////////////////////////////////////

static std::string const TabGamesName {"conGames"};
static std::string const TabSettingsName {"conSettings"};
static std::string const TabThemesName {"conThemes"};
static std::string const TabCardsetsName {"conCardsets"};
static std::string const MenuName {"menu"};

form_menu::form_menu(gfx::window* window, assets::group& resGrp, std::shared_ptr<menu_sources> sources)
    : form {"Menu", window}
    , _resGrp {resGrp}
    , _sources {std::move(sources)}
{
    // tooltip
    _tooltip = make_tooltip(this);

    create_section_games();
    create_section_settings();
    create_section_themes();
    create_section_cardset();
    create_menubar();
}

void form_menu::submit_settings(data::config::object& obj)
{
    _tabSettings->submit(obj);
}

void form_menu::create_section_games()
{
    // Games
    auto panelGames {create_container<panel>(dock_style::Left, TabGamesName)};
    panelGames->Flex   = {85_pct, 100_pct};
    panelGames->ZOrder = 5;
    auto panelLayout {panelGames->create_layout<dock_layout>()};

    // Filter
    auto panelFilter {panelLayout->create_widget<panel>(dock_style::Top, "pnlFilter")};
    panelFilter->Flex = {100_pct, 5_pct};
    auto panelFilterLayout {panelFilter->create_layout<grid_layout>(size_i {10, 1})};
    auto txbFilter {panelFilterLayout->create_widget<text_box>({0, 0, 9, 1}, "txbFilter")};
    txbFilter->MaxLength = 30;
    auto btnFilter {panelFilterLayout->create_widget<button>({9, 0, 1, 1}, "btnFilter")};
    btnFilter->Icon = _resGrp.get<gfx::texture>("lens");
    btnFilter->Click.connect([tb = txbFilter.get()]() { tb->Text = ""; });
    btnFilter->Tooltip = _tooltip;

    std::vector<list_box*> listBoxes;
    {
        auto tabGames {panelLayout->create_widget<tab_container>(dock_style::Left, "tabGames")};
        tabGames->Flex    = {50_pct, 100_pct};
        tabGames->MaxTabs = 5;

        auto const createListBox {[&](std::shared_ptr<dock_layout>& tabPanelLayout, std::string const& name, auto&& pred) -> std::shared_ptr<list_box> {
            auto listBox {tabPanelLayout->create_widget<list_box>(dock_style::Fill, "lbxGames" + name)};
            listBoxes.push_back(listBox.get());

            for (auto const& game : _sources->Games) {
                if (pred(game.second.first)) { listBox->add_item(game.first); }
            }

            listBox->SelectedItemIndex.Changed.connect([&, lb = listBox.get()](auto val) {
                if (val != -1) { _sources->Settings.Game = lb->get_selected_item(); }
            });
            listBox->select_item(_sources->Settings.Game);
            _sources->Settings.Game.Changed.connect([&, lb = listBox.get()](auto const& val) {
                if (lb->select_item(val)) {
                    if (!lb->is_focused()) { lb->scroll_to_selected(); }
                } else {
                    lb->SelectedItemIndex = -1;
                }
            });

            listBox->DoubleClick.connect([&, lb = listBox.get()] {
                if (lb->SelectedItemIndex >= 0) {
                    StartGame(_txbSeed->Text());
                    _txbSeed->Text = "";
                    hide();
                }
            });
            return listBox;
        }};

        // By Name
        {
            auto tabPanel {tabGames->create_tab<panel>("byName", "By Name")}; // translate
            auto tabPanelLayout {tabPanel->create_layout<dock_layout>()};
            auto listBox {createListBox(tabPanelLayout, "0", [](auto const&) { return true; })};
            _sources->GameAdded.connect([&, lb = listBox.get()] {
                // TODO: update all listboxes
                lb->clear_items();
                for (auto const& game : _sources->Games) { lb->add_item(game.first); }
            });
        }
        // Recent
        {
            auto tabPanel {tabGames->create_tab<panel>("recent", "Recent")}; // translate
            auto tabPanelLayout {tabPanel->create_layout<dock_layout>()};
            auto listBox {createListBox(tabPanelLayout, "Recent", [](auto const&) { return false; })};
            for (auto const& game : _sources->Settings.Recent()) { listBox->add_item(game); }
            _sources->Settings.Recent.Changed.connect([&, lb = listBox.get()] {
                lb->clear_items();
                for (auto const& game : _sources->Settings.Recent()) { lb->add_item(game); }
            });
        }
        // By Family
        {
            auto tabContainer {tabGames->create_tab<tab_container>("byFamily", "By Family")}; // translate
            tabContainer->MaxTabs = 5;

            auto const createTab {[&](family family, std::string const& name) {
                auto tabPanel {tabContainer->create_tab<panel>(name)};
                auto tabPanelLayout {tabPanel->create_layout<dock_layout>()};
                createListBox(tabPanelLayout, name, [family](auto const& gameInfo) { return gameInfo.Family == family; });
            }};
            createTab(family::BakersDozen, "Baker's Dozen");
            createTab(family::BeleagueredCastle, "Beleaguered Castle");
            createTab(family::Canfield, "Canfield");
            createTab(family::Fan, "Fan");
            createTab(family::FlowerGarden, "Flower Garden");
            createTab(family::FortyThieves, "Forty Thieves");
            createTab(family::FreeCell, "FreeCell");
            createTab(family::Golf, "Golf");
            createTab(family::Gypsy, "Gypsy");
            createTab(family::Klondike, "Klondike");
            createTab(family::Montana, "Montana");
            createTab(family::Numerica, "Numerica");
            createTab(family::PictureGallery, "Picture Gallery");
            createTab(family::Spider, "Spider");
            createTab(family::Terrace, "Terrace");
            createTab(family::Yukon, "Yukon");

            createTab(family::Other, "Other");
        }
        // By Deck Count
        {
            auto tabContainer {tabGames->create_tab<tab_container>("byDeckCount", "By Deck Count")}; // translate
            tabContainer->MaxTabs = 5;

            auto const createTab {[&](isize count, std::string const& name) {
                auto tabPanel {tabContainer->create_tab<panel>(name)};
                auto tabPanelLayout {tabPanel->create_layout<dock_layout>()};
                createListBox(tabPanelLayout, name, [count](auto const& gameInfo) { return gameInfo.DeckCount == count; });
            }};
            createTab(1, "1");
            createTab(2, "2");
            createTab(3, "3");

            {
                auto tabPanel {tabContainer->create_tab<panel>(">= 4")}; // translate
                auto tabPanelLayout {tabPanel->create_layout<dock_layout>()};
                createListBox(tabPanelLayout, "4+", [](auto const& gameInfo) { return gameInfo.DeckCount >= 4; });
            }
            {
                auto tabPanel {tabContainer->create_tab<panel>("stripped")}; // translate
                auto tabPanelLayout {tabPanel->create_layout<dock_layout>()};
                createListBox(tabPanelLayout, "stripped", [](auto const& gameInfo) { return gameInfo.DeckRanks.size() < 13 || gameInfo.DeckSuits.size() < 4; });
            }
        }

        txbFilter->Text.Changed.connect([listBoxes](auto const& val) {
            for (auto* lb : listBoxes) {
                lb->Filter = val;
            }
        });
    }
    {
        auto panelGameStats {panelLayout->create_widget<panel>(dock_style::Right, "panelGameStats")};
        panelGameStats->Flex   = {50_pct, 100_pct};
        panelGameStats->ZOrder = 1;
        auto panelGameStatsLayout {panelGameStats->create_layout<grid_layout>(size_i {20, 40})};

        auto gvWL {panelGameStatsLayout->create_widget<grid_view>({1, 1, 9, 4}, "gvWinLose")};
        gvWL->Class = "grid_view2";
        gvWL->set_columns({"Won", "Lost", "W/L"});                         // translate
        auto gvTT {panelGameStatsLayout->create_widget<grid_view>({10, 1, 9, 4}, "gvBest")};
        gvTT->Class = "grid_view2";
        gvTT->set_columns({"Highscore", "Least Turns", "Fastest Time"});   // translate

        auto gvHistory {panelGameStatsLayout->create_widget<grid_view>({1, 6, 18, 25}, "gvHistory")};
        gvHistory->set_columns({"Seed", "Score", "Turns", "Time", "Won"}); // translate

        _sources->CurrentHistory.Changed.connect([wl = gvWL.get(), tt = gvTT.get(), history = gvHistory.get()](auto const& stats) {
            wl->clear_rows();
            wl->add_row(
                {std::to_string(stats.Won),
                 std::to_string(stats.Lost),
                 stats.Lost + stats.Won > 0 ? std::format("{:.2f}%", static_cast<f32>(stats.Won) / (stats.Lost + stats.Won) * 100) : "-"});
            tt->clear_rows();

            std::optional<i64> bestTime;
            std::optional<i64> bestTurns;
            std::optional<i64> bestScore;

            history->clear_rows();
            for (auto const& entry : stats.Entries) {
                if (entry.Won) {
                    bestTime  = !bestTime ? entry.Time : std::min(entry.Time, *bestTime);
                    bestTurns = !bestTurns ? entry.Turns : std::min(entry.Turns, *bestTurns);
                }
                bestScore = !bestScore ? entry.Score : std::max(entry.Score, *bestScore);

                history->add_row(
                    {std::to_string(entry.Seed),
                     std::to_string(entry.Score),
                     std::to_string(entry.Turns),
                     std::format("{:%M:%S}", seconds {entry.Time / 1000.f}),
                     entry.Won ? "X" : "-"});
            }
            tt->add_row({bestScore ? std::to_string(*bestScore) : "-",
                         bestTurns ? std::to_string(*bestTurns) : "-",
                         bestTime ? std::format("{:%M:%S}", seconds {*bestTime / 1000.f}) : "--:--"});
        });

        auto lblSeed {panelGameStatsLayout->create_widget<label>({1, 33, 4, 2}, "lblSeed")};
        lblSeed->Label = "Seed";
        lblSeed->Class = "label-small";
        _txbSeed       = panelGameStatsLayout->create_widget<text_box>({6, 33, 8, 2}, "txbSeed");
        _txbSeed->BeforeTextInserted.connect([](text_event& ev) {
            if (ev.Text.empty()) { return; }
            if (ev.Text.size() != 1 || !std::isdigit(ev.Text[0])) {
                ev.Text = "";
            }
        });

        auto btnStartGame {panelGameStatsLayout->create_widget<button>({1, 36, 4, 3}, "btnStartGame")};
        btnStartGame->Icon = _resGrp.get<gfx::texture>("play");
        btnStartGame->Click.connect([&]() {
            StartGame(_txbSeed->Text());
            _txbSeed->Text = "";
            hide();
        });
        btnStartGame->Tooltip = _tooltip;
    }
}

void form_menu::create_section_settings()
{
    // Setting
    _tabSettings         = create_container<tab_container>(dock_style::Left, TabSettingsName);
    _tabSettings->ZOrder = 4;
    _tabSettings->Flex   = {0_pct, 0_pct};

    {
        auto const& config {locate_service<data::config_file>()};
        auto        tabPanel {_tabSettings->create_tab<panel>("Video")};
        auto        tabPanelLayout {tabPanel->create_layout<grid_layout>(size_i {40, 40})};

        // resolution
        {
            auto const displayModes {locate_service<gfx::render_system>().get_displays()};
            auto       ddlRes {tabPanelLayout->create_widget<drop_down_list>({6, 1, 6, 4}, "ddlResolution")};
            ddlRes->ZOrder = 1;
            for (auto const& dm : displayModes.at(0).Modes) {
                ddlRes->add_item(std::format("{}x{}", dm.Size.Width, dm.Size.Height));
            }

            auto const res {config[Cfg::Video::Name][Cfg::Video::resolution].as<size_i>()};
            ddlRes->select_item(std::format("{}x{}", res.Width, res.Height));

            auto lbl {tabPanelLayout->create_widget<label>({1, 2, 4, 2}, "lblResolution")};
            lbl->Label = translate(lbl->get_name());
        }

        // fullscreen
        {
            auto chk {tabPanelLayout->create_widget<checkbox>({6, 5, 3, 4}, "chkFullScreen")};
            chk->Checked = config[Cfg::Video::Name][Cfg::Video::fullscreen].as<bool>();

            auto lbl {tabPanelLayout->create_widget<label>({1, 6, 4, 2}, "lblFullScreen")};
            lbl->Label = translate(lbl->get_name());
        }

        // vsync
        {
            auto chk {tabPanelLayout->create_widget<checkbox>({6, 9, 3, 4}, "chkVSync")};
            chk->Checked = config[Cfg::Video::Name][Cfg::Video::vsync].as<bool>();

            auto lbl {tabPanelLayout->create_widget<label>({1, 10, 4, 2}, "lblVSync")};
            lbl->Label = translate(lbl->get_name());
        }

        auto btnApplyVideoSettings {tabPanelLayout->create_widget<button>({33, 35, 6, 4}, "btnApplyVideoSettings")};
        btnApplyVideoSettings->Icon    = _resGrp.get<gfx::texture>("apply");
        btnApplyVideoSettings->Tooltip = _tooltip;
        btnApplyVideoSettings->Click.connect([&]() { VideoSettingsChanged(); });
    }
    {
        auto tabPanel {_tabSettings->create_tab<panel>("Hints")};
        auto tabPanelLayout {tabPanel->create_layout<grid_layout>(size_i {40, 40})};
        // highlight movable
        {
            auto chk {tabPanelLayout->create_widget<checkbox>({10, 1, 3, 4}, "chkHintMovable")};
            chk->Checked = _sources->Settings.HintMovable;
            chk->Checked.Changed.connect([this](auto val) { _sources->Settings.HintMovable = val; });

            auto lbl {tabPanelLayout->create_widget<label>({1, 2, 8, 2}, "lblHintMovable")};
            lbl->Label = translate(lbl->get_name());
        }
        // highlight drops
        {
            auto chk {tabPanelLayout->create_widget<checkbox>({10, 5, 3, 4}, "chkHintDrops")};
            chk->Checked = _sources->Settings.HintTarget;
            chk->Checked.Changed.connect([this](auto val) { _sources->Settings.HintTarget = val; });

            auto lbl {tabPanelLayout->create_widget<label>({1, 6, 8, 2}, "lblHintDrops")};
            lbl->Label = translate(lbl->get_name());
        }
    }
}

void form_menu::create_section_themes()
{
    // Themes
    auto panelThemes {create_container<panel>(dock_style::Left, TabThemesName)};
    panelThemes->ZOrder = 3;
    panelThemes->Flex   = {0_pct, 0_pct};
    {
        auto panelLayout {panelThemes->create_layout<dock_layout>()};
        auto lbxThemes {panelLayout->create_widget<list_box>(dock_style::Fill, "lbxThemes")};
        for (auto const& colorTheme : _sources->Themes) { lbxThemes->add_item(colorTheme.first); }
        lbxThemes->SelectedItemIndex.Changed.connect([&, lb = lbxThemes.get()](auto val) {
            if (val != -1) { _sources->Settings.Theme = lb->get_selected_item(); }
        });
        lbxThemes->DoubleClick.connect([&] { hide(); });
        _sources->Settings.Theme.Changed.connect([lb = lbxThemes.get()](auto const& val) { lb->select_item(val); });
        lbxThemes->select_item(_sources->Settings.Theme);
    }
}

void form_menu::create_section_cardset()
{
    // Cardsets
    auto panelCardsets {create_container<panel>(dock_style::Left, TabCardsetsName)};
    panelCardsets->ZOrder = 2;
    panelCardsets->Flex   = {0_pct, 0_pct};
    auto panelLayout {panelCardsets->create_layout<dock_layout>()};

    // listbox
    auto lbxCardsets {panelLayout->create_widget<list_box>(dock_style::Top, "lbxCardsets")};
    lbxCardsets->Class = "list_box_log";
    lbxCardsets->Flex  = {50_pct, 25_pct};
    for (auto const& cardSet : _sources->Cardsets) { lbxCardsets->add_item(cardSet.first); }
    lbxCardsets->SelectedItemIndex.Changed.connect([&, lb = lbxCardsets.get()](auto val) {
        if (val != -1) { _sources->Settings.Cardset = lb->get_selected_item(); }
    });
    lbxCardsets->DoubleClick.connect([&, lb = lbxCardsets.get()] {
        if (lb->SelectedItemIndex >= 0) { hide(); }
    });

    // preview
    auto panelCards {panelLayout->create_widget<panel>(dock_style::Bottom, "panelCardsets")};
    panelCards->Flex   = {100_pct, 75_pct};
    panelCards->ZOrder = 1;
    auto panelCardsLayout {panelCards->create_layout<grid_layout>(size_i {20, 40})};

    auto const cardsetChanged {[&, lb = lbxCardsets.get(), panel = panelCards.get()](auto const& val) {
        lb->select_item(val);

        auto layout {panel->create_layout<box_layout>(size_i {13, 4})};

        auto const& cards {_sources->Cardsets.at(val)};
        auto const& tex {cards->get_material()->Texture};

        for (u8 s {static_cast<u8>(suit::Hearts)}; s <= static_cast<u8>(suit::Spades); ++s) {
            for (u8 r {static_cast<u8>(rank::Ace)}; r <= static_cast<u8>(rank::King); ++r) {
                auto imgBox {layout->create_widget<image_box>("")};
                imgBox->Image = {tex, card {static_cast<suit>(s), static_cast<rank>(r), 0, false}.get_texture_name()};
            }
        }
    }};
    _sources->Settings.Cardset.Changed.connect([cardsetChanged](auto const& val) { cardsetChanged(val); });
    cardsetChanged(_sources->Settings.Cardset);
}

void form_menu::create_menubar()
{
    auto menu {create_container<panel>(dock_style::Fill, MenuName)};
    auto menuLayout {menu->create_layout<grid_layout>(size_i {7, 40})};

    auto const enableContainer {[](form const* f, std::string const& enable) {
        for (auto const& w : f->get_widgets()) {
            if (w->get_name() != MenuName) {
                w->Flex = w->get_name() == enable ? dimensions {85_pct, 100_pct} : dimensions {0_pct, 0_pct};
            }
        }
    }};

    rect_i     btnRect {1, 1, 1, 2};
    auto const create {[&](std::string const& text) {
        auto btn {menuLayout->create_widget<radio_button>(btnRect, "btn" + text)};

        auto lbl {menuLayout->create_widget<label>({btnRect.Width + 2, btnRect.Y, btnRect.Width + 2, btnRect.Height}, "lblBtn" + text)};
        lbl->Label = text;
        lbl->For   = btn;

        btnRect.Y += btnRect.Height + 1;
        return btn;
    }};

    {
        auto btn {create("Games")};
        btn->Checked = true;
        btn->Click.connect([enableContainer](auto const& ev) { enableContainer(ev.Sender->get_form(), TabGamesName); });
    }
    {
        auto btn {create("Settings")};
        btn->Click.connect([enableContainer](auto const& ev) { enableContainer(ev.Sender->get_form(), TabSettingsName); });
    }
    {
        auto btn {create("Themes")};
        btn->Click.connect([enableContainer](auto const& ev) { enableContainer(ev.Sender->get_form(), TabThemesName); });
    }
    {
        auto btn {create("Cardsets")};
        btn->Click.connect([enableContainer](auto const& ev) { enableContainer(ev.Sender->get_form(), TabCardsetsName); });
    }

    auto btnBack {menuLayout->create_widget<button>({1, 35, 5, 3}, "btnBack")};
    btnBack->Icon = _resGrp.get<gfx::texture>("back");
    btnBack->Click.connect([&](auto&) { hide(); });
    btnBack->Tooltip = _tooltip;
}
}
