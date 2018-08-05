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

import inet.physicallayer.base.packetlevel.FlatRadioBase;

module Ieee802154NarrowbandScalarRadio extends FlatRadioBase
{
    parameters:
        antennaType = default("IsotropicAntenna");
        transmitterType = default("Ieee802154NarrowbandScalarTransmitter");
        receiverType = default("Ieee802154NarrowbandScalarReceiver");

        carrierFrequency = 2450 MHz;

        // B_20dB ATmega256RFR2 (page 564)
        bandwidth = default(2.8 MHz);

        // 802.15.4-2006 (page 28)
        *.bitrate = default(250 kbps);

        // PHY Header (without preamble), 802.15.4-2006 (page 43)
        // 1 octet SFD
        // 7 bit Frame length
        // 1 bit Reserved
        *.headerLength = (1*8 + 7 + 1) * 1 b;

        // Preamble
        // 4 octets Preamble
        // 1 symbol of 16us -> 4 bit
        transmitter.preambleDuration = (4*8/4) * 16 us;

        // RSSI sensitivity (ATmega256RFR2, page 566)
        // Note that a busy channel is also reported if
        // a reception was started and for this the signal
        // only has to be larger than the sensitivity.
        receiver.energyDetection = default(-90dBm);

        // Minimum SNIR
        // -3.3 dB SNIR results in 98% PER for a PSDU of 20 octets.
        // Use this as cut-off since in theory,
        // there is no fixed boundary.
        // The PSDU excludes preamble, SFD and PHR, but the latter
        // two are also relevant for the PER, so the total length
        // for the calculation is 22 octets.
        receiver.snirThreshold = default(-3.3 dB);

        // Receiver sensitivity
        // While, e.g. in the datasheet of the ATmega256RFR2,
        // the sensitivity is specified as the input signal power
        // resulting in 1% PER, the sensitivity in INET is
        // in fact the minimum reception power.
        // So we use the same cut-off as for the SNIR of 98%.
        // -100.44 dBm - 3.3 dB = -103.74 dBm
        receiver.sensitivity = default(-103.74dBm);

        // Minimum interference power
        // Also very weak signals can in principle influence the
        // SNIR. However, the physcial layer of IEEE 802.15.4
        // is quite robust against interference:
        // For a signal power of -100 dBm an interfering signal
        // of -103.74 dBm = 4.23e-14 W is increasing the noise
        // power from -100.44 dBm = 9.04e-14 W to
        // 13.27e-14 W = -98.77 dBm. This results in a PER
        // of 26% instead of 1%.
        receiver.minInterferencePower = default(-103.74dBm);

        // TX Output power (typ. 3.5 dBm, ATmega256RFR2, page 564)
        transmitter.power = default(2.24mW);

        @class(FlatRadioBase);
}
