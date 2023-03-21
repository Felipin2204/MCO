//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "InterferenceMonitor.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IReception.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/FlatReceptionBase.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211ScalarTransmission.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Radio.h"


using namespace omnetpp;

namespace inet {

Define_Module(InterferenceMonitor);

void InterferenceMonitor::initialize(int stage) {
    if(stage == INITSTAGE_LOCAL) {
        numChannels = par("numChannels");
        myNode = getParentModule()->getParentModule()->getIndex();
    } else if(stage == INITSTAGE_LINK_LAYER) {
        for (int i = 0; i < numChannels; i++) {
            std::string r = "^.^.wlan[" + std::to_string(i) + "].radio";
            auto radio = check_and_cast<cComponent*>(getModuleByPath(r.c_str()));
            radios.push_back(radio);
        }
        getModuleByPath("<root>.radioMedium")->subscribe("signalArrivalStarted", this);
    }
}

void InterferenceMonitor::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) {
    //TODO: Remove the part 'getRadioGate()->getOwnerModule()'
    //Reception
    auto reception = check_and_cast<const physicallayer::FlatReceptionBase*>(obj);
    auto radioRx = reception->getReceiver();
    //We are assuming that the index of the radio module always coincide with the channel used
    int rIndex = radioRx->getRadioGate()->getOwnerModule()->getParentModule()->getIndex();

    //Power tests
    W minReceptionPower = reception->computeMinPower(reception->getStartTime(), reception->getEndTime());
    ASSERT(W(0.0) <= minReceptionPower);

    //TODO: Should we use minInterferencePower or minReceptionPower
    if (minReceptionPower < radioRx->getReceiver()->getMinInterferencePower() ) {
        //No interference for this signal on this channel, too weak
        return;
    }

    //Transmission
    auto transmission = check_and_cast<const physicallayer::Ieee80211ScalarTransmission*>(reception->getTransmission());
    auto radioTx = transmission->getTransmitter()->getRadioGate()->getOwnerModule();
    int channel = transmission->getChannel()->getChannelNumber();
    int nodeIndex = radioTx->getParentModule()->getParentModule()->getIndex();

    std::cout << "node[" << myNode << "]: Signal received on channel " << rIndex << " from node[" << nodeIndex << "] on channel " << channel << endl;

    physicallayer::ICommunicationCache* comCache = const_cast<physicallayer::ICommunicationCache*>(radioRx->getMedium()->getCommunicationCache());
    auto interference = comCache->computeInterferingTransmissions(radioRx, reception->getStartTime(), reception->getEndTime());
    for (const auto i : *interference) {
        auto txi = i->getTransmitter()->getRadioGate()->getOwnerModule();
        int nodeIndexI = txi->getParentModule()->getParentModule()->getIndex();
        auto ieeeI = check_and_cast<const physicallayer::Ieee80211ScalarTransmission*>(i);
        int channelI = ieeeI->getChannel()->getChannelNumber();
        std::cout << "node[" << myNode << "]: transmission in progress from node[" << nodeIndexI << "] on channel " << channelI << endl;
        if (channelI != channel) {
            std::cout << "Interference from channel " << channel << " on channel " << channelI << endl;
        }
    }
}

} //namespace inet
