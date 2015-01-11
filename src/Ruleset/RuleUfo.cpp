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
#include "RuleUfo.h"
#include "RuleTerrain.h"

namespace OpenXcom
{

/**
 * Creates a blank ruleset for a certain
 * type of UFO.
 * @param type String defining the type.
 */
RuleUfo::RuleUfo(const std::string &type) : _type(type), _size("STR_VERY_SMALL"), 
	_sprite(-1), _damageMax(0), _speedMax(0), 
	_accel(0), _power(0), _range(0), _score(0), 
	_reload(0), _breakOffTime(0), _sightRange(250), 
	_battlescapeTerrainData(0),	_modSprite(""), _stats(), _statsRaceBonus(), 
	_crashSiteTime(100)
{
	_stats.sightRange = 250;
	_statsRaceBonus[""] = RuleUfoStats();
}

/**
 *
 */
RuleUfo::~RuleUfo()
{
	delete _battlescapeTerrainData;
}

/**
 * Loads the UFO from a YAML file.
 * @param node YAML node.
 * @param ruleset Ruleset for the UFO.
 */
void RuleUfo::load(const YAML::Node &node, Ruleset *ruleset)
{
	_type = node["type"].as<std::string>(_type);
	_size = node["size"].as<std::string>(_size);
	_sprite = node["sprite"].as<int>(_sprite);	
	_score = node["score"].as<int>(_score);
	if( node["sightRange"] ) 
	{
		_sightRange = node["sightRange"].as<int>(_sightRange);
	}
	_breakOffTime = node["breakOffTime"].as<int>(_breakOffTime);
	_crashSiteTime = _breakOffTime;
	_crashSiteTime = node["crashSiteTime"].as<int>(_crashSiteTime);

	_stats.load(node);

	if (const YAML::Node &terrain = node["battlescapeTerrainData"])
	{
		RuleTerrain *rule = new RuleTerrain(terrain["name"].as<std::string>());
		rule->load(terrain, ruleset);
		_battlescapeTerrainData = rule;
	}
	_modSprite = node["modSprite"].as<std::string>(_modSprite);
	if (const YAML::Node &raceBonus = node["raceBonus"])
	{
		for (YAML::const_iterator i = raceBonus.begin(); i != raceBonus.end(); ++i)
		{
			_statsRaceBonus[i->first.as<std::string>()].load(i->second);
		}
	}
}

/**
 * Gets the language string that names
 * this UFO. Each UFO type has a unique name.
 * @return The Ufo's name.
 */
std::string RuleUfo::getType() const
{
	return _type;
}

/**
 * Gets the size of this type of UFO.
 * @return The Ufo's size.
 */
std::string RuleUfo::getSize() const
{
	return _size;
}

/**
 * Gets the radius of this type of UFO
 * on the dogfighting window.
 * @return The radius in pixels.
 */
int RuleUfo::getRadius() const
{
	if (_size == "STR_VERY_SMALL")
	{
		return 2;
	}
	else if (_size == "STR_SMALL")
	{
		return 3;
	}
	else if (_size == "STR_MEDIUM_UC")
	{
		return 4;
	}
	else if (_size == "STR_LARGE")
	{
		return 5;
	}
	else if (_size == "STR_VERY_LARGE")
	{
		return 6;
	}
	//TODO ADD MORE CRAFTS
	return 0;
}

/**
 * Gets the ID of the sprite used to draw the UFO
 * in the Dogfight window.
 * @return The sprite ID.
 */
int RuleUfo::getSprite() const
{
	return _sprite;
}

/**
 * Gets the maximum damage (damage the UFO can take)
 * of the UFO.
 * @return The maximum damage.
 */
int RuleUfo::getMaxDamage() const
{
	return _stats.damageMax;
}

/**
 * Gets the maximum speed of the UFO flying
 * around the Geoscape.
 * @return The maximum speed.
 */
int RuleUfo::getMaxSpeed() const
{
	return _stats.speedMax;
}

/**
 * Gets the acceleration of the UFO for
 * taking off / stopping.
 * @return The acceleration.
 */
int RuleUfo::getAcceleration() const
{
	return _stats.accel;
}

/**
 * Gets the maximum damage done by the
 * UFO's weapons per shot.
 * @return The weapon power.
 */
int RuleUfo::getWeaponPower() const
{
	return _stats.power;
}

/**
 * Gets the maximum range for the
 * UFO's weapons.
 * @return The weapon range.
 */
int RuleUfo::getWeaponRange() const
{
	return _stats.range;
}

/**
 * Gets the amount of points the player
 * gets for shooting down the UFO.
 * @return The score.
 */
int RuleUfo::getScore() const
{
	return _score;
}

/**
 * Gets the terrain data needed to draw the UFO in the battlescape.
 * @return The RuleTerrain.
 */
RuleTerrain *RuleUfo::getBattlescapeTerrainData()  const
{
	return _battlescapeTerrainData;
}

/**
 * Gets the weapon reload for UFO ships.
 * @return The UFO weapon reload time.
 */
int RuleUfo::getWeaponReload() const
{
	return _stats.reload;
}

/**
 * Gets the UFO's break off time.
 * @return The UFO's break off time in game seconds.
 */
int RuleUfo::getBreakOffTime() const
{
	return _breakOffTime;
}

/**
 * For user-defined UFOs, use a surface for the "preview" image.
 * @return The name of the surface that represents this UFO.
 */
std::string RuleUfo::getModSprite() const
{
	return _modSprite;
}

/**
 * Gets the UFO's radar range
 * for detecting bases.
 * @return The range in nautical miles.
 */
int RuleUfo::getSightRange() const
{
	return _stats.sightRange;
}

/**
 * Gets the UFO's stats
 * @return Ufo stats (with races bonus).
 */
const RuleUfoStats& RuleUfo::getStats() const
{
	return _stats;
}

/**
 * Gets the UFO's weapon chance to hit.
 * @return The chance to hit in percantage.
*/ 
int RuleUfo::getWeaponAccuracy() const	
{
	return 60 + _stats.hitChanceBonus;
	//TODO alien hit chance grows 1% per month of game 
}

/**
 * Gets the UFO's avoid chance
 * @return The chance to hit in percantage.
*/
int RuleUfo::getAvoidChance() const
{
	return 30 - getRadius() * 10 + _stats.avoidBonus;
}

/**
 * Gets bonus statistic of UFO based on race.
 * @param s Race name.
 * @return Bonus stats.
 */
const RuleUfoStats& RuleUfo::getRaceBonus(const std::string& s) const
{
	std::map<std::string, RuleUfoStats>::const_iterator i = _statsRaceBonus.find(s);
	if (i != _statsRaceBonus.end())
		return i->second;
	else
		return _statsRaceBonus.find("")->second;
}

/**
 * Get the time UFO is on the crash site time
 * @return Time on crash site.
 */
int RuleUfo::getCrashSiteTime() const
{
	return _crashSiteTime;
}

}
