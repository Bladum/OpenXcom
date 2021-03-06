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
#define _USE_MATH_DEFINES
#include <cmath>
#include "ProjectileFlyBState.h"
#include "ExplosionBState.h"
#include "Projectile.h"
#include "TileEngine.h"
#include "Map.h"
#include "Pathfinding.h"
#include "../Engine/Game.h"
#include "../Savegame/BattleUnit.h"
#include "../Savegame/BattleItem.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/SavedBattleGame.h"
#include "../Savegame/Tile.h"
#include "../Resource/ResourcePack.h"
#include "../Engine/Sound.h"
#include "../Ruleset/Armor.h"
#include "../Ruleset/RuleItem.h"
#include "../Engine/Options.h"
#include "AlienBAIState.h"
#include "Camera.h"
#include "Explosion.h"
#include "BattlescapeState.h"

namespace OpenXcom
{

/**
 * Sets up an ProjectileFlyBState.
 */
ProjectileFlyBState::ProjectileFlyBState(BattlescapeGame *parent, BattleAction action, Position origin, int range) : BattleState(parent, action), _unit(0), _ammo(0), _projectileItem(0), _origin(origin), _originVoxel(-1,-1,-1), _projectileImpact(0), _range(range), _initialized(false), _targetFloor(false)
{
}

ProjectileFlyBState::ProjectileFlyBState(BattlescapeGame *parent, BattleAction action) : BattleState(parent, action), _unit(0), _ammo(0), _projectileItem(0), _origin(action.actor->getPosition()), _originVoxel(-1,-1,-1), _projectileImpact(0), _range(0), _initialized(false), _targetFloor(false)
{
}

/**
 * Deletes the ProjectileFlyBState.
 */
ProjectileFlyBState::~ProjectileFlyBState()
{
}

/**
 * Initializes the sequence:
 * - checks if the shot is valid,
 * - calculates the base accuracy.
 */
void ProjectileFlyBState::init()
{
	if (_initialized) return;
	_initialized = true;

	BattleItem *weapon = _action.weapon;
	_projectileItem = 0;

	if (!weapon) // can't shoot without weapon
	{
		_parent->popState();
		return;
	}

	if (!_parent->getSave()->getTile(_action.target)) // invalid target position
	{
		_parent->popState();
		return;
	}

	if (_parent->getPanicHandled() &&
		_action.type != BA_HIT &&
		_action.type != BA_STUN &&
		_action.actor->getTimeUnits() < _action.TU)
	{
		_action.result = "STR_NOT_ENOUGH_TIME_UNITS";
		_parent->popState();
		return;
	}

	_unit = _action.actor;

	_ammo = weapon->getAmmoItem();

	if (_unit->isOut() || _unit->getHealth() <= 0 || _unit->getHealth() < _unit->getStunlevel())
	{
		// something went wrong - we can't shoot when dead or unconscious, or if we're about to fall over.
		_parent->popState();
		return;
	}

	// reaction fire
	if (_unit->getFaction() != _parent->getSave()->getSide())
	{
		// no ammo or target is dead: give the time units back and cancel the shot.
		if (_ammo == 0
			|| !_parent->getSave()->getTile(_action.target)->getUnit()
			|| _parent->getSave()->getTile(_action.target)->getUnit()->isOut()
			|| _parent->getSave()->getTile(_action.target)->getUnit() != _parent->getSave()->getSelectedUnit())
		{
			_unit->setTimeUnits(_unit->getTimeUnits() + _unit->getActionTUs(_action.type, _action.weapon));
			_parent->popState();
			return;
		}
	}

	// autoshot will default back to snapshot if it's not possible
	if (weapon->getRules()->getAccuracyAuto() == 0 && _action.type == BA_AUTOSHOT)
		_action.type = BA_SNAPSHOT;

	// snapshot defaults to "hit" if it's a melee weapon
	// (in case of reaction "shots" with a melee weapon)
	if (weapon->getRules()->getBattleType() == BT_MELEE && _action.type == BA_SNAPSHOT)
		_action.type = BA_HIT;
	Tile *endTile = _parent->getSave()->getTile(_action.target);
	switch (_action.type)
	{
	case BA_SNAPSHOT:
	case BA_AIMEDSHOT:
	case BA_AUTOSHOT:
	case BA_LAUNCH:
		if (_ammo == 0)
		{
			_action.result = "STR_NO_AMMUNITION_LOADED";
			_parent->popState();
			return;
		}
		if (_ammo->getAmmoQuantity() == 0)
		{
			_action.result = "STR_NO_ROUNDS_LEFT";
			_parent->popState();
			return;
		}
		if (_parent->getTileEngine()->distance(_action.actor->getPosition(), _action.target) > weapon->getRules()->getMaxRange())
		{
			// out of range
			_action.result = "STR_OUT_OF_RANGE";
			_parent->popState();
			return;
		}
		break;
	case BA_THROW:
		if (!validThrowRange(&_action, _parent->getTileEngine()->getOriginVoxel(_action, 0), _parent->getSave()->getTile(_action.target)))
		{
			// out of range
			_action.result = "STR_OUT_OF_RANGE";
			_parent->popState();
			return;
		}
		if (endTile &&
			endTile->getTerrainLevel() == -24 &&
			endTile->getPosition().z + 1 < _parent->getSave()->getMapSizeZ())
		{
			_action.target.z += 1;
		}
		_projectileItem = weapon;
		break;
	case BA_HIT:
		performMeleeAttack();
		return;
	case BA_USE:
 	case BA_PANIC:
 	case BA_MINDCONTROL:
		if (_parent->getTileEngine()->distance(_action.actor->getPosition(), _action.target) > weapon->getRules()->getMaxRange())
		{
			// out of range
			_action.result = "STR_OUT_OF_RANGE";
			_parent->popState();
			return;
		}
	default:
		_parent->popState();
		return;
	}
	
	if (_action.type == BA_LAUNCH || (Options::forceFire && (SDL_GetModState() & KMOD_CTRL) != 0 && _parent->getSave()->getSide() == FACTION_PLAYER) || !_parent->getPanicHandled())
	{
		// target nothing, targets the middle of the tile
		_targetVoxel = Position(_action.target.x*16 + 8, _action.target.y*16 + 8, _action.target.z*24 + 12);
		if (_action.type == BA_LAUNCH)
		{
			if (_targetFloor)
			{
				// launched missiles with two waypoints placed on the same tile: target the floor.
				_targetVoxel.z -= 10;
			}
			else
			{
				// launched missiles go slightly higher than the middle.
				_targetVoxel.z += 4;
			}
		}
	}
	else
	{
		// determine the target voxel.
		// aim at the center of the unit, the object, the walls or the floor (in that priority)
		// if there is no LOF to the center, try elsewhere (more outward).
		// Store this target voxel.
		Tile *targetTile = _parent->getSave()->getTile(_action.target);
		Position hitPos;
		Position originVoxel = _parent->getTileEngine()->getOriginVoxel(_action, _parent->getSave()->getTile(_origin));
		if (targetTile->getUnit() != 0)
		{
			if (_origin == _action.target || targetTile->getUnit() == _unit)
			{
				// don't shoot at yourself but shoot at the floor
				_targetVoxel = Position(_action.target.x*16 + 8, _action.target.y*16 + 8, _action.target.z*24);
			}
			else
			{
				_parent->getTileEngine()->canTargetUnit(&originVoxel, targetTile, &_targetVoxel, _unit);
			}
		}
		else if (targetTile->getMapData(MapData::O_OBJECT) != 0)
		{
			if (!_parent->getTileEngine()->canTargetTile(&originVoxel, targetTile, MapData::O_OBJECT, &_targetVoxel, _unit))
			{
				_targetVoxel = Position(_action.target.x*16 + 8, _action.target.y*16 + 8, _action.target.z*24 + 10);
			}
		}
		else if (targetTile->getMapData(MapData::O_NORTHWALL) != 0)
		{
			if (!_parent->getTileEngine()->canTargetTile(&originVoxel, targetTile, MapData::O_NORTHWALL, &_targetVoxel, _unit))
			{
				_targetVoxel = Position(_action.target.x*16 + 8, _action.target.y*16, _action.target.z*24 + 9);
			}
		}
		else if (targetTile->getMapData(MapData::O_WESTWALL) != 0)
		{
			if (!_parent->getTileEngine()->canTargetTile(&originVoxel, targetTile, MapData::O_WESTWALL, &_targetVoxel, _unit))
			{
				_targetVoxel = Position(_action.target.x*16, _action.target.y*16 + 8, _action.target.z*24 + 9);
			}
		}
		else if (targetTile->getMapData(MapData::O_FLOOR) != 0)
		{
			if (!_parent->getTileEngine()->canTargetTile(&originVoxel, targetTile, MapData::O_FLOOR, &_targetVoxel, _unit))
			{
				_targetVoxel = Position(_action.target.x*16 + 8, _action.target.y*16 + 8, _action.target.z*24 + 2);
			}
		}
		else
		{
			// target nothing, targets the middle of the tile
			_targetVoxel = Position(_action.target.x*16 + 8, _action.target.y*16 + 8, _action.target.z*24 + 12);
		}
	}
	if (createNewProjectile())
	{
		_parent->getMap()->setCursorType(CT_NONE);
		_parent->getMap()->getCamera()->stopMouseScrolling();
	}
}

/**
 * Tries to create a projectile sprite and add it to the map,
 * calculating its trajectory.
 * @return True, if the projectile was successfully created.
 */
bool ProjectileFlyBState::createNewProjectile()
{
	++_action.autoShotCounter;
	
	// create a new projectile
	Projectile *projectile = new Projectile(_parent->getResourcePack(), _parent->getSave(), _action, _origin, _targetVoxel, _ammo);

	// add the projectile on the map
	_parent->getMap()->setProjectile(projectile);

	// set the speed of the state think cycle to 16 ms (roughly one think cycle per frame)
	_parent->setStateInterval(1000/60);

	// let it calculate a trajectory
	_projectileImpact = V_EMPTY;
	if (_action.type == BA_THROW)
	{
		_projectileImpact = projectile->calculateThrow(_unit->getThrowingAccuracy() / 100.0);
		if (_projectileImpact == V_FLOOR || _projectileImpact == V_UNIT || _projectileImpact == V_OBJECT)
		{
			if (_unit->getFaction() != FACTION_PLAYER && _projectileItem->getRules()->getBattleType() == BT_GRENADE)
			{
				_projectileItem->setFuseTimer(0);
			}
			_projectileItem->moveToOwner(0);
			_unit->setCache(0);
			_parent->getMap()->cacheUnit(_unit);
			_parent->getResourcePack()->getSoundByDepth(_parent->getDepth(), ResourcePack::ITEM_THROW)->play(-1, _parent->getMap()->getSoundAngle(_unit->getPosition()));
			_unit->addThrowingExp();
		}
		else
		{
			// unable to throw here
			delete projectile;
			_parent->getMap()->setProjectile(0);
			_action.result = "STR_UNABLE_TO_THROW_HERE";
			_action.TU = 0;
			_parent->popState();
			return false;
		}
	}
	else if (_action.weapon->getRules()->getArcingShot()) // special code for the "spit" trajectory
	{
		_projectileImpact = projectile->calculateThrow(_unit->getFiringAccuracy(_action.type, _action.weapon) / 100.0);
		if (_projectileImpact != V_EMPTY && _projectileImpact != V_OUTOFBOUNDS)
		{
			// set the soldier in an aiming position
			_unit->aim(true);
			_unit->setCache(0);
			_parent->getMap()->cacheUnit(_unit);
			// and we have a lift-off
			if (_ammo->getRules()->getFireSound() != -1)
			{
				_parent->getResourcePack()->getSoundByDepth(_parent->getDepth(), _ammo->getRules()->getFireSound())->play(-1, _parent->getMap()->getSoundAngle(_unit->getPosition()));
			}
			else if (_action.weapon->getRules()->getFireSound() != -1)
			{
				_parent->getResourcePack()->getSoundByDepth(_parent->getDepth(), _action.weapon->getRules()->getFireSound())->play(-1, _parent->getMap()->getSoundAngle(_unit->getPosition()));
			}
			if (!_parent->getSave()->getDebugMode() && _action.type != BA_LAUNCH && _ammo->spendBullet() == false)
			{
				_parent->getSave()->removeItem(_ammo);
				_action.weapon->setAmmoItem(0);
			}
		}
		else
		{
			// no line of fire
			delete projectile;
			_parent->getMap()->setProjectile(0);
			_action.result = "STR_NO_LINE_OF_FIRE";
			_unit->abortTurn();
			_parent->popState();
			return false;
		}
	}
	else
	{
		if (_originVoxel != Position(-1,-1,-1))
		{
			_projectileImpact = projectile->calculateTrajectory(_unit->getFiringAccuracy(_action.type, _action.weapon) / 100.0, _originVoxel);
		}
		else
		{
			_projectileImpact = projectile->calculateTrajectory(_unit->getFiringAccuracy(_action.type, _action.weapon) / 100.0);
		}
		if (_projectileImpact != V_EMPTY || _action.type == BA_LAUNCH)
		{
			// set the soldier in an aiming position
			_unit->aim(true);
			_unit->setCache(0);
			_parent->getMap()->cacheUnit(_unit);
			// and we have a lift-off
			if (_ammo->getRules()->getFireSound() != -1)
			{
				_parent->getResourcePack()->getSoundByDepth(_parent->getDepth(), _ammo->getRules()->getFireSound())->play(-1, _parent->getMap()->getSoundAngle(projectile->getOrigin()));
			}
			else if (_action.weapon->getRules()->getFireSound() != -1)
			{
				_parent->getResourcePack()->getSoundByDepth(_parent->getDepth(), _action.weapon->getRules()->getFireSound())->play(-1, _parent->getMap()->getSoundAngle(projectile->getOrigin()));
			}
			if (!_parent->getSave()->getDebugMode() && _action.type != BA_LAUNCH && _ammo->spendBullet() == false)
			{
				_parent->getSave()->removeItem(_ammo);
				_action.weapon->setAmmoItem(0);
			}
		}
		else
		{
			// no line of fire
			delete projectile;
			_parent->getMap()->setProjectile(0);
			_action.result = "STR_NO_LINE_OF_FIRE";
			_unit->abortTurn();
			_parent->popState();
			return false;
		}
	}
	return true;
}

/**
 * Animates the projectile (moves to the next point in its trajectory).
 * If the animation is finished the projectile sprite is removed from the map,
 * and this state is finished.
 */
void ProjectileFlyBState::think()
{
	_parent->getSave()->getBattleState()->clearMouseScrollingState();
	/* TODO refactoring : store the projectile in this state, instead of getting it from the map each time? */
	if (_parent->getMap()->getProjectile() == 0)
	{
		Tile *t = _parent->getSave()->getTile(_action.actor->getPosition());
		Tile *bt = _parent->getSave()->getTile(_action.actor->getPosition() + Position(0,0,-1));
		bool hasFloor = t && !t->hasNoFloor(bt);
		bool unitCanFly = _action.actor->getMovementType() == MT_FLY;

		if (_action.type == BA_AUTOSHOT
			&& _action.autoShotCounter < _action.weapon->getRules()->getAutoShots()
			&& !_action.actor->isOut()
			&& _ammo->getAmmoQuantity() != 0
			&& (hasFloor || unitCanFly))
		{
			createNewProjectile();
			if (_action.cameraPosition.z != -1)
			{
				_parent->getMap()->getCamera()->setMapOffset(_action.cameraPosition);
				_parent->getMap()->invalidate();
			}
		}
		else
		{
			if (_action.cameraPosition.z != -1 && _action.waypoints.size() <= 1)
			{
				_parent->getMap()->getCamera()->setMapOffset(_action.cameraPosition);
				_parent->getMap()->invalidate();
			}
			if (_action.type != BA_PANIC && _action.type != BA_MINDCONTROL && !_parent->getSave()->getUnitsFalling())
			{
				_parent->getTileEngine()->checkReactionFire(_unit);
			}
			if (!_unit->isOut() && _action.type != BA_HIT)
			{
				_unit->abortTurn();
			}
			if (_parent->getSave()->getSide() == FACTION_PLAYER || _parent->getSave()->getDebugMode())
			{
				_parent->setupCursor();
			}
			_parent->popState();
		}
	}
	else
	{
		if (_action.type != BA_THROW && _ammo && _ammo->getRules()->getShotgunPellets() != 0)
		{
			// shotgun pellets move to their terminal location instantly as fast as possible
			_parent->getMap()->getProjectile()->skipTrajectory();
		}
		if (!_parent->getMap()->getProjectile()->move())
		{
			// impact !
			if (_action.type == BA_THROW)
			{
				_parent->getMap()->resetCameraSmoothing();
				Position pos = _parent->getMap()->getProjectile()->getPosition(-1).toTile();
				if (pos.y > _parent->getSave()->getMapSizeY())
				{
					pos.y--;
				}
				if (pos.x > _parent->getSave()->getMapSizeX())
				{
					pos.x--;
				}
				BattleItem *item = _parent->getMap()->getProjectile()->getItem();
				_parent->getResourcePack()->getSoundByDepth(_parent->getDepth(), ResourcePack::ITEM_DROP)->play(-1, _parent->getMap()->getSoundAngle(pos));

				if (Options::battleInstantGrenade && item->getRules()->getBattleType() == BT_GRENADE && item->getFuseTimer() == 0)
				{
					// it's a hot grenade to explode immediately
					_parent->statePushFront(new ExplosionBState(_parent, _parent->getMap()->getProjectile()->getPosition(-1), item, _action.actor));
				}
				else
				{
					_parent->dropItem(pos, item);
					if (_unit->getFaction() != FACTION_PLAYER && _projectileItem->getRules()->getBattleType() == BT_GRENADE)
					{
						_parent->getTileEngine()->setDangerZone(pos, item->getRules()->getExplosionRadius(), _action.actor);
					}
				}
			}
			else if (_action.type == BA_LAUNCH && _action.waypoints.size() > 1 && _projectileImpact == -1)
			{
				_origin = _action.waypoints.front();
				_action.waypoints.pop_front();
				_action.target = _action.waypoints.front();
				// launch the next projectile in the waypoint cascade
				ProjectileFlyBState *nextWaypoint = new ProjectileFlyBState(_parent, _action, _origin, _range + _parent->getMap()->getProjectile()->getDistance());
				nextWaypoint->setOriginVoxel(_parent->getMap()->getProjectile()->getPosition(-1));
				if (_origin == _action.target)
				{
					nextWaypoint->targetFloor();
				}
				_parent->statePushNext(nextWaypoint);

			}
			else
			{
				_parent->getMap()->resetCameraSmoothing();
				if (!_parent->getSave()->getDebugMode() && _ammo && _action.type == BA_LAUNCH && _ammo->spendBullet() == false)
				{
					_parent->getSave()->removeItem(_ammo);
					_action.weapon->setAmmoItem(0);
				}

				if (_projectileImpact != V_OUTOFBOUNDS)
				{
					bool shotgun = _ammo && _ammo->getRules()->getShotgunPellets() != 0 && _ammo->getRules()->getDamageType()->isDirect();
 					
					int offset = 0;
					// explosions impact not inside the voxel but two steps back (projectiles generally move 2 voxels at a time)
					if (_ammo && _ammo->getRules()->getExplosionRadius() != 0 && _projectileImpact != V_UNIT)
					{
						offset = -2;
					}
					_parent->statePushFront(new ExplosionBState(
						_parent, _parent->getMap()->getProjectile()->getPosition(offset),
						_ammo, _action.actor, 0,
						(_action.type != BA_AUTOSHOT || _action.autoShotCounter == _action.weapon->getRules()->getAutoShots() || !_action.weapon->getAmmoItem()),
						shotgun ? 0 : _range + _parent->getMap()->getProjectile()->getDistance()
					));

					// special shotgun behaviour: trace extra projectile paths, and add bullet hits at their termination points.
					if (shotgun)
 					{
						int i = 1;
						while (i != _ammo->getRules()->getShotgunPellets())
						{
							// create a projectile
							Projectile *proj = new Projectile(_parent->getResourcePack(), _parent->getSave(), _action, _origin, _targetVoxel, _ammo);
							// let it trace to the point where it hits
							_projectileImpact = proj->calculateTrajectory(std::max(0.0, (_unit->getFiringAccuracy(_action.type, _action.weapon) / 100.0) - i * 5.0));
							if (_projectileImpact != V_EMPTY)
							{
								// as above: skip the shot to the end of it's path
								proj->skipTrajectory();
								// insert an explosion and hit 
								if (_projectileImpact != V_OUTOFBOUNDS)
								{
									Explosion *explosion = new Explosion(proj->getPosition(1), _ammo->getRules()->getHitAnimation());
									_parent->getMap()->getExplosions()->push_back(explosion);
									_parent->getSave()->getTileEngine()->hit(proj->getPosition(1), _ammo->getRules()->getPower(), _ammo->getRules()->getDamageType(), 0);
								}
								++i;
							}
							delete proj;
						}
					}

					if (_projectileImpact == 4)
					{
						BattleUnit *victim = _parent->getSave()->getTile(_parent->getMap()->getProjectile()->getPosition(offset) / Position(16,16,24))->getUnit();
						if (victim && !victim->isOut() && victim->getFaction() == FACTION_HOSTILE)
						{
							AlienBAIState *aggro = dynamic_cast<AlienBAIState*>(victim->getCurrentAIState());
							if (aggro != 0)
							{
								aggro->setWasHitBy(_unit);
								_unit->setTurnsSinceSpotted(0);
							}
						}
					}
				}
				else if (_action.type != BA_AUTOSHOT || _action.autoShotCounter == _action.weapon->getRules()->getAutoShots() || !_action.weapon->getAmmoItem())
				{
					_unit->aim(false);
					_unit->setCache(0);
					_parent->getMap()->cacheUnits();
				}
			}

			delete _parent->getMap()->getProjectile();
			_parent->getMap()->setProjectile(0);
		}
	}
}

/**
 * Flying projectiles cannot be cancelled,
 * but they can be "skipped".
 */
void ProjectileFlyBState::cancel()
{
	if (_parent->getMap()->getProjectile())
	{
		_parent->getMap()->getProjectile()->skipTrajectory();
		Position p = _parent->getMap()->getProjectile()->getPosition().toTile();
		if (!_parent->getMap()->getCamera()->isOnScreen(p, false))
			_parent->getMap()->getCamera()->centerOnPosition(p);
	}
}

/**
 * Validates the throwing range.
 * @param action Pointer to throw action.
 * @param origin Position to throw from.
 * @param target Tile to throw to.
 * @return True when the range is valid.
 */
bool ProjectileFlyBState::validThrowRange(BattleAction *action, Position origin, Tile *target)
{
	// note that all coordinates and thus also distances below are in number of tiles (not in voxels).
	if (action->type != BA_THROW)
	{
		return true;
	}
	int offset = 2;
	int zd = (origin.z)-((action->target.z * 24 + offset) - target->getTerrainLevel());
	int weight = action->weapon->getRules()->getWeight();
	if (action->weapon->getAmmoItem() && action->weapon->getAmmoItem() != action->weapon)
	{
		weight += action->weapon->getAmmoItem()->getRules()->getWeight();
	}
	double maxDistance = (getMaxThrowDistance(weight, action->actor->getBaseStats()->strength, zd) + 8) / 16.0;
	int xdiff = action->target.x - action->actor->getPosition().x;
	int ydiff = action->target.y - action->actor->getPosition().y;
	double realDistance = sqrt((double)(xdiff*xdiff)+(double)(ydiff*ydiff));

	return realDistance <= maxDistance;
}

/**
 * Validates the throwing range.
 * @param weight the weight of the object.
 * @param strength the strength of the thrower.
 * @param level the difference in height between the thrower and the target.
 * @return the maximum throwing range.
 */
int ProjectileFlyBState::getMaxThrowDistance(int weight, int strength, int level)
{
    double curZ = level + 0.5;
    double dz = 1.0;
    int dist = 0;
    while (dist < 4000) //just in case
    {
        dist += 8;
        if (dz<-1)
            curZ -= 8;
        else
            curZ += dz * 8;

        if (curZ < 0 && dz < 0) //roll back
        {
            dz = std::max(dz, -1.0);
            if (std::abs(dz)>1e-10) //rollback horizontal
                dist -= curZ / dz;
            break;
        }
        dz -= (double)(50 * weight / strength)/100;
        if (dz <= -2.0) //become falling
            break;
    }
    return dist;
}

/**
 * Set the origin voxel, used for the blaster launcher.
 * @param pos the origin voxel.
 */
void ProjectileFlyBState::setOriginVoxel(Position pos)
{
	_originVoxel = pos;
}

/**
 * Set the boolean flag to angle a blaster bomb towards the floor.
 */
void ProjectileFlyBState::targetFloor()
{
	_targetFloor = true;
}

void ProjectileFlyBState::performMeleeAttack()
{
	BattleUnit *target = _parent->getSave()->getTile(_action.target)->getUnit();
	int height = target->getFloatHeight() + (target->getHeight() / 2);
	Position voxel;
	_parent->getSave()->getPathfinding()->directionToVector(_unit->getDirection(), &voxel);
	voxel = _action.target.toVexel() + Position(8,8,height - _parent->getSave()->getTile(_action.target)->getTerrainLevel()) - voxel;
	// set the soldier in an aiming position
	_unit->aim(true);
	_unit->setCache(0);
	_parent->getMap()->cacheUnit(_unit);
	// and we have a lift-off
	if (_ammo && _ammo->getRules()->getMeleeAttackSound() != -1)
	{
		_parent->getResourcePack()->getSoundByDepth(_parent->getDepth(), _ammo->getRules()->getMeleeAttackSound())->play(-1, _parent->getMap()->getSoundAngle(_action.target));
	}
	else if (_action.weapon->getRules()->getMeleeAttackSound() != -1)
	{
		_parent->getResourcePack()->getSoundByDepth(_parent->getDepth(), _action.weapon->getRules()->getMeleeAttackSound())->play(-1, _parent->getMap()->getSoundAngle(_action.target));
	}
	if (!_parent->getSave()->getDebugMode() && _action.weapon->getRules()->getBattleType() == BT_MELEE && _ammo && _ammo->spendBullet() == false)
	{
		_parent->getSave()->removeItem(_ammo);
		_action.weapon->setAmmoItem(0);
	}
	// if the unit burns floortiles, burn floortiles
	if (_unit->getSpecialAbility() == SPECAB_BURNFLOOR || _unit->getSpecialAbility() == SPECAB_BURN_AND_EXPLODE)
	{
		_parent->getSave()->getTile(_action.target)->ignite(15);
	}
	_parent->getMap()->setCursorType(CT_NONE);
	_parent->statePushNext(new ExplosionBState(_parent, voxel, _action.weapon, _action.actor, 0, true));
}
}
