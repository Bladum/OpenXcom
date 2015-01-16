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
#include "BaseDefenseState.h"
#include <sstream>
#include "../Engine/Game.h"
#include "../Resource/ResourcePack.h"
#include "../Engine/Language.h"
#include "../Engine/Palette.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Base.h"
#include "../Savegame/BaseFacility.h"
#include "../Ruleset/RuleBaseFacility.h"
#include "../Savegame/Ufo.h"
#include "../Interface/TextList.h"
#include "GeoscapeState.h"
#include "../Engine/Action.h"
#include "../Engine/RNG.h"
#include "../Savegame/ItemContainer.h"
#include "../Battlescape/BriefingState.h"
#include "../Battlescape/BattlescapeGenerator.h"
#include "../Engine/Sound.h"
#include "BaseDestroyedState.h"
#include "../Engine/Timer.h"
#include "../Engine/Options.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Base Defense screen.
 * @param game Pointer to the core game.
 * @param base Pointer to the base being attacked.
 * @param ufo Pointer to the attacking ufo.
 * @param state Pointer to the Geoscape.
 */
BaseDefenseState::BaseDefenseState(Base *base, Ufo *ufo, GeoscapeState *state) : _state(state)
{
	_base = base;
	_action = BDA_NEW;
	_row = -1;
	_passes = 0;
	_attacks = 0;
	_thinkcycles = 0;
	_ufo = ufo;
	_fireCount = 0;

	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_txtTitle = new Text(300, 17, 16, 6);
	_txtInit = new Text(300, 10, 16, 24);
	_lstDefenses = new TextList(300, 128, 16, 40);
	_btnOk = new TextButton(120, 18, 100, 170);

	// Set palette
	setPalette("PAL_BASESCAPE", _game->getRuleset()->getInterface("baseDefense")->getElement("palette")->color);

	add(_window, "window", "baseDefense");
	add(_btnOk, "button", "baseDefense");
	add(_txtTitle, "text", "baseDefense");
	add(_txtInit, "text", "baseDefense");
	add(_lstDefenses, "text", "baseDefense");

	centerAllSurfaces();

	// Set up objects
	_window->setBackground(_game->getResourcePack()->getSurface("BACK04.SCR"));

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&BaseDefenseState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&BaseDefenseState::btnOkClick, Options::keyOk);
	_btnOk->onKeyboardPress((ActionHandler)&BaseDefenseState::btnOkClick, Options::keyCancel);
	_btnOk->setVisible(false);

	_txtTitle->setBig();
	_txtTitle->setText(tr("STR_BASE_UNDER_ATTACK").arg(_base->getName()));
	_txtInit->setVisible(false);

	_txtInit->setText(tr("STR_BASE_DEFENSES_INITIATED"));

	//                          MISSILE DEFENCES     FIRING  0/1   HIT!  95%
	_lstDefenses->setColumns(5, 100,                 50,     45,   45,   45);
	
	_gravShields = _base->getGravShields(); 
	_defenses = _base->getDefenses()->size();
	_timer = new Timer(500);
	_timer->onTimer((StateHandler)&BaseDefenseState::nextStep);
	_timer->start();

	_lstDefenses->addRow(5, tr("STR_FACILITY").c_str() ,tr("STR_STATUS").c_str(),
			tr("STR_SHOTS").c_str(),tr("STR_RESULT").c_str(),tr("STR_UFO_LIFE").c_str());
	++_row;
	_explosionCount = 0;
}
/**
 *
 */
BaseDefenseState::~BaseDefenseState()
{
	delete _timer;
}

void BaseDefenseState::think()
{
	_timer->think(this, 0);
}

void BaseDefenseState::nextStep()
{
	int power;
	int chance;
	int interval;
	std::string ammo ;
	std::wstring dd;
	std::wstring aa;

	if (_thinkcycles == -1)
		return;
	
	++_thinkcycles;

	if (_thinkcycles == 1)
	{
		_txtInit->setVisible(true);
		return;
	}

	if (_thinkcycles > 1)
	{
		switch (_action)
		{
		case BDA_DESTROY:
			if (!_explosionCount)
			{
				_lstDefenses->addRow(5, tr("STR_UFO_DESTROYED").c_str(),L" ",L" ",L" ",L" ");
				++_row;
				if (_row > 14)
				{
					_lstDefenses->scrollDown(true);
				}
			}
			_game->getResourcePack()->getSound("GEO.CAT", ResourcePack::UFO_EXPLODE)->play();
			if (++_explosionCount == 3)
			{
				_action = BDA_END;
			}
			return;
		case BDA_END:
			_btnOk->setVisible(true);
			_thinkcycles = -1;
			return;
		default:
			break;
		}
		if (_attacks >= _defenses && _passes == _gravShields)
		{
			_action = BDA_END;
			return;
		}
		else if (_attacks >= _defenses && _passes < _gravShields)
		{
			_lstDefenses->addRow(5, tr("STR_GRAV_SHIELD_REPELS_UFO").c_str(),L" ",L" ",L" ",L" ");
			if (_row > 14)
			{
				_lstDefenses->scrollDown(true);
			}
			++_row;
			++_passes;
			_attacks = 0;
			return;
		}	

		BaseFacility* def = _base->getDefenses()->at(_attacks);
		power = def->getRules()->getDefenseValue();
		chance = def->getRules()->getHitRatio();
		interval = def->getRules()->getFireInterval();
		ammo = def->getRules()->getClipUsed();

		switch (_action)
		{
		case  BDA_NEW:
						
			// MISSILE DEFENCES     FIRING   0/1    HIT!  95%
			_lstDefenses->addRow(5, tr((def)->getRules()->getType()).c_str(),L" ",L" ",L" ",L" ");
			++_row;
			_action = BDA_FIRE;

			// if no more fires get fire counter from new weapon
			if(!_fireCount)
			{
				_fireCount = def->getRules()->getFireCount();
			}

			if (_row > 14)
			{
				_lstDefenses->scrollDown(true);
			}
			
			return;
		case BDA_FIRE:

			// CHECK AMMO 
			if( !ammo.empty() )
			{
				if( ! _base->getItems()->getItem( ammo ) )
				{
					_lstDefenses->setCellText(_row, 1, tr("STR_NO_AMMO").c_str());
					_fireCount = 0;
					++_attacks;
					_action = BDA_NEW;
					_timer->setInterval( interval );

					return;
				}
				else
				{
					_lstDefenses->setCellText(_row, 1, tr("STR_FIRING").c_str());
					_game->getResourcePack()->getSound("GEO.CAT", (def)->getRules()->getFireSound())->play();
			
					aa = Text::formatNumber( def->getRules()->getFireCount() - _fireCount + 1) + L"/" + Text::formatNumber( def->getRules()->getFireCount() );
					_lstDefenses->setCellText(_row, 2, aa );

					_timer->setInterval( interval );
					_action = BDA_RESOLVE;
			
					// FIRE IS DONE
					--_fireCount;

					// remove ammo from the inventory
					_base->getItems()->removeItem(ammo, 1);						

					return;

				}
			}
			
			_lstDefenses->setCellText(_row, 1, tr("STR_FIRING").c_str());
			_game->getResourcePack()->getSound("GEO.CAT", (def)->getRules()->getFireSound())->play();
			
			aa = Text::formatNumber( def->getRules()->getFireCount() - _fireCount + 1) + L"/" + Text::formatNumber( def->getRules()->getFireCount() );
			_lstDefenses->setCellText(_row, 2, aa );

			_timer->setInterval( interval );
			_action = BDA_RESOLVE;
			
			// FIRE IS DONE
			--_fireCount;

			return;

		case BDA_RESOLVE:
			
			if (!RNG::percent((def)->getRules()->getHitRatio()))
			{
				_lstDefenses->setCellText(_row, 3, tr("STR_MISSED").c_str());
			}
			else
			{				
				_game->getResourcePack()->getSound("GEO.CAT", (def)->getRules()->getHitSound())->play();
				
				_ufo->setDamage(_ufo->getDamage() + (RNG::generate(power / 2, power * 3 / 2 )));
				int mmm = ( _ufo->getRules()->getMaxDamage() - _ufo->getDamage() ) * 100 / _ufo->getRules()->getMaxDamage();
				_lstDefenses->setCellText(_row, 3, tr("STR_HIT").c_str() );
				_lstDefenses->setCellText(_row, 4, Text::formatPercentage( mmm ) );
			}
						
			if (_ufo->getStatus() == Ufo::DESTROYED)
			{
				++_attacks;
				_action = BDA_DESTROY;
				_timer->setInterval(500);
			}
			else
			{
				// NO MORE FIRES FROM THIS WEAPON THEN GO TO NEXT WEAPON
				if(!_fireCount)
				{
					++_attacks;
					_action = BDA_NEW;
					_timer->setInterval( 500 );
				}
				else
				{
					_action = BDA_FIRE;
					_timer->setInterval( interval );
				}
			}			
			
			return;
		default:
			break;
		}
	}
}
/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void BaseDefenseState::btnOkClick(Action *)
{
	_timer->stop();
	_game->popState();
	if (_ufo->getStatus() != Ufo::DESTROYED)
	{
		_state->handleBaseDefense(_base, _ufo);
	}
	else
	{
		_base->cleanupDefenses(true);
	}
}
}
