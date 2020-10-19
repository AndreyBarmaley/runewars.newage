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

#ifndef _RWNA_GAMEDATA_
#define _RWNA_GAMEDATA_

#include <list>
#include <array>

#include "actions.h"

namespace Menu
{
    enum { GameExit, SelectPerson, ShowPlayers, MahjongPart, MahjongSummaryPart, AdventurePart, BattleSummaryPart, GameSummaryPart, MahjongInitPart, GameLoadPart };
}

struct FileInfo
{
    std::string			id;
    std::string 		file;

    FileInfo() {}
};

struct FontInfo : public FileInfo
{
    int				size;
    CharRender                  blend;
    int                         style;
    CharHinting                 hinting;

    FontInfo() : size(0), blend(RenderBlended), style(StyleNormal), hinting(HintingNormal) {   }
};

struct ImageInfo : public FileInfo
{
    std::string 		colorkey;
    Rect			crop;

    ImageInfo() {}
    ImageInfo(const FileInfo & fi) : FileInfo(fi) {}
};

struct StoneInfo
{
    Stone			id;
    std::string 		name;
    std::string 		large;
    std::string 		medium;
    std::string 		small;

    StoneInfo() {}
};

struct WindInfo
{
    Wind			id;
    std::string 		name;
    std::string 		image;

    WindInfo() {}
};

struct ClanInfo
{
    Clan			id;
    std::string 		name;
    std::string 		image;
    std::string 		flag1;
    std::string 		flag2;
    std::string 		town;
    std::string 		button;
    std::string 		townflag1;
    std::string 		townflag2;

    ClanInfo() {}
};

struct SpecialityInfo
{
    Speciality			id;
    std::string			name;
    std::string 		description;

    SpecialityInfo() {}
};

struct CreatureInfo
{
    Creature			id;
    CreatureStat		stat;
    bool         		unique;
    bool         		fly;
    int				cost;
    Specials			specials;
    std::string 		name;
    std::string 		image1;
    std::string 		image2;
    std::string			description;
    std::string 		sound1;
    Stones			stones;

    CreatureInfo() : unique(false), fly(false), cost(0) {}
};

struct SpellInfo
{
    Spell			id;
    SpellTarget 		target;
    std::string 		name;
    std::string 		image;
    std::string 		description;
    std::string 		sound;
    Stones			stones;
    BaseStat			effect;
    bool         		persistent;
    int				cost;
    int				extval;

    SpellInfo() :  persistent(false), cost(0), extval(0) {}

    bool			isCurse(void) const { return persistent && (SpellTarget::Enemy & target()); }
    int				duration(void) const { return extval; }

    std::string			effectDescription(void) const;
};

struct AbilityInfo
{
    Ability			id;
    std::string 		name;
    std::string 		description;
    
    AbilityInfo() {}
};

struct AvatarInfo
{
    Avatar			id;
    std::string 		name;
    std::string 		dignity;
    std::string 		description;
    std::string 		portrait;
    std::string 		image;
    Ability			ability;
    Clans			clans;
    Spells			spells;
    Creatures			creatures;

    AvatarInfo() {}
    std::string			toStringClans(void) const;
};

struct LandInfo
{
    Land			id;
    Clan			clan;
    TownStat			stat;
    Point			center;
    Rect			area;
    Rect			iconrt;
    std::string 		name;
    std::vector<Land>		borders;
    Points			points;

    LandInfo() {}
};

struct LocalData
{
    VecStones			trashSet;
    std::array<LocalPlayer, 4>	players; // left, right, top, bottom
    int				stoneLastCount;

    Wind			roundWind;
    Wind			partWind;
    Wind			currentWind;
    WindCompass			compass;

    Stone			dropStone;
    WinResults			winResult;

    bool			newRound(void) const { return partWind() == Wind::East && stoneLastCount == 84; }
    bool			yourTurn(void) const { return currentWind == compass.bottom(); }

    LocalPlayer &		myPlayer(void);
    const LocalPlayer &		myPlayer(void) const;

    const RemotePlayer &	playerCurrentWind(void) const;
    const RemotePlayer &	playerOfWind(const Wind &) const;
    const RemotePlayer &	playerOfClan(const Clan &) const;
    const RemotePlayer &	playerOfAvatar(const Avatar &) const;

    const LocalPlayer &		remoteLeft(void) const { return players[0]; }
    const LocalPlayer &		remoteRight(void) const { return players[1]; }
    const LocalPlayer &		remoteTop(void) const { return players[2]; }
    const LocalPlayer &		remoteBottom(void) const { return players[3]; }

    Persons			toPersons(void) const;
};

struct ActionList : std::list<ActionMessage>
{
};

namespace GameData
{
    bool			init(const JsonObject &);

    LocalData                   toLocalData(const Avatar &);

    void			dumpOrderPersons(void);

    const StoneInfo &		stoneInfo(const Stone &);
    const WindInfo &		windInfo(const Wind &);
    const LandInfo &		landInfo(const Land &);
    const AvatarInfo &		avatarInfo(const Avatar &);
    const ClanInfo &		clanInfo(const Clan &);
    const AbilityInfo &		abilityInfo(const Ability &);
    const SpecialityInfo &	specialityInfo(const Speciality &);
    const CreatureInfo &	creatureInfo(const Creature &);
    const SpellInfo &		spellInfo(const Spell &);
    Avatars			avatarsOfClan(const Clan &);

    void			initPersons(const Person &);

    bool			initMahjong(void);
    bool			mahjong2Client(const Avatar &, ActionList &);
    bool			client2Mahjong(const Avatar &, const ClientMessage &, ActionList &);

    bool			initAdventure(void);
    bool			adventure2Client(const Avatar &, ActionList &);
    bool			client2Adventure(const Avatar &, const ClientMessage &, ActionList &);

    const Person &		myPerson(void);

    bool			saveGame(const JsonObject &);
    bool			loadGame(void);
    bool			loadGame(const std::string &);
    bool			isGameOver(void);

    int				loadedGamePart(void);
    int				nextBattleUnitId(void);

    const JsonObject &		jsonGUI(void);

    bool			findCreatureUnique(const Creature &);
    BattleParty*		getBattleParty(int);
    BattleCreature*		getBattleCreature(int);
    RemotePlayer*		getBattleArmyOwner(const BattleArmy &);
    BattleArmy &		getBattleArmy(const Clan &);
    std::list<BattleLegend>	getBattleHistoryFor(const Avatar &);
};

#endif
