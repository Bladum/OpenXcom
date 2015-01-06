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

#include <sstream>
#include <algorithm>
#include "Ufopaedia.h"
#include "ArticleStateItem.h"
#include "../Ruleset/Ruleset.h"
#include "../Ruleset/ArticleDefinition.h"
#include "../Ruleset/RuleItem.h"
#include "../Engine/Game.h"
#include "../Engine/Palette.h"
#include "../Engine/Surface.h"
#include "../Engine/SurfaceSet.h"
#include "../Engine/Language.h"
#include "../Resource/ResourcePack.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Interface/TextList.h"

namespace OpenXcom
{

	ArticleStateItem::ArticleStateItem(ArticleDefinitionItem *defs) : ArticleState(defs->id)
	{
		RuleItem *item = _game->getRuleset()->getItem(defs->id);

		// add screen elements
		_txtTitle = new Text(148, 32, 5, 24);

		// Set palette
		setPalette("PAL_BATTLEPEDIA");

		ArticleState::initLayout();

		// add other elements
		add(_txtTitle);

		// Set up objects
		_game->getResourcePack()->getSurface("BACK08.SCR")->blit(_bg);
		_btnOk->setColor(Palette::blockOffset(9));
		_btnPrev->setColor(Palette::blockOffset(9));
		_btnNext->setColor(Palette::blockOffset(9));

		_txtTitle->setColor(Palette::blockOffset(14)+15);
		_txtTitle->setBig();
		_txtTitle->setWordWrap(true);
		_txtTitle->setText(tr(defs->title));

		// IMAGE
		_image = new Surface(32, 48, 157, 5);
		add(_image);

		// ITEM WEIGHT
		_txtWeight = new Text(40, 17, 117, 8);
		add(_txtWeight);
		_txtWeight->setColor(Palette::blockOffset(14)+15);
		_txtWeight->setWordWrap(true);
		std::wstring wgh = Text::formatNumber(item->getWeight());
		wgh += L"kg";
		_txtWeight->setText( wgh.c_str() );

		item->drawHandSprite(_game->getResourcePack()->getSurfaceSet("BIGOBS.PCK"), _image);

		std::vector<std::string> *ammo_data = item->getCompatibleAmmo();

		// SHOT STATS TABLE (for firearms AND melee only)
		if (item->getBattleType() == BT_FIREARM || item->getBattleType() == BT_MELEE)
		{
			_txtShotType = new Text(100, 17, 8, 66);
			add(_txtShotType);
			_txtShotType->setColor(Palette::blockOffset(14)+15);
			_txtShotType->setWordWrap(true);
			_txtShotType->setText(tr("STR_SHOT_TYPE"));

			_txtRange = new Text(50, 17, 80, 66);
			add(_txtRange);
			_txtRange->setColor(Palette::blockOffset(14)+15);
			_txtRange->setWordWrap(true);
			_txtRange->setText(tr("STR_RANGE_UC"));

			_txtAccuracy = new Text(50, 17, 122, 66);
			add(_txtAccuracy);
			_txtAccuracy->setColor(Palette::blockOffset(14)+15);
			_txtAccuracy->setWordWrap(true);
			_txtAccuracy->setText(tr("STR_ACCURACY_UC"));

			_txtTuCost = new Text(50, 17, 160, 66);
			add(_txtTuCost);
			_txtTuCost->setColor(Palette::blockOffset(14)+15);
			_txtTuCost->setWordWrap(true);
			_txtTuCost->setText(tr("STR_TIME_UNIT_COST"));

			_lstInfo = new TextList(204, 55, 8, 82);
			add(_lstInfo);

			_lstInfo->setColor(Palette::blockOffset(15)+4); // color for %-data!
			_lstInfo->setColumns(4, 72, 42, 40, 40);
			_lstInfo->setBig();

			int current_row = 0;
			if (item->getTUAuto()>0)
			{
				std::wstring tu = Text::formatPercentage(item->getTUAuto());
				std::wstring acc = Text::formatPercentage(item->getAccuracyAuto());
				std::wstring txt = tr("STR_SHOT_TYPE_AUTO");
				int rngMin = item->getMinRange();
				int rngMax = item->getAutoRange();
				std::wstring rng = Text::formatNumber(rngMin);
				rng += L"-";
				rng += Text::formatNumber(rngMax);

				if (item->getFlatRate())
				{
					tu.erase(tu.end() - 1);
				}
				if( item->getAutoShots() )
				{
					txt += L" x";
					txt += Text::formatNumber(item->getAutoShots()); 
				}
				_lstInfo->addRow(4,
								 txt.c_str(),
								 rng.c_str(),
								 acc.c_str(),
								 tu.c_str());
				_lstInfo->setCellColor(current_row, 0, Palette::blockOffset(14)+15);
				current_row++;
			}

			if (item->getTUSnap()>0)
			{
				std::wstring tu = Text::formatPercentage(item->getTUSnap());
				std::wstring acc = Text::formatPercentage(item->getAccuracySnap());
				std::wstring txt = tr("STR_SHOT_TYPE_SNAP");
				int rngMin = item->getMinRange();
				int rngMax = item->getSnapRange();
				std::wstring rng = Text::formatNumber(rngMin);
				rng += L"-";
				rng += Text::formatNumber(rngMax);

				if (item->getFlatRate())
				{
					tu.erase(tu.end() - 1);
				}
				_lstInfo->addRow(4,
								 txt.c_str(),
								 rng.c_str(),
								 acc.c_str(),
								 tu.c_str());
				_lstInfo->setCellColor(current_row, 0, Palette::blockOffset(14)+15);
				current_row++;
			}

			if (item->getTUAimed()>0)
			{
				std::wstring tu = Text::formatPercentage(item->getTUAimed());
				std::wstring acc = Text::formatPercentage(item->getAccuracyAimed());
				std::wstring txt = tr("STR_SHOT_TYPE_AIMED");
				int rngMin = item->getMinRange();
				int rngMax = item->getAimRange();
				std::wstring rng = Text::formatNumber(rngMin);
				rng += L"-";
				rng += Text::formatNumber(rngMax);

				if (item->getFlatRate())
				{
					tu.erase(tu.end() - 1);
				}
				_lstInfo->addRow(4,
								 txt.c_str(),
								 rng.c_str(),
								 acc.c_str(),
								 tu.c_str());
				_lstInfo->setCellColor(current_row, 0, Palette::blockOffset(14)+15);
				current_row++;
			}
			if (item->getTUMelee()>0)
			{
				std::wstring tu = Text::formatPercentage(item->getTUMelee());
				std::wstring acc = Text::formatPercentage(item->getAccuracyMelee());
				std::wstring txt = tr("STR_SHOT_TYPE_MELEE");
				int rngMin = 1;
				int rngMax = 1;
				std::wstring rng = Text::formatNumber(rngMin);
				rng += L"-";
				rng += Text::formatNumber(rngMax);

				if (item->getFlatRate())
				{
					tu.erase(tu.end() - 1);
				}
				_lstInfo->addRow(4,
								 txt.c_str(),
								 rng.c_str(),
								 acc.c_str(),
								 tu.c_str());
				_lstInfo->setCellColor(current_row, 0, Palette::blockOffset(14)+15);
				current_row++;
			}

			// text_info is BELOW the info table
			_txtInfo = new Text((ammo_data->size()<3 ? 300 : 180), 56, 8, 138);
		}
		else
		{
			// text_info is larger and starts on top
			_txtInfo = new Text(300, 125, 8, 67);
		}

		add(_txtInfo);

		_txtInfo->setColor(Palette::blockOffset(14)+15);
		_txtInfo->setWordWrap(true);
		_txtInfo->setText(tr(defs->text));


		// AMMO column
		std::wostringstream ss;

		for (int i = 0; i<3; ++i)
		{
			_txtAmmoType[i] = new Text(82, 16, 194, 20 + i*49);
			add(_txtAmmoType[i]);
			_txtAmmoType[i]->setColor(Palette::blockOffset(14)+15);
			_txtAmmoType[i]->setAlign(ALIGN_CENTER);
			_txtAmmoType[i]->setVerticalAlign(ALIGN_MIDDLE);
			_txtAmmoType[i]->setWordWrap(true);

			_txtAmmoDamage[i] = new Text(82, 17, 194, 40 + i*49);
			add(_txtAmmoDamage[i]);
			_txtAmmoDamage[i]->setColor(Palette::blockOffset(2));
			_txtAmmoDamage[i]->setAlign(ALIGN_CENTER);
			_txtAmmoDamage[i]->setBig();

			_imageAmmo[i] = new Surface(32, 48, 280, 16 + i*49);
			add(_imageAmmo[i]);
		}

		switch (item->getBattleType())
		{
			case BT_FIREARM:
				_txtDamage = new Text(82, 10, 194, 7);
				add(_txtDamage);
				_txtDamage->setColor(Palette::blockOffset(14)+15);
				_txtDamage->setAlign(ALIGN_CENTER);
				_txtDamage->setText(tr("STR_DAMAGE_UC"));

				_txtAmmo = new Text(50, 10, 268, 7);
				add(_txtAmmo);
				_txtAmmo->setColor(Palette::blockOffset(14)+15);
				_txtAmmo->setAlign(ALIGN_CENTER);
				_txtAmmo->setText(tr("STR_AMMO"));

				if (ammo_data->empty())
				{
					_txtAmmoType[0]->setText(tr(getDamageTypeText(item->getDamageType())));

					ss.str(L"");ss.clear();
					ss << item->getPower();
					if (item->getShotgunPellets())
					{
						ss << L"x" << item->getShotgunPellets();
					}
					// display ammo clip size
					if (item->getClipSize() > 1)
					{
						ss << L" C" << item->getClipSize() ;
					}
					// display explosion radius 
					if (item->getExplosionRadius())
					{
						ss << L" R" << item->getExplosionRadius();
					}
					_txtAmmoDamage[0]->setText(ss.str());
				}
				else
				{
					for (size_t i = 0; i < std::min(ammo_data->size(), (size_t)3); ++i)
					{
						ArticleDefinition *ammo_article = _game->getRuleset()->getUfopaediaArticle((*ammo_data)[i]);
						if (Ufopaedia::isArticleAvailable(_game->getSavedGame(), ammo_article))
						{
							RuleItem *ammo_rule = _game->getRuleset()->getItem((*ammo_data)[i]);
							_txtAmmoType[i]->setText(tr(getDamageTypeText(ammo_rule->getDamageType())));

							ss.str(L"");ss.clear();
							ss << ammo_rule->getPower();
							if (ammo_rule->getShotgunPellets())
							{
								ss << L"x" << ammo_rule->getShotgunPellets();
							}
							// display ammo clip size
							if (ammo_rule->getClipSize() > 1)
							{
								ss << L" C" << ammo_rule->getClipSize();
							}
							// display explosion radius
							if (ammo_rule->getExplosionRadius())
							{
								ss << L" R" << ammo_rule->getExplosionRadius();
							}
							_txtAmmoDamage[i]->setText(ss.str());

							ammo_rule->drawHandSprite(_game->getResourcePack()->getSurfaceSet("BIGOBS.PCK"), _imageAmmo[i]);
						}
					}
				}
				break;
			case BT_AMMO:
			case BT_GRENADE:
			case BT_PROXIMITYGRENADE:
			case BT_MELEE:
				_txtDamage = new Text(82, 10, 194, 7);
				add(_txtDamage);
				_txtDamage->setColor(Palette::blockOffset(14)+15);
				_txtDamage->setAlign(ALIGN_CENTER);
				_txtDamage->setText(tr("STR_DAMAGE_UC"));
				_txtAmmoType[0]->setText(tr(getDamageTypeText(item->getDamageType())));

				ss.str(L"");ss.clear();
				ss << item->getPower();
				
				// display ammo clip size
				if (item->getClipSize() > 1)
				{
					ss << L" C" << item->getClipSize();
				}
				// display explosion radius for grenades
				if (item->getExplosionRadius())
				{
					ss << L" R" << item->getExplosionRadius();
				}
				_txtAmmoDamage[0]->setText(ss.str());
				break;
			default: break;
		}

		centerAllSurfaces();
	}

	ArticleStateItem::~ArticleStateItem()
	{}

}
