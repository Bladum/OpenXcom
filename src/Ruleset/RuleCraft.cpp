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
#include "RuleCraft.h"
#include "RuleTerrain.h"

namespace OpenXcom
{

/**
 * Creates a blank ruleset for a certain
 * type of craft.
 * @param type String defining the type.
 */
RuleCraft::RuleCraft(const std::string &type) :
    _type(type), _sprite(-1), _weapons(0), _soldiers(0), _vehicles(0),
    _costBuy(0), _costRent(0), _costSell(0), _refuelItem(""), _repairRate(1), _refuelRate(1),
	_transferTime(0), _score(0), _battlescapeTerrainData(0),
	_spacecraft(false), _listOrder(0), _maxItems(0), _stats()
{
	for (int i = 0; i < WeaponMax; ++ i)
		_weaponTypes[i] = 0;
	_stats.radarRange = 600;
	_stats.radarChance = 100;
	_stats.sightRange = 1200;
}

/**
 *
 */
RuleCraft::~RuleCraft()
{
	delete _battlescapeTerrainData;
}

/**
 * Loads the craft from a YAML file.
 * @param node YAML node.
 * @param ruleset Ruleset for the craft.
 * @param modIndex A value that offsets the sounds and sprite values to avoid conflicts.
 * @param listOrder The list weight for this craft.
 */
void RuleCraft::load(const YAML::Node &node, Ruleset *ruleset, int modIndex, int listOrder)
{
	_type = node["type"].as<std::string>(_type);
	_requires = node["requires"].as< std::vector<std::string> >(_requires);
	if (node["sprite"])
	{
		_sprite = node["sprite"].as<int>(_sprite);
		// this is an offset in BASEBITS.PCK, and two in INTICONS.PCK
		if (_sprite > 4)
			_sprite += modIndex;
	}
	_stats.load(node);
	_weapons = node["weapons"].as<int>(_weapons);
	_soldiers = node["soldiers"].as<int>(_soldiers);
	_vehicles = node["vehicles"].as<int>(_vehicles);
	_costBuy = node["costBuy"].as<int>(_costBuy);
	_costRent = node["costRent"].as<int>(_costRent);
	_costSell = node["costSell"].as<int>(_costSell);
	_refuelItem = node["refuelItem"].as<std::string>(_refuelItem);
	_repairRate = node["repairRate"].as<int>(_repairRate);
	_refuelRate = node["refuelRate"].as<int>(_refuelRate);
	_transferTime = node["transferTime"].as<int>(_transferTime);
	_score = node["score"].as<int>(_score);
	
	if (const YAML::Node &terrain = node["battlescapeTerrainData"])
	{
		RuleTerrain *rule = new RuleTerrain(terrain["name"].as<std::string>());
		rule->load(terrain, ruleset);
		_battlescapeTerrainData = rule;
		
		if (const YAML::Node &deployment = node["deployment"])
		{
			_deployment = deployment.as<std::vector<std::vector<int> > >(_deployment);
		}
	}
	_spacecraft = node["spacecraft"].as<bool>(_spacecraft);
	_listOrder = node["listOrder"].as<int>(_listOrder);
	if (!_listOrder)
	{
		_listOrder = listOrder;
	}
	_maxItems = node["maxItems"].as<int>(_maxItems);

	if (const YAML::Node &types = node["weaponTypes"])
	{
		for (int i = 0; (size_t)i < types.size() &&  i < WeaponMax; ++i)
			_weaponTypes[i] = types[i].as<int>();
	}
}

/**
 * Gets the language string that names
 * this craft. Each craft type has a unique name.
 * @return The craft's name.
 */
std::string RuleCraft::getType() const
{
	return _type;
}

/**
 * Gets the list of research required to
 * acquire this craft.
 * @return The list of research IDs.
 */
const std::vector<std::string> &RuleCraft::getRequirements() const
{
	return _requires;
}

/**
 * Gets the ID of the sprite used to draw the craft
 * in the Basescape and Equip Craft screens.
 * @return The Sprite ID.
 */
int RuleCraft::getSprite() const
{
	return _sprite;
}


/**
 * Gets the maximum number of weapons that
 * can be equipped onto the craft.
 * @return The weapon capacity.
 */
int RuleCraft::getWeapons() const
{
	return _weapons;
}

/**
 * Gets the maximum number of soldiers that
 * the craft can carry.
 * @return The soldier capacity.
 */
int RuleCraft::getSoldiers() const
{
	return _soldiers;
}

/**
 * Gets the maximum number of vehicles that
 * the craft can carry.
 * @return The vehicle capacity.
 */
int RuleCraft::getVehicles() const
{
	return _vehicles;
}

/**
 * Gets the cost of this craft for
 * purchase/rent (0 if not purchasable).
 * @return The cost.
 */
int RuleCraft::getBuyCost() const
{
	return _costBuy;
}

/**
 * Gets the cost of rent for a month.
 * @return The cost.
 */
int RuleCraft::getRentCost() const
{
	return _costRent;
}

/**
 * Gets the sell value of this craft
 * Rented craft should use 0.
 * @return The sell value.
 */
int RuleCraft::getSellCost() const
{
	return _costSell;
}

/**
 * Gets what item is required while
 * the craft is refuelling.
 * @return The item ID or "" if none.
 */
std::string RuleCraft::getRefuelItem() const
{
	return _refuelItem;
}

/**
 * Gets how much damage is removed from the
 * craft while repairing.
 * @return The amount of damage.
 */
int RuleCraft::getRepairRate() const
{
	return _repairRate;
}

/**
 * Gets how much fuel is added to the
 * craft while refuelling.
 * @return The amount of fuel.
 */
int RuleCraft::getRefuelRate() const
{
	return _refuelRate;
}

/**
 * Gets the maximum fuel the craft can contain.
 * @return The fuel amount.
 */
int RuleCraft::getMaxFuel() const
{
	return _stats.fuelMax;
}

/**
 * Gets the maximum damage (damage the craft can take)
 * of the craft.
 * @return The maximum damage.
 */
int RuleCraft::getMaxDamage() const
{
	return _stats.damageMax;
}

/**
 * Gets the maximum speed of the craft flying
 * around the Geoscape.
 * @return The speed in knots.
 */
int RuleCraft::getMaxSpeed() const
{
	return _stats.speedMax;
}

/**
 * Gets the acceleration of the craft for
 * taking off / stopping.
 * @return The acceleration.
 */
int RuleCraft::getAcceleration() const
{
	return _stats.accel;
}

/**
 * Gets the craft's radar range
 * for detecting UFOs.
 * @return The range in nautical miles.
 */
int RuleCraft::getRadarRange() const
{
	return _stats.radarRange;
}

/**
 * Gets the craft's radar chance
 * for detecting UFOs.
 * @return The chance in percentage.
 */
int RuleCraft::getRadarChance() const
{
	return _stats.radarChance;
}

/**
 * Gets the craft's sight range
 * for detecting bases.
 * @return The range in nautical miles.
 */
int RuleCraft::getSightRange() const
{
	return _stats.sightRange;
}

/**
 * Gets the craft's avoid chance
 * @return Base chance for craft to be hit
 */
int RuleCraft::getAvoidBonus() const
{
	return _stats.avoidBonus;
}

/**
 * Gets the craft's accurancy modifier
 * @return Base chance for craft to hit Ufo
 */
int RuleCraft::getWeaponsAccurancy() const
{
	return _stats.accuracy;
}

/**
 * Gets the craft's armour
 * @return Base chance for craft to hit Ufo
 */
int RuleCraft::getArmor() const
{
	return _stats.armor;
}

/**
 * Gets the craft's range of weapons modifier
 * @return Modifier for range of all weapons
 */
int RuleCraft::getWeaponsRange() const
{
	return _stats.range;
}

/**
 * Gets the craft's damage of weapons 
 * @return Modifier for damage of all weapons
 */
int RuleCraft::getWeaponsDamage() const
{
	return _stats.damage;
}

/**
 * Gets the craft's reload time of weapons 
 * @return Modifier for reload time of all weapons
 */
int RuleCraft::getWeaponsReload() const
{
	return _stats.reload;
}

/**
 * Gets the amount of time this item
 * takes to arrive at a base.
 * @return The time in hours.
 */
int RuleCraft::getTransferTime() const
{
	return _transferTime;
}

/**
 * Gets the number of points you lose
 * when this craft is destroyed.
 * @return The score in points.
 */
int RuleCraft::getScore() const
{
	return _score;
}

/**
 * Gets the terrain data needed to draw the Craft in the battlescape.
 * @return The terrain data.
 */
RuleTerrain *RuleCraft::getBattlescapeTerrainData()
{
	return _battlescapeTerrainData;
}

/**
 * Checks if this ship is capable of going to mars.
 * @return True if this ship is capable of going to mars.
 */
bool RuleCraft::getSpacecraft() const
{
	return _spacecraft;
}

/**
 * Gets the list weight for this research item.
 * @return The list weight.
 */
int RuleCraft::getListOrder() const
{
	 return _listOrder;
}

/**
 * Gets the deployment layout for this craft.
 * @return The deployment layout.
 */
std::vector<std::vector<int> > &RuleCraft::getDeployment()
{
	return _deployment;
}

/**
 * Gets the item limit for this craft.
 * @return the item limit.
 */
int RuleCraft::getMaxItems() const
{
	return _maxItems;
}

/**
 * Test for possibility of usage of weapon type in weapon slot.
 * @return the item limit.
 */
bool RuleCraft::isValidWeaponSlot(int slot, int weaponType) const
{
	return _weaponTypes[slot] == weaponType;
}

/**
 * Return weaponType
 * @return the item limit.
 */
int RuleCraft::getWeaponTypes(int slot) const
{
	return _weaponTypes[slot];
}

/**
 * Gets basic statistic of craft.
 * @return Basic stats of craft.
 */
const RuleCraftStats& RuleCraft::getStats() const
{
	return _stats;
}

}

