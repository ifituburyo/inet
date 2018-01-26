//
// Copyright (C) 2016 Florian Kauer <florian.kauer@koalo.de>
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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

#ifndef __INET_PRRTRAFGEN_H
#define __INET_PRRTRAFGEN_H

#include <vector>

#include "inet/common/INETDefs.h"

#include "inet/networklayer/common/L3Address.h"
#include "inet/applications/generic/IpvxTrafGen.h"

namespace inet {

/**
 * IP traffic generator application for measuring PRR.
 */
class INET_API PRRTrafGen : public IpvxTrafGen, public cIListener
{
  protected:
    // statistic
    static simsignal_t sinkRcvdPkSignal;
    static simsignal_t sentDummyPkSignal;
    std::map<L3Address,simsignal_t> rcvdPkFromSignals;

    static int initializedCount;
    static int finishedCount;
    bool finished = false;

    simtime_t warmUpDuration;
    simtime_t coolDownDuration;
    bool continueSendingDummyPackets;
    cMessage *shutdownTimer = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(cPacket *msg) override;
    virtual void sendPacket() override;
    virtual bool isEnabled() override;
    virtual void handleMessage(cMessage *msg) override;

    virtual void receiveSignal(cComponent *prev, simsignal_t t, bool b DETAILS_ARG) override {
    }

    virtual void receiveSignal(cComponent *prev, simsignal_t t, long l DETAILS_ARG) override {
    }

    virtual void receiveSignal(cComponent *prev, simsignal_t t, unsigned long l DETAILS_ARG) override {
    }

    virtual void receiveSignal(cComponent *prev, simsignal_t t, double d DETAILS_ARG) override {
    }

    virtual void receiveSignal(cComponent *prev, simsignal_t t, const SimTime& v DETAILS_ARG) override {
    }

    virtual void receiveSignal(cComponent *prev, simsignal_t t, const char *s DETAILS_ARG) override {
    }

    virtual void receiveSignal(cComponent *prev, simsignal_t t, cObject *obj DETAILS_ARG) override;

    std::vector<bool> packetReceived;

  public:
    PRRTrafGen();
    virtual ~PRRTrafGen();

private:
    std::string extractHostName(const std::string& sourceName);
};

} // namespace inet

#endif // ifndef __INET_PRRTRAFGEN_H

