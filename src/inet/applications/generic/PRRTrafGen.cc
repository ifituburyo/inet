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

#include "PRRTrafGen.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/networklayer/contract/INetworkProtocolControlInfo.h"

namespace inet {

Define_Module(PRRTrafGen);

simsignal_t PRRTrafGen::sinkRcvdPkSignal = registerSignal("sinkRcvdPk");
simsignal_t PRRTrafGen::sentDummyPkSignal = registerSignal("sentDummyPk");
simsignal_t PRRTrafGen::intermediatePRRSignal = registerSignal("intermediatePRR");
int PRRTrafGen::initializedCount = 0;
int PRRTrafGen::finishedCount = 0;
int PRRTrafGen::receivedCurrentInterval = 0;
int PRRTrafGen::sentCurrentInterval = 0;
int PRRTrafGen::receivedPreviousInterval = 0;
int PRRTrafGen::sentPreviousInterval = 0;
cMessage *PRRTrafGen::intermediatePRRTimer = 0;

PRRTrafGen::PRRTrafGen()
{
}

PRRTrafGen::~PRRTrafGen()
{
    cancelAndDelete(shutdownTimer);

    if(intermediatePRRTimer != nullptr) {
        cancelAndDelete(intermediatePRRTimer);
        intermediatePRRTimer = nullptr;
    }
}

void PRRTrafGen::initialize(int stage)
{
    IPvXTrafGen::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        initializedCount++;
        warmUpDuration = par("warmUpDuration");
        coolDownDuration = par("coolDownDuration");
        continueSendingDummyPackets = par("continueSendingDummyPackets");
        intermediatePRRInterval = par("intermediatePRRInterval");

        // subscribe to sink signal
        std::string signalName = extractHostName(this->getFullPath());
        getSimulation()->getSystemModule()->subscribe(signalName.c_str(), this);
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        shutdownTimer = new cMessage("shutdownTimer");

        if(intermediatePRRTimer == nullptr) { // only one node shall handle these events
            intermediatePRRTimer = new cMessage("intermediatePRRTimer");
            scheduleAt(simTime()+intermediatePRRInterval, intermediatePRRTimer);
        }
    }
}

void PRRTrafGen::receiveSignal(cComponent *prev, simsignal_t t, cObject *obj DETAILS_ARG) {
    unsigned int num = atoi(((cPacket*)obj)->getName()+strlen("appData-"));

    if(packetReceived.size() < num+1) {
        packetReceived.resize(num+1,false);
    }

    if(!packetReceived[num]) { // handle duplicates
        packetReceived[num] = true;
        receivedCurrentInterval++;
        emit(sinkRcvdPkSignal, obj);
    }
}

bool PRRTrafGen::isEnabled()
{
    if(!finished && numPackets != -1 && numSent >= numPackets) {
        finished = true;
        finishedCount++;

        if(finishedCount >= initializedCount) {
            scheduleAt(simTime()+coolDownDuration, shutdownTimer);
        }
    }

    return (numPackets == -1 || numSent < numPackets || continueSendingDummyPackets);
}

void PRRTrafGen::handleMessage(cMessage *msg)
{
    if(msg == shutdownTimer) {
        endSimulation();
    }
    else if(msg == intermediatePRRTimer) {
        emit(intermediatePRRSignal, (receivedPreviousInterval + receivedCurrentInterval) / (double)(sentPreviousInterval + sentCurrentInterval));
        receivedPreviousInterval = receivedCurrentInterval;
        sentPreviousInterval = sentCurrentInterval;
        receivedCurrentInterval = 0;
        sentCurrentInterval = 0;
        scheduleAt(simTime()+intermediatePRRInterval, intermediatePRRTimer);
    }
    else {
        IPvXTrafGen::handleMessage(msg);
    }
}


void PRRTrafGen::sendPacket()
{
    char msgName[32];
    sprintf(msgName, "appData-%d", numSent);

    cPacket *payload = new cPacket(msgName);
    payload->setByteLength(packetLengthPar->longValue());

    simtime_t now = simTime();
    bool dummy = now < startTime+warmUpDuration || (numPackets != -1 && numSent >= numPackets);
    payload->addPar("dummy") = dummy;

    L3Address destAddr = chooseDestAddr();

    IL3AddressType *addressType = destAddr.getAddressType();
    INetworkProtocolControlInfo *controlInfo = addressType->createNetworkProtocolControlInfo();
    //controlInfo->setSourceAddress();
    controlInfo->setDestinationAddress(destAddr);
    controlInfo->setTransportProtocol(protocol);
    payload->setControlInfo(check_and_cast<cObject *>(controlInfo));

    sentCurrentInterval++;

    if(!dummy) {
        EV_INFO << "Sending packet: ";
        printPacket(payload);
        emit(sentPkSignal, payload);
        send(payload, "ipOut");
        numSent++;
    }
    else {
        EV_INFO << "Sending dummy packet: ";
        printPacket(payload);
        emit(sentDummyPkSignal, payload);
        send(payload, "ipOut");
    }
}

std::string PRRTrafGen::extractHostName(const std::string& sourceName) {
    std::string signalName = "";
    std::size_t hostStart = sourceName.find("host[");
    if (hostStart != std::string::npos) {
        std::size_t hostEnd = sourceName.find("]", hostStart);
        if (hostEnd != std::string::npos) {
            std::stringstream s;
            s << "rcvdPkFrom-host[" << sourceName.substr(hostStart + 5, hostEnd - hostStart - 5) << "]";
            signalName = s.str();
        }
    }
    return signalName;
}

void PRRTrafGen::processPacket(cPacket *msg)
{
    // Throw away dummy packets
    if(msg->par("dummy")) {
        delete msg;
        return;
    }

    // Emit rcvdPkFrom signal
    INetworkProtocolControlInfo *ctrl = dynamic_cast<INetworkProtocolControlInfo *>(msg->getControlInfo());
    if (ctrl != nullptr) {
        auto it = rcvdPkFromSignals.find(ctrl->getSourceAddress());
        if(it == rcvdPkFromSignals.end()) {
            std::string signalName = extractHostName(ctrl->getSourceAddress().str());
            auto signal = registerSignal(signalName.c_str());

            cProperty *statisticTemplate =
                getProperties()->get("statisticTemplate", "rcvdPkFrom");
            getSimulation()->getActiveEnvir()->addResultRecorders(this, signal, signalName.c_str(), statisticTemplate);

            rcvdPkFromSignals[ctrl->getSourceAddress()] = signal;
            it = rcvdPkFromSignals.find(ctrl->getSourceAddress());
        }

        emit(it->second, msg);
    }

    IPvXTrafGen::processPacket(msg);
}

} // namespace inet

