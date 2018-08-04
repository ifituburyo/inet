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

#include "inet/physicallayer/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/base/packetlevel/FlatReceiverBase.h"
#include "inet/physicallayer/base/packetlevel/FlatTransmissionBase.h"
#include "inet/physicallayer/base/packetlevel/FlatReceptionBase.h"
#include "inet/physicallayer/base/packetlevel/NarrowbandNoiseBase.h"
#include "inet/physicallayer/common/packetlevel/ListeningDecision.h"
#include "inet/physicallayer/common/packetlevel/ReceptionDecision.h"
#include "inet/physicallayer/common/packetlevel/SignalTag_m.h"
#include "inet/physicallayer/ieee802154/packetlevel/Ieee802154NarrowbandScalarReceiver.h"
#include "inet/physicallayer/ieee802154/packetlevel/Ieee802154NarrowbandScalarTransmitter.h"
#include "inet/linklayer/ieee802154/Ieee802154MacHeader_m.h"

namespace inet {

namespace physicallayer {

FlatReceiverBase::FlatReceiverBase() :
    NarrowbandReceiverBase(),
    errorModel(nullptr),
    energyDetection(W(NaN)),
    sensitivity(W(NaN))
{
}

void FlatReceiverBase::initialize(int stage)
{
    NarrowbandReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        errorModel = dynamic_cast<IErrorModel *>(getSubmodule("errorModel"));
        energyDetection = mW(math::dBm2mW(par("energyDetection")));
        sensitivity = mW(math::dBm2mW(par("sensitivity")));
    }
}

std::ostream& FlatReceiverBase::printToStream(std::ostream& stream, int level) const
{
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", errorModel = " << printObjectToString(errorModel, level + 1);
    if (level <= PRINT_LEVEL_INFO)
        stream << ", energyDetection = " << energyDetection
               << ", sensitivity = " << sensitivity;
    return NarrowbandReceiverBase::printToStream(stream, level);
}

const IListeningDecision *FlatReceiverBase::computeListeningDecision(const IListening *listening, const IInterference *interference) const
{
    const IRadio *receiver = listening->getReceiver();
    const IRadioMedium *radioMedium = receiver->getMedium();
    const IAnalogModel *analogModel = radioMedium->getAnalogModel();
    const INoise *noise = analogModel->computeNoise(listening, interference);
    const NarrowbandNoiseBase *narrowbandNoise = check_and_cast<const NarrowbandNoiseBase *>(noise);
    W maxPower = narrowbandNoise->computeMaxPower(listening->getStartTime(), listening->getEndTime());
    bool isListeningPossible = maxPower >= energyDetection;
    delete noise;
    EV_DEBUG << "Computing whether listening is possible: maximum power = " << maxPower << ", energy detection = " << energyDetection << " -> listening is " << (isListeningPossible ? "possible" : "impossible") << endl;
    return new ListeningDecision(listening, isListeningPossible);
}

// TODO: this is not purely functional, see interface comment
bool FlatReceiverBase::computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const
{
    if (!NarrowbandReceiverBase::computeIsReceptionPossible(listening, reception, part))
        return false;
    else {
        const FlatReceptionBase *flatReception = check_and_cast<const FlatReceptionBase *>(reception);
        W minReceptionPower = flatReception->computeMinPower(reception->getStartTime(part), reception->getEndTime(part));
        bool isReceptionPossible = minReceptionPower >= sensitivity;
        EV_DEBUG << "Computing whether reception is possible: minimum reception power = " << minReceptionPower << ", sensitivity = " << sensitivity << " -> reception is " << (isReceptionPossible ? "possible" : "impossible") << endl;
        return isReceptionPossible;
    }
}

int extractHostId(const std::string& path) {
    std::string signalName = "";
    std::size_t hostStart = path.find("host[");
    assert(hostStart != std::string::npos);
    std::size_t hostEnd = path.find("]", hostStart);
    assert(hostEnd != std::string::npos);
    std::stringstream s;
    int id = atoi(path.substr(hostStart + 5, hostEnd - hostStart - 5).c_str());
    return id;
}

double secondsToUBP(const omnetpp::SimTime simtime) {
    double seconds = simtime.dbl();
    seconds *= 1000; // ms
    seconds *= 1000; // us
    seconds /= 16; // 16 us
    seconds /= 20; // unit backoff periods
    return seconds;
}

bool FlatReceiverBase::computeIsReceptionSuccessful(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir) const
{
#if 0
    auto macHdr = reception->getTransmission()->getPacket()->peekAtFront<Ieee802154MacHeader>();
    int src = macHdr->getSrcAddr().getAddressByte(5)-1;
    int dst = macHdr->getDestAddr().getAddressByte(5)-1;
    int receiver = extractHostId((dynamic_cast<const Ieee802154NarrowbandScalarReceiver*>(reception->getReceiver()->getReceiver()))->getFullPath());

    double originalBegin = secondsToUBP(reception->getStartTime());
    double originalDuration = secondsToUBP(reception->getDuration());
    bool originalIsAck = originalDuration < 5;

    if(dst == receiver) {
        if(originalIsAck) {
            const_cast<FlatReceiverBase*>(this)->totalACKReceptions[src]++;
        }
        else {
            const_cast<FlatReceiverBase*>(this)->totalPacketReceptions[src]++;
        }
    }

    if(interference->getInterferingReceptions()->size() > 0) {
        for(auto& rec : *(interference->getInterferingReceptions())) {
//EV_ERROR << (inet::physicallayer::FlatRadioBase*)(rec->getReceiver()) << " Interferer " << ((rec->getTransmission())) << endl;
            //EV_ERROR << (dynamic_cast<const Ieee802154NarrowbandScalarReceiver*>(rec->getReceiver()->getReceiver()))->getFullPath() << endl;
            int receiverB = extractHostId((dynamic_cast<const Ieee802154NarrowbandScalarReceiver*>(rec->getReceiver()->getReceiver()))->getFullPath());
            assert(receiver == receiverB);
            while(receiver != receiverB) {
                EV_ERROR << "GGG" << endl;
            }

            int interferingTransmitter = extractHostId((dynamic_cast<const Ieee802154NarrowbandScalarTransmitter*>(rec->getTransmission()->getTransmitter()->getTransmitter()))->getFullPath());
            int originalTransmitter = extractHostId((dynamic_cast<const Ieee802154NarrowbandScalarTransmitter*>(reception->getTransmission()->getTransmitter()->getTransmitter()))->getFullPath());
            if(dst != receiver) {
                return false; // will be dropped later anyway
            }
            EV_ERROR << dst << endl;
            //EV_ERROR << reception->getDuration() << " " << rec->getDuration() << " " << secondsToSymbols(reception->getStartTime()) << " " << secondsToSymbols(rec->getStartTime()) << endl;
            double interferenceBegin = secondsToUBP(rec->getStartTime());
            double interferenceDuration = secondsToUBP(rec->getDuration());
            double diff = interferenceBegin-originalBegin;
            EV_ERROR << "diff " << diff << " " << originalDuration << " " << interferenceDuration << endl;
            EV_ERROR << "Interference receiver " << receiver << " original transmitter " << originalTransmitter << " interfering transmitter " << interferingTransmitter << " ";

            bool interferenceIsAck = interferenceDuration < 5;

            if(originalIsAck) {
                EV_ERROR << "origACK ";
            }
            else {
                EV_ERROR << "origPKT ";
            }

            if(interferenceIsAck) {
                EV_ERROR << "intACK ";
            }
            else {
                EV_ERROR << "intPKT ";
            }

            bool pm2 = diff >= -2 && diff <= 2;
            if(pm2) {
                EV_ERROR << " short";
            }
            else {
                EV_ERROR << " long";
            }


            EV_ERROR << endl;

            if(dst == receiver) {
                if(!originalIsAck && !interferenceIsAck) {
                    const_cast<FlatReceiverBase*>(this)->collisionWithPacket[src]++;
                }

                if(!originalIsAck && interferenceIsAck) {
                    const_cast<FlatReceiverBase*>(this)->collisionWithACK[src]++;
                }
            }

            EV_ERROR << totalPacketReceptions[src] << " " << collisionWithPacket[src]/(double)totalPacketReceptions[src] << " " << collisionWithACK[src]/(double)totalPacketReceptions[src] << endl;
            return false; // TODO ?
            //return true;
        }
        return false;
    }

    //return true; // TODO
    return true; // TODO
#endif

    if (!SnirReceiverBase::computeIsReceptionSuccessful(listening, reception, part, interference, snir))
        return false;
    else if (!errorModel)
        return true;
    else {
        double packetErrorRate = errorModel->computePacketErrorRate(snir, part);
        if (packetErrorRate == 0.0)
            return true;
        else if (packetErrorRate == 1.0)
            return false;
        else
            return dblrand() > packetErrorRate;
    }
}

const IReceptionResult *FlatReceiverBase::computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISnir *snir, const std::vector<const IReceptionDecision *> *decisions) const
{
    auto receptionResult = NarrowbandReceiverBase::computeReceptionResult(listening, reception, interference, snir, decisions);
    auto errorRateInd = const_cast<Packet *>(receptionResult->getPacket())->addTagIfAbsent<ErrorRateInd>();
    errorRateInd->setPacketErrorRate(errorModel ? errorModel->computePacketErrorRate(snir, IRadioSignal::SIGNAL_PART_WHOLE) : 0.0);
    errorRateInd->setBitErrorRate(errorModel ? errorModel->computeBitErrorRate(snir, IRadioSignal::SIGNAL_PART_WHOLE) : 0.0);
    errorRateInd->setSymbolErrorRate(errorModel ? errorModel->computeSymbolErrorRate(snir, IRadioSignal::SIGNAL_PART_WHOLE) : 0.0);
    return receptionResult;
}

} // namespace physicallayer

} // namespace inet

