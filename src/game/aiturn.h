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

#ifndef _RWNA_AITURN_
#define _RWNA_AITURN_

#include "gametheme.h"

namespace AI
{
    bool        mahjongTurn(const Wind &, const Avatar &, const VecStones & trash,
                            const WinRules & left, const WinRules & right, const WinRules & top,
                            bool showGame, bool showKong, ActionList &);

    bool        mahjongGameKongPungChao(const Wind & currentWind, const Wind & roundWind,
			    const Stone & dropStone, WinResults &, ActionList &, bool sayOnly);

    int         mahjongSelect(const GameStones &, const VecStones &, const WinRules &);
    void        mahjongOtherPass(const Wind &, ActionList &, const Wind &);
    void        mahjongSummonCast(const Avatar &, const Creatures &, const Spells &, ActionList &);

    void        adventureMove(const RemotePlayer &, ActionList &);
}

#endif
