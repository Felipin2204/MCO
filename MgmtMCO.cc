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

#include <string>
#include "MgmtMCO.h"
#include "inet/common/ModuleAccess.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Radio.h"
#include "inet/common/Simsignals.h"
#include "TrafficPacket_m.h"


namespace inet {

Define_Module(MgmtMCO);

void MgmtMCO::initialize(int stage)
{
    if(stage == INITSTAGE_LOCAL) {
        numNIC = getModuleByPath("^.^")->par("numNIC").intValue();
    } else if(stage == INITSTAGE_LINK_LAYER) {
        for (int i = 0; i < numNIC; i++) {
            queues.push_back(findConnectedModule<queueing::PacketQueue>(gate("inQueue", i)));

            std::string aux = "^.^.wlan[" + std::to_string(i) + "].mac.dcf.channelAccess";
            macDcafs.push_back(check_and_cast<ieee80211::Dcaf*>(getModuleByPath(aux.c_str())));

            //Set the channel of the radios
            aux = "^.^.wlan[" + std::to_string(i) + "].radio";
            physicallayer::Ieee80211Radio *radio = check_and_cast<physicallayer::Ieee80211Radio*>(getModuleByPath(aux.c_str()));
            radio->setChannelNumber(i);
        }
        //MCO module is subscribed to this signal which is emitted when a packet is pushed into one of the queues of the MCO
        getParentModule()->subscribe(packetPushEndedSignal, this);

        //TrafficGeneratorHost (node) module is subscribed to this other signal which is emitted when a transmission is finished
        getModuleByPath("^.^")->subscribe(transmissionEndedSignal, this);
    }
}

void MgmtMCO::handleMessage(cMessage *msg)
{
    if(msg->arrivedOn("inWLAN")) {
        Packet *pkt = static_cast<Packet*>(msg);
        auto p = pkt->peekData<TrafficPacket>();
        send(msg, findGate("outApp", p->getAppIdentifier()));
    }
}

void MgmtMCO::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) {
    if(signalID == packetPushEndedSignal) {
        queueing::PacketQueue *queue = check_and_cast<queueing::PacketQueue*>(source);
        int indexQueue;
        if(queue->isVector()) indexQueue = queue->getIndex(); //Required for the MCOSequencialFilling case
        else indexQueue = numNIC-1;

        //Pull packet if the Dcf don't have a frame to transmit(Dcf::hasFrameToTransmit)
        if(macDcafs[indexQueue]->getPendingQueue()->isEmpty() && !macDcafs[indexQueue]->getInProgressFrames()->hasInProgressFrames()) {
            auto packet = queue->pullPacket(nullptr);
            take(packet);
            EV_INFO << "Collecting packet" << EV_FIELD(packet) << EV_ENDL;
            send(packet, gate("outWLAN", indexQueue));
        }
    } else if (signalID == transmissionEndedSignal) {
        int indexWLAN = source->getParentModule()->getIndex();
        //Pull packet if the corresponding queue is non empty
        if(!queues[indexWLAN]->isEmpty()) {
            auto packet = queues[indexWLAN]->pullPacket(nullptr);
            take(packet);
            EV_INFO << "Collecting packet" << EV_FIELD(packet) << EV_ENDL;
            send(packet, gate("outWLAN", indexWLAN));
        }
    }
}

} /* namespace inet */
