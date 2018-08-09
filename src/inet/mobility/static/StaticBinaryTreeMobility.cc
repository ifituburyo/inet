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

#include "StaticBinaryTreeMobility.h"

namespace inet {

Define_Module(StaticBinaryTreeMobility);


void StaticBinaryTreeMobility::setInitialPosition()
{
    unsigned int numHosts = par("numHosts");
    double distance = par("distance");
    unsigned int index = visualRepresentation->getIndex()+1;
    
    unsigned int layer = (unsigned int)log2(index); 
    unsigned int layer_nodes = 1<<layer;
    double layer_angle = (layer_nodes-1) / (double)layer;
    
    unsigned int max_layer = (unsigned int)log2(numHosts);
    unsigned int max_layer_nodes = 1<<max_layer;
    double max_layer_angle = (max_layer_nodes-1) / double(max_layer); 
    
    unsigned int max_layer_node = (index-layer_nodes) * (1<<(max_layer-layer)) + (1<<(max_layer-layer-1)) - 1;
    double self_angle = (2*max_layer_node+1) / (double)(max_layer_nodes-1) * max_layer_angle / 2.0 - max_layer_angle / 2.0;
    if(layer == max_layer) {
        self_angle = (index-layer_nodes) / (double)(layer_nodes-1) * layer_angle - layer_angle / 2.0; 
    }
 
    double initialX = par("initialX");
    double initialY = par("initialY");
    double initialZ = par("initialZ");
    lastPosition.x = initialX + distance*layer*cos(self_angle);
    lastPosition.y = initialY + distance*layer*sin(self_angle);
    lastPosition.z = initialZ;
    
    if(constraintAreaMin.x > -INFINITY) {
        lastPosition.x += constraintAreaMin.x;
    }
    if(constraintAreaMin.y > -INFINITY) {
        lastPosition.y += constraintAreaMin.y;
    }

    double jitter = par("jitter");
    if(jitter > 0) {
        lastPosition.x += (this->dblrand()-0.5)*jitter;
        lastPosition.y += (this->dblrand()-0.5)*jitter;
        lastPosition.z += (this->dblrand()-0.5)*jitter;
    }

    recordScalar("x", lastPosition.x);
    recordScalar("y", lastPosition.y);
    recordScalar("z", lastPosition.z);
}

} // namespace inet

