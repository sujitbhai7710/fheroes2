/***************************************************************************
 *   fheroes2: https://github.com/ihhub/fheroes2                           *
 *   Copyright (C) 2019 - 2023                                             *
 *                                                                         *
 *   Free Heroes2 Engine: http://sourceforge.net/projects/fheroes2         *
 *   Copyright (C) 2009 by Andrey Afletdinov <fheroes2@gmail.com>          *
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

#include "ground.h"

#include <cassert>

#include "maps_tiles.h"
#include "rand.h"
#include "skill.h"
#include "translations.h"

const char * Maps::Ground::String( int groundId )
{
    switch ( groundId ) {
    case DESERT:
        return _( "Desert" );
    case SNOW:
        return _( "Snow" );
    case SWAMP:
        return _( "Swamp" );
    case WASTELAND:
        return _( "Wasteland" );
    case BEACH:
        return _( "Beach" );
    case LAVA:
        return _( "Lava" );
    case DIRT:
        return _( "Dirt" );
    case GRASS:
        return _( "Grass" );
    case WATER:
        return _( "Ocean" );
    default:
        return "Unknown";
    }
}

uint32_t Maps::Ground::GetPenalty( const Maps::Tiles & tile, uint32_t level )
{
    //              none   basc   advd   expr
    //    Desert    2.00   1.75   1.50   1.00
    //    Swamp     1.75   1.50   1.25   1.00
    //    Snow      1.50   1.25   1.00   1.00
    //    Wasteland 1.25   1.00   1.00   1.00
    //    Beach     1.25   1.00   1.00   1.00
    //    Lava      1.00   1.00   1.00   1.00
    //    Dirt      1.00   1.00   1.00   1.00
    //    Grass     1.00   1.00   1.00   1.00
    //    Water     1.00   1.00   1.00   1.00
    //    Road      0.75   0.75   0.75   0.75

    uint32_t result = defaultGroundPenalty;

    switch ( tile.GetGround() ) {
    case DESERT:
        switch ( level ) {
        case Skill::Level::EXPERT:
            break;
        case Skill::Level::ADVANCED:
            result += 50;
            break;
        case Skill::Level::BASIC:
            result += 75;
            break;
        default:
            result += 100;
            break;
        }
        break;

    case SWAMP:
        switch ( level ) {
        case Skill::Level::EXPERT:
            break;
        case Skill::Level::ADVANCED:
            result += 25;
            break;
        case Skill::Level::BASIC:
            result += 50;
            break;
        default:
            result += 75;
            break;
        }
        break;

    case SNOW:
        switch ( level ) {
        case Skill::Level::EXPERT:
        case Skill::Level::ADVANCED:
            break;
        case Skill::Level::BASIC:
            result += 25;
            break;
        default:
            result += 50;
            break;
        }
        break;

    case WASTELAND:
    case BEACH:
        result += ( Skill::Level::NONE == level ? 25 : 0 );
        break;

    default:
        break;
    }

    return result;
}

uint16_t Maps::Ground::getRandomTerrainImageIndex( const int groundId )
{
    if ( groundId == WATER ) {
        return static_cast<uint16_t>( Rand::Get( 3 ) ) + 16u;
    }

    const uint16_t indexOffset = static_cast<uint16_t>( Rand::Get( 7 ) );
    switch ( groundId ) {
    case DESERT:
        return indexOffset + 300U;
    case SNOW:
        return indexOffset + 130U;
    case SWAMP:
        return indexOffset + 184U;
    case WASTELAND:
        return indexOffset + 399U;
    case BEACH:
        return indexOffset + 415U;
    case LAVA:
        return indexOffset + 246U;
    case DIRT:
        return indexOffset + 337U;
    case GRASS:
        return indexOffset + 68U;
    default:
        // Have you added a new ground? Add the logic above!
        assert( 0 );
        return 0;
    }
}

uint16_t Maps::Ground::getRandomTerrainSpecialImageIndex( const int groundId )
{
    switch ( groundId ) {
    case DESERT:
        return static_cast<uint16_t>( Rand::Get( 12 ) ) + 308U;
    case SNOW:
        return static_cast<uint16_t>( Rand::Get( 7 ) ) + 138U;
    case SWAMP:
        return static_cast<uint16_t>( Rand::Get( 15 ) ) + 192U;
    case WASTELAND:
        return static_cast<uint16_t>( Rand::Get( 7 ) ) + 407U;
    case BEACH:
        return static_cast<uint16_t>( Rand::Get( 8 ) ) + 423U;
    case LAVA:
        return static_cast<uint16_t>( Rand::Get( 7 ) ) + 254U;
    case DIRT:
        return static_cast<uint16_t>( Rand::Get( 15 ) ) + 345U;
    case GRASS:
        return static_cast<uint16_t>( Rand::Get( 15 ) ) + 76U;
    case WATER:
        // There are no extra water terrain tiles. We return normal tiles instead.
        return getRandomTerrainImageIndex( groundId );
    default:
        // Have you added a new ground? Add the logic above!
        assert( 0 );
        return 0;
    }
}
