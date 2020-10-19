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

#ifndef _RWNA_BATTLESUMMARYPART_
#define _RWNA_BATTLESUMMARYPART_

#include "gamedata.h"

class BattleSummaryScreen : public JsonWindow
{
    LocalData		ld;
    Color               defaultColor;
    std::string         defaultFont;
    std::string         areaFont;

    JsonButton*         buttonNext;
    Point		offsetPlayers[4];
    Sprite		spriteAvatarIcon;
    Texture		spriteAttack;

    Point		offsetAvatarPort;
    Point		offsetAvatarText;
    Point		offsetClanIcon;
    Point		offsetClanText;
    Point		offsetArmy;
    Point		offsetTextCombat;
    Point		offsetTextWinners;
    Rect		areaHistory;

    void		renderPlayerSection(const RemotePlayer &, const Point &);

protected:
    bool		keyPressEvent(const KeySym &) override;
    bool		userEvent(int, void*) override;

public:
    BattleSummaryScreen();

    void		renderWindow(void) override;
};

#endif
