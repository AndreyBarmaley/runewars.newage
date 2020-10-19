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

#include <cstring>
#include <sstream>
#include <algorithm>

#include "settings.h"
#include "aiturn.h"
#include "actions.h"
#include "battle.h"
#include "gamedata.h"

std::string SpellInfo::effectDescription(void) const
{
    switch(id())
    {
	case Spell::LightningBolt:
	case Spell::MassPanic:
	case Spell::HellBlast:		return StringFormat(_("-%1 Loyalty")).arg(std::abs(effect.loyalty));
	case Spell::DemonicCompulsion:	return StringFormat(_("+%1 Loyalty")).arg(std::abs(effect.loyalty));

	case Spell::BattleFury:		return StringFormat(_("+%1 Attack")).arg(std::abs(effect.attack));
	case Spell::DustCloud:		return StringFormat(_("-%1 Attack")).arg(std::abs(effect.attack));

	case Spell::Guidance:		return StringFormat(_("+%1 Missile")).arg(std::abs(effect.ranger));
	case Spell::Smoke:		return StringFormat(_("-%1 Missile")).arg(std::abs(effect.ranger));

	case Spell::MagicalAura:	return StringFormat(_("+%1 Defense")).arg(std::abs(effect.defense));
	case Spell::BrilliantLights:	return StringFormat(_("-%1 Defense")).arg(std::abs(effect.defense));

	case Spell::Heroism:		return StringFormat(_("+%1 Attack, +%2 Max Loyalty")).arg(std::abs(effect.attack)).arg(std::abs(effect.loyalty));
	case Spell::MysticalFountain:	return StringFormat(_("+%1 Attack or +%2 Missile or +%3 Max Loyalty")).arg(std::abs(effect.attack)).arg(std::abs(effect.ranger)).arg(std::abs(effect.loyalty));
	case Spell::BlindAmbition:	return StringFormat(_("+%1 Attack, +%2 Defense, -%3 Max Loyalty")).arg(std::abs(effect.attack)).arg(std::abs(effect.defense)).arg(std::abs(effect.loyalty));
	case Spell::Healing:		return StringFormat(_("+%1 Loyalty, up to max")).arg(std::abs(effect.loyalty));
	case Spell::Reduction:		return StringFormat(_("-%1 Defense, -%2 Attack")).arg(std::abs(effect.defense)).arg(std::abs(effect.attack));
	case Spell::ForceShield:	return StringFormat(_("Missile attacks do %1 less damage")).arg(std::abs(effect.ranger));

	case Spell::Paralyze:		return _("Freeze creature");
	case Spell::Teleport:		return _("Move one unit to any friendly territory");
	case Spell::DispelMagic:
	case Spell::MassDispel:		return _("Remove enchantments");

	default:		break;
    }

    return name;
}

std::string AvatarInfo::toStringClans(void) const
{
    StringList res;

    for(auto & id : clans)
    {
	const ClanInfo & clan = GameData::clanInfo(id);
	res << clan.name;
    }

    return res.join(", ");
}

namespace GameData
{
    std::vector<StoneInfo>		stonesInfo;
    std::vector<WindInfo>		windsInfo;
    std::vector<ClanInfo>		clansInfo;
    std::vector<CreatureInfo>		creaturesInfo;
    std::vector<SpellInfo>		spellsInfo;
    std::vector<SpecialityInfo>		specialsInfo;
    std::vector<AbilityInfo>		abilitiesInfo;
    std::vector<AvatarInfo>		avatarsInfo;
    std::vector<LandInfo>		landsInfo;

    int					bonusStart;
    int					bonusGame;
    int					bonusKong;
    int					bonusPung;
    int					bonusChao;
    int					bonusPass;

    template<typename T>
    bool loadJson(const char* json, std::vector<T> & v)
    {
	JsonContent jc = GameTheme::jsonResource(json);
	if(jc.isArray())
	{
	    auto list = jc.toArray().toStdList<T>();
	    v.resize(list.size() + 1); // nul reserve

	    for(auto & val : list)
		v[val.id.index()] = val;

	    return true;
	}

	ERROR("incorrect json array: " << json);
	return false;
    }

    bool 		loadIndexes(const JsonObject &);

    LocalPlayer &	playerOfClan(const Clan &);
    LocalPlayer &	playerOfAvatar(const Avatar &);
    LocalPlayer &	playerOfWind(const Wind &);
}

bool GameData::init(const JsonObject & jo)
{
    if(! loadIndexes(jo))
	return false;

    bonusStart = jo.getInteger("bonus:start", 250);
    bonusPass = jo.getInteger("bonus:pass", 10);
    bonusChao = jo.getInteger("bonus:chao", 20);
    bonusPung = jo.getInteger("bonus:pung", 30);
    bonusKong = jo.getInteger("bonus:kong", 40);

    // load game jsons
    if(! loadJson<StoneInfo>("stones.json", stonesInfo))
	return false;

    if(! loadJson<WindInfo>("winds.json", windsInfo))
	return false;

    if(! loadJson<ClanInfo>("clans.json", clansInfo))
	return false;

    if(! loadJson<CreatureInfo>("creatures.json", creaturesInfo))
	return false;

    if(! loadJson<SpellInfo>("spells.json", spellsInfo))
	return false;

    if(! loadJson<SpecialityInfo>("specials.json", specialsInfo))
	return false;

    if(! loadJson<AbilityInfo>("abilities.json", abilitiesInfo))
	return false;

    if(! loadJson<AvatarInfo>("avatars.json", avatarsInfo))
	return false;

    if(! loadJson<LandInfo>("lands.json", landsInfo))
	return false;

    // check path valid
    for(auto & info1 : landsInfo)
    {
        const Land & land1 = info1.id;
        for(auto & land2 : info1.borders)
        {
            const LandInfo & info2 = landInfo(land2);

            if(std::none_of(info2.borders.begin(), info2.borders.end(), [&](const Land & land){ return land == land1; }))
                ERROR("land path error: " << land1.toString() << "<->" << land2.toString());
        }
    }

    return true;
}

const LandInfo & GameData::landInfo(const Land & landId)
{
    return landsInfo[landId()];
}

const AvatarInfo & GameData::avatarInfo(const Avatar & avatarId)
{
    return avatarsInfo[avatarId()];
}

const AbilityInfo & GameData::abilityInfo(const Ability & abilityId)
{
    return abilitiesInfo[abilityId()];
}

const SpecialityInfo & GameData::specialityInfo(const Speciality & speciality)
{
    return specialsInfo[speciality.index()];
}

const CreatureInfo & GameData::creatureInfo(const Creature & creatureId)
{
    return creaturesInfo[creatureId()];
}

const SpellInfo & GameData::spellInfo(const Spell & spellId)
{
    return spellsInfo[spellId()];
}

const StoneInfo & GameData::stoneInfo(const Stone & stone)
{
    return stonesInfo[stone.index()];
}

const WindInfo & GameData::windInfo(const Wind & windId)
{
    return windsInfo[windId()];
}

const ClanInfo & GameData::clanInfo(const Clan & clanId)
{
    return clansInfo[clanId()];
}

Avatars GameData::avatarsOfClan(const Clan & clanId)
{
    Avatars res;

    for(auto id : avatars_all)
    {
	const AvatarInfo & info = GameData::avatarInfo(id);
	if(std::any_of(info.clans.begin(), info.clans.end(), [&](const Clan & id){ return id == clanId; }))
	    res.emplace_back(id);
    }

    return res;
}

const RemotePlayer & LocalData::playerCurrentWind(void) const
{
    return playerOfWind(currentWind);
}

LocalPlayer & LocalData::myPlayer(void)
{
    return players[3];
}

const LocalPlayer & LocalData::myPlayer(void) const
{
    return players[3];
}

const RemotePlayer & LocalData::playerOfWind(const Wind & wind) const
{
    auto it = std::find_if(players.begin(), players.end(),
			    [&](const Person & pers){ return pers.isWind(wind); });

    if(it == players.end())
    {
	ERROR("player not found" << ", " << "wind: " << wind.toString());
	Engine::except(__FUNCTION__, "exit");
    }

    return *it;
}

const RemotePlayer & LocalData::playerOfClan(const Clan & clan) const
{
    auto it = std::find_if(players.begin(), players.end(),
			    [&](const Person & pers){ return pers.isClan(clan); });

    if(it == players.end())
    {
	ERROR("player not found" << ", " << "clan: " << clan.toString());
	Engine::except(__FUNCTION__, "exit");
    }

    return *it;
}

const RemotePlayer & LocalData::playerOfAvatar(const Avatar & avatar) const
{
    auto it = std::find_if(players.begin(), players.end(),
			    [&](const Person & pers){ return pers.isAvatar(avatar); });

    if(it == players.end())
    {
	ERROR("player not found" << ", " << "avatar: " << avatar.toString());
	Engine::except(__FUNCTION__, "exit");
    }

    return *it;
}

Persons LocalData::toPersons(void) const
{
    Persons res;
    res.assign(players.begin(), players.end());
    return res;
}

namespace GameData
{
    // remote data
    Person				person;
    LocalPlayers			gamers;
    Wind				currentWind;
    Wind				roundWind;
    Wind				partWind;
    CroupierSet				croupier;
    int					stoneLastCount = 0;
    Stone				dropStone;
    WinResults				winResult;
    std::list<BattleLegend>		battleHistory;
    bool				skipRepeatSay = false;
    bool				skipNewStone = false;
    bool				skipNewTurn = false;
    int					gamePart = 0;
    int					battleUnitId = 1;
    JsonObject				stateGUI;

    Wind                                prevWindCompass(const Wind &);
    Wind                                nextWindCompass(const Wind &);
    Wind                                oppositeWindCompass(const Wind &);

    void				dumpOrderPersons(void);
    void				validateMahjongSummary(void);

    bool				adventureBattleAction(const Avatar &, ActionList &);

    bool				clientReady(const Avatar &, const ClientMessage &, ActionList &);
    bool				clientSayGame(const Avatar &, const ClientMessage &, ActionList &);
    bool				clientSayChao(const Avatar &, const ClientMessage &, ActionList &);
    bool				clientSayPung(const Avatar &, const ClientMessage &, ActionList &);
    bool				clientSayKong(const Avatar &, const ClientMessage &, ActionList &);
    bool				clientButtonGame(const Avatar &, const ClientMessage &, ActionList &);
    bool				clientButtonPass(const Avatar &, const ClientMessage &, ActionList &);
    bool				clientButtonPung(const Avatar &, const ClientMessage &, ActionList &);
    bool				clientButtonKong1(const Avatar &, const ClientMessage &, ActionList &);
    bool				clientButtonKong2(const Avatar &, const ClientMessage &, ActionList &);
    bool				clientChaoVariant(const Avatar &, const ClientMessage &, ActionList &);
    bool				clientDropIndex(const Avatar &, const ClientMessage &, ActionList &);
    bool				clientSummonCreature(const Avatar &, const ClientMessage &, ActionList &);
    bool				clientCastSpell(const Avatar &, const ClientMessage &, ActionList &);
    bool				clientUnitMoved(const Avatar &, const ClientMessage &, ActionList &);
    bool				clientBattleReady(const Avatar &, const ClientMessage &, ActionList &);

    JsonObject                  	toJsonObject(const JsonObject &);
    bool				fromJsonObject(const JsonObject &);
}

int GameData::nextBattleUnitId(void)
{
    return battleUnitId++;
}

bool GameData::isGameOver(void)
{
    return roundWind() == Wind::North && partWind() == Wind::North;
}

std::list<BattleLegend> GameData::getBattleHistoryFor(const Avatar & avatar)
{
    std::list<BattleLegend> res;

    for(auto & legend : battleHistory)
	if(legend.attacker == avatar) res.push_back(legend);

    return res;
}

const JsonObject & GameData::jsonGUI(void)
{
    return stateGUI;
}

int GameData::loadedGamePart(void)
{
    return gamePart;
}

const Person & GameData::myPerson(void)
{
    return person;
}

JsonObject GameData::toJsonObject(const JsonObject & gui)
{
    JsonObject jo;
    jo.addInteger("version", FORMAT_VERSION_CURRENT);
    jo.addString("wind:round", roundWind.toString());
    jo.addString("wind:part", partWind.toString());
    jo.addString("wind:current", currentWind.toString());
    jo.addInteger("lastcount", stoneLastCount);
    jo.addString("stone:drop", dropStone.toString());
    jo.addBoolean("skiprepeat", skipRepeatSay);
    jo.addBoolean("skipnew", skipNewStone);
    jo.addBoolean("skipturn", skipNewTurn);
    jo.addInteger("gamepart", gamePart);
    jo.addInteger("nextBattleUnitId", battleUnitId);

    jo.addObject("myperson", person.toJsonObject());
    jo.addObject("croupier", croupier.toJsonObject());
    jo.addObject("winresult", winResult.toJsonObject());
    jo.addArray("players", gamers.toJsonArray());

    JsonArray ja;
    for(auto it = battleHistory.begin(); it != battleHistory.end(); ++it)
	ja.addObject((*it).toJsonObject());
    jo.addArray("history", ja);

    jo.addObject("gui", gui);

    return jo;
}

bool GameData::fromJsonObject(const JsonObject & jo)
{
    int version = jo.getInteger("version");

    if(version < FORMAT_VERSION_LAST || version > FORMAT_VERSION_CURRENT)
    {
	ERROR("unknown version: " << version << ", " <<
	    "supported release: " << FORMAT_VERSION_LAST);
	return false;
    }

    VERBOSE("load gamedata, version: " << version);

    stoneLastCount = jo.getInteger("lastcount");
    roundWind = Wind(jo.getString("wind:round"));
    partWind = Wind(jo.getString("wind:part"));
    currentWind = Wind(jo.getString("wind:current"));
    dropStone = Stone(jo.getString("stone:drop"));
    skipNewStone = jo.getBoolean("skipnew");
    skipNewTurn = jo.getBoolean("skipturn");
    skipRepeatSay = false; // initial say need! jo.getBoolean("skiprepeat");
    gamePart = jo.getInteger("gamepart");
    battleUnitId = jo.getInteger("nextBattleUnitId");

    const JsonObject* jo2 = nullptr;

    jo2 = jo.getObject("myperson");
    if(! jo2)
    {
	ERROR("json parse: " << "croupier");
	return false;
    }
    person = Person::fromJsonObject(*jo2);

    jo2 = jo.getObject("croupier");
    if(! jo2)
    {
	ERROR("json parse: " << "croupier");
	return false;
    }
    croupier = CroupierSet::fromJsonObject(*jo2);

    jo2 = jo.getObject("winresult");
    if(! jo2)
    {
	ERROR("json parse: " << "winresult");
	return false;
    }
    winResult = WinResults::fromJsonObject(*jo2);

    const JsonArray* ja2 = nullptr;

    ja2 = jo.getArray("players");
    if(! ja2)
    {
	ERROR("json parse: " << "players");
	return false;
    }
    gamers = LocalPlayers::fromJsonArray(*ja2);

    battleHistory.clear();

    ja2 = jo.getArray("history");
    if(! ja2)
    {
	ERROR("json parse: " << "history");
	return false;
    }

    for(int it = 0; it < ja2->count(); ++it)
    {
	jo2 = ja2->getObject(it);
	if(jo2) battleHistory.push_back(BattleLegend::fromJsonObject(*jo2));
    }

    stateGUI.clear();

    jo2 = jo.getObject("gui");
    if(jo2) stateGUI = *jo2;

    return true;
}

bool GameData::saveGame(const JsonObject & gui)
{
    const std::string & share = Settings::shareDir();
    if(!Systems::isDirectory(share)) Systems::makeDirectory(share);
    Display::renderScreenshot(Settings::fileSave("game.png"));
    return Systems::saveString2File(GameData::toJsonObject(gui).toString(), Settings::fileSaveGame());
}

bool GameData::loadGame(void)
{
    return loadGame(Settings::fileSaveGame());
}

bool GameData::loadGame(const std::string & fn)
{
    std::string str;

    if(Systems::readFile2String(fn, str))
	return fromJsonObject(JsonContentString(str).toObject());

    return false;
}

LocalData GameData::toLocalData(const Avatar & ava)
{
    LocalPlayer* lp = gamers.playerOfAvatar(ava);

    const Wind & wind = lp->wind;
    const AvatarInfo & avaInfo = avatarInfo(ava);

    LocalData ld;
    ld.trashSet = croupier.trash;

    ld.roundWind = roundWind;
    ld.partWind = partWind;
    ld.currentWind = currentWind;
    ld.compass = WindCompass(wind);
    ld.dropStone = dropStone;
    ld.stoneLastCount = stoneLastCount;
    ld.winResult = winResult;


    lp = gamers.playerOfWind(ld.compass.left());
    if(lp) ld.players[0] = *lp;
    else ERROR("player not found" << ", wind: " << ld.compass.left().toString());

    lp = gamers.playerOfWind(ld.compass.right());
    if(lp) ld.players[1] = *lp;
    else ERROR("player not found" << ", wind: " << ld.compass.right().toString());

    lp = gamers.playerOfWind(ld.compass.top());
    if(lp) ld.players[2] = *lp;
    else ERROR("player not found" << ", wind: " << ld.compass.top().toString());

    lp = gamers.playerOfWind(ld.compass.bottom());
    if(lp) ld.players[3] = *lp;
    else ERROR("player not found" << ", wind: " << ld.compass.bottom().toString());

    // check: Affected ScryRunes
    for(int it = 0; it < 3; ++it)
    {
	if(! ld.players[it].isAffectedSpell(Spell::ScryRunes))
	{
	    // hide: private game info
	    ld.players[it].stones.clear();
	    ld.players[it].newStone.reset();
	}
    }

    // check: Ability Monacle
    if(avaInfo.ability() != Ability::Monacle)
    {
	ld.players[0].army.applyInvisibility();
	ld.players[1].army.applyInvisibility();
	ld.players[2].army.applyInvisibility();
    }

    return ld;
}

BattleArmy & GameData::getBattleArmy(const Clan & clan)
{
    return playerOfClan(clan).army;
}

bool GameData::findCreatureUnique(const Creature & cr)
{
    for(auto it = gamers.begin(); it != gamers.end(); ++it)
	if((*it).army.findCreature(cr)) return true;

    return false;
}

BattleParty* GameData::getBattleParty(int unit)
{
    if(0 < unit)
    {
	for(auto itg = gamers.begin(); itg != gamers.end(); ++itg)
	{
	    for(auto itp = (*itg).army.begin(); itp != (*itg).army.end(); ++itp)
		if((*itp).findBattleUnitConst(unit)) return & (*itp);
	}
    }

    ERROR("battle unit not found" << ", " << "id: " << String::hex(unit));
    Engine::except(__FUNCTION__, "exit");

    return nullptr;
}

BattleCreature* GameData::getBattleCreature(int unit)
{
    if(0 < unit)
    {
	for(auto it = gamers.begin(); it != gamers.end(); ++it)
	{
	    auto bcr = (*it).army.findBattleUnit(unit);
	    if(bcr) return bcr;
	}
    }

    ERROR("battle unit not found" << ", " << "id: " << String::hex(unit));
    Engine::except(__FUNCTION__, "exit");

    return nullptr;
}

RemotePlayer* GameData::getBattleArmyOwner(const BattleArmy & army)
{
    for(auto it = gamers.begin(); it != gamers.end(); ++it)
	if(& (*it).army == & army) return & (*it);

    ERROR("battle army not found");
    Engine::except(__FUNCTION__, "exit");

    return nullptr;
}

Wind GameData::oppositeWindCompass(const Wind & wind)
{
    return wind.isValid() ? WindCompass(wind).top() : Wind(Wind::South);
}

Wind GameData::nextWindCompass(const Wind & wind)
{
    return wind.isValid() ? WindCompass(wind).right() : Wind(Wind::East);
}

Wind GameData::prevWindCompass(const Wind & wind)
{
    return wind.isValid() ? WindCompass(wind).left() : Wind(Wind::North);
}

void GameData::initPersons(const Person & cur)
{
    Persons persons(cur);
    gamers.setPersons(persons);

    person = cur;
    roundWind = Wind(Wind::None);
    partWind = Wind(Wind::None);
    currentWind = Wind(Wind::None);
}

bool GameData::initMahjong(void)
{
    do
    {
	for(auto & lp : gamers)
	    lp.initMahjongPart();

	croupier.reset();
	gamers.distributeStones(croupier);
    }
    // fix kong startup
    while(gamers.findKongs());

    stoneLastCount = GAME_STONE_MAX;
    skipRepeatSay = false;
    gamePart = Menu::MahjongPart;
    skipNewStone = false;
    skipNewTurn = false;
    stateGUI.clear();

    if(partWind() == Wind::North && roundWind() == Wind::North)
	return false;
    else
    if(! partWind.isValid() && ! roundWind.isValid())
    {
	// new round, new part
        partWind = Wind(Wind::East);
        roundWind = Wind(Wind::East);
    }
    else
    if(partWind() == Wind::North)
    {
	// new round, new part
	roundWind.shift();
	partWind = Wind(Wind::East);
    }
    else
    {
	// new part
	gamers.shiftWinds();
	partWind.shift();
    }

    currentWind = Wind(Wind::East);
    dropStone = Stone(Stone::None);
    winResult = WinResults();

    battleHistory.clear();

    VERBOSE("wind round: " << roundWind.toString());
    VERBOSE("wind part: " << partWind.toString());

    dumpOrderPersons();
    return true;
}

void GameData::dumpOrderPersons(void)
{
    DEBUG("players: ");
    for(auto & id : winds_all)
    {
	LocalPlayer & player = playerOfWind(id);

	DEBUG("wind: " << player.wind.toString() << ", " << "player: " << player.name() << ", " <<
		(! player.isAI() ? " (*)" : "") << ", " << "clan: " << player.clan.toString() << ", " <<
		"stones: " <<  player.stones.toString() << ", " << "new stone: " << player.newStone.toString());
    }
}

LocalPlayer & GameData::playerOfAvatar(const Avatar & avatar)
{
    LocalPlayer* res = gamers.playerOfAvatar(avatar);

    if(! res)
    {
	ERROR("unknown avatar id: " << avatar());
	Engine::except(__FUNCTION__, "exit");
    }

    return *res;
}

LocalPlayer & GameData::playerOfClan(const Clan & clan)
{
    LocalPlayer* res = gamers.playerOfClan(clan);

    if(! res)
    {
	ERROR("unknown clan id: " << clan());
	Engine::except(__FUNCTION__, "exit");
    }

    return *res;
}

LocalPlayer & GameData::playerOfWind(const Wind & wind)
{
    LocalPlayer* res = gamers.playerOfWind(wind);

    if(! res)
    {
	ERROR("unknown wind: " << wind());
	Engine::except(__FUNCTION__, "exit");
    }

    return *res;
}

bool GameData::mahjong2Client(const Avatar & avatar, ActionList & actions)
{
    LocalPlayer & current = playerOfWind(currentWind);

    if(current.newStone.isValid() || skipNewTurn)
    {
	//DEBUG("wind: " << currentWind.toString() << ", " << "person: " << current.name() << ", " << 
	//	"new stone: " << current.newStone() << ", " << "wait action");
	return false;
    }
    else
    if(dropStone.isValid())
    {
	if(skipRepeatSay)
	    return false;

	skipRepeatSay = true;

	if(AI::mahjongGameKongPungChao(currentWind, roundWind, dropStone, winResult, actions, true))
	    return true;

	DEBUG("wait player pass" << ", " << "current: " << current.toString());
	return false;
    }

    DEBUG("new turn: " << "last count: " << stoneLastCount);

    if(0 == stoneLastCount)
    {
	actions.push_back(MahjongEnd(currentWind));
	validateMahjongSummary();
    	gamePart = Menu::MahjongSummaryPart;
	return true;
    }

    bool showGame2 = false;
    bool showKong2 = false;

    current.newTurnEvent(croupier, skipNewStone);

    if(! skipNewStone)
    {
	stoneLastCount--;

	showGame2 = current.isWinMahjong(currentWind, roundWind, dropStone, & winResult);
	showKong2 = current.isMahjongKong2(currentWind);
    }
    else
    {
	skipNewTurn = true;
    }

    if(current.isAI())
    {
	const WinRules & left = playerOfWind(prevWindCompass(currentWind)).rules;
	const WinRules & right = playerOfWind(nextWindCompass(currentWind)).rules;
	const WinRules & top = playerOfWind(oppositeWindCompass(currentWind)).rules;

	AI::mahjongTurn(currentWind, current.avatar, croupier.trash, left, right, top, showGame2, showKong2, actions);
    }
    else
    {
	actions.push_back(MahjongTurn(currentWind, current.newStone, showKong2, showGame2));
	actions.push_back(MahjongData(currentWind));
    }

    return true;
}

bool GameData::clientReady(const Avatar & avatar, const ClientMessage & act, ActionList & actions)
{
    LocalPlayer & client = playerOfAvatar(avatar);
    DEBUG(client.toString());

    actions.push_back(MahjongBegin(currentWind, roundWind, partWind == Wind(Wind::East)));

    return true;
}

bool GameData::clientSayGame(const Avatar & avatar, const ClientMessage & act, ActionList & actions)
{
    LocalPlayer & client = playerOfAvatar(avatar);

    // need fill winResult
    if(client.isWinMahjong(currentWind, roundWind, dropStone, & winResult))
    {
	DEBUG(client.toString());

	actions.push_back(MahjongSayGame(client.wind));
	AI::mahjongOtherPass(currentWind, actions, client.wind);
	return true;
    }

    ERROR("isWinMahjong: false");
    return false;
}

bool GameData::clientSayChao(const Avatar & avatar, const ClientMessage & act, ActionList & actions)
{
    LocalPlayer & client = playerOfAvatar(avatar);

    DEBUG(client.toString());

    if(client.isAffectedSpell(Spell::Silence))
    {
	ERROR("player silence mode: " << client.name());
	return false;
    }

    if(client.isMahjongChao(currentWind, dropStone))
    {
	actions.push_back(MahjongSayChao(client.wind));
	AI::mahjongOtherPass(currentWind, actions, client.wind);
	return true;
    }

    ERROR("isMahjongChao: false");
    return false;
}

bool GameData::clientSayPung(const Avatar & avatar, const ClientMessage & act, ActionList & actions)
{
    LocalPlayer & client = playerOfAvatar(avatar);

    DEBUG(client.toString());

    if(client.isAffectedSpell(Spell::Silence))
    {
	ERROR("player silence mode: " << client.name());
	return false;
    }

    if(client.isMahjongPung(currentWind, dropStone))
    {
	actions.push_back(MahjongSayPung(client.wind));
	AI::mahjongOtherPass(currentWind, actions, client.wind);
	return true;
    }

    ERROR("isMahjongPung: false");
    return false;
}

bool GameData::clientSayKong(const Avatar & avatar, const ClientMessage & act, ActionList & actions)
{
    LocalPlayer & client = playerOfAvatar(avatar);
    auto action = static_cast<const ClientSayKong &>(act);

    DEBUG(client.toString());

    if(client.isAffectedSpell(Spell::Silence))
    {
	ERROR("player silence mode: " << client.name());
	return false;
    }

    if(1 == action.kongType() && client.isMahjongKong1(currentWind, dropStone))
    {
	actions.push_back(MahjongSayKong(client.wind));
	AI::mahjongOtherPass(currentWind, actions, client.wind);
	return true;
    }
    else
    if(2 == action.kongType() && client.isMahjongKong2(currentWind))
    {
	actions.push_back(MahjongSayKong(client.wind));
	return true;
    }
    
    ERROR("isMahjongKong: false");
    return false;
}

bool GameData::clientButtonGame(const Avatar & avatar, const ClientMessage & act, ActionList & actions)
{
    LocalPlayer & client = playerOfAvatar(avatar);

    DEBUG(client.toString());

    client.setMahjongGame(winResult);

    actions.push_back(MahjongGame(client.wind));
    AI::mahjongOtherPass(currentWind, actions, client.wind);
    actions.push_back(MahjongData(currentWind));

    actions.push_back(MahjongEnd(currentWind));
    validateMahjongSummary();
    gamePart = Menu::MahjongSummaryPart;

    return true;
}

bool GameData::clientButtonPass(const Avatar & avatar, const ClientMessage & act, ActionList & actions)
{
    LocalPlayer & client = playerOfAvatar(avatar);

    DEBUG(client.toString());

    if(AI::mahjongGameKongPungChao(currentWind, roundWind, dropStone, winResult, actions, false))
	return true;

    if(! client.isAI())
    {
	currentWind.shift();
	croupier.put(dropStone);
	dropStone = Stone(Stone::None);
    }

    actions.push_back(MahjongData(currentWind));
    skipRepeatSay = false;
    return true;
}

bool GameData::clientButtonPung(const Avatar & avatar, const ClientMessage & act, ActionList & actions)
{
    LocalPlayer & client = playerOfAvatar(avatar);

    DEBUG(client.toString());

    if(client.isAffectedSpell(Spell::Silence))
    {
	ERROR("player silence mode: " << client.name());
	return false;
    }

    actions.push_back(MahjongPung(client.wind, dropStone));
    client.setMahjongPung(dropStone);
    dropStone.reset();
    currentWind = client.wind;
    actions.push_back(MahjongData(currentWind));
    skipNewStone = true;

    return true;
}

bool GameData::clientButtonKong1(const Avatar & avatar, const ClientMessage & act, ActionList & actions)
{
    LocalPlayer & client = playerOfAvatar(avatar);

    DEBUG(client.toString());

    actions.push_back(MahjongKong1(client.wind, dropStone));
    client.setMahjongKong1(dropStone);
    dropStone.reset();
    currentWind = client.wind;
    actions.push_back(MahjongData(currentWind));
    skipNewStone = true;

    return true;
}

bool GameData::clientButtonKong2(const Avatar & avatar, const ClientMessage & act, ActionList & actions)
{
    LocalPlayer & client = playerOfAvatar(avatar);

    DEBUG(client.toString());

    actions.push_back(MahjongKong2(client.wind));
    client.setMahjongKong2();
    actions.push_back(MahjongData(currentWind));

    return true;
}

bool GameData::clientChaoVariant(const Avatar & avatar, const ClientMessage & act, ActionList & actions)
{
    LocalPlayer & client = playerOfAvatar(avatar);

    auto ca = static_cast<const ClientChaoVariant &>(act);

    DEBUG(client.toString() << ", " << "variant: " << ca.chaoVariant());

    actions.push_back(MahjongChao(client.wind, dropStone));
    client.setMahjongChao(dropStone, ca.chaoVariant());
    dropStone.reset();
    currentWind = client.wind;
    actions.push_back(MahjongData(currentWind));
    skipNewStone = true;

    return true;
}

bool GameData::clientDropIndex(const Avatar & avatar, const ClientMessage & act, ActionList & actions)
{
    LocalPlayer & client = playerOfAvatar(avatar);

    auto ca = static_cast<const ClientDropIndex &>(act);

    DEBUG(client.toString() << ", " << "index: " << ca.dropIndex() << ", " << "stones: " << client.stones.toString());
    
    if(dropStone.isValid())
    {
	ERROR("drop stone: " << dropStone() << ", " << "(" << dropStone.toString() << ")");
	return false;
    }

    dropStone = client.setMahjongDrop(ca.dropIndex());
    actions.push_back(MahjongDrop(currentWind, dropStone));
    actions.push_back(MahjongData(currentWind));

    DEBUG("drop stone: " << dropStone() << ", " << "(" << dropStone.toString() << ")");

    AI::mahjongGameKongPungChao(currentWind, roundWind, dropStone, winResult, actions, true);
    skipRepeatSay = true;
    skipNewStone = false;
    skipNewTurn = false;

    return true;
}

bool GameData::clientSummonCreature(const Avatar & avatar, const ClientMessage & act, ActionList & actions)
{
    LocalPlayer & client = playerOfAvatar(avatar);

    if(currentWind != client.wind)
    {
	ERROR("wind not current: " << client.wind.toString());
	return false;
    }

    auto ca = static_cast<const ClientSummonCreature &>(act);

    if(client.isAffectedSpell(Spell::Silence))
    {
	ERROR("player silence mode: " << client.name());
	return false;
    }

    if(client.isCasted() && !ca.isForce())
    {
	ERROR("player casted: " << client.name());
	return false;
    }

    if(client.army.isMaximumSummoning())
    {
	ERROR("player summmoning maximum: " << client.name());
	return false;
    }

    Creature creature = ca.creature();
    Land land = ca.land();

    if(! land.isValid())
    {
	ERROR("land invalid: " << land.toString());
	return false;
    }

    const LandInfo & landInfo = GameData::landInfo(land);

    if(! landInfo.stat.power || (Land::TowerOf4Winds != land() && landInfo.clan != client.clan))
    {
	ERROR("land incorrect: " << land.toString());
	return false;
    }

    const CreatureInfo & creatureInfo = GameData::creatureInfo(creature);
    if(! client.stones.allowCast(creatureInfo.stones, client.newStone) && !ca.isForce())
    {
	ERROR("player can not cast rule: " << creatureInfo.stones.toString());
	return false;
    }

    if(creatureInfo.unique && GameData::findCreatureUnique(creature))
    {
	ERROR("unique creature found on map, there can be only one!");
	return false;
    }

    if(client.points < creatureInfo.cost && !ca.isForce())
    {
	ERROR("points error: " << client.points << ", " << creatureInfo.cost);
	return false;
    }

    if(! client.army.join(creature, land))
	return false;

    if(!ca.isForce())
    {
	client.points -= creatureInfo.cost;
	client.stones.setCasted(creatureInfo.stones, client.newStone);
	client.setCasted(true);
    }

    actions.push_back(MahjongSummon(currentWind, creature, land));
    actions.push_back(MahjongData(currentWind));

    DEBUG(client.toString() << ", " << "creature: " << creature.toString() << ", " << "land: " << land.toString());
    return true;
}

bool GameData::clientCastSpell(const Avatar & avatar, const ClientMessage & act, ActionList & actions)
{
    LocalPlayer & client = playerOfAvatar(avatar);

    if(currentWind != client.wind)
    {
	ERROR("wind not current: " << client.wind.toString());
	return false;
    }

    auto ca = static_cast<const ClientCastSpell &>(act);

    if(client.isAffectedSpell(Spell::Silence))
    {
	ERROR("player silence mode: " << client.name());
	return false;
    }

    if(client.isCasted() && !ca.isForce())
    {
	ERROR("player already casted: " << client.name());
	return false;
    }

    Spell spell = ca.spell();

    const SpellInfo & spellInfo = GameData::spellInfo(spell);

    if(! client.allowCastSpell(spell) && ! ca.isForce())
    {
	if(spellInfo.stones.size())
	{
	    ERROR("player can not cast rule: " << spellInfo.stones.toString());
	}

	ERROR("player can not cast spell: " << spell.toString());
	return false;
    }

    if(client.points < spellInfo.cost)
    {
	ERROR("points error: " << client.points << ", " << spellInfo.cost);
	return false;
    }

    if(spellInfo.target() == SpellTarget::AllPlayers ||
       spellInfo.target() == SpellTarget::MyPlayer)
    {
	DEBUG(client.toString() << ", " << "spell: " << spell.toString());

	actions.push_back(MahjongCast(currentWind, spell));
	client.mahjongApplySpell(spell);
    }
    else
    if(spellInfo.target() == SpellTarget::OtherPlayer)
    {
	Avatar target = ca.target();
	DEBUG(client.toString() << ", " << "spell: " << spell.toString() << ", " << "target: " << target.toString());

	actions.push_back(MahjongCast(currentWind, spell, target));
	playerOfAvatar(target).mahjongApplySpell(spell);
    }
    else
    {
	Land land = ca.land();
	int unit = ca.unit();

	DEBUG(client.toString() << ", " << "spell: " << spell.toString() << ", " <<
	    "land: " << land.toString() << ", " << "unit: " << String::hex(unit, 8) << ", " << "spell effect: " << spellInfo.effect.toString());

	BattleTargets targets;
	targets.reserve(10);

	if(spellInfo.target() == SpellTarget::Land)
	{
	    for(auto & lp : gamers)
	    {
		auto party = lp.army.findPartyConst(land);
		if(party) targets << BattleTargets(party->toBattleCreatures());
	    }
	}
	else
	if(spellInfo.target() & SpellTarget::Party)
	{
	    if(spellInfo.target() & SpellTarget::Friendly)
	    {
		auto party = client.army.findPartyConst(land);
		if(party) targets << BattleTargets(party->toBattleCreatures());
	    }
	    else
	    if(spellInfo.target() & SpellTarget::Enemy)
	    {
		for(auto & lp : gamers)
		{
	    	    if(client.clan != lp.clan)
		    {
			auto party = lp.army.findPartyConst(land);
			if(party) targets << BattleTargets(party->toBattleCreatures());
		    }
		}
	    }
	}
	else
	if(unit)
	{
	    BattleCreature* bcr = getBattleCreature(unit);
	    if(bcr) targets.push_back(bcr);
	}

	// targets: apply spell
	std::vector<int> resistence;
	resistence.reserve(5);

	for(auto & tgt : targets)
	{
	    if(! tgt->applySpell(spell))
		resistence.push_back(tgt->battleUnit());
	}

	actions.push_back(MahjongCast(currentWind, spell, land, targets, resistence));
	std::set<Clan> checkArmy;

	// check dead creatures
	for(auto & tgt : targets)
	{
	    if(0 >= tgt->loyalty())
	    {
		const LocalPlayer & other = playerOfClan(tgt->clan());
		const std::string & info = StringFormat("%1's %2 vas vanquished").arg(other.name()).arg(tgt->name());
		actions.push_back(MahjongInfo(currentWind, info));
		checkArmy.insert(tgt->clan());
	    }
	}

	if(checkArmy.size())
	{
	    for(auto & clan : checkArmy)
	    {
		LocalPlayer & other = playerOfClan(clan);
		other.army.shrinkEmpty();
	    }
	}
    }

    // client apply
    client.points -= spellInfo.cost;
    client.stones.setCasted(spellInfo.stones, client.newStone);
    client.setCasted(true);

    // copy all data
    actions.push_back(MahjongData(currentWind));
    return true;
}

bool GameData::client2Mahjong(const Avatar & avatar, const ClientMessage & act, ActionList & actions)
{
    switch(act.type())
    {
	case Action::ClientReady:	return clientReady(avatar, act, actions);
	case Action::ClientSayGame:	return clientSayGame(avatar, act, actions);
	case Action::ClientSayChao:	return clientSayChao(avatar, act, actions);
	case Action::ClientSayPung:	return clientSayPung(avatar, act, actions);
	case Action::ClientSayKong:	return clientSayKong(avatar, act, actions);
	case Action::ClientButtonGame:	return clientButtonGame(avatar, act, actions);
	case Action::ClientButtonPass:	return clientButtonPass(avatar, act, actions);
	case Action::ClientButtonPung:	return clientButtonPung(avatar, act, actions);
	case Action::ClientButtonKong1:	return clientButtonKong1(avatar, act, actions);
	case Action::ClientButtonKong2:	return clientButtonKong2(avatar, act, actions);
	case Action::ClientChaoVariant:	return clientChaoVariant(avatar, act, actions);
	case Action::ClientDropIndex:	return clientDropIndex(avatar, act, actions);
	case Action::ClientSummonCreature: return clientSummonCreature(avatar, act, actions);
	case Action::ClientCastSpell:	return clientCastSpell(avatar, act, actions);

	default: break;
    }

    return false;
}

void GameData::validateMahjongSummary(void)
{
    int total = 0;

    for(auto & id : winds_all)
    {
	const LocalPlayer & player = GameData::playerOfWind(id);

        DEBUG(player.toString() << ", " << 
                "stones: " <<  player.stones.toString() << ", " << "rules: " << player.rules.toString());

	total += player.stones.size();
	total += player.rules.count();

	if(player.newStone.isValid())
	{
	    DEBUG("new stone: " << player.newStone());
	    total += 1;
	}
    }

    if(dropStone.isValid())
    {
	DEBUG("drop stone: " << dropStone());
	total += 1;
    }

    DEBUG("croupier trash: " << croupier.trash.toString());
    DEBUG("croupier bank: " << croupier.bank.toString());
    total += croupier.trash.size() + croupier.bank.size();

    DEBUG("game total: " << total << ", " << "(" << (total != 136 ? "FALSE" : "TRUE") << ")");
}

namespace Battle
{
    BattleStrike	applyRangerAttack(const BattleUnit &, BattleCreature &);
    BattleStrike	applyMeleeAttack(BattleUnit &, BattleUnit &, int bonus);
    BattleStrikes	rangersAttack(const BattleCreatures &, BattleParty &);
    BattleStrikes	meleeAttack(BattleUnit &, BattleParty &);
    BattleStrikes	meleesAttack(BattleCreatures, BattleParty &);
    BattleStrikes	doAttackParty(BattleParty &, BattleTown &, BattleParty*);
    int			calculateDamage(const BattleUnit &, const BattleUnit &, int bonus);
}

bool GameData::initAdventure(void)
{
    VERBOSE("wind round: " << roundWind.toString());
    VERBOSE("wind part: " << partWind.toString());

    gamePart = Menu::AdventurePart;
    currentWind = Wind(Wind::East);

    for(auto & lp : gamers)
	lp.initAdventurePart();

    skipRepeatSay = false;
    return true;
}

bool GameData::adventure2Client(const Avatar & avatar, ActionList & actions)
{
    const LocalPlayer & player = GameData::playerOfWind(currentWind);

    if(player.adventurePartDone())
    {
	if(gamePart == Menu::AdventurePart)
	{
	    adventureBattleAction(avatar, actions);
	    if(currentWind() == Wind::North) gamePart = Menu::BattleSummaryPart;
	    currentWind.shift();
	}
	else
	{
	    actions.push_back(AdventureEnd(currentWind));
	    return true;
	}
    }
    else
    {
	// DEBUG("wind: " << currentWind.toString() << ", " << "person: " << player.name());

	if(player.isAI())
	{
	    actions.push_back(AdventureTurn(currentWind));
	    AI::adventureMove(player, actions);
	    client2Adventure(player.avatar, ClientBattleReady(), actions);
	}
	else
	{
	    if(skipRepeatSay)
		return false;

	    actions.push_back(AdventureTurn(currentWind));
	    skipRepeatSay = true;
	}
    }

    return true;
}

bool GameData::client2Adventure(const Avatar & avatar, const ClientMessage & act, ActionList & actions)
{
    switch(act.type())
    {
	case Action::ClientUnitMoved:	return clientUnitMoved(avatar, act, actions);
	case Action::ClientBattleReady:	return clientBattleReady(avatar, act, actions);
	default: break;
    }

    return true;
}

bool GameData::clientUnitMoved(const Avatar & avatar, const ClientMessage & act, ActionList & actions)
{
    LocalPlayer & client = playerOfAvatar(avatar);
    auto ca = static_cast<const ClientUnitMoved &>(act);

    Land land = ca.land();
    int unit = ca.unit();

    DEBUG(client.toString() << ", " << "unit: " << String::hex(unit, 8) << ", to land: " << Land(land).toString());

    const BattleCreature* bcr = client.army.findBattleUnitConst(unit);

    if(! bcr)
    {
        ERROR("unit not found: " << String::hex(unit, 8));
        return false;
    }

    if(client.army.moveCreature(*bcr, land))
    {
	actions.push_back(AdventureMoves(currentWind, unit, land));
	return true;
    }

    return false;
}

bool GameData::clientBattleReady(const Avatar & avatar, const ClientMessage & act, ActionList & actions)
{
    LocalPlayer & player = playerOfAvatar(avatar);

    //auto ca = static_cast<const ClientBattleReady &>(act);
    DEBUG(player.toString());

    player.setAdventurePartDone();
    return true;
}

bool GameData::adventureBattleAction(const Avatar & avatar, ActionList & actions)
{
    LocalPlayer & player = playerOfAvatar(avatar);

    for(auto it = player.army.begin(); it != player.army.end(); ++it)
    {
	const Land & land = (*it).land();

	// skip public zone
	if(land() == Land::TowerOf4Winds) continue;

	LandInfo & landInfo = landsInfo[land()];
	LocalPlayer & other = playerOfClan(landInfo.clan);

	// all armies moved to dest: need check clan
	if(player.clan != other.clan)
	{
	    BattleParty* defenders = other.army.findParty(land);
	    BattleTown town = BattleTown(land);

	    DEBUG("attacker " << (*it).toString());
	    DEBUG("defender tower: " << town.toString());
	    DEBUG("defender " << (defenders ? defenders->toString() : "party: empty"));

	    BattleLegend legend(player.avatar, *it, other.avatar, (defenders ? *defenders : BattleParty()), town, false);

	    // doAttackParty: modify BattleParties
	    const BattleStrikes strikes = Battle::doAttackParty(*it, town, defenders);

	    // wins?
	    if(! town.isAlive())
	    {
		DEBUG("battle wins");
		if(defenders)
		{
		    defenders->dismiss();
		    other.army.shrinkEmpty();
		}
		legend.wins = true;
		landInfo.clan = player.clan;
	    }
	    else
	    {
		DEBUG("battle loose");
		(*it).dismiss();
	    }

	    DEBUG("legend: " << legend.toString());
	    actions.push_back(AdventureCombat(currentWind, legend, strikes));
	    battleHistory.push_back(legend);
	}
    }

    player.army.shrinkEmpty();

    return true;
}
