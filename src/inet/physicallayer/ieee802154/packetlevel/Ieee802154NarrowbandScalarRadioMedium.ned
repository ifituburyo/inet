//
// Copyright (C) 2014 Florian Meier
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

package inet.physicallayer.ieee802154.packetlevel;

import inet.physicallayer.common.packetlevel.RadioMedium;

module Ieee802154NarrowbandScalarRadioMedium extends RadioMedium
{
    parameters:
        propagationType = default("ConstantSpeedPropagation");
        analogModelType = default("ScalarAnalogModel");
        backgroundNoiseType = default("IsotropicScalarBackgroundNoise");

        mediumLimitCache.carrierFrequency = 2450 MHz;

        // 802.15.4-2006, page 266
        pathLossType = default("BreakpointPathLoss");
        pathLoss.breakpointDistance = 8 m;
        pathLoss.l01 = 40.2;
        pathLoss.alpha1 = 2;
        pathLoss.l02 = 58.5;
        pathLoss.alpha2 = 3.3;

        // Reflects thermal noise and other noise sources of the
        // receiver circuit.
        //
        // Sensitivity is specified as -100 dBm and defined as
        // PER of 1% for a PSDU of 20 octets.
        // The PSDU excludes preamble, SFD and PHR, but the latter
        // two are also relevant for the PER, so the total length
        // for the calculation is 22 octets.
        // For 22 octets, 1% PER is achieved at 0.44 dB SNIR,
        // so with a specified sensitivity of -100 dBm this results
        // in a noise power of -100.44 dBm.
        backgroundNoise.power = default(-100.44dBm);
        backgroundNoise.dimensions = default("time");
}
