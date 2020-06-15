//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/physicallayer/analogmodel/packetlevel/ScalarNoise.h"
#include "inet/physicallayer/backgroundnoise/IsotropicScalarBackgroundNoiseJamming.h"
#include "inet/physicallayer/common/packetlevel/BandListening.h"

namespace inet {

namespace physicallayer {

Define_Module(IsotropicScalarBackgroundNoiseJamming);

const INoise *IsotropicScalarBackgroundNoiseJamming::computeNoise(const IListening *listening) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    simtime_t startTime = listening->getStartTime();
    simtime_t endTime = listening->getEndTime();
    Hz centerFrequency = bandListening->getCenterFrequency();
    Hz listeningBandwidth = bandListening->getBandwidth();
    if (std::isnan(bandwidth.get()))
        bandwidth = listeningBandwidth;
    else if (bandwidth != listeningBandwidth)
        throw cRuntimeError("Background noise bandwidth doesn't match listening bandwidth");
    std::map<simtime_t, W> *powerChanges = new std::map<simtime_t, W>();

    W tmpPower = power;
    std::string jammingFrequencies = par("jammingFrequencies").stdstringValue();
    for(auto &frequency : cStringTokenizer(jammingFrequencies.c_str()).asDoubleVector()) {
        std::cout << (MHz)frequency << std::endl;
        if(((MHz)frequency) == centerFrequency) {
            tmpPower = mW(math::dBmW2mW(par("jammingPower")));
        }
    }

    powerChanges->insert(std::pair<simtime_t, W>(startTime, tmpPower));
    powerChanges->insert(std::pair<simtime_t, W>(endTime, -tmpPower));
    return new ScalarNoise(startTime, endTime, centerFrequency, bandwidth, powerChanges);
}

} // namespace physicallayer

} // namespace inet

