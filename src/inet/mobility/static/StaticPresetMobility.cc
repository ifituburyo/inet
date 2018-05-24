/*
 * Copyright (C) 2014 Florian Meier <florian.meier@koalo.de>
 *
 * Based on:
 * Copyright (C) 2006 Isabel Dietrich <isabel.dietrich@informatik.uni-erlangen.de>
 * Copyright (C) 2013 OpenSim Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "StaticPresetMobility.h"

namespace inet {

Define_Module(StaticPresetMobility);


void StaticPresetMobility::setInitialPosition()
{
    double posX = par("posX");
    double posY = par("posY");
    double posZ = par("posZ");

    lastPosition.x = posX;
    lastPosition.y = posY;
    lastPosition.z = posZ;
   
    if(isOutside()) {
        lastPosition.z = -1;
    }    
         
    recordScalar("x", lastPosition.x);
    recordScalar("y", lastPosition.y);
    recordScalar("z", lastPosition.z);
}

} // namespace inet

