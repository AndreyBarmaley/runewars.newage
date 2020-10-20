/***************************************************************************
 *   Copyright (C) 2020 by RuneWarsNA team <runewars.newage@gmail.com>     *
 *                                                                         *
 *   Part of the RuneWars: NewAge engine:                                  *
 *   https://github.com/AndreyBarmaley/runewars.newage                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "settings.h"
#include "actions.h"
#include "gametheme.h"
#include "battlesummarypart.h"

BattleSummaryScreen::BattleSummaryScreen() : JsonWindow("screen_battle_summary.json", nullptr)
{
    const Person & pers = GameData::myPerson();
    ld = GameData::toLocalData(pers.avatar);

    defaultColor = GameTheme::jsonColor(jobject, "default:color");
    defaultFont = jobject.getString("default:font");
    areaFont = jobject.getString("area:font");

    spriteAttack = GameTheme::jsonSprite(jobject, "sprite:attack");
    spriteAvatarIcon = GameTheme::jsonSprite(jobject, "sprite:avatar_icon");

    offsetAvatarPort = GameTheme::jsonPoint(jobject, "offset:avatar_port");
    offsetAvatarText = GameTheme::jsonPoint(jobject, "offset:avatar_text");
    offsetClanIcon = GameTheme::jsonPoint(jobject, "offset:clan_icon");
    offsetClanText = GameTheme::jsonPoint(jobject, "offset:clan_text");
    offsetArmy = GameTheme::jsonPoint(jobject, "offset:army");
    offsetTextCombat = GameTheme::jsonPoint(jobject, "offset:text_combat");
    offsetTextWinners = GameTheme::jsonPoint(jobject, "offset:text_winners");
    areaHistory = GameTheme::jsonRect(jobject, "area:history");

    offsetPlayers[0] = GameTheme::jsonPoint(jobject, "offset:east");
    offsetPlayers[1] = GameTheme::jsonPoint(jobject, "offset:west");
    offsetPlayers[2] = GameTheme::jsonPoint(jobject, "offset:south");
    offsetPlayers[3] = GameTheme::jsonPoint(jobject, "offset:north");

    buttonNext = buttons.findIds("but_done");
    if(buttonNext)
        buttonNext->setAction(Action::ButtonDone);

    setVisible(true);
}

void BattleSummaryScreen::renderPlayerSection(const RemotePlayer & player, const Point & pos)
{
    const AvatarInfo & avatarInfo = GameData::avatarInfo(player.avatar);
    const ClanInfo & clanInfo = GameData::clanInfo(player.clan);

    // render portrait
    renderTexture(GameTheme::texture(avatarInfo.portrait), pos + offsetAvatarPort);

    // render avatar icon
    renderTexture(spriteAvatarIcon, pos + spriteAvatarIcon.position());

    // render clan icon
    renderTexture(GameTheme::texture(clanInfo.button), pos + offsetClanIcon);

    // army
    int offx = 0;
    for(auto & bcr : player.army.toBattleCreatures())
    {
	const Texture & icon = GameTheme::texture(GameData::creatureInfo(*bcr).image2);
	renderTexture(icon, pos + offsetArmy + Size(offx, 0));
	offx += icon.width() + 2;
    }

    const FontRender & frs = GameTheme::fontRender(defaultFont);
    const FontRender & frs2 = GameTheme::fontRender(areaFont);

    // render avatar name
    renderText(frs, player.name(), defaultColor, pos + offsetAvatarText, AlignLeft);
    // render clan name
    renderText(frs, clanInfo.name, defaultColor, pos + offsetClanText, AlignLeft);
    // render texts
    renderText(frs2, _("Combat Summary"), defaultColor, pos + offsetTextCombat, AlignLeft);
    renderText(frs2, _("Winner is"), defaultColor, pos + offsetTextWinners, AlignRight);

    // history
    int offy = 0;
    for(auto & legend : GameData::getBattleHistoryFor(player.avatar))
    {
	const AvatarInfo & defender = GameData::avatarInfo(legend.defender);
	const std::string & nameWin = legend.wins ? player.name() : defender.name;
	const LandInfo & landInfo = GameData::landInfo(legend.town.land());
	std::string info = StringFormat(_("%1 in %2")).arg(defender.name).arg(landInfo.name);

	Rect rt = renderText(frs2, player.name(), defaultColor, pos + areaHistory.toPoint() + Point(0, offy), AlignLeft);
	if(spriteAttack.isValid())
	{
	    renderTexture(spriteAttack, Point(rt.x + rt.w + 20, rt.y));
	    renderText(frs2, info, defaultColor, Point(rt.x + rt.w + 20 + spriteAttack.width() + 20, rt.y), AlignLeft);
	}
	else
	{
	    renderText(frs2, info, defaultColor, Point(rt.x + rt.w + 20, rt.y), AlignLeft);
	}

	renderText(frs2, nameWin, defaultColor, pos + areaHistory.toPoint() + Point(400, offy), AlignLeft);
	offy += rt.h + 5;
    }
}

void BattleSummaryScreen::renderWindow(void)
{
    JsonWindow::renderWindow();

    renderPlayerSection(ld.playerOfWind(Wind::East), offsetPlayers[0]);
    renderPlayerSection(ld.playerOfWind(Wind::West), offsetPlayers[1]);
    renderPlayerSection(ld.playerOfWind(Wind::South), offsetPlayers[2]);
    renderPlayerSection(ld.playerOfWind(Wind::North), offsetPlayers[3]);
}

bool BattleSummaryScreen::keyPressEvent(const KeySym & key)
{
    switch(key.keycode())
    {
	case Key::ESCAPE:
	case Key::RETURN:
	    pushEventAction(Action::ButtonDone, this, nullptr);
	    return true;

	default: break;
    }

    return false;
}

bool BattleSummaryScreen::userEvent(int act, void* data)
{
    switch(act)
    {
        case Action::ButtonDone:
	    setResultCode(GameData::isGameOver() ? Menu::GameSummaryPart : Menu::MahjongInitPart);
            setVisible(false);
            break;

        default: break;
    }

    return true;
}
