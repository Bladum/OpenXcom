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
#include "../Engine/Logger.h"
#include "DogfightState.h"
#include <sstream>
#include "../Engine/Game.h"
#include "../Resource/ResourcePack.h"
#include "../Engine/Palette.h"
#include "../Engine/Screen.h"
#include "../Engine/Language.h"
#include "../Engine/SurfaceSet.h"
#include "../Engine/Surface.h"
#include "../Interface/ImageButton.h"
#include "../Interface/Text.h"
#include "../Engine/Timer.h"
#include "Globe.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Craft.h"
#include "../Ruleset/RuleCraft.h"
#include "../Savegame/CraftWeapon.h"
#include "../Ruleset/RuleCraftWeapon.h"
#include "../Savegame/Ufo.h"
#include "../Ruleset/RuleUfo.h"
#include "../Engine/Music.h"
#include "../Engine/RNG.h"
#include "../Engine/Sound.h"
#include "../Savegame/Base.h"
#include "../Savegame/CraftWeaponProjectile.h"
#include "../Savegame/Country.h"
#include "../Ruleset/RuleCountry.h"
#include "../Savegame/Region.h"
#include "../Ruleset/RuleRegion.h"
#include "../Savegame/AlienMission.h"
#include "../Ruleset/AlienRace.h"
#include "../Savegame/AlienStrategy.h"
#include "../Engine/Options.h"
#include "../Engine/Action.h"
#include <cstdlib>

namespace OpenXcom
{

// UFO blobs graphics ...
const int DogfightState::_ufoBlobs[8][13][13] = 
{
		/*0 STR_VERY_SMALL */ 
	{ 
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 1, 2, 3, 2, 1, 0, 0, 0, 0},
		{0, 0, 0, 0, 1, 3, 4, 3, 1, 0, 0, 0, 0},
		{0, 0, 0, 0, 1, 2, 3, 2, 1, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
	},
		/*1 STR_SMALL */
	{
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 1, 2, 2, 2, 1, 0, 0, 0, 0},
		{0, 0, 0, 1, 2, 3, 4, 3, 2, 1, 0, 0, 0},
		{0, 0, 0, 1, 2, 4, 5, 4, 2, 1, 0, 0, 0},
		{0, 0, 0, 1, 2, 3, 4, 3, 2, 1, 0, 0, 0},
		{0, 0, 0, 0, 1, 2, 2, 2, 1, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
	},
		/*2 STR_MEDIUM_UC */
	{
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0},
		{0, 0, 0, 1, 1, 2, 2, 2, 1, 1, 0, 0, 0},
		{0, 0, 0, 1, 2, 3, 3, 3, 2, 1, 0, 0, 0},
		{0, 0, 1, 2, 3, 4, 5, 4, 3, 2, 1, 0, 0},
		{0, 0, 1, 2, 3, 5, 5, 5, 3, 2, 1, 0, 0},
		{0, 0, 1, 2, 3, 4, 5, 4, 3, 2, 1, 0, 0},
		{0, 0, 0, 1, 2, 3, 3, 3, 2, 1, 0, 0, 0},
		{0, 0, 0, 1, 1, 2, 2, 2, 1, 1, 0, 0, 0},
		{0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
	},
		/*3 STR_LARGE */
	{
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0},
		{0, 0, 0, 1, 1, 2, 2, 2, 1, 1, 0, 0, 0},
		{0, 0, 1, 2, 2, 3, 3, 3, 2, 2, 1, 0, 0},
		{0, 0, 1, 2, 3, 4, 4, 4, 3, 2, 1, 0, 0},
		{0, 1, 2, 3, 4, 5, 5, 5, 4, 3, 2, 1, 0},
		{0, 1, 2, 3, 4, 5, 5, 5, 4, 3, 2, 1, 0},
		{0, 1, 2, 3, 4, 5, 5, 5, 4, 3, 2, 1, 0},
		{0, 0, 1, 2, 3, 4, 4, 4, 3, 2, 1, 0, 0},
		{0, 0, 1, 2, 2, 3, 3, 3, 2, 2, 1, 0, 0},
		{0, 0, 0, 1, 1, 2, 2, 2, 1, 1, 0, 0, 0},
		{0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
	},
		/*4 STR_VERY_LARGE */
	{
		{0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0},
		{0, 0, 0, 1, 1, 2, 2, 2, 1, 1, 0, 0, 0},
		{0, 0, 1, 2, 2, 3, 3, 3, 2, 2, 1, 0, 0},
		{0, 1, 2, 3, 3, 4, 4, 4, 3, 3, 2, 1, 0},
		{0, 1, 2, 3, 4, 5, 5, 5, 4, 3, 2, 1, 0},
		{1, 2, 3, 4, 5, 5, 5, 5, 5, 4, 3, 2, 1},
		{1, 2, 3, 4, 5, 5, 5, 5, 5, 4, 3, 2, 1},
		{1, 2, 3, 4, 5, 5, 5, 5, 5, 4, 3, 2, 1},
		{0, 1, 2, 3, 4, 5, 5, 5, 4, 3, 2, 1, 0},
		{0, 1, 2, 3, 3, 4, 4, 4, 3, 3, 2, 1, 0},
		{0, 0, 1, 2, 2, 3, 3, 3, 2, 2, 1, 0, 0},
		{0, 0, 0, 1, 1, 2, 2, 2, 1, 1, 0, 0, 0},
		{0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0}
	},
		/*5 STR_HUGE */
	{
		{0, 0, 0, 1, 1, 2, 2, 2, 1, 1, 0, 0, 0},
		{0, 0, 1, 2, 2, 3, 3, 3, 2, 2, 1, 0, 0},
		{0, 1, 2, 3, 3, 4, 4, 4, 3, 3, 2, 1, 0},
		{1, 2, 3, 4, 4, 5, 5, 5, 4, 4, 3, 2, 1},
		{1, 2, 3, 4, 5, 5, 5, 5, 5, 4, 3, 2, 1},
		{2, 3, 4, 5, 5, 5, 5, 5, 5, 5, 4, 3, 2},
		{2, 3, 4, 5, 5, 5, 5, 5, 5, 5, 4, 3, 2},
		{2, 3, 4, 5, 5, 5, 5, 5, 5, 5, 4, 3, 2},
		{1, 2, 3, 4, 5, 5, 5, 5, 5, 4, 3, 2, 1},
		{1, 2, 3, 4, 4, 5, 5, 5, 4, 4, 3, 2, 1},
		{0, 1, 2, 3, 3, 4, 4, 4, 3, 3, 2, 1, 0},
		{0, 0, 1, 2, 2, 3, 3, 3, 2, 2, 1, 0, 0},
		{0, 0, 0, 1, 1, 2, 2, 2, 1, 1, 0, 0, 0}
	},
		/*6 STR_VERY_HUGE :p */
	{
		{0, 0, 0, 2, 2, 3, 3, 3, 2, 2, 0, 0, 0},
		{0, 0, 2, 3, 3, 4, 4, 4, 3, 3, 2, 0, 0},
		{0, 2, 3, 4, 4, 5, 5, 5, 4, 4, 3, 2, 0},
		{2, 3, 4, 5, 5, 5, 5, 5, 5, 5, 4, 3, 2},
		{2, 3, 4, 5, 5, 5, 5, 5, 5, 5, 4, 3, 2},
		{3, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 3},
		{3, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 3},
		{3, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 3},
		{2, 3, 4, 5, 5, 5, 5, 5, 5, 5, 4, 3, 2},
		{2, 3, 4, 5, 5, 5, 5, 5, 5, 5, 4, 3, 2},
		{0, 2, 3, 4, 4, 5, 5, 5, 4, 4, 3, 2, 0},
		{0, 0, 2, 3, 3, 4, 4, 4, 3, 3, 2, 0, 0},
		{0, 0, 0, 2, 2, 3, 3, 3, 2, 2, 0, 0, 0}
	},
		/*7 STR_ENOURMOUS */
	{
		{0, 0, 0, 3, 3, 4, 4, 4, 3, 3, 0, 0, 0},
		{0, 0, 3, 4, 4, 5, 5, 5, 4, 4, 3, 0, 0},
		{0, 3, 4, 5, 5, 5, 5, 5, 5, 5, 4, 3, 0},
		{3, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 3},
		{3, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 3},
		{4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4},
		{4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4},
		{4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4},
		{3, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 3},
		{3, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 3},
		{0, 3, 4, 5, 5, 5, 5, 5, 5, 5, 4, 3, 0},
		{0, 0, 3, 4, 4, 5, 5, 5, 4, 4, 3, 0, 0},
		{0, 0, 0, 3, 3, 4, 4, 4, 3, 3, 0, 0, 0}
	}
};

// Projectile blobs
const int DogfightState::_projectileBlobs[5][6][3] = 
{
		/*0 STR_STINGRAY_MISSILE ?*/
	{
		{0, 2, 0},
		{1, 8, 1},

		{0, 4, 0},
		{0, 3, 0},
		{0, 2, 0},
		{0, 1, 0}
	},
		/*1 LARGE MISSILE ?*/
	{

		{1, 4, 1},
		{2, 9, 2},
		{1, 6, 1},
		{0, 4, 0},
		{0, 3, 0},
		{1, 2, 1}
	},
		/*2 STR_CANNON_ROUND ?*/
	{
		{0, 0, 0},
		{0, 6, 0},
		{0, 3, 0},
		{0, 1, 0},
		{0, 0, 0},
		{0, 0, 0}
	},
		/*3 STR_FUSION_BALL ?*/
	{
		{2, 4, 2},
		{4, 9, 4},
		{2, 4, 2},
		{0, 0, 0},
		{0, 0, 0},
		{0, 0, 0}
	},
		/*4 ALLOY ROUND*/
	{
		{0, 0, 0},
		{6, 0, 6},
		{3, 0, 3},
		{1, 0, 1},
		{0, 0, 0},
		{0, 0, 0}
	}
};
/**
 * Initializes all the elements in the Dogfight window.
 * @param game Pointer to the core game.
 * @param globe Pointer to the Geoscape globe.
 * @param craft Pointer to the craft intercepting.
 * @param ufo Pointer to the UFO being intercepted.
 */
DogfightState::DogfightState(Globe *globe, Craft *craft, Ufo *ufo) : 
	_globe(globe), _craft(craft), _ufo(ufo), _timeout(50), _currentDist(640), 

		_targetDist(560), _end(false), _destroyUfo(false), _destroyCraft(false), 
		_ufoBreakingOff(false), 
		_minimized(false), _endDogfight(false), _animatingHit(false), 
		_ufoSize(0), _craftHeight(0), _currentCraftDamageColor(0), 
		_interceptionNumber(0), _interceptionsCount(0), _x(0), _y(0), 
		_minimizedIconX(0), _minimizedIconY(0)
{
	_screen = false;

	_craft->setInDogfight(true);
	_timeScale = 50 + Options::dogfightSpeed;

	_weaponNum = _craft->getRules()->getWeapons();
	if (_weaponNum > RuleCraft::WeaponMax)
		_weaponNum = RuleCraft::WeaponMax;

	for(int i = 0; i < _weaponNum; ++i)
		_weaponEnabled[i] = true;

	// Create objects
	_window = new Surface(160, 96, _x, _y);
	_battle = new Surface(77, 74, _x + 3, _y + 3);
	_damage = new Surface(22, 25, _x + 93, _y + 40);

	for(int i = 0; i < _weaponNum; ++i)
	{
		_weapon[i] = new InteractiveSurface(15, 17, 0, 0);
		_range[i] = new Surface(21, 74, 0, 0);
	}

	
	_btnMinimize = new InteractiveSurface(12, 12, _x, _y);
	_preview = new InteractiveSurface(160, 96, _x, _y);
	_btnStandoff = new ImageButton(36, 15, _x + 83, _y + 4);
	_btnCautious = new ImageButton(36, 15, _x + 120, _y + 4);
	_btnStandard = new ImageButton(36, 15, _x + 83, _y + 20);
	_btnAggressive = new ImageButton(36, 15, _x + 120, _y + 20);
	_btnDisengage = new ImageButton(36, 15, _x + 120, _y + 36);
	_btnUfo = new ImageButton(36, 17, _x + 120, _y + 52);

	for(int i = 0; i < _weaponNum; ++i)
	{
		_txtAmmo[i] = new Text(16, 9, 0, 0);
	}

	_txtDistance = new Text(40, 9, _x + 116, _y + 72);
	_txtStatus = new Text(150, 9, _x + 4, _y + 85);
	_btnMinimizedIcon = new InteractiveSurface(32, 20, _minimizedIconX, _minimizedIconY);
	_txtInterceptionNumber = new Text(16, 9, _minimizedIconX + 18, _minimizedIconY + 6);

	_animTimer = new Timer(Options::dogfightSpeed + 10);
	_moveTimer = new Timer(Options::dogfightSpeed);
	for(int i = 0; i < _weaponNum; ++i)
	{
		_wTimer[i] = new Timer(0);
	}

	_mode = _btnStandoff;
	_ufoWtimer = new Timer(0);
	_ufoEscapeTimer = new Timer(0);
	_craftDamageAnimTimer = new Timer(500);

	moveWindow();

	// Set palette
	setPalette("PAL_GEOSCAPE");

	add(_window);
	add(_battle);
	for(int i = 0; i < _weaponNum; ++i)
	{
		add(_weapon[i]);
		add(_range[i]);
	}
	add(_damage);
	add(_btnMinimize);
	add(_btnStandoff, "button", "dogfight");
	add(_btnCautious, "button", "dogfight");
	add(_btnStandard, "button", "dogfight");
	add(_btnAggressive, "button", "dogfight");
	add(_btnDisengage, "button", "dogfight");
	add(_btnUfo, "button", "dogfight");

	for(int i = 0; i < _weaponNum; ++i)
	{
		  add(_txtAmmo[i], "text", "dogfight");
	}

	add(_txtDistance, "text", "dogfight");
	add(_preview);
	add(_txtStatus, "text", "dogfight");
	add(_btnMinimizedIcon);
	add(_txtInterceptionNumber, "minimizedNumber", "dogfight");

	// Set up objects
	Surface *graphic;
	graphic = _game->getResourcePack()->getSurface("INTERWIN.DAT");
	graphic->setX(0);
	graphic->setY(0);
	graphic->getCrop()->x = 0;
	graphic->getCrop()->y = 0;
	graphic->getCrop()->w = 160;
	graphic->getCrop()->h = 96;
	_window->drawRect(graphic->getCrop(), 15);
	graphic->blit(_window);

	_preview->drawRect(graphic->getCrop(), 15);
	graphic->getCrop()->y = 96;
	graphic->getCrop()->h = 15;
	graphic->blit(_preview);
	graphic->setY(67);
	graphic->getCrop()->y = 111;
	graphic->getCrop()->h = 29;
	graphic->blit(_preview);
	if (ufo->getRules()->getModSprite().empty())
	{
		graphic->setY(15);
		graphic->getCrop()->y = 140 + 52 * _ufo->getRules()->getSprite();
		graphic->getCrop()->h = 52;
	}
	else
	{
		graphic = _game->getResourcePack()->getSurface(ufo->getRules()->getModSprite());
		graphic->setX(0);
		graphic->setY(15);
	}
	graphic->blit(_preview);
	_preview->setVisible(false);
	_preview->onMouseClick((ActionHandler)&DogfightState::previewClick);

	_btnMinimize->onMouseClick((ActionHandler)&DogfightState::btnMinimizeClick);

	_btnStandoff->copy(_window);

	_btnStandoff->setGroup(&_mode);
	_btnStandoff->onMousePress((ActionHandler)&DogfightState::btnStandoffPress);

	_btnCautious->copy(_window);

	_btnCautious->setGroup(&_mode);
	_btnCautious->onMousePress((ActionHandler)&DogfightState::btnCautiousPress);

	_btnStandard->copy(_window);

	_btnStandard->setGroup(&_mode);
	_btnStandard->onMousePress((ActionHandler)&DogfightState::btnStandardPress);

	_btnAggressive->copy(_window);

	_btnAggressive->setGroup(&_mode);
	_btnAggressive->onMousePress((ActionHandler)&DogfightState::btnAggressivePress);

	_btnDisengage->copy(_window);

	_btnDisengage->onMousePress((ActionHandler)&DogfightState::btnDisengagePress);
	_btnDisengage->setGroup(&_mode);

	_btnUfo->copy(_window);

	_btnUfo->onMouseClick((ActionHandler)&DogfightState::btnUfoClick);
	
	_txtDistance->setText(L"640");

	_txtStatus->setText(tr("STR_STANDOFF"));

	SurfaceSet *set = _game->getResourcePack()->getSurfaceSet("INTICON.PCK");

	// Create the minimized dogfight icon.
	Surface *frame = set->getFrame(_craft->getRules()->getSprite());
	frame->setX(0);
	frame->setY(0);
	frame->blit(_btnMinimizedIcon);
	_btnMinimizedIcon->onMouseClick((ActionHandler)&DogfightState::btnMinimizedIconClick);
	_btnMinimizedIcon->setVisible(false);

	// Draw correct number on the minimized dogfight icon.
	std::wostringstream ss1;
	ss1 << _craft->getInterceptionOrder();

	_txtInterceptionNumber->setText(ss1.str());
	_txtInterceptionNumber->setVisible(false);

	for (int i = 0; i < _weaponNum; ++i)
	{
		CraftWeapon *w = _craft->getWeapons()->at(i);
		if (w == 0 || w->getRules()->getAmmoMax() == 0)
 			continue;

		Surface *weapon = _weapon[i], *range = _range[i];
		Text *ammo = _txtAmmo[i];
		int x1, x2;
		int x_off = 2 * (i / 2 + 1);
		if (i % 2 == 0)
		{
			x1 = x_off;
 			x2 = 0;
		}
		else
		{
 			x1 = 0;
			x2 = 20 - x_off;
		}

		// Draw weapon icon
		frame = set->getFrame(w->getRules()->getSprite() + 5);

		frame->setX(0);
		frame->setY(0);
		frame->blit(weapon);

		// Draw ammo
		std::wostringstream ss;
		ss << w->getAmmo();
		ammo->setText(ss.str());

		// Draw range (1 km = 1 pixel)

		Uint8 color = _game->getRuleset()->getInterface("dogfight")->getElement("background")->color;
		range->lock();

		int rangeY = range->getHeight() - w->getRules()->getRange() ;
		int connectY = weapon->getHeight() / 2 + weapon->getY() - range->getY();
		for (int x = x1; x <= x1 + 20 - x_off; x += 2)		for (int x = x1; x <= x1 + 18; x += 2)
		{
			range->setPixel(x, rangeY, color);
		}

		int minY = 0, maxY = 0;
		if (rangeY < connectY)
		{
			minY = rangeY;
			maxY = connectY;
		}
		else if (rangeY > connectY)
		{
			minY = connectY;
			maxY = rangeY;
		}
		for (int y = minY; y <= maxY; ++y)
		{
			range->setPixel(x1 + x2, y, color);
		}
		for (int x = x2; x <= x2 + x_off; ++x)
		{
			range->setPixel(x, connectY, color);
		}
		range->unlock();
	}

	for (int i = 0; i < _weaponNum; ++i)
 	{
		if (_craft->getWeapons()->at(0) == 0)
		{
			_weapon[i]->setVisible(false);
			_range[i]->setVisible(false);
			_txtAmmo[i]->setVisible(false);
		}
 	}

	// Draw damage indicator.
	frame = set->getFrame(_craft->getRules()->getSprite() + 11);
	frame->setX(0);
	frame->setY(0);
	frame->blit(_damage);

	_animTimer->onTimer((StateHandler)&DogfightState::animate);
	_animTimer->start();

	_moveTimer->onTimer((StateHandler)&DogfightState::move);
	_moveTimer->start();

	StateHandler fireCallback[RuleCraft::WeaponMax] =
	{
		(StateHandler)&DogfightState::fireWeapon1,
		(StateHandler)&DogfightState::fireWeapon2,
		(StateHandler)&DogfightState::fireWeapon3,
		(StateHandler)&DogfightState::fireWeapon4,
	};
	for (int i = 0; i < _weaponNum; ++i)
	{
		_wTimer[i]->onTimer(fireCallback[i]);
	}

	_ufoWtimer->onTimer((StateHandler)&DogfightState::ufoFireWeapon);
	_ufoFireInterval = (_ufo->getRules()->getWeaponReload() - (int)(_game->getSavedGame()->getDifficulty()));
	_ufoFireInterval = (RNG::generate(0, _ufoFireInterval) + _ufoFireInterval) * _timeScale;
	_ufoWtimer->setInterval(_ufoFireInterval);

	_ufoEscapeTimer->onTimer((StateHandler)&DogfightState::ufoBreakOff);
	int ufoBreakOffInterval = (_ufo->getRules()->getBreakOffTime() + RNG::generate(0, _ufo->getRules()->getBreakOffTime()) - 15 * (int)(_game->getSavedGame()->getDifficulty())) * _timeScale;
	_ufoEscapeTimer->setInterval(ufoBreakOffInterval);

	_craftDamageAnimTimer->onTimer((StateHandler)&DogfightState::animateCraftDamage);

	// Set UFO size - going to be moved to Ufo class to implement simultanous dogfights.
	std::string ufoSize = _ufo->getRules()->getSize();
	if (ufoSize.compare("STR_VERY_SMALL") == 0)
	{
		_ufoSize = 0;
	}
	else if (ufoSize.compare("STR_SMALL") == 0)
	{
		_ufoSize = 1;
	}
	else if (ufoSize.compare("STR_MEDIUM_UC") == 0)
	{
		_ufoSize = 2;
	}
	else if (ufoSize.compare("STR_LARGE") == 0)
	{
		_ufoSize = 3;
	}
	else
	{
		_ufoSize = 4;
	}

	_color[0] = _game->getRuleset()->getInterface("dogfight")->getElement("craftRange")->color;
	_color[1] = _game->getRuleset()->getInterface("dogfight")->getElement("craftRange")->color2;
	_color[2] = _game->getRuleset()->getInterface("dogfight")->getElement("radarRange")->color;
	_color[3] = _game->getRuleset()->getInterface("dogfight")->getElement("radarRange")->color2;
	_color[4] = _game->getRuleset()->getInterface("dogfight")->getElement("damageRange")->color;
	_color[5] = _game->getRuleset()->getInterface("dogfight")->getElement("damageRange")->color2;

	// Get crafts height. Used for damage indication.
	int x =_damage->getWidth() / 2;
	for (int y = 0; y < _damage->getHeight(); ++y)
	{
		Uint8 pixelColor = _damage->getPixel(x, y);

		if (pixelColor >= _color[0] && pixelColor < _color[1])
		{
			++_craftHeight;
		}
	}

	drawCraftDamage();

	// Used for weapon toggling.

	for (int i = 0; i < _weaponNum; ++i)
	{
		_weapon[i]->onMouseClick((ActionHandler)&DogfightState::weaponClick);
	}
}

/**
 * Deletes timers.
 */
DogfightState::~DogfightState()
{
	delete _animTimer;
	delete _moveTimer;
	for(int i = 0; i < _weaponNum; ++i)
		delete _wTimer[i];
	delete _ufoWtimer;
	delete _ufoEscapeTimer;
	delete _craftDamageAnimTimer;
	while (!_projectiles.empty())
	{
		delete _projectiles.back();
		_projectiles.pop_back();
	}
	if (_craft)
		_craft->setInDogfight(false);
}

/**
 * Runs the dogfighter timers.
 */
void DogfightState::think()
{
	if (!_endDogfight)
	{
		_moveTimer->think(this, 0);
		if (!_endDogfight && !_minimized) // check _endDogfight again, because moveTimer can change it
		{
			_animTimer->think(this, 0);
			for(int i = 0; i < _weaponNum; ++i)
				_wTimer[i]->think(this, 0);
			_ufoWtimer->think(this, 0);
			_ufoEscapeTimer->think(this, 0);
			_craftDamageAnimTimer->think(this, 0);
		}
		else if (!_endDogfight && (_craft->getDestination() != _ufo || _ufo->getStatus() == Ufo::LANDED))
		{
			endDogfight();
		}
	}
}

/**
 * Animates interceptor damage by changing the color and redrawing the image.
 */
void DogfightState::animateCraftDamage()
{
	if (_minimized)
	{
		return;
	}
	--_currentCraftDamageColor;
	if (_currentCraftDamageColor < _color[4])
	{
		_currentCraftDamageColor = _color[5];
	}
	drawCraftDamage();
}

/**
 * Draws interceptor damage according to percentage of HP's left.
 */
void DogfightState::drawCraftDamage()
{
	if (_minimized)
	{
		return;
	}
	if (_craft->getDamagePercentage() != 0)
	{
		if (!_craftDamageAnimTimer->isRunning())
		{
			_craftDamageAnimTimer->start();
			if (_currentCraftDamageColor < _color[4])
			{
				_currentCraftDamageColor = _color[4];
			}
		}
		int damagePercentage = _craft->getDamagePercentage();
		int rowsToColor = (int)floor((double)_craftHeight * (double)(damagePercentage / 100.));
		if (rowsToColor == 0)
		{
			return;
		}
		int rowsColored = 0;
		bool rowColored = false;
		for (int y = 0; y < _damage->getHeight(); ++y)
		{
			rowColored = false;
			for (int x = 0; x < _damage->getWidth(); ++x)
			{
				int pixelColor = _damage->getPixel(x, y);
				if (pixelColor >= _color[4] && pixelColor <= _color[5])
				{
					_damage->setPixel(x, y, _currentCraftDamageColor);
					rowColored = true;
				}

				if (pixelColor >= _color[0] && pixelColor < _color[1])
				{
					_damage->setPixel(x, y, _currentCraftDamageColor);
					rowColored = true;
				}
			}
			if (rowColored)
			{
				++rowsColored;
			}
			if (rowsColored == rowsToColor)
			{
				break;
			}
		}
	}
}

/**
 * Animates the window with a palette effect.
 */
void DogfightState::animate()
{
	if (_minimized)
	{
		return;
	}
	// Animate radar waves and other stuff.
	for (int x = 0; x < _window->getWidth(); ++x)
	{
		for (int y = 0; y < _window->getHeight(); ++y)
		{
			Uint8 radarPixelColor = _window->getPixel(x, y);
			if (radarPixelColor >= _color[2] && radarPixelColor < _color[3])
			{
				++radarPixelColor;
				if (radarPixelColor >= _color[3])
				{
					radarPixelColor = _color[2];
				}
				_window->setPixel(x, y, radarPixelColor);
			}
		}
	}

	_battle->clear();

	// Draw UFO.
	if (!_ufo->isDestroyed())
	{
		drawUfo();
	}

	// Draw projectiles.
	for (std::vector<CraftWeaponProjectile*>::iterator it = _projectiles.begin(); it != _projectiles.end(); ++it)
	{
		drawProjectile((*it));
	}

	
	// Clears text after a while
	if (_timeout == 0)
	{
		_txtStatus->setText(L"");
	}
	else
	{
		_timeout--;
	}

	// Animate UFO hit.
	bool lastHitAnimFrame = false;
	if (_animatingHit && _ufo->getHitFrame() > 0)
	{
		_ufo->setHitFrame(_ufo->getHitFrame() - 1);
		if (_ufo->getHitFrame() == 0)
		{
			_animatingHit = false;
			lastHitAnimFrame = true;
		}
	}

	// Animate UFO crash landing.
	if (_ufo->isCrashed() && _ufo->getHitFrame() == 0 && !lastHitAnimFrame)
	{
		--_ufoSize;
	}
}

/**
 * Moves the craft towards the UFO according to
 * the current interception mode. Handles projectile movements as well.
 */
void DogfightState::move()
{
	bool finalRun = false;
	// Check if craft is not low on fuel when window minimized, and
	// Check if crafts destination hasn't been changed when window minimized.
	Ufo* u = dynamic_cast<Ufo*>(_craft->getDestination());
	if (u != _ufo || _craft->getLowFuel() || (_minimized && _ufo->isCrashed()))
	{
		endDogfight();
		return;
	}
	
	// acceleration difference makes UFO flys slower / faster
	int accelDif = _craft->getCraftStats().accel - _ufo->getRules()->getAcceleration();
	int dogfightSpeed = 4;
	if(accelDif > 2) accelDif = 5;
	if(accelDif > 5) accelDif = 6;
	if(accelDif < -2) accelDif = 3;
	if(accelDif < -5) accelDif = 2;

	if (_minimized && _ufo->getSpeed() > _craft->getSpeed())
	{
		_craft->setSpeed( _craft->getCraftStats().speedMax );
		if (_ufo->getSpeed() > _craft->getSpeed())
		{
			_ufoBreakingOff = true;
		}
	}
	// Check if UFO is not breaking off.
	if (_ufo->getSpeed() == _ufo->getRules()->getMaxSpeed() )
	{ 
		_craft->setSpeed( _craft->getCraftStats().speedMax );
		// Crappy craft is chasing UFO.
		if (_ufo->getSpeed() > _craft->getSpeed())
		{
			_ufoBreakingOff = true;
			finalRun = true;
			setStatus("STR_UFO_OUTRUNNING_INTERCEPTOR");
		}
		else //ufo cannot break off, because it's too slow
		{
			_ufoBreakingOff = false;
		}

	}
	bool projectileInFlight = false;
	if (!_minimized)
	{
		int distanceChange = 0;

		// Update distance
		if (!_ufoBreakingOff)
		{
			if (_currentDist < _targetDist && !_ufo->isCrashed() && !_craft->isDestroyed())
			{
				distanceChange = dogfightSpeed;
				if (_currentDist + distanceChange >_targetDist)
				{
					distanceChange = _targetDist - _currentDist;
				}
			}
			else if (_currentDist > _targetDist && !_ufo->isCrashed() && !_craft->isDestroyed())
			{
				distanceChange = -dogfightSpeed/2;
			}

			// don't let the interceptor mystically push or pull its fired projectiles
			for (std::vector<CraftWeaponProjectile*>::iterator it = _projectiles.begin(); it != _projectiles.end(); ++it)
			{
				if ((*it)->getGlobalType() != CWPGT_BEAM && (*it)->getDirection() == D_UP) (*it)->setPosition((*it)->getPosition() + distanceChange);
			}
		}
		else
		{
			distanceChange = dogfightSpeed;

			// UFOs can try to outrun our missiles, don't adjust projectile positions here
			// If UFOs ever fire anything but beams, those positions need to be adjust here though.
		}

		_currentDist += distanceChange; 

		std::wostringstream ss;
		ss << _currentDist;
		_txtDistance->setText(ss.str());

		// Move projectiles and check for hits.
		for (std::vector<CraftWeaponProjectile*>::iterator it = _projectiles.begin(); it != _projectiles.end(); ++it)
		{
			CraftWeaponProjectile *p = (*it);
			p->move();
			// Projectiles fired by interceptor.
			if (p->getDirection() == D_UP)
			{
				// Projectile reached the UFO - determine if it's been hit.
				if (((p->getPosition() >= _currentDist) || (p->getGlobalType() == CWPGT_BEAM && p->toBeRemoved())) && !_ufo->isCrashed() && !p->getMissed())
				{
					// UFO hit by x-com
					// WEAPON CHANCE * UFO CHANCE TO AVOID * CRAFT CHANCE TO HIT
					int chanceToHitUfo = p->getAccuracy() * ( 100 - _ufo->getRules()->getAvoidChance() + _craft->getCraftStats().accuracy ) / 100;
					Log(LOG_DEBUG) << "Chance to hit UFO " << chanceToHitUfo << ", " << _ufo->getRules()->getAvoidChance() << ", " << _craft->getCraftStats().accuracy;
					
					if (RNG::percent( chanceToHitUfo ) )
					{
						// WEAPON DAMAGE * CRAFT DAMAGE MULTIPLIER
						int power = p->getDamage() * ( 100 + _craft->getCraftStats().damage ) / 100 ;									
						int damage = RNG::generate(power / 2, power * 3 / 2);

						Log(LOG_DEBUG) << "Ufo hit on damage " << damage << ", " << power;
						_ufo->setDamage(_ufo->getDamage() + damage);

						if (_ufo->isCrashed())
						{
							_ufo->setShotDownByCraftId(_craft->getUniqueId());
							_ufoBreakingOff = false;
							_ufo->setSpeed(0);
						}
						if (_ufo->getHitFrame() == 0)
						{
							_animatingHit = true;
							_ufo->setHitFrame(3);
						}

						setStatus("STR_UFO_HIT");
						_game->getResourcePack()->getSound("GEO.CAT", ResourcePack::UFO_HIT)->play();
						p->remove();
					}
					// Missed.
					else
					{
						if (p->getGlobalType() == CWPGT_BEAM)
						{
							p->remove();
						}
						else
						{
							p->setMissed(true);
						}
					}
				}
				// Check if projectile passed it's maximum range.
				if (p->getGlobalType() == CWPGT_MISSILE)
				{
					if (p->getPosition() / 8 >= p->getRange() )
					{
						p->remove();
					}
					else if (!_ufo->isCrashed())
					{
						projectileInFlight = true;
					}
				}
			}
			// Projectiles fired by UFO.
			else if (p->getDirection() == D_DOWN)
			{
				if (p->getGlobalType() == CWPGT_MISSILE || (p->getGlobalType() == CWPGT_BEAM && p->toBeRemoved()))
				{
					// CHANCE TO HIT INTERCEPTOR by UFO
					// INTERCEPTOR AVOID BONUS
					int chanceToHitCraft = p->getAccuracy() - _craft->getCraftStats().avoidBonus;
					Log(LOG_DEBUG) << "Craft will be hit by UFO with chance " << chanceToHitCraft;
					if (RNG::percent( chanceToHitCraft ) )
					{
						// UFO POWER 50-150%
						// CRAFT ARMOUR FROM BONUS
						int power = p->getDamage();
						Log(LOG_DEBUG) << "Craft hit by UFO before armor " << power;
						power = power * ( 100 - _craft->getCraftStats().armor ) / 100;						
						Log(LOG_DEBUG) << "Craft hit by UFO after armor " << power;
						int damage = RNG::generate(power / 2, power * 3 / 2);		
						Log(LOG_DEBUG) << "Craft hit by UFO after armor + randomized " << damage;
						if (damage)
						{
							_craft->setDamage( _craft->getDamage() + damage);
							drawCraftDamage();
							setStatus("STR_INTERCEPTOR_DAMAGED");
							_game->getResourcePack()->getSound("GEO.CAT", ResourcePack::INTERCEPTOR_HIT)->play(); //10
							if (_mode == _btnCautious && _craft->getDamagePercentage() >= 50)
							{
								_targetDist = STANDOFF_DIST;
							}
						}
					}
					p->remove();
				}
			}
		}

		
		// Remove projectiles that hit or missed their target.
		for (std::vector<CraftWeaponProjectile*>::iterator it = _projectiles.begin(); it != _projectiles.end();)
		{
			if ((*it)->toBeRemoved() == true || ((*it)->getMissed() == true && (*it)->getPosition() <= 0))
			{
				delete *it;
				it = _projectiles.erase(it);
			}
			else
			{
				++it;
			}
		}

		// Handle weapons and craft distance.
		for (int i = 0; i < _weaponNum; ++i)
		{
			CraftWeapon *w = _craft->getWeapons()->at(i);
			if (w == 0)
			{
				continue;
			}
			Timer *wTimer = _wTimer[i];

			// Handle weapon firing
			int weaponRange =  w->getRules()->getRange() * 8;
			if (!wTimer->isRunning() && _currentDist <= weaponRange && w->getAmmo() > 0 && _mode != _btnStandoff
				&& _mode != _btnDisengage && !_ufo->isCrashed() && !_craft->isDestroyed())
			{
				wTimer->start();
				fireWeapon(i);
			}
			else if (wTimer->isRunning() && (_currentDist > weaponRange || (w->getAmmo() == 0 && !projectileInFlight) || _mode == _btnStandoff
				|| _mode == _btnDisengage || _ufo->isCrashed() || _craft->isDestroyed()))
			{
				wTimer->stop();
				// Handle craft distance according to option set by user and available ammo.
				if (w->getAmmo() == 0 && !_craft->isDestroyed())
				{
					if (_mode == _btnCautious)
					{
						minimumDistance();
					}
					else if (_mode == _btnStandard)
					{
						maximumDistance();
					}
				}
			}
		}

		// Handle UFO firing.
		if (!_ufoWtimer->isRunning() && _currentDist <= _ufo->getRules()->getWeaponRange() * 8 && !_ufo->isCrashed() && !_craft->isDestroyed())
		{
			if (_ufo->getShootingAt() == 0)
			{
				_ufo->setShootingAt(_interceptionNumber);
				_ufoWtimer->start();
				ufoFireWeapon();
			}
		}
		else if (_ufoWtimer->isRunning() && (_currentDist > _ufo->getRules()->getWeaponRange() * 8 || _ufo->isCrashed() || _craft->isDestroyed()))
		{
			_ufo->setShootingAt(0);
			_ufoWtimer->stop();
		}
	}

	// Check when battle is over.
	if (_end == true && (((_currentDist > 640 || _minimized) && (_mode == _btnDisengage || _ufoBreakingOff == true)) || (_timeout == 0 && (_ufo->isCrashed() || _craft->isDestroyed()))))
	{
		if (_ufoBreakingOff)
		{
			_ufo->move();
			_craft->setDestination(_ufo);
		}
		if (!_destroyCraft && (_destroyUfo || _mode == _btnDisengage))
		{
			_craft->returnToBase();
		}
		endDogfight();
	}

	if (_currentDist > 640 && _ufoBreakingOff)
	{
		finalRun = true;
	}

	// End dogfight if craft is destroyed.
	if (!_end && _craft->isDestroyed())
	{
		setStatus("STR_INTERCEPTOR_DESTROYED");
		_timeout += 30;
		_game->getResourcePack()->getSound("GEO.CAT", ResourcePack::INTERCEPTOR_EXPLODE)->play();
		finalRun = true;
		_destroyCraft = true;
		_ufo->setShootingAt(0);
		_ufoWtimer->stop();
		for(int i = 0; i < _weaponNum; ++i)
			_wTimer[i]->stop();
	}

	
	// End dogfight if UFO is crashed or destroyed.
	if (!_end && _ufo->isCrashed())
	{
		AlienMission *mission = _ufo->getMission();
		mission->ufoShotDown(*_ufo, *_game, *_globe);
		std::string _raceString = _ufo->getAlienRace();
		AlienRace *_race = _game->getRuleset()->getAlienRace( _raceString );
		int chanceToRetaliation = _race->canRetaliate();

		// Check for retaliation trigger.
		if ( RNG::percent( chanceToRetaliation + 2 * _game->getSavedGame()->getDifficulty() ))
		{
			// Spawn retaliation mission.
			std::string targetRegion;
			if (RNG::percent(50 - 6 * (int)(_game->getSavedGame()->getDifficulty())))
			{
				// Attack on UFO's mission region
				targetRegion = _ufo->getMission()->getRegion();
			}
			else
			{
				// Try to find and attack the originating base.
				targetRegion = _game->getSavedGame()->locateRegion(*_craft->getBase())->getRules()->getType();
				// TODO: If the base is removed, the mission is canceled.
			}
			// Difference from original: No retaliation until final UFO lands (Original: Is spawned).
			if (!_game->getSavedGame()->getAlienMission(targetRegion, "STR_ALIEN_RETALIATION"))
			{
				const RuleAlienMission &rule = *_game->getRuleset()->getAlienMission("STR_ALIEN_RETALIATION");
				AlienMission *mission = new AlienMission(rule);
				mission->setId(_game->getSavedGame()->getId("ALIEN_MISSIONS"));
				mission->setRegion(targetRegion, *_game->getRuleset());
				mission->setRace(_ufo->getAlienRace());
				mission->start();
				_game->getSavedGame()->getAlienMissions().push_back(mission);
			}
		}
		_ufoEscapeTimer->stop();
		if (_ufo->isDestroyed())
		{
			if (_ufo->getShotDownByCraftId() == _craft->getUniqueId())
			{
				for (std::vector<Country*>::iterator country = _game->getSavedGame()->getCountries()->begin(); country != _game->getSavedGame()->getCountries()->end(); ++country)
				{
					if ((*country)->getRules()->insideCountry(_ufo->getLongitude(), _ufo->getLatitude()))
					{
						(*country)->addActivityXcom(_ufo->getRules()->getScore()*2);
						break;
					}
				}
				for (std::vector<Region*>::iterator region = _game->getSavedGame()->getRegions()->begin(); region != _game->getSavedGame()->getRegions()->end(); ++region)
				{
					if ((*region)->getRules()->insideRegion(_ufo->getLongitude(), _ufo->getLatitude()))
					{
						(*region)->addActivityXcom(_ufo->getRules()->getScore()*2);
						break;
					}
				}
				setStatus("STR_UFO_DESTROYED");
				_game->getResourcePack()->getSound("GEO.CAT", ResourcePack::UFO_EXPLODE)->play(); //11
			}
			_destroyUfo = true;
		}
		else
		{
			if (_ufo->getShotDownByCraftId() == _craft->getUniqueId())
			{
				setStatus("STR_UFO_CRASH_LANDS");
				_game->getResourcePack()->getSound("GEO.CAT", ResourcePack::UFO_CRASH)->play(); //10
				for (std::vector<Country*>::iterator country = _game->getSavedGame()->getCountries()->begin(); country != _game->getSavedGame()->getCountries()->end(); ++country)
				{
					if ((*country)->getRules()->insideCountry(_ufo->getLongitude(), _ufo->getLatitude()))
					{
						(*country)->addActivityXcom(_ufo->getRules()->getScore());
						break;
					}
				}
				for (std::vector<Region*>::iterator region = _game->getSavedGame()->getRegions()->begin(); region != _game->getSavedGame()->getRegions()->end(); ++region)
				{
					if ((*region)->getRules()->insideRegion(_ufo->getLongitude(), _ufo->getLatitude()))
					{
						(*region)->addActivityXcom(_ufo->getRules()->getScore());
						break;
					}
				}
			}
			if (!_globe->insideLand(_ufo->getLongitude(), _ufo->getLatitude()))
			{
				_ufo->setStatus(Ufo::DESTROYED);
				_destroyUfo = true;
			}
			else
			{
				// CRASH SITE IS UFO BASED NOT RANDOM
				int crashTime =_ufo->getRules()->getCrashSiteTime();
				crashTime = RNG::generate(crashTime / 2, crashTime * 3 / 2);
				_ufo->setSecondsRemaining( crashTime * 3600);
				_ufo->setAltitude("STR_GROUND");
				if (_ufo->getCrashId() == 0)
				{
					_ufo->setCrashId(_game->getSavedGame()->getId("STR_CRASH_SITE"));
				}
			}
		}
		_timeout += 30;
		if (_ufo->getShotDownByCraftId() != _craft->getUniqueId())
		{
			_timeout += 50;
			_ufo->setHitFrame(3);
		}
		finalRun = true;
	}

	if (!_end && _ufo->getStatus() == Ufo::LANDED)
	{
		_timeout += 30;
		finalRun = true;
		_ufo->setShootingAt(0);
		_ufoWtimer->stop();
		for(int i = 0; i < _weaponNum; ++i)
			_wTimer[i]->stop();
	}

	if (!projectileInFlight && finalRun)
	{
		_end = true;
	}
}

/**
 * Fires a shot from the first weapon
 * equipped on the craft.
 */
void DogfightState::fireWeapon(int i)
{
	if (_weaponEnabled[i])
	{
		CraftWeapon *w1 = _craft->getWeapons()->at(i);
		if (w1->setAmmo(w1->getAmmo() - 1))
		{

			std::wostringstream ss;
			ss << w1->getAmmo();
			_txtAmmo[i]->setText(ss.str());

			CraftWeaponProjectile *p = w1->fire();
			p->setDirection(D_UP);
			p->setHorizontalPosition((i % 2 ? HP_RIGHT : HP_LEFT) * (1 + 2 * (i / 2)));
			_projectiles.push_back(p);

			_game->getResourcePack()->getSound("GEO.CAT", w1->getRules()->getSound())->play();
		}
	}
}

/**
 * Fires a shot from the first weapon
 * equipped on the craft.
 */
void DogfightState::fireWeapon1()
{
	fireWeapon(0);
}
/**

/**
 * Fires a shot from the second weapon
 * equipped on the craft.
 */
void DogfightState::fireWeapon2()
{
	fireWeapon(1);
}
/**
 * Fires a shot from the third weapon
 * equipped on the craft.
 */
void DogfightState::fireWeapon3()
{
	fireWeapon(2);
}
/**
 * Fires a shot from the fourth weapon
 * equipped on the craft.
 */
void DogfightState::fireWeapon4()
{
	fireWeapon(3);
}

/**
 *	Each time a UFO will try to fire it's cannons
 *	a calculation is made. There's only 10% chance
 *	that it will actually fire.
 */
void DogfightState::ufoFireWeapon()
{
	_ufoFireInterval = (_ufo->getRules()->getWeaponReload() - (int)(_game->getSavedGame()->getDifficulty()));
	_ufoFireInterval = (RNG::generate(0, _ufoFireInterval) + _ufoFireInterval) * _timeScale;
	_ufoWtimer->setInterval(_ufoFireInterval);

	setStatus("STR_UFO_RETURN_FIRE");
	CraftWeaponProjectile *p = new CraftWeaponProjectile();
	p->setType(CWPT_PLASMA_BEAM);
	p->setAccuracy( _ufo->getRules()->getWeaponAccuracy() );
	p->setDamage(_ufo->getRules()->getWeaponPower() );
	p->setDirection(D_DOWN);
	p->setHorizontalPosition(HP_CENTER);
	p->setPosition(_currentDist - (_ufo->getRules()->getRadius() / 2));
	_projectiles.push_back(p);
	_game->getResourcePack()->getSound("GEO.CAT", ResourcePack::UFO_FIRE)->play();
}

/**
 * Sets the craft to the minimum distance
 * required to fire a weapon.
 */
void DogfightState::minimumDistance()
{
	int max = 0;
	for (std::vector<CraftWeapon*>::iterator i = _craft->getWeapons()->begin(); i < _craft->getWeapons()->end(); ++i)
	{
		if (*i == 0)
			continue;
		if ((*i)->getRules()->getRange() > max && (*i)->getAmmo() > 0)
		{
			max = (*i)->getRules()->getRange() ;
		}
	}
	if (max == 0)
	{
		_targetDist = STANDOFF_DIST;
	}
	else
	{
		_targetDist = max * 8;
	}
}

/**
 * Sets the craft to the maximum distance
 * required to fire a weapon.
 */
void DogfightState::maximumDistance()
{
	int min = 1000;
	for (std::vector<CraftWeapon*>::iterator i = _craft->getWeapons()->begin(); i < _craft->getWeapons()->end(); ++i)
	{
		if (*i == 0)
			continue;
		if ((*i)->getRules()->getRange() < min && (*i)->getAmmo() > 0)
		{
			min = (*i)->getRules()->getRange() ;
		}
	}
	if (min == 1000)
	{
		_targetDist = STANDOFF_DIST;
	}
	else
	{
		_targetDist = min * 8;
	}
}

/**
 * Updates the status text and restarts
 * the text timeout counter.
 * @param status New status text.
 */
void DogfightState::setStatus(const std::string &status)
{
	_txtStatus->setText(tr(status));
	_timeout = 50;
}

/**
 * Minimizes the dogfight window.
 * @param action Pointer to an action.
 */
void DogfightState::btnMinimizeClick(Action *)
{
	if (!_ufo->isCrashed() && !_craft->isDestroyed() && !_ufoBreakingOff)
	{
		if (_currentDist >= STANDOFF_DIST)
		{
			setMinimized(true);
			_window->setVisible(false);
			_preview->setVisible(false);
			_btnStandoff->setVisible(false);
			_btnCautious->setVisible(false);
			_btnStandard->setVisible(false);
			_btnAggressive->setVisible(false);
			_btnDisengage->setVisible(false);
			_btnUfo->setVisible(false);
			_btnMinimize->setVisible(false);
			_battle->setVisible(false);
			for (int i = 0; i < _weaponNum; ++i)
			{
				_weapon[i]->setVisible(false);
				_range[i]->setVisible(false);
				_txtAmmo[i]->setVisible(false);
			}
			_damage->setVisible(false);
			_txtDistance->setVisible(false);
			_preview->setVisible(false);
			_txtStatus->setVisible(false);
			_btnMinimizedIcon->setVisible(true);
			_txtInterceptionNumber->setVisible(true);
			_ufoEscapeTimer->stop();
		}
		else
		{
			setStatus("STR_MINIMISE_AT_STANDOFF_RANGE_ONLY");
		}
	}
}

/**
 * Switches to Standoff mode (maximum range).
 * @param action Pointer to an action.
 */
void DogfightState::btnStandoffPress(Action *)
{
	if (!_ufo->isCrashed() && !_craft->isDestroyed() && !_ufoBreakingOff)
	{
		_end = false;
		setStatus("STR_STANDOFF");
		_targetDist = STANDOFF_DIST;
	}
}

/**
 * Switches to Cautious mode (maximum weapon range).
 * @param action Pointer to an action.
 */
void DogfightState::btnCautiousPress(Action *)
{
	if (!_ufo->isCrashed() && !_craft->isDestroyed() && !_ufoBreakingOff)
	{
		_end = false;
		setStatus("STR_CAUTIOUS_ATTACK");
		for (int i = 0; i < _weaponNum; ++i)

 		{
			CraftWeapon* w = _craft->getWeapons()->at(i);
			if (w != 0)
			{
				float timeModifier = (100 + _craft->getCraftStats().reload ) / 100.0f;
				Log(LOG_DEBUG) << "reload time " << timeModifier << "," << w->getRules()->getCautiousReload() * _timeScale * timeModifier;
				_wTimer[i]->setInterval(w->getRules()->getCautiousReload() * _timeScale * timeModifier);
			}

 		}
		minimumDistance();
		_ufoEscapeTimer->start();
	}
}

/**
 * Switches to Standard mode (minimum weapon range).
 * @param action Pointer to an action.
 */
void DogfightState::btnStandardPress(Action *)
{
	if (!_ufo->isCrashed() && !_craft->isDestroyed() && !_ufoBreakingOff)
	{
		_end = false;
		setStatus("STR_STANDARD_ATTACK");
		for (int i = 0; i < _weaponNum; ++i)

 		{
			CraftWeapon* w = _craft->getWeapons()->at(i);
			if (w != 0)
			{
				float timeModifier = (100 + _craft->getCraftStats().reload ) / 100.0f;
				Log(LOG_DEBUG) << "reload time " << timeModifier << "," << w->getRules()->getStandardReload() * _timeScale * timeModifier;
				_wTimer[i]->setInterval(w->getRules()->getStandardReload() * _timeScale * timeModifier);
			}

 		}
		maximumDistance();
		_ufoEscapeTimer->start();
	}
}

/**
 * Switches to Aggressive mode (minimum range).
 * @param action Pointer to an action.
 */
void DogfightState::btnAggressivePress(Action *)
{
	if (!_ufo->isCrashed() && !_craft->isDestroyed() && !_ufoBreakingOff)
	{
		_end = false;
		setStatus("STR_AGGRESSIVE_ATTACK");
		for (int i = 0; i < _weaponNum; ++i)

 		{
			CraftWeapon* w = _craft->getWeapons()->at(i);
			if (w != 0)
			{
				float timeModifier = (100 + _craft->getCraftStats().reload ) / 100.0f;
				Log(LOG_DEBUG) << "reload time " << timeModifier << "," << w->getRules()->getAggressiveReload() * _timeScale * timeModifier;
				_wTimer[i]->setInterval(w->getRules()->getAggressiveReload() * _timeScale * timeModifier);
			}

 		}
		_targetDist = 64;
		_ufoEscapeTimer->start();
	}
}

/**
 * Disengages from the UFO.
 * @param action Pointer to an action.
 */
void DogfightState::btnDisengagePress(Action *)
{
	if (!_ufo->isCrashed() && !_craft->isDestroyed() && !_ufoBreakingOff)
	{
		_end = true;
		setStatus("STR_DISENGAGING");
		_targetDist = 800;
		_ufoEscapeTimer->stop();
	}
}

/**
 * Shows a front view of the UFO.
 * @param action Pointer to an action.
 */
void DogfightState::btnUfoClick(Action *)
{
	_preview->setVisible(true);
	// Disable all other buttons to prevent misclicks
	_btnStandoff->setVisible(false);
	_btnCautious->setVisible(false);
	_btnStandard->setVisible(false);
	_btnAggressive->setVisible(false);
	_btnDisengage->setVisible(false);
	_btnUfo->setVisible(false);
	_btnMinimize->setVisible(false);
	for (int i = 0; i < _weaponNum; ++i)
	{
		_weapon[i]->setVisible(false);
	}
}

/**
 * Hides the front view of the UFO.
 * @param action Pointer to an action.
 */
void DogfightState::previewClick(Action *)
{
	_preview->setVisible(false);
	// Reenable all other buttons to prevent misclicks
	_btnStandoff->setVisible(true);
	_btnCautious->setVisible(true);
	_btnStandard->setVisible(true);
	_btnAggressive->setVisible(true);
	_btnDisengage->setVisible(true);
	_btnUfo->setVisible(true);
	_btnMinimize->setVisible(true);
	for (int i = 0; i < _weaponNum; ++i)
	{
		_weapon[i]->setVisible(true);
	}
}

/*
 * Sets UFO to break off mode. Started via timer.
 */
void DogfightState::ufoBreakOff()
{
	if (!_ufo->isCrashed() && !_ufo->isDestroyed() && !_craft->isDestroyed())
	{
		_ufo->setSpeed(_ufo->getRules()->getMaxSpeed() );
		_ufoBreakingOff = true;
	}
}

/*
 * Draws the UFO blob on the radar screen.
 * Currently works only for original sized blobs
 * 13 x 13 pixels.
 */
void DogfightState::drawUfo()
{
	if (_ufoSize < 0 || _ufo->isDestroyed() || _minimized)
	{
		return;
	}
	int currentUfoXposition = _battle->getWidth() / 2 - 6;
	int currentUfoYposition = _battle->getHeight() - (_currentDist / 8) - 6;
	for (int y = 0; y < 13; ++y)
	{
		for (int x = 0; x < 13; ++x)
		{
			Uint8 pixelOffset = _ufoBlobs[_ufoSize + _ufo->getHitFrame()][y][x];
			if (pixelOffset == 0)
			{
				continue;
			}
			else
			{
				if (_ufo->isCrashed() || _ufo->getHitFrame() > 0)
				{
					pixelOffset *= 2;
				}
				Uint8 radarPixelColor = _window->getPixel(currentUfoXposition + x + 3, currentUfoYposition + y + 3); // + 3 cause of the window frame
				Uint8 color = radarPixelColor - pixelOffset;
				if (color < 108)
				{
					color = 108;
				}
				_battle->setPixel(currentUfoXposition + x, currentUfoYposition + y, color);
			}
		}
	}
}

/*
 * Draws projectiles on the radar screen.
 * Depending on what type of projectile it is, it's
 * shape will be different. Currently works for 
 * original sized blobs 3 x 6 pixels.
 */
void DogfightState::drawProjectile(const CraftWeaponProjectile* p)
{

	if (_minimized)
	{
		return;
	}
	int xPos = _battle->getWidth() / 2 + p->getHorizontalPosition();
	// Draw missiles.
	if (p->getGlobalType() == CWPGT_MISSILE)
	{
		xPos -= 1;
		int yPos = _battle->getHeight() - p->getPosition() / 8;
		for (int x = 0; x < 3; ++x)
		{
			for (int y = 0; y < 6; ++y)
			{
				int pixelOffset = _projectileBlobs[p->getType()][y][x];
				if (pixelOffset == 0)
				{
					continue;
				}
				else
				{
					Uint8 radarPixelColor = _window->getPixel(xPos + x + 3, yPos + y + 3); // + 3 cause of the window frame
					Uint8 color = radarPixelColor - pixelOffset;
					if (color < 108)
					{
						color = 108;
					}
					_battle->setPixel(xPos + x, yPos + y, color);
				}
			}
		}
	}
	// Draw beams.
	else if (p->getGlobalType() == CWPGT_BEAM)
	{
		int yStart = _battle->getHeight() - 2;
		int yEnd = _battle->getHeight() - (_currentDist / 8);
		Uint8 pixelOffset = p->getState();
		for (int y = yStart; y > yEnd; --y)
		{
			Uint8 radarPixelColor = _window->getPixel(xPos + 3, y + 3);
			Uint8 color = radarPixelColor - pixelOffset;
			if (color < 108)
			{
				color = 108;
			}
			_battle->setPixel(xPos, y, color);
		}
	}
}

/**
 * Toggles usage of weapon number 1.
 * @param action Pointer to an action.
 */
void DogfightState::weaponClick(Action * a)
{
	for(int i = 0; i < _weaponNum; ++i)
	{
		if (a->getSender() == _weapon[i])
		{
			_weaponEnabled[i] = !_weaponEnabled[i];
			recolor(i, _weaponEnabled[i]);
			return;
		}
	}
}

/**
 * Changes colors of weapon icons, range indicators and ammo texts base on current weapon state.
 * @param weaponNo - number of weapon for which colors must be changed.
 * @param currentState - state of weapon (enabled = true, disabled = false).
 */
void DogfightState::recolor(const int weaponNo, const bool currentState)
{
	InteractiveSurface *weapon = _weapon[weaponNo];
	Text *ammo = _txtAmmo[weaponNo];
	Surface *range = _range[weaponNo];
	int weaponAndAmmoOffset = 24, rangeOffset = 7;

	if (currentState)
	{
		weapon->offset(-weaponAndAmmoOffset);
		ammo->offset(-weaponAndAmmoOffset);
		range->offset(-rangeOffset);
	}
	else
	{
		weapon->offset(weaponAndAmmoOffset);
		ammo->offset(weaponAndAmmoOffset);
		range->offset(rangeOffset);
	}
}

/**
 * Returns true if state is minimized. Otherwise returns false.
 * @return Is the dogfight minimized?
 */
bool DogfightState::isMinimized() const
{
	return _minimized;
}

/**
 * Sets the state to minimized/maximized status.
 * @param minimized Is the dogfight minimized?
 */
void DogfightState::setMinimized(const bool minimized)
{
	_minimized = minimized;
}

/**
 * Maximizes the interception window.
 * @param action Pointer to an action.
 */
void DogfightState::btnMinimizedIconClick(Action *)
{
	setMinimized(false);
	_window->setVisible(true);
	_btnStandoff->setVisible(true);
	_btnCautious->setVisible(true);
	_btnStandard->setVisible(true);
	_btnAggressive->setVisible(true);
	_btnDisengage->setVisible(true);
	_btnUfo->setVisible(true);
	_btnMinimize->setVisible(true);
	_battle->setVisible(true);
	for (int i = 0; i < _weaponNum; ++i)
	{
		_weapon[i]->setVisible(true);
		_range[i]->setVisible(true);
		_txtAmmo[i]->setVisible(true);
	}

	_damage->setVisible(true);
	_txtDistance->setVisible(true);
	_txtStatus->setVisible(true);
	_btnMinimizedIcon->setVisible(false);
	_txtInterceptionNumber->setVisible(false);
	_preview->setVisible(false);
}

/**
 * Sets interception number. Used to draw proper number when window minimized.
 * @param number ID number.
 */
void DogfightState::setInterceptionNumber(const int number)
{
	_interceptionNumber = number;
}

/**
 * Sets interceptions count. Used to properly position the window.
 * @param count Amount of interception windows.
 */
void DogfightState::setInterceptionsCount(const size_t count)
{
	_interceptionsCount = count;
	calculateWindowPosition();
	moveWindow();
}

/**
 * Calculates dogfight window position according to
 * number of active interceptions.
 */
void DogfightState::calculateWindowPosition()
{

	_minimizedIconX = 5;
	_minimizedIconY = (5 * _interceptionNumber) + (16 * (_interceptionNumber - 1));

	
	if (_interceptionsCount == 1)
	{
		_x = 80;
		_y = 52;
	}
	else if (_interceptionsCount == 2)
	{
		if (_interceptionNumber == 1)
		{
			_x = 80;
			_y = 0;
		}
		else // 2
		{
			_x = 80;
			//_y = (_game->getScreen()->getHeight() / 2) - 96;
			_y = 200 - _window->getHeight();//96;
		}
	}
	else if (_interceptionsCount == 3)
	{
		if (_interceptionNumber == 1)
		{
			_x = 80;
			_y = 0;
		}
		else if (_interceptionNumber == 2)
		{
			_x = 0;
			//_y = (_game->getScreen()->getHeight() / 2) - 96;
			_y = 200 - _window->getHeight();//96;
		}
		else // 3
		{
			//_x = (_game->getScreen()->getWidth() / 2) - 160;
			//_y = (_game->getScreen()->getHeight() / 2) - 96;
			_x = 320 - _window->getWidth();//160;
			_y = 200 - _window->getHeight();//96;
		}
	}
	else
	{
		if (_interceptionNumber == 1)
		{
			_x = 0;
			_y = 0;
		}
		else if (_interceptionNumber == 2)
		{
			//_x = (_game->getScreen()->getWidth() / 2) - 160;
			_x = 320 - _window->getWidth();//160;
			_y = 0;
		}
		else if (_interceptionNumber == 3)
		{
			_x = 0;
			//_y = (_game->getScreen()->getHeight() / 2) - 96;
			_y = 200 - _window->getHeight();//96;
		}
		else // 4
		{
			//_x = (_game->getScreen()->getWidth() / 2) - 160;
			//_y = (_game->getScreen()->getHeight() / 2) - 96;
			_x = 320 - _window->getWidth();//160;
			_y = 200 - _window->getHeight();//96;
		}
	}
	_x += _game->getScreen()->getDX();
	_y += _game->getScreen()->getDY();
}

/**
 * Relocates all dogfight window elements to
 * calculated position. This is used when multiple
 * interceptions are running.
 */
void DogfightState::moveWindow()
{
	_window->setX(_x); _window->setY(_y);
	_battle->setX(_x + 3); _battle->setY(_y + 3);
	for(int i = 0; i < _weaponNum; ++i)
	{
		const int w_off = i % 2 ? 64 : 4;
		const int r_off = i % 2 ? 43 : 19;
		const int y_off = 52 - (i / 2) * 28;
		_weapon[i]->setX(_x + w_off); _weapon[i]->setY(_y + y_off);
		_range[i]->setX(_x + r_off); _range[i]->setY(_y + 3);
		_txtAmmo[i]->setX(_x + w_off); _txtAmmo[i]->setY(_y + y_off + 18);
	}

	_damage->setX(_x + 93); _damage->setY(_y + 40);
	_btnMinimize->setX(_x); _btnMinimize->setY(_y);
	_preview->setX(_x); _preview->setY(_y);
	_btnStandoff->setX(_x + 83); _btnStandoff->setY(_y + 4);
	_btnCautious->setX(_x + 120); _btnCautious->setY(_y + 4);
	_btnStandard->setX(_x + 83); _btnStandard->setY(_y + 20);
	_btnAggressive->setX(_x + 120); _btnAggressive->setY(_y + 20);
	_btnDisengage->setX(_x + 120); _btnDisengage->setY(_y + 36);
	_btnUfo->setX(_x + 120); _btnUfo->setY(_y + 52);
	_txtDistance->setX(_x + 116); _txtDistance->setY(_y + 72);
	_txtStatus->setX(_x + 4); _txtStatus->setY(_y + 85);
	_btnMinimizedIcon->setX(_minimizedIconX); _btnMinimizedIcon->setY(_minimizedIconY);
	_txtInterceptionNumber->setX(_minimizedIconX + 18); _txtInterceptionNumber->setY(_minimizedIconY + 6);
}

/**
 * Checks whether the dogfight should end.
 * @return Returns true if the dogfight should end, otherwise returns false.
 */
bool DogfightState::dogfightEnded() const
{
	return _endDogfight;
}

/**
 * Returns the UFO associated to this dogfight.
 * @return Returns pointer to UFO object associated to this dogfight.
 */
Ufo* DogfightState::getUfo() const
{
	return _ufo;
}

/**
 * Ends the dogfight.
 */
void DogfightState::endDogfight()
{
	if (_craft)
		_craft->setInDogfight(false);
	_endDogfight = true;
}

/**
 * Returns interception number.
 * @return interception number
 */
int DogfightState::getInterceptionNumber() const
{
	return _interceptionNumber;
}

}
