/***************************************************************************
 *   fheroes2: https://github.com/ihhub/fheroes2                           *
 *   Copyright (C) 2020 - 2023                                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
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

#pragma once

#include <cstdint>
#include <list>
#include <vector>

#include "color.h"
#include "mp2.h"
#include "pathfinding.h"
#include "skill.h"

class Heroes;
class IndexObject;

namespace Route
{
    class Step;
}

struct WorldNode : public PathfindingNode<MP2::MapObjectType>
{
    // The number of movement points remaining for the hero after moving to this node
    uint32_t _remainingMovePoints = 0;

    WorldNode() = default;

    WorldNode( const int node, const uint32_t cost, const MP2::MapObjectType object, const uint32_t remainingMovePoints )
        : PathfindingNode( node, cost, object )
        , _remainingMovePoints( remainingMovePoints )
    {}

    WorldNode( const WorldNode & ) = delete;
    WorldNode( WorldNode && ) = default;

    ~WorldNode() override = default;

    WorldNode & operator=( const WorldNode & ) = delete;
    WorldNode & operator=( WorldNode && ) = default;

    void resetNode() override
    {
        PathfindingNode::resetNode();

        _remainingMovePoints = 0;
    }
};

// Abstract class that provides base functionality to path through World map
class WorldPathfinder : public Pathfinder<WorldNode>
{
public:
    WorldPathfinder() = default;
    WorldPathfinder( const WorldPathfinder & ) = delete;

    ~WorldPathfinder() override = default;

    WorldPathfinder & operator=( const WorldPathfinder & ) = delete;

    // This method resizes the cache and re-calculates map offsets if values are out of sync with World class
    virtual void checkWorldSize();

    static uint32_t calculatePathPenalty( const std::list<Route::Step> & path );

protected:
    virtual void processWorldMap();
    void checkAdjacentNodes( std::vector<int> & nodesToExplore, int currentNodeIdx );

    // Checks whether moving from the source tile in the specified direction is allowed. The default implementation
    // can be overridden by a derived class.
    virtual bool isMovementAllowed( const int from, const int direction ) const;

    // This method defines pathfinding rules. This has to be implemented by the derived class.
    virtual void processCurrentNode( std::vector<int> & nodesToExplore, const int currentNodeIdx ) = 0;

    // Calculates the movement penalty when moving from the source tile to the adjacent destination tile in the
    // specified direction. If the "last move" logic should be taken into account (when performing pathfinding
    // for a real hero on the map), then the source tile should be already accessible for this hero and it should
    // also have a valid information about the hero's remaining movement points. The default implementation can be
    // overridden by a derived class.
    virtual uint32_t getMovementPenalty( const int from, const int to, const int direction ) const;

    // Subtracts movement points taking the transition between turns into account
    uint32_t subtractMovePoints( const uint32_t movePoints, const uint32_t subtractedMovePoints ) const;

    std::vector<int> _mapOffset;

    // Hero properties should be cached here because they can change even if the hero's position does not change,
    // so it should be possible to compare the old values with the new ones to detect the need to recalculate the
    // pathfinder's cache
    int _color = Color::NONE;
    uint32_t _remainingMovePoints = 0;
    uint32_t _maxMovePoints = 0;
    uint8_t _pathfindingSkill = Skill::Level::EXPERT;
};

class PlayerWorldPathfinder final : public WorldPathfinder
{
public:
    PlayerWorldPathfinder() = default;
    PlayerWorldPathfinder( const PlayerWorldPathfinder & ) = delete;

    ~PlayerWorldPathfinder() override = default;

    PlayerWorldPathfinder & operator=( const PlayerWorldPathfinder & ) = delete;

    void reset() override;

    void reEvaluateIfNeeded( const Heroes & hero );

    // Builds and returns a path to the tile with the index 'targetIndex'. If the destination tile is not reachable,
    // then an empty path is returned.
    std::list<Route::Step> buildPath( const int targetIndex ) const;

private:
    // Follows regular passability rules (for the human player)
    void processCurrentNode( std::vector<int> & nodesToExplore, const int currentNodeIdx ) override;
};

class AIWorldPathfinder final : public WorldPathfinder
{
public:
    explicit AIWorldPathfinder( double advantage )
        : _minimalArmyStrengthAdvantage( advantage )
    {}

    AIWorldPathfinder( const AIWorldPathfinder & ) = delete;

    ~AIWorldPathfinder() override = default;

    AIWorldPathfinder & operator=( const AIWorldPathfinder & ) = delete;

    void reset() override;

    void reEvaluateIfNeeded( const Heroes & hero );
    void reEvaluateIfNeeded( const int start, const int color, const double armyStrength, const uint8_t skill );

    int getFogDiscoveryTile( const Heroes & hero, bool & isTerritoryExpansion );

    // Used for cases when heroes are stuck because one hero might be blocking the way and we have to move him.
    int getNearestTileToMove( const Heroes & hero );

    static bool isHeroPossiblyBlockingWay( const Heroes & hero );

    std::vector<IndexObject> getObjectsOnTheWay( const int targetIndex, const bool checkAdjacent = false ) const;

    std::list<Route::Step> getDimensionDoorPath( const Heroes & hero, int targetIndex ) const;

    // Builds and returns a path to the tile with the index 'targetIndex'. If there is a need to pass through any objects
    // on the way to this tile, then a path to the nearest such object is returned. If the destination tile is not reachable
    // in principle, then an empty path is returned.
    std::list<Route::Step> buildPath( const int targetIndex ) const;

    // Used for non-hero armies, like castles or monsters
    uint32_t getDistance( int start, int targetIndex, int color, double armyStrength, uint8_t skill = Skill::Level::EXPERT );
    // Faster, but does not re-evaluate the map (exposed method of the base class)
    using Pathfinder::getDistance;

    // Returns the coefficient of the minimum required advantage in army strength in order to be able to "pass through"
    // protected tiles from the AI pathfinder's point of view
    double getMinimalArmyStrengthAdvantage() const
    {
        return _minimalArmyStrengthAdvantage;
    }

    // Sets the coefficient of the minimum required advantage in army strength in order to be able to "pass through"
    // protected tiles from the AI pathfinder's point of view
    void setMinimalArmyStrengthAdvantage( const double advantage );

    // Returns the spell points reservation factor for spells associated with the movement of the hero on the adventure
    // map (such as Dimension Door, Town Gate or Town Portal)
    double getSpellPointsReserveRatio() const
    {
        return _spellPointsReserveRatio;
    }

    // Sets the spell points reservation factor for spells associated with the movement of the hero on the adventure map
    // (such as Dimension Door, Town Gate or Town Portal)
    void setSpellPointsReserveRatio( const double ratio );

private:
    void processWorldMap() override;

    // Adds special logic for AI-controlled heroes to use Summon Boat spell to overcome water obstacles (if available)
    bool isMovementAllowed( const int from, const int direction ) const override;

    // Follows custom passability rules (for the AI)
    void processCurrentNode( std::vector<int> & nodesToExplore, const int currentNodeIdx ) override;

    // Adds special logic for AI-controlled heroes to correctly calculate movement penalties when such a hero passes
    // through objects on the map or overcomes water obstacles using boats. If this logic should be taken into account
    // (when performing pathfinding for a real hero on the map), then the source tile should be already accessible for
    // this hero and it should also have a valid information about the hero's remaining movement points.
    uint32_t getMovementPenalty( const int from, const int to, const int direction ) const override;

    // Hero properties should be cached here because they can change even if the hero's position does not change,
    // so it should be possible to compare the old values with the new ones to detect the need to recalculate the
    // pathfinder's cache
    double _armyStrength{ -1 };
    bool _isArtifactsBagFull{ false };
    bool _isSummonBoatSpellAvailable{ false };

    // The potential destinations of the Town Gate and Town Portal spells should be cached here because they can
    // change even if the hero's position does not change (e.g. when a new hero was hired in the nearby castle),
    // so it should be possible to compare the old values with the new ones to detect the need to recalculate the
    // pathfinder's cache
    int32_t _townGateCastleIndex{ -1 };
    std::vector<int32_t> _townPortalCastleIndexes;

    // Coefficient of the minimum required advantage in army strength in order to be able to "pass through" protected
    // tiles from the AI pathfinder's point of view
    double _minimalArmyStrengthAdvantage{ 1.0 };

    // Spell points reservation factor for spells associated with the movement of the hero on the adventure map
    // (such as Dimension Door, Town Gate or Town Portal)
    double _spellPointsReserveRatio{ 0.5 };
};
