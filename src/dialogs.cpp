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

#include <iterator>
#include <sstream>
#include <algorithm>

#include "gametheme.h"
#include "actions.h"
#include "settings.h"
#include "dialogs.h"

void MessageTop(const std::string & hdr, const std::string & msg, Window & win1)
{
    TermGUI::MessageBox win2(hdr, msg, 0, systemFont(), & win1);
    DisplayScene::sceneRedraw();
}

DialogWindow::DialogWindow(const char* json, Window & win) : Window(& win)
{
    jobject = GameTheme::jsonResource(json).toObject();
    if(jobject.isValid())
    {
	setSize(JsonUnpack::size(jobject, "size"));
	setPosition((Display::size() - size()) / 2);

	font = jobject.getString("font");
	borderWidth = jobject.getInteger("width:border");

	backgroundColor = jobject.getString("color:background");
	borderColor = jobject.getString("color:border");
	headerColor = jobject.getString("color:header");
	textColor = jobject.getString("color:text");
    }
    else
    {
	ERROR("parse error: " << json);
    }

    setState(FlagModality);
}

void DialogWindow::actionDialogClose(void)
{
    setVisible(false);
}

bool DialogWindow::keyPressEvent(const KeySym & key)
{
    switch(key.keycode())
    {
        case Key::ESCAPE:
	    actionDialogClose();
            return true;

        default: break;
    }

    return false;
}

bool DialogWindow::mouseClickEvent(const ButtonsEvent & coords)
{
    if(coords.isButtonLeft() || coords.isButtonRight())
    {
	actionDialogClose();
	return true;
    }
    return false;
}

/* TextAreaScroll */
TextScroll::TextScroll(const JsonObject & jo, ListWidget & win) : ScrollBar(true, win)
{
    colorRect = JsonUnpack::color(jo, "scroll:color:rect", Color::DarkSlateGray);
    colorFill = JsonUnpack::color(jo, "scroll:color:fill", Color::SlateGray);

    Color cursRect = JsonUnpack::color(jo, "scroll:cursor:color:rect", Color::Lime);
    Color cursFill = JsonUnpack::color(jo, "scroll:cursor:color:fill", Color::Gray);
    Size curSize = JsonUnpack::size(jo, "scroll:cursor:size", Size(8, 30));

    setTextureCursor(Display::renderRect(cursRect, cursFill, Size(8 /* scroll width harcode 10 pt, fixme below */, curSize.h)));
}

Rect TextScroll::scrollArea(void) const
{
    return rect() + Point(1, 1) - Size(2, 2);
}

void TextScroll::renderWindow(void)
{
    renderColor(colorFill, rect());
    renderRect(colorRect, rect());
    ScrollBar::renderWindow();
}

void TextAreaScroll::setPositionSize(int px, int py, int sw, int sh)
{
    const int scrollWidth = 10;

    setPosition(px, py);
    setSize(sw, sh);
    setVisible(true);

    textScroll.setPosition(Point(sw + 2, 0));
    textScroll.setSize(Size(scrollWidth, sh - 2));
    textScroll.setVisible(true);
}

void TextAreaScroll::itemClicked(ListWidgetItem*, int buttons)
{
    parent()->setVisible(false);
}

/* CreatureInfoDialog */
CreatureInfoDialog::CreatureInfoDialog(const Creature & creature, Window & win)
    : DialogWindow("dialog_creatureinfo.json", win), creatureInfo(GameData::creatureInfo(creature)), textArea(jobject, *this)
{
    skill1 = GameTheme::jsonSprite(jobject, "sprite:attack");
    skill2 = GameTheme::jsonSprite(jobject, "sprite:ranger");
    skill3 = GameTheme::jsonSprite(jobject, "sprite:defense");
    skill4 = GameTheme::jsonSprite(jobject, "sprite:loyalty");
    skill5 = GameTheme::jsonSprite(jobject, "sprite:move");

    std::string text = creatureInfo.description;

    if(creatureInfo.specials.any())
    {
	text.clear();

	for(auto & spec : creatureInfo.specials.toList())
	{
    	    const SpecialityInfo & info = GameData::specialityInfo(spec);
    
	    text.append("[color:green]").append(_("Speciality")).append(":[color:red] ").append(info.name).append("[color:default]");

	    if(info.description.size())
	    {
		if(spec.id() == Speciality::MagicResistence)
		{
		    std::string description = StringFormat(info.description.c_str()).arg(SpecialityMagicResistence().chance(creature.id()));
		    text.append(". ").append(description).append("\n");
		}
		else
		    text.append(". ").append(info.description).append("\n");
	    }
	    else
		text.append("\n");
    	}
	text.append("\n").append(creatureInfo.description);
    }

    const FontRender & defaultFont = GameTheme::fontRender(font);
    const Texture & icon = GameTheme::texture(creatureInfo.image1);

    Point pos(10, 6 * (defaultFont.lineSkipHeight() + 10) + icon.height() + 20);
    const int scrollWidth = 10;

    textArea.setPositionSize(pos.x, pos.y, width() - pos.x - 4 - scrollWidth - borderWidth, height() - pos.y - borderWidth);
    textArea.appendString(defaultFont, text, textColor, AlignLeft, true);

    setVisible(true);
}

void CreatureInfoDialog::renderWindow(void)
{
    // fill and border
    renderColor(backgroundColor, rect());
    renderColor(borderColor, Rect(Point(0, 0), Size(width(), borderWidth)));
    renderColor(borderColor, Rect(Point(0, height() - borderWidth), Size(width(), borderWidth)));
    renderColor(borderColor, Rect(Point(0, 0), Size(borderWidth, height())));
    renderColor(borderColor, Rect(Point(width() - borderWidth, 0), Size(borderWidth, height())));

    const FontRender & defaultFont = GameTheme::fontRender(font);

    int posy = 10;
    // name
    renderText(defaultFont,
	creatureInfo.unique ? std::string(creatureInfo.name).append(" (").append(_("Unique")).append(")") : creatureInfo.name,
	headerColor, Point(width() / 2, posy), AlignCenter);
    posy += defaultFont.lineSkipHeight() + 10;

    // icon
    const Texture & icon = GameTheme::texture(creatureInfo.image1);
    renderRect(SWE::Color::Black, Rect(Point((width() - icon.width()) / 2 - 1, posy - 1), icon.size() + Size(2, 2)));
    renderTexture(icon, Point((width() - icon.width()) / 2, posy));

    const int spacex = 30;
    posy += icon.height() + 10;

    // attack
    renderTexture(skill1, Point(spacex, posy));
    renderText(defaultFont, _("Attack:"), textColor, Point(spacex + skill1.width() + 10, posy));
    renderText(defaultFont, String::number(creatureInfo.stat.attack), textColor, Point(width() - spacex, posy), AlignRight);

    posy += 30;
    // ranger
    renderTexture(skill2, Point(spacex, posy));
    renderText(defaultFont, _("Ranger:"), textColor, Point(spacex + skill2.width() + 10, posy));
    renderText(defaultFont, String::number(creatureInfo.stat.ranger), textColor, Point(width() - spacex, posy), AlignRight);

    posy += 30;
    // defense
    renderTexture(skill3, Point(spacex, posy));
    renderText(defaultFont, _("Defense:"), textColor, Point(spacex + skill3.width() + 10, posy));
    renderText(defaultFont, String::number(creatureInfo.stat.defense), textColor, Point(width() - spacex, posy), AlignRight);

    posy += 30;
    // loyalty
    renderTexture(skill4, Point(spacex, posy));
    renderText(defaultFont, _("Loyalty:"), textColor, Point(spacex + skill4.width() + 10, posy));
    renderText(defaultFont, String::number(creatureInfo.stat.loyalty), textColor, Point(width() - spacex, posy), AlignRight);

    posy += 30;
    // move
    renderTexture(skill5, Point(spacex, posy));
    renderText(defaultFont, _("Move Point:"), textColor, Point(spacex + skill5.width() + 10, posy));
    renderText(defaultFont, String::number(creatureInfo.stat.move), textColor, Point(width() - spacex, posy), AlignRight);
}

/* SpellInfoDialog */
SpellInfoDialog::SpellInfoDialog(const Spell & spell, Window & win)
    : DialogWindow("dialog_spellinfo.json", win), spellInfo(GameData::spellInfo(spell)), textArea(jobject, *this)
{
    const FontRender & defaultFont = GameTheme::fontRender(font);
    Point pos(10, 3 * (defaultFont.lineSkipHeight() + 10) + 10);

    const int scrollWidth = 10;

    textArea.setPositionSize(pos.x, pos.y, width() - pos.x - 4 - scrollWidth - borderWidth, height() - pos.y - borderWidth);
    textArea.appendString(defaultFont, spellInfo.description, textColor, AlignLeft, true);

    setVisible(true);
}

void SpellInfoDialog::renderWindow(void)
{
    // fill and border
    renderColor(backgroundColor, rect());
    renderColor(borderColor, Rect(Point(0, 0), Size(width(), borderWidth)));
    renderColor(borderColor, Rect(Point(0, height() - borderWidth), Size(width(), borderWidth)));
    renderColor(borderColor, Rect(Point(0, 0), Size(borderWidth, height())));
    renderColor(borderColor, Rect(Point(width() - borderWidth, 0), Size(borderWidth, height())));

    const FontRender & defaultFont = GameTheme::fontRender(font);
    int posy = 10;

    // name
    renderText(defaultFont,
	spellInfo.isCurse() ? std::string(spellInfo.name).append(" (").append(_("Curse")).append(")") : spellInfo.name,
	headerColor, Point(width() / 2, posy), AlignCenter);
    posy += defaultFont.lineSkipHeight() + 10;

    // effect
    renderText(defaultFont, std::string(_("Effect: ")).append(spellInfo.effectDescription()), headerColor, Point(10, posy), AlignLeft);
    posy += defaultFont.lineSkipHeight() + 10;

    // target
    renderText(defaultFont, std::string(_("Target: ")).append(spellInfo.target.toString()), headerColor, Point(10, posy), AlignLeft);
    //posy += defaultFont.lineSkipHeight() + 10;
}

/* AvatarInfoDialog */
AvatarInfoDialog::AvatarInfoDialog(const AvatarInfo & avatar, Window & win)
    : DialogWindow("dialog_avatarinfo.json", win), avatarInfo(avatar), textArea(jobject, *this)
{
    std::string text = avatarInfo.description;

    if(avatar.ability.isValid())
    {
	const AbilityInfo & info = GameData::abilityInfo(avatar.ability);

	text = std::string("[color:green]").append(_("Ability"));
	text.append(": ").append("[color:red]").append(info.name).append("[color:default]");

	if(info.description.size())
	    text.append(". ").append(info.description);

	text.append("\n\n").append(avatarInfo.description);
    }

    const FontRender & defaultFont = GameTheme::fontRender(font);
    const Texture & tx = GameTheme::texture(avatarInfo.portrait);

    Point pos(10, tx.height() + 60);
    const int scrollWidth = 10;

    textArea.setPositionSize(pos.x, pos.y, width() - pos.x - 4 - scrollWidth - borderWidth, height() - pos.y - borderWidth);
    textArea.appendString(defaultFont, String::replace(text, "%1", avatarInfo.name), textColor, AlignLeft, true);

    setVisible(true);
}

void AvatarInfoDialog::renderWindow(void)
{
    // fill and border
    renderColor(backgroundColor, rect());
    renderColor(borderColor, Rect(Point(0, 0), Size(width(), borderWidth)));
    renderColor(borderColor, Rect(Point(0, height() - borderWidth), Size(width(), borderWidth)));
    renderColor(borderColor, Rect(Point(0, 0), Size(borderWidth, height())));
    renderColor(borderColor, Rect(Point(width() - borderWidth, 0), Size(borderWidth, height())));

    const FontRender & defaultFont = GameTheme::fontRender(font);

    int posy = 10;
    // dignity
    Rect rt = renderText(defaultFont, avatarInfo.dignity, headerColor, Point(width() / 2, posy), AlignCenter);
    
    posy += rt.h + 10;
    // icon
    const Texture & icon = GameTheme::texture(avatarInfo.portrait);
    renderTexture(icon, Point((width() - icon.width()) / 2, posy));
}


/* RuneCastContent */
RuneCastContent::RuneCastContent() : rowHeight(34)
{
    reserve(24);
}

void RuneCastContent::addRow(const Enum & id, const Stones & stones, const std::string & name, const std::string & desc, int cost, const LocalPlayer & player, const Stone & newStone)
{
    Rect pos;

    if(size() < 12)
    {
	if(size())
	{
	    pos = back().pos;
	    pos.y += rowHeight;
	}
	else
	    pos = offsetColumn1;
    }
    else
    {
	if(12 == size())
	    pos = offsetColumn2;
	else
	{
	    pos = back().pos;
	    pos.y += rowHeight;
	}
    }

    push_back(RuneCastRow());
    back().stones = stones;
    back().name = name;
    back().description = desc;
    back().cost = cost;
    back().allowCast = !player.isCasted() && cost <= player.points && player.stones.allowCast(stones, newStone);
    back().pos = pos;

    if(id.isCreature())
	back().val.reset(new Creature(static_cast<const Creature &>(id)));
    else
    if(id.isSpell())
	back().val.reset(new Spell(static_cast<const Spell &>(id)));
    else
	back().val.reset();
}

#define SET_CREATURE 0x80000000

RuneCastDialog::RuneCastDialog(const LocalPlayer & local, const Stone & newStone, Window & win)
    : DialogWindow("dialog_runecast.json", win), player(& local), selected(-1)
{
    background = GameTheme::jsonSprite(jobject, "background");
    content.offsetColumn1 = GameTheme::jsonRect(jobject, "area:row1");
    content.offsetColumn2 = GameTheme::jsonRect(jobject, "area:row2");
    content.rowHeight = GameTheme::jsonSize(jobject, "size:row").h;

    const AvatarInfo & avatarInfo = GameData::avatarInfo(local.avatar);
    Spells spells = avatarInfo.spells;

    // add spells from party (creature speciality)
    const Spells & armySpells = local.army.allCastSpells();
    spells.insert(spells.end(), armySpells.begin(), armySpells.end());

    std::ostringstream os;

    // generate content
    for(auto & cr : avatarInfo.creatures)
    {
	const CreatureInfo & creatureInfo = GameData::creatureInfo(cr);
	os.str("");
	os << creatureInfo.name << (creatureInfo.unique ? _(" (unique)") : "") << ": ";
	content.addRow(creatureInfo.id, creatureInfo.stones, creatureInfo.name, os.str(), creatureInfo.cost, local, newStone);
	content.back().disabledUnique = (creatureInfo.unique && GameData::findCreatureUnique(cr));
    }

    // spell with stones first
    for(auto & sp : spells)
    {
	const SpellInfo & spellInfo = GameData::spellInfo(sp);
	if(spellInfo.stones.size())
	{
	    os.str("");
	    os << spellInfo.name << (spellInfo.isCurse() ? _(" (curse)") : "") << ": " << spellInfo.effectDescription();
	    content.addRow(spellInfo.id, spellInfo.stones, spellInfo.name, os.str(), spellInfo.cost, local, newStone);
	}
    }

    // other spells
    for(auto & sp : spells)
    {
	const SpellInfo & spellInfo = GameData::spellInfo(sp);
	if(spellInfo.stones.empty())
	{
	    os.str("");
	    os << spellInfo.name << (spellInfo.isCurse() ? " (curse): " : ": ") << spellInfo.effectDescription();
	    content.addRow(spellInfo.id, spellInfo.stones, spellInfo.name, os.str(), spellInfo.cost, local, newStone);
	}
    }

    offsetRuleCost = GameTheme::jsonPoint(jobject, "offset:rulecost");
    offsetRuleName = GameTheme::jsonPoint(jobject, "offset:rulename");
    offsetRuleDescription = GameTheme::jsonPoint(jobject, "offset:ruledescr");
    offsetPlayerName = GameTheme::jsonPoint(jobject, "offset:playername");
    offsetPlayerPoints = GameTheme::jsonPoint(jobject, "offset:playerpoints");
    offsetCloseWindow = GameTheme::jsonRect(jobject, "area:closewin");

    normalColor = GameTheme::jsonColor(jobject, "color:normal");
    allowColor = GameTheme::jsonColor(jobject, "color:allow");
    selectedColor = GameTheme::jsonColor(jobject, "color:selected");

    skill1 = GameTheme::jsonSprite(jobject, "sprite:attack");
    skill2 = GameTheme::jsonSprite(jobject, "sprite:ranger");
    skill3 = GameTheme::jsonSprite(jobject, "sprite:defense");
    skill4 = GameTheme::jsonSprite(jobject, "sprite:loyalty");
    skill5 = GameTheme::jsonSprite(jobject, "sprite:move");

    setVisible(true);
}

void RuneCastDialog::renderWindow(void)
{
    renderColor(backgroundColor, rect());
    renderTexture(background, Point(0, 0));

    const FontRender & defaultFont = GameTheme::fontRender(font);

    // header
    renderText(defaultFont, String::replace(_("%{name}'s Spells"), "%{name}", player->name()), normalColor, offsetPlayerName);
    renderText(defaultFont, String::replace(_("Spell Points: %{points}"), "%{points}", player->points), normalColor, offsetPlayerPoints);
    renderText(defaultFont, "[X]", normalColor, offsetCloseWindow, AlignLeft);

    // content
    renderContent();

    // description
    if(0 <= selected && selected < content.size())
    {
	const RuneCastRow & row = content[selected];

	// creature description
	if(row.val && row.val->isCreature())
	{
	    Point dst = offsetRuleDescription;
	    const CreatureInfo & info = GameData::creatureInfo(static_cast<const Creature &>(*row.val));

	    // creature icon
	    Texture icon32x32 = Display::createTexture(Size(32, 32));
	    const Texture & icon56x56 = GameTheme::texture(info.image2);
	    Display::renderTexture(icon56x56, icon56x56.rect(), icon32x32, icon32x32.rect());

	    renderTexture(icon32x32, dst + Point(-1, -3));
	    dst.x += icon32x32.width() + 5;

	    // creature name
	    Rect txt = renderText(defaultFont, row.description, normalColor, dst);
	    dst.x += txt.w + 5;

	    // creature skills
	    renderTexture(skill1, dst);
	    dst.x += skill1.width() + 5;
	    Rect num1 = renderText(defaultFont, String::number(info.stat.attack), normalColor, dst);
	    dst.x += num1.w + 10;

	    renderTexture(skill2, dst);
	    dst.x += skill2.width() + 5;
	    Rect num2 = renderText(defaultFont, String::number(info.stat.ranger), normalColor, dst);
	    dst.x += num2.w + 10;

	    renderTexture(skill3, dst);
	    dst.x += skill3.width() + 5;
	    Rect num3 = renderText(defaultFont, String::number(info.stat.defense), normalColor, dst);
	    dst.x += num3.w + 10;

	    renderTexture(skill4, dst);
	    dst.x += skill4.width() + 5;
	    Rect num4 = renderText(defaultFont, String::number(info.stat.loyalty), normalColor, dst);
	    dst.x += num4.w + 10;

	    renderTexture(skill5, dst);
	    dst.x += skill5.width() + 5;
	    Rect num5 = renderText(defaultFont, String::number(info.stat.move), normalColor, dst);
	    dst.x += num5.w + 10;
	}
	// spell description
	else
	if(row.val && row.val->isSpell())
	{
	    Point dst = offsetRuleDescription;
	    const SpellInfo & info = GameData::spellInfo(static_cast<const Spell &>(*row.val));

	    if(info.image.size())
	    {
		// spell icon
		Texture icon32x32 = Display::createTexture(Size(32, 32));
		const Texture & icon20x20 = GameTheme::texture(info.image);
		Display::renderTexture(icon20x20, icon20x20.rect(), icon32x32, icon32x32.rect());

		renderTexture(icon32x32, dst + Point(-1, -3));
		dst.x += icon32x32.width() + 5;
	    }

	    // spell info
	    renderText(defaultFont, row.description, normalColor, dst);
	}
	else
	{
	    ERROR("unknown value");
	}
    }
}

void RuneCastDialog::renderContent(void)
{
    const FontRender & defaultFont = GameTheme::fontRender(font);

    for(auto it = content.begin(); it != content.end(); ++it)
    {
	auto & row = *it;
	bool isSelected = std::distance(content.begin(), it) == selected;
	int offx = 0;

	// stones
	for(auto & st : row.stones)
	{
	    const StoneInfo & stoneInfo = GameData::stoneInfo(st);
	    const Texture & small = GameTheme::texture(stoneInfo.small);
	    renderTexture(small, Point(row.pos.x + offx, row.pos.y));
	    offx += small.width();
	}

	Color renderColor = normalColor;
	if(row.disabledUnique)
	    renderColor = Color::Brown;
	else
	if(row.allowCast)
	    renderColor = isSelected ? selectedColor : allowColor;

	// cost
	renderText(defaultFont, String::number(row.cost), renderColor, row.pos.toPoint() + offsetRuleCost);

	// name
	renderText(defaultFont, row.name, renderColor, row.pos.toPoint() + offsetRuleName);
    }
}

bool RuneCastDialog::mouseClickEvent(const ButtonsEvent & coords)
{
    // click area
    for(auto it = content.begin(); it != content.end(); ++it)
    {
	auto & row = *it;
	const Rect rect = Rect(row.pos.toPoint(), row.pos.toSize());

	if(coords.isClick(rect))
	{
	    int index = std::distance(content.begin(), it);
	    // double click: close window
	    if(selected == index && row.allowCast && ! row.disabledUnique)
	    {
		auto & sel = content[selected];
		//if(content[selected].id & SET_CREATURE) iscreature = true;
		if(sel.val) setResultCode(sel.val->id());
    		actionDialogClose();
	    }
	    else
	    {
		selected = index;
		renderWindow();
	    }

	    return true;
	}
    }

    // any click: close window
    if(coords.isClick(area()) || coords.isClick(offsetCloseWindow))
    {
	selected = -1;
	actionDialogClose();
	return true;
    }

    return false;
}

bool RuneCastDialog::resultIsCreature(void) const
{
    return 0 <= selected && selected < content.size() &&
	    content[selected].val && content[selected].val->isCreature();
}

Creature RuneCastDialog::resultCreature(void) const
{
    return 0 <= selected && selected < content.size() && content[selected].val ?
	static_cast<const Creature &>(*content[selected].val) : Creature();
}

Spell RuneCastDialog::resultSpell(void) const
{
    return 0 <= selected && selected < content.size() && content[selected].val ?
	static_cast<const Spell &>(*content[selected].val) : Spell();
}

/* ShowLogDialog */
ShowLogsDialog::ShowLogsDialog(const UnicodeList & content, Window & win)
    : DialogWindow("dialog_showlog.json", win), textArea(jobject, *this)
{
    background = GameTheme::jsonSprite(jobject, "background");
    Rect bodyArea = GameTheme::jsonRect(jobject, "area:body");
    Color lastColor = GameTheme::jsonColor(jobject, "color:last");

    const FontRender & defaultFont = GameTheme::fontRender(font);
    const int scrollWidth = 10;

    textArea.setPositionSize(bodyArea.x, bodyArea.y, bodyArea.w - 2 - scrollWidth, bodyArea.h);

    // reverse
    for(auto it = content.rbegin(); it != content.rend(); ++it)
	textArea.appendString(defaultFont, *it, (it != content.rbegin() ? textColor : lastColor), AlignLeft, true);

    setVisible(true);
}

void ShowLogsDialog::renderWindow(void)
{
    renderColor(backgroundColor, rect());
    renderTexture(background, Point(0, 0));
}

bool ShowLogsDialog::mouseClickEvent(const ButtonsEvent & coords)
{
    if(coords.isButtonLeft() || coords.isButtonRight())
    {
	actionDialogClose();
	return true;
    }
    return false;
}

/* MessageBoxDialog */
MessageBox::MessageBox(const std::string & hdr, const std::string & msg, Window & win, bool buttons)
    : DialogWindow("dialog_messagebox.json", win), textArea(jobject, *this),
	header(hdr), buttonOk(this), buttonCancel(this)
{
    Rect bodyArea = GameTheme::jsonRect(jobject, "area:body");

    auto txOk1 = GameTheme::jsonSprite(jobject, "sprite:ok1");
    auto txOk2 = GameTheme::jsonSprite(jobject, "sprite:ok2");

    buttonOk.setSprites(txOk1, txOk2);
    buttonOk.setAction(Action::ButtonOk);
    buttonOk.setHotKey(Key::RETURN);
    buttonOk.setPosition(Point(buttons ? 20 : (width() - buttonOk.width()) / 2, height() - 20 - buttonOk.height()));
    buttonOk.setVisible(true);

    if(buttons)
    {
	auto txCancel1 = GameTheme::jsonSprite(jobject, "sprite:cancel1");
	auto txCancel2 = GameTheme::jsonSprite(jobject, "sprite:cancel2");

	buttonCancel.setSprites(txCancel1, txCancel2);
	buttonCancel.setAction(Action::ButtonCancel);
	buttonCancel.setHotKey(Key::ESCAPE);
	buttonCancel.setPosition(Point(width() - 20 - buttonCancel.width(), height() - 20 - buttonCancel.height()));
	buttonCancel.setVisible(true);
    }
    else
    {
	buttonCancel.setVisible(false);
    }

    const FontRender & defaultFont = GameTheme::fontRender(font);
    const int scrollWidth = 10;

    textArea.setPositionSize(bodyArea.x, bodyArea.y, bodyArea.w - 2 - scrollWidth, bodyArea.h);
    textArea.appendString(defaultFont, msg, textColor, AlignCenter, true);

    setVisible(true);
}

void MessageBox::renderWindow(void)
{
    // fill and border
    renderColor(backgroundColor, rect());
    renderColor(borderColor, Rect(Point(0, 0), Size(width(), borderWidth)));
    renderColor(borderColor, Rect(Point(0, height() - borderWidth), Size(width(), borderWidth)));
    renderColor(borderColor, Rect(Point(0, 0), Size(borderWidth, height())));
    renderColor(borderColor, Rect(Point(width() - borderWidth, 0), Size(borderWidth, height())));

    if(header.size())
    {
	const FontRender & defaultFont = GameTheme::fontRender(font);
	renderText(defaultFont, header, headerColor, Point(width() / 2, 15), AlignCenter);
    }
}

bool MessageBox::userEvent(int act, void* data)
{
    switch(act)
    {
        case Action::ButtonOk:
            setResultCode(1);
	    actionDialogClose();
            return true;

        case Action::ButtonCancel:
	    actionDialogClose();
            setVisible(false);
            return true;
    
        default:
            break;
    }

    return false;
}

/* MapStatusDialog */
MapStatusDialog::MapStatusDialog(const LocalData & data, Window & win)
    : DialogWindow("dialog_mapstatus.json", win), localData(data), buttonClose(jobject, "button:close", Action::ButtonClose, this)
{
    background = GameTheme::jsonSprite(jobject, "background");
    iconSprites = GameTheme::jsonSprites(jobject, "playerinfo:sprites");

    offsetArmy = GameTheme::jsonPoint(jobject, "playerinfo:offset:army");
    offsetAvatarPort = GameTheme::jsonPoint(jobject, "playerinfo:offset:portrait");
    offsetTextAvatar = GameTheme::jsonPoint(jobject, "playerinfo:offset:name");
    offsetClanIcon = GameTheme::jsonPoint(jobject, "playerinfo:offset:clan_icon");
    offsetTextClan = GameTheme::jsonPoint(jobject, "playerinfo:offset:clan_name");
    offsetTextPoint = GameTheme::jsonPoint(jobject, "playerinfo:offset:point");
    offsetTextLands = GameTheme::jsonPoint(jobject, "playerinfo:offset:lands");
    offsetTextLandPoint = GameTheme::jsonPoint(jobject, "playerinfo:offset:land_point");
    offsetTextLandPower = GameTheme::jsonPoint(jobject, "playerinfo:offset:land_power");

    const JsonArray* ja = nullptr;

    ja = jobject.getArray("playerinfo:offset:clan_vals");
    if(ja) offsetClanValues = JsonUnpack::points(*ja);
    if(3 > offsetClanValues.size()) offsetClanValues.resize(3);

    ja = jobject.getArray("playerinfo:offset:clan_icons");
    if(ja) offsetClanIcons = JsonUnpack::points(*ja);
    if(3 > offsetClanIcons.size()) offsetClanIcons.resize(3);

    offsetWinds[0] = GameTheme::jsonPoint(jobject, "players:offset:wind_east");
    offsetWinds[1] = GameTheme::jsonPoint(jobject, "players:offset:wind_south");
    offsetWinds[2] = GameTheme::jsonPoint(jobject, "players:offset:wind_west");
    offsetWinds[3] = GameTheme::jsonPoint(jobject, "players:offset:wind_north");

    orderPlayPos = GameTheme::jsonPoint(jobject, "offset:orderplay");
    youArePos = GameTheme::jsonPoint(jobject, "offset:youarepos");

    setVisible(true);
}

void MapStatusDialog::renderWindow(void)
{
    renderTexture(background, Point(0, 0));

    renderWindSection(localData.playerOfWind(Wind::East), offsetWinds[0]);
    renderWindSection(localData.playerOfWind(Wind::South), offsetWinds[1]);
    renderWindSection(localData.playerOfWind(Wind::West), offsetWinds[2]);
    renderWindSection(localData.playerOfWind(Wind::North), offsetWinds[3]);

    const FontRender & defaultFont = GameTheme::fontRender(font);

    renderText(defaultFont, StringFormat(_("You Are %1")).arg(localData.myPlayer().name()), textColor, youArePos);
    Rect pos = renderText(defaultFont, _("Order Of Play"), textColor, orderPlayPos);

    pos.x += pos.w + 10;
    for(auto & id : winds_all)
    {
	const RemotePlayer & player = localData.playerOfWind(id);
	const ClanInfo & info = GameData::clanInfo(player.clan);
	const Texture & flag = GameTheme::texture(info.flag1);

	renderTexture(flag, Point(pos.x, pos.y + (pos.h - flag.height()) / 2));
	pos.x += flag.width() + 10;
    }
}

void MapStatusDialog::renderWindSection(const RemotePlayer & player, const Point & pos)
{
    const AvatarInfo & avatarInfo = GameData::avatarInfo(player.avatar);
    const ClanInfo & clanInfo = GameData::clanInfo(player.clan);

    // render portrait
    renderTexture(GameTheme::texture(avatarInfo.portrait), pos + offsetAvatarPort);

    // render clan icon
    renderTexture(GameTheme::texture(clanInfo.button), pos + offsetClanIcon);

    // render sprites
    for(auto & icon : iconSprites)
        renderTexture(icon, pos + icon.position());

    const FontRender & defaultFont = GameTheme::fontRender(font);

    // avatar name
    renderText(defaultFont, avatarInfo.name, textColor, pos + offsetTextAvatar);

    // clan name
    renderText(defaultFont, clanInfo.name, textColor, pos + offsetTextClan);

    // avatar point
    renderText(defaultFont, String::number(player.points), textColor, pos + offsetTextPoint);

    const Lands lands = player.lands();

    // lands count
    renderText(defaultFont, String::number(lands.size()), textColor, pos + offsetTextLands);

    int landPower = 0;
    int landPoint = 0;

    for(auto & land : lands)
    {
	const LandInfo & info = GameData::landInfo(land);
	if(info.stat.power) landPower++;
	landPoint += info.stat.point;
    }

    renderText(defaultFont, String::number(landPoint), textColor, pos + offsetTextLandPoint);
    renderText(defaultFont, String::number(landPower), textColor, pos + offsetTextLandPower);

    int index = 0;
    // render other clan icons
    for(auto & clan : clans_all)
    {
	if(player.clan() != clan)
	{
	    const ClanInfo & other = GameData::clanInfo(clan);
	    renderTexture(GameTheme::texture(other.button), pos + offsetClanIcons[index]);
	    renderText(defaultFont, "FIXME" /* */, textColor, pos + offsetClanValues[index]);
	    index++;
	}
    }

    renderBattleArmy(player.army.toBattleCreatures(), pos + offsetArmy);
}

void MapStatusDialog::renderBattleArmy(const BattleCreatures & creatures, const Point & pos)
{
    for(auto it = creatures.begin(); it != creatures.end(); ++it)
    {
	int index = std::distance(creatures.begin(), it);
	const CreatureInfo & creatureInfo = GameData::creatureInfo(**it);
	const Texture & icon = GameTheme::texture(creatureInfo.image2);
	renderTexture(icon, pos + Point(index % 2 ? icon.width() : 0, icon.height() * (index / 2)));
    }
}

bool MapStatusDialog::userEvent(int act, void* data)
{
    switch(act)
    {
        case Action::ButtonClose:
            actionDialogClose();
            return true;

        default:
            break;
    }

    return false;
}

CombatUnit::CombatUnit(BattleUnit* ptr, const Point & pt, const Texture & tx1, const Texture & tx2)
    : battle(ptr), pos(pt), back(tx1), status(tx2)
{
    if(battle)
    {
	if(battle->isCreature())
	{
	    BattleCreature* creature = static_cast<BattleCreature*>(battle);
	    image = GameTheme::texture(GameData::creatureInfo(*creature).image1);
	}
	else
	if(battle->isTown())
	{
	    BattleTown* town = static_cast<BattleTown*>(battle);
	    image = GameTheme::texture(GameData::clanInfo(town->currentClan()).image);
	}
    }
}

void CombatUnit::render(Window & win, const std::pair<Color, Color> & damageColors, bool showCorpse)
{
    if(battle && (battle->isAlive() || showCorpse))
    {
	win.renderTexture(image, pos);
	Rect rt = Rect(pos + Size(image.width(), 0), status.size());
	win.renderTexture(status, rt);
	rt = Rect(rt.x + 4, rt.y + 4, rt.w - 8, rt.h - 8);
	win.renderColor(damageColors.second, rt);
	if(battle->isAlive())
	{
	    int curh = battle->loyalty() * rt.h / std::max(battle->loyalty(), battle->baseLoyalty());
	    rt.y += rt.h - curh;
	    rt.h = curh;
	    win.renderColor(damageColors.first, rt);
	}
    }
    else
    {
	win.renderTexture(back, pos);
    }
}

void CombatUnits::push_back(const Point & pos1, const Point & pos2, const Point & pos3, const Texture & back, const Texture & status, BattleCreatures bcrs)
{
    BattleUnit* unit1 = nullptr;
    BattleUnit* unit2 = nullptr;
    BattleUnit* unit3 = nullptr;

    switch(bcrs.size())
    {
	case 0: break;
	case 1:	unit2 = bcrs[0]; break;
	case 2: unit1 = bcrs[0]; unit2 = bcrs[1]; break;
	default: unit1 = bcrs[0]; unit2 = bcrs[1]; unit3 = bcrs[2]; break;
    }

    // push back: valid - render unit, invalid: render background
    push_back(CombatUnit(unit1, pos1, back, status));
    push_back(CombatUnit(unit2, pos2, back, status));
    push_back(CombatUnit(unit3, pos3, back, status));
}

CombatUnit* CombatUnits::find(int uid)
{
    auto it = std::find_if(begin(), end(), [=](const CombatUnit & unit){ return unit.isUid(uid); });
    return it != end() ? & (*it) : nullptr;
}

/* CombatScreenDialog */
CombatScreenDialog::CombatScreenDialog(const BattleLegend & leg, const BattleStrikes & str, Window & win)
    : DialogWindow("dialog_combat.json", win), legend(leg), strikes(str),
	animationStrikeMelee(jobject, "animation:strike_melee"),
	animationStrikeRanger(jobject, "animation:strike_ranger"),
	animationFireShield(jobject, "animation:fire_shield"),
	renderCorpse(true), doDialogClose(false)
{
    background = GameTheme::jsonSprite(jobject, "background");
    delayStrikeAnim = jobject.getInteger("delay:strike", 300);
    delayCombatScreen = jobject.getInteger("delay:combatscreen", 600);

    auto lifeStatus = GameTheme::jsonSprite(jobject, "sprite:lifestatus");
    auto cellFill = GameTheme::jsonSprite(jobject, "sprite:cellfill");

    const AvatarInfo & avatar1Info = GameData::avatarInfo(legend.attacker);
    const AvatarInfo & avatar2Info = GameData::avatarInfo(legend.defender);

    name1 = avatar1Info.name;
    name2 = avatar2Info.name;

    spritePort1.setTexture(GameTheme::texture(avatar1Info.portrait));
    spritePort1.setPosition(GameTheme::jsonPoint(jobject, "offset:port1"));

    spritePort2.setTexture(GameTheme::texture(avatar2Info.portrait));
    spritePort2.setPosition(GameTheme::jsonPoint(jobject, "offset:port2"));

    Point offsetTower = GameTheme::jsonPoint(jobject, "offset:tower");
    Point offsetParty11 = GameTheme::jsonPoint(jobject, "offset:party11");
    Point offsetParty12 = GameTheme::jsonPoint(jobject, "offset:party12");
    Point offsetParty13 = GameTheme::jsonPoint(jobject, "offset:party13");
    Point offsetParty21 = GameTheme::jsonPoint(jobject, "offset:party21");
    Point offsetParty22 = GameTheme::jsonPoint(jobject, "offset:party22");
    Point offsetParty23 = GameTheme::jsonPoint(jobject, "offset:party23");

    textColor = GameTheme::jsonColor(jobject, "color:text");
    damageColors.first = GameTheme::jsonColor(jobject, "color:alive");
    damageColors.second = GameTheme::jsonColor(jobject, "color:damage");

    units.push_back(CombatUnit(& legend.town, offsetTower, cellFill, lifeStatus));
    units.push_back(offsetParty11, offsetParty12, offsetParty13, cellFill, lifeStatus, legend.attackers.toBattleCreatures());
    units.push_back(offsetParty21, offsetParty22, offsetParty23, cellFill, lifeStatus, legend.defenders.toBattleCreatures());

    center1Pos = GameTheme::jsonPoint(jobject, "offset:center1");
    center2Pos = GameTheme::jsonPoint(jobject, "offset:center2");

    renderWindow();
    setVisible(true);
}

void CombatScreenDialog::tickEvent(u32 ms)
{
    if(animationFireShield.isEnabled())
    {
    	if(animationFireShield.next(ms))
    	    renderWindow();
    }
    else
    if(animationStrikeMelee.isEnabled())
    {
    	if(animationStrikeMelee.next(ms))
    	    renderWindow();
    }
    else
    if(animationStrikeRanger.isEnabled())
    {
    	if(animationStrikeRanger.next(ms))
    	    renderWindow();
    }
    else
    if(tt.check(ms, 200))
    {
	if(doDialogClose)
	{
	    Tools::delay(delayCombatScreen);
	    actionDialogClose();
	}
	else
	{
	    if(strikes.empty())
	    {
		renderSummary(legend.wins ? legend.attacker : legend.defender);
		doDialogClose = true;
	    }
	    else
	    // apply strike
	    {
		SpritesAnimation* animation = nullptr;

		if(strikes.front().type == BattleStrike::FireShield)
    		    animation = & animationFireShield;
		else
		if(strikes.front().type == BattleStrike::Ranger)
    		    animation = & animationStrikeRanger;
		else
    		    animation = & animationStrikeMelee;

		Rect strikeArea = applyStrikeDamage(strikes.front());

    		animation->setEnabled(true);
		animation->setPosition(strikeArea.toPoint() + (animation->spriteSize() - strikeArea.toSize()) / 2);
		animation->next();

		strikes.pop_front();
		Tools::delay(delayStrikeAnim);
	    }

	    renderWindow();
	}
    }
}

void CombatScreenDialog::renderWindow(void)
{
    //renderRect(Color::Black, rect());

    // renderUnits
    for(auto & unit : units)
	unit.render(*this, damageColors, renderCorpse);

    renderTexture(background, Point(0, 0));
    renderTexture(spritePort1);
    renderTexture(spritePort2);

    const FontRender & defaultFont = GameTheme::fontRender(font);

    renderText(defaultFont, name1, textColor, center1Pos, AlignCenter, AlignCenter);
    renderText(defaultFont, name2, textColor, center2Pos, AlignCenter, AlignCenter);

    if(animationFireShield.isEnabled()) animationFireShield.render(*this);
    else
    if(animationStrikeRanger.isEnabled()) animationStrikeRanger.render(*this);
    else
    if(animationStrikeMelee.isEnabled()) animationStrikeMelee.render(*this);
}

Rect CombatScreenDialog::applyStrikeDamage(const BattleStrike & strike)
{
    CombatUnit* unit1 = units.find(strike.unit1);
    CombatUnit* unit2 = units.find(strike.unit2);
    BattleUnit* attacker = unit1 ? unit1->battle : nullptr;
    BattleUnit* target = unit2 ? unit2->battle : nullptr;
    Rect res;

    DEBUG("strike: " <<
	"unit1: " << String::hex(strike.unit1, 8) << (strike.is_creature1 ? "(true)" : "(false)") << ", " <<
	"unit2: " << String::hex(strike.unit2, 8) << (strike.is_creature2 ? "(true)" : "(false)") << ", " <<
	"damage: " << strike.damage << ", " <<
	"type: " << strike.type);

    if(attacker && target)
    {
	target->applyDamage(strike.damage);

	DEBUG("attacker: " << attacker->name() << ", " << "do damage: " << strike.damage << ", " <<
		"target: " << target->name() << ", " <<
		"loyalty: " << target->loyalty() << ", " <<
		"status: " << (target->isAlive() ? "is alive" : (target->isTown() ? "is captured" : "is dead")));

	res = Rect(unit2->pos, unit2->back.size());
    }
    else
    {
	if(!attacker) ERROR("attacker not found" << ", " << "unit: " << String::hex(strike.unit1, 8));
	if(!target) ERROR("target not found" << ", " << "unit: " << String::hex(strike.unit2, 8));
    }

    return res;
}

void CombatScreenDialog::renderSummary(const Avatar & winner)
{
    name1 = _("Winner is");
    name2 = GameData::avatarInfo(winner).name;

    if(legend.attacker == winner)
	spritePort2.reset();
    else
    if(legend.defender == winner)
	spritePort1.reset();

    renderCorpse = false;
}

bool CombatScreenDialog::mouseClickEvent(const ButtonsEvent &)
{
    return false;
}

/* TargetPlayerDialog */
TargetPlayerButton::TargetPlayerButton(const RemotePlayer & target, Window & win) : JsonButton(& win)
{
    const AvatarInfo & avatarInfo = GameData::avatarInfo(target.avatar);
    const Texture & icon = GameTheme::sprite(avatarInfo.portrait);

    Texture txBorder = Display::renderRect(Color::Yellow, Color::Transparent, icon.size(), 2);
    Texture txSelected = Display::renderRect(Color::Red, Color::Transparent, icon.size(), 2);
 
    setSize(icon.size());
    setAction(target.avatar.id());
    setVisible(true);

    sprite1 = Display::createTexture(icon);
    sprite3 = Display::createTexture(icon);
    
    Display::renderTexture(txBorder, txBorder.rect(), sprite1, sprite1.rect());
    Display::renderTexture(txSelected, txSelected.rect(), sprite3, sprite3.rect());
}

TargetPlayerDialog::TargetPlayerDialog(const LocalData & ld, Window & win)
    : DialogWindow("dialog_targetplayer.json", win),
    buttonTargetLeft(ld.remoteLeft(), *this),
    buttonTargetRight(ld.remoteRight(), *this),
    buttonTargetTop(ld.remoteTop(), *this)
{
    buttonTargetLeft.setPosition(GameTheme::jsonPoint(jobject, "offset:target_left"));
    buttonTargetRight.setPosition(GameTheme::jsonPoint(jobject, "offset:target_right"));
    buttonTargetTop.setPosition(GameTheme::jsonPoint(jobject, "offset:target_top"));

    signalSubscribe(buttonTargetLeft, Signal::ButtonClicked);
    signalSubscribe(buttonTargetRight, Signal::ButtonClicked);
    signalSubscribe(buttonTargetTop, Signal::ButtonClicked);

    setVisible(true);
}

void TargetPlayerDialog::renderWindow(void)
{
    Color col = Color::Gray;
    Size sz = size();
    int space = 8;

    for(int xx = 0; xx < sz.w; xx += space)
        renderLine(col, Point(xx, 0), Point(xx, height() - 1));

    for(int yy = 0; yy < sz.h; yy += space)
	renderLine(col, Point(0, yy), Point(width() - 1, yy));
}

void TargetPlayerDialog::signalReceive(int sig, const SignalMember* sm)
{
    switch(sig)
    {
        case Signal::ButtonClicked:
	    if(sm == &buttonTargetRight || sm == &buttonTargetLeft || sm == &buttonTargetTop)
	    {
		const JsonButton* jb = static_cast<const JsonButton*>(sm);
		setResultCode(jb->action());
		// close post action
		pushEventAction(Action::ButtonClose, this, nullptr);
	    }
            break;

	default:
	    break;
    }
}

bool TargetPlayerDialog::userEvent(int act, void* data)
{
    if(act == Action::ButtonClose)
    {
	actionDialogClose();
	return true;
    }

    return false;
}
