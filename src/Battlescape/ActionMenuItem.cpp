/*
 * Copyright 2010-2014 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "ActionMenuItem.h"
#include "../Interface/Text.h"
#include "../Interface/Frame.h"
#include "../Engine/Palette.h"
#include "../Engine/Game.h"
#include "../Resource/ResourcePack.h"

namespace OpenXcom
{

/**
 * Sets up an Action menu item.
 * @param id The unique identifier of the menu item.
 * @param game Pointer to the game.
 * @param x Position on the x-axis.
 * @param y Position on the y-asis.
 */
ActionMenuItem::ActionMenuItem(int id, Game *game, int x, int y) : 
	InteractiveSurface(270, 20, x + 25, y - (id*25)), _highlighted(false),
		_action(BA_NONE), _tu(0)
{
	Font *big = game->getResourcePack()->getFont("FONT_BIG"), *small = game->getResourcePack()->getFont("FONT_SMALL");
	Language *lang = game->getLanguage();

	Element *actionMenu = game->getRuleset()->getInterface("battlescape")->getElement("actionMenu");

	_highlightModifier = actionMenu->TFTDMode ? 12 : 3;

	_frame = new Frame(getWidth(), getHeight(), 0, 0);
	_frame->setHighContrast(true);
	_frame->setColor(actionMenu->border);
	_frame->setSecondaryColor(actionMenu->color2);
	_frame->setThickness(3);

	_txtDescription = new Text(110, 9, 10, 6);
	_txtDescription->initText(big, small, lang);
	_txtDescription->setHighContrast(true);
	_txtDescription->setColor(actionMenu->color);
	_txtDescription->setVisible(true);

	_txtRange = new Text(55, 9, 110, 6);
	_txtRange->initText(big, small, lang);
	_txtRange->setHighContrast(true);
	_txtRange->setColor(actionMenu->color);

	_txtAcc = new Text(55, 9, 160, 6);
	_txtAcc->initText(big, small, lang);
	_txtAcc->setHighContrast(true);
	_txtAcc->setColor(actionMenu->color);

	_txtTU = new Text(55, 9, 215, 6);
	_txtTU->initText(big, small, lang);
	_txtTU->setHighContrast(true);
	_txtTU->setColor(actionMenu->color);
}

/**
 * Deletes the ActionMenuItem.
 */
ActionMenuItem::~ActionMenuItem()
{
	delete _frame;
	delete _txtDescription;
	delete _txtRange;
	delete _txtAcc;
	delete _txtTU;
	
}

/**
 * Links with an action and fills in the text fields.
 * @param action The battlescape action.
 * @param description The actions description.
 * @param accuracy The actions accuracy, including the Acc> prefix.
 * @param timeunits The timeunits string, including the TUs> prefix.
 * @param tu The timeunits value.
 */
void ActionMenuItem::setAction(BattleActionType action, const std::wstring &description, 
		const std::wstring &accuracy, const std::wstring &timeunits, const std::wstring &range, 
		int tu)
{
	_action = action;
	_txtDescription->setText(description);
	_txtRange->setText(range);
	_txtAcc->setText(accuracy);
	_txtTU->setText(timeunits);
	_tu = tu;
	_redraw = true;
}

/**
 * Gets the action that was linked to this menu item.
 * @return Action that was linked to this menu item.
 */
BattleActionType ActionMenuItem::getAction() const
{
	return _action;
}

/**
 * Gets the action tus that were linked to this menu item.
 * @return The timeunits that were linked to this menu item.
 */
int ActionMenuItem::getTUs() const
{
	return _tu;
}

/**
 * Replaces a certain amount of colors in the surface's palette.
 * @param colors Pointer to the set of colors.
 * @param firstcolor Offset of the first color to replace.
 * @param ncolors Amount of colors to replace.
 */
void ActionMenuItem::setPalette(SDL_Color *colors, int firstcolor, int ncolors)
{
	Surface::setPalette(colors, firstcolor, ncolors);
	_frame->setPalette(colors, firstcolor, ncolors);
	_txtDescription->setPalette(colors, firstcolor, ncolors);
	_txtRange->setPalette(colors, firstcolor, ncolors);
	_txtAcc->setPalette(colors, firstcolor, ncolors);
	_txtTU->setPalette(colors, firstcolor, ncolors);
}

/**
 * Draws the bordered box.
 */
void ActionMenuItem::draw()
{
	_frame->blit(this);
	_txtDescription->blit(this);
	_txtRange->blit(this);
	_txtAcc->blit(this);
	_txtTU->blit(this);	
}

/**
 * Processes a mouse hover in event.
 * @param action Pointer to an action.
 * @param state Pointer to a state.
 */
void ActionMenuItem::mouseIn(Action *action, State *state)
{
	_highlighted = true;
	_frame->setSecondaryColor(_frame->getSecondaryColor() - _highlightModifier);
	draw();
	InteractiveSurface::mouseIn(action, state);
}


/**
 * Processes a mouse hover out event.
 * @param action Pointer to an action.
 * @param state Pointer to a state.
 */
void ActionMenuItem::mouseOut(Action *action, State *state)
{
	_highlighted = false;
	_frame->setSecondaryColor(_frame->getSecondaryColor() + _highlightModifier);
	draw();
	InteractiveSurface::mouseOut(action, state);
}


}
