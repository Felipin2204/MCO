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

#include "TRSimulationMgmtMCO.h"

#include "inet/common/ModuleAccess.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
#include "inet/common/Simsignals.h"
#include "TrafficPacket_m.h"
#include "MCOPacket_m.h"
#include "TrafficGenerator.h"
#include "inet/linklayer/common/Ieee802SapTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include <string>

#define CBT_TO 1003

namespace inet {

Define_Module(TRSimulationMgmtMCO);

simsignal_t TRSimulationMgmtMCO::MCOReceivedSignal = SIMSIGNAL_NULL;
simsignal_t TRSimulationMgmtMCO::cbtSignal = SIMSIGNAL_NULL;

TRSimulationMgmtMCO::TRSimulationMgmtMCO(): info(nullptr), mob(nullptr), vehicleTable(nullptr), cbtSampleTimer(nullptr), pdrSampleTimer(nullptr) {}

TRSimulationMgmtMCO::~TRSimulationMgmtMCO() {
    if (cbtSampleTimer) cancelAndDelete(cbtSampleTimer);
    if (pdrSampleTimer) cancelAndDelete(pdrSampleTimer);
}

void TRSimulationMgmtMCO::initialize(int stage)
{
    if(stage == INITSTAGE_LOCAL) {
        std::string r = "^.^.wlan[0].radio";
        channel = getModuleByPath(r.c_str())->par("channelNumber");

        myId = getParentModule()->getParentModule()->par("id");
        MCOSent = 0;
        Coord pos(-1, -1);
        info = new TRSimulationVehicleInfo(-1, -1, pos);

        MCOReceivedSignal = registerSignal("MCOPacketReceived");

        inWlanId = gate("inWLAN", 0)->getId();
        outWlanId = gate("outWLAN", 0)->getId();

        cbtWindow = par("cbtWindow");
        cbtSignal = registerSignal("cbt");

        pdrWindow = par("pdrWindow");
        pdrUpdate = par("pdrUpdate");
        pdrDistanceStep = par("pdrDistanceStep");
        pdrNumberIntervals = par("pdrNumberIntervals");
        pdrSignals.resize(pdrNumberIntervals);

        //Statistics recording for dynamically registered signals
        for (int j = 0; j < pdrNumberIntervals; j++) {
            std::string sname("pdr");
            sname += std::to_string(j*(int)pdrDistanceStep) + "-" + std::to_string((j+1)*(int)pdrDistanceStep);
            pdrSignals[j] = registerSignal(sname.c_str());
            cProperty *statisticTemplate = getProperties()->get("statisticTemplate", "pdr");
            getEnvir()->addResultRecorders(this, pdrSignals[j], sname.c_str(), statisticTemplate);
        }

        cbtSampleTimer = new cMessage("CBT Timer", CBT_TO);
        setCbtWindow(cbtWindow);

        pdrSampleTimer = new cMessage("PDR Timer");
        scheduleAt(simTime() + pdrUpdate, pdrSampleTimer);
        nodesInPdrIntervals.resize(pdrNumberIntervals);

    } else if(stage == INITSTAGE_PHYSICAL_ENVIRONMENT) {
        mob = check_and_cast<IMobility*>(getModuleByPath("^.^.mobility"));
        vehicleTable = check_and_cast<TRSimulationVehicleTable*>(getModuleByPath("^.vehicleTable"));

        //Have to register to mobility before is initialized, otherwise we miss mobility events.
        //This signal is used to update the nodes in the PDR intervals.
        getSimulation()->getSystemModule()->subscribe(IMobility::mobilityStateChangedSignal, this);

    } else if(stage == INITSTAGE_LINK_LAYER) {
        outAppId = gate("outApp", 0)->getId();

        std::string r = "^.^.wlan[0].radio";
        radio = check_and_cast<physicallayer::Ieee80211Radio*>(getModuleByPath(r.c_str()));

        //Radio module is subscribed to this other signal which is emitted by himself when a transmission is finished
        radio->subscribe(transmissionEndedSignal, this);

        //Changed to collect all the queues in the MCOSequencialFilling case
        queue = findConnectedModule<queueing::PacketQueue>(gate("inQueue", 0));

        std::string d = "^.^.wlan[0].mac.dcf.channelAccess";
        macDcaf = check_and_cast<ieee80211::Dcaf*>(getModuleByPath(d.c_str()));

        //CBT measurement
        radio->subscribe(physicallayer::IRadio::receptionStateChangedSignal, this); //Emited by the radio
        radio->subscribe(physicallayer::IRadio::transmissionStateChangedSignal, this); //Emited by the radio

        cbt_idletime = 0.0;
        lu_idletime = simTime();
        lastReceptionState = check_and_cast<physicallayer::IRadio*>(radio)->getReceptionState();
        lastTransmissionState = check_and_cast<physicallayer::IRadio*>(radio)->getTransmissionState();

        //PDR measurement
        radio->subscribe(transmissionStartedSignal, this); //Emited by the radio

        //PDR measurement
        getSimulation()->getSystemModule()->subscribe(MCOReceivedSignal, this); //Emited in the receiveMCOPacket function

        //MCO module is subscribed to this signal which is emitted when a packet is pushed into one of the queues of the MCO
        getParentModule()->subscribe(packetPushEndedSignal, this);
    }
}

void TRSimulationMgmtMCO::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        if (msg == cbtSampleTimer) {
            getMeasuredCBT(cbtWindow.dbl());
            scheduleAt(simTime()+cbtWindow, cbtSampleTimer);
        } else if (msg == pdrSampleTimer) {
            computePDR();
            scheduleAt(simTime()+pdrUpdate, pdrSampleTimer);
        }
    } else receiveMCOPacket(msg);
}

void TRSimulationMgmtMCO::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) {
    Enter_Method("checking queues");
    if (signalID == packetPushEndedSignal) {
        //Dequeue packet if the Dcf don't have a frame to transmit(Dcf::hasFrameToTransmit)
        if (macDcaf->getPendingQueue()->isEmpty() && !macDcaf->getInProgressFrames()->hasInProgressFrames()){
            auto packet = queue->dequeuePacket();
            //take(packet);
            Packet *newpacket = createMCOPacket(packet, channel);
            EV_INFO << "Collecting packet" << EV_FIELD(packet) << " and sending to NIC" << EV_ENDL;
            delete packet;
            send(newpacket, outWlanId);
        }
    } else if (signalID == transmissionEndedSignal) {
        if (!queue->isEmpty()){
            auto packet = queue->dequeuePacket();
            //take(packet);
            Packet *newpacket = createMCOPacket(packet, channel);
            EV_INFO << "Collecting packet" << EV_FIELD(packet) << " and sending to NIC" << EV_ENDL;
            delete packet;
            send(newpacket, outWlanId);
        }
    //Here we check if the vehicles are in the PDR range
    } else if (signalID == IMobility::mobilityStateChangedSignal) {
        auto mobility = check_and_cast<IMobility*>(source);
        int channelNumber = source->getParentModule()->getSubmodule("wlan", 0)->getSubmodule("radio")->par("channelNumber");
        if (mobility != mob && channel == channelNumber) {
            double sqrd = mob->getCurrentPosition().sqrdist(mobility->getCurrentPosition());
            unsigned int k = floor(pow(sqrd, 0.5) / pdrDistanceStep);
            if (k < pdrNumberIntervals){
                //Check if previously this node was in other interval and if true remove it and then add it to the new interval
                for (int i = 0; i < nodesInPdrIntervals.size(); i++) {
                    for (auto it = nodesInPdrIntervals[i].begin(); it != nodesInPdrIntervals[i].end(); ++it) {
                        if ((*it) == source) {
                            if (i != k) {
                                nodesInPdrIntervals[i].erase(it);
                                nodesInPdrIntervals[k].push_back(source);
                            }
                            return;
                        }
                    }
                }
                nodesInPdrIntervals[k].push_back(source);
            } else {
                //Remove vehicles out of range
                for (int i = 0; i < nodesInPdrIntervals.size(); i++) {
                    for (auto it = nodesInPdrIntervals[i].begin(); it != nodesInPdrIntervals[i].end(); ++it) {
                        if ((*it) == source) {
                            nodesInPdrIntervals[i].erase(it);
                            return;
                        }
                    }
                }
            }
        }
    }
    else {
        processPDRSignal(source, signalID, obj, details);
    }
}

void TRSimulationMgmtMCO::finish() {
    computePDR();
}

void TRSimulationMgmtMCO::receiveSignal(cComponent *source, simsignal_t signal, intval_t value, cObject *details) {
    //CBT measurement
    if (signal == physicallayer::IRadio::receptionStateChangedSignal) {
        physicallayer::IRadio::ReceptionState newState = static_cast<physicallayer::IRadio::ReceptionState>(value);
        //It is actually easier to compute the CBT with the IDLE state. Anything that is not idle must be busy somehow (transmitting, receiving or busy)
        if (newState == physicallayer::IRadio::ReceptionState::RECEPTION_STATE_IDLE) lu_idletime = simTime();

        if (newState != physicallayer::IRadio::ReceptionState::RECEPTION_STATE_IDLE && lastReceptionState == physicallayer::IRadio::ReceptionState::RECEPTION_STATE_IDLE) {
            cbt_idletime += (simTime() - lu_idletime);
        }

        lastReceptionState = newState;

    } else if (signal == physicallayer::IRadio::transmissionStateChangedSignal) {
        physicallayer::IRadio::TransmissionState newState = static_cast<physicallayer::IRadio::TransmissionState>(value);
        if (newState == physicallayer::IRadio::TransmissionState::TRANSMISSION_STATE_IDLE) lu_idletime = simTime();

        if (newState != physicallayer::IRadio::TransmissionState::TRANSMISSION_STATE_IDLE && lastTransmissionState == physicallayer::IRadio::TransmissionState::TRANSMISSION_STATE_IDLE) {
            cbt_idletime += (simTime() - lu_idletime);
        }

        lastTransmissionState = newState;
    }
}

double TRSimulationMgmtMCO::getMeasuredCBT(double period) {
        if (radio->getReceptionState() == physicallayer::IRadio::ReceptionState::RECEPTION_STATE_IDLE || radio->getTransmissionState() == physicallayer::IRadio::TransmissionState::TRANSMISSION_STATE_IDLE) {
            cbt_idletime += (simTime() - lu_idletime);
            lu_idletime = simTime();
        }

        double mcbt = 1.0 - (cbt_idletime.dbl()/period);
        emit(cbtSignal, mcbt);
        cbt_idletime = 0.0;
        return mcbt;
}

void TRSimulationMgmtMCO::setCbtWindow(const simtime_t& cbtWindow, double offset) {
    Enter_Method_Silent();
    this->cbtWindow = cbtWindow;
    if (cbtSampleTimer->isScheduled()) cancelEvent(cbtSampleTimer);

    if (offset>0) scheduleAt(simTime() + offset + cbtWindow, cbtSampleTimer);
    else scheduleAt(simTime()+cbtWindow, cbtSampleTimer);
}

void TRSimulationMgmtMCO::processPDRSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) {
    //PDR Measurement
    if (signalID == transmissionStartedSignal) {
        //If there aren't vehicles in any of the intervals don't consider this transmission for PDR
        bool vehiclesAround = false;
        for (int j = 0; j < nodesInPdrIntervals.size(); j++) {
            if (!nodesInPdrIntervals[j].empty()) {
                vehiclesAround = true;
                break;
            }
        }
        if (!vehiclesAround) return;

        //Our transmission has started
        auto ct = check_and_cast<const physicallayer::ITransmission*>(obj);
        const Packet* pkt = ct->getPacket();

        auto p = pkt->peekDataAt<MCOPacket>(B(33)); //PhyHeader(5)+MACHeader(24+2(QoS))+LLCEDP(2), there is a trail afterwards

        //If the frame aggregation is activated, now in one transmission we have more than one packet and also one extra header for every packet.
        //So that if we want frame aggregation we have to change the following code to register all the packets.
        //auto p = pkt->peekDataAt<MCOPacket>(B(47)); //PhyHeader(5)+MACHeader(24+2(QoS))+MSDUSubFrameHeader(14)+LLCEDP(2), there is a trail afterwards

        TRSimulationPDR pdr;
        pdr.insertTime = simTime();

        for (int j = 0; j < nodesInPdrIntervals.size(); j++) {
            if (!nodesInPdrIntervals[j].empty()) {
                pdr.vehicles[j] = nodesInPdrIntervals[j].size();
                pdr.received[j] = 0;
            }
        }

        //Insert in map
        pdrAtChannel[p->getSequenceNumber()] = pdr;

    } else if (signalID == MCOReceivedSignal) {
        MCOReceivedInfo* info = check_and_cast<MCOReceivedInfo*>(obj);
        if (info->source == myId) {
            auto f = pdrAtChannel.find(info->sequenceNumber);
            if (f != pdrAtChannel.end()) {
                double sqrd = mob->getCurrentPosition().sqrdist(info->position);
                unsigned int k = floor(pow(sqrd, 0.5) / pdrDistanceStep);
                if (k < pdrNumberIntervals) {
                    f->second.received[k]++;
                }
            }
        }
    }
}

void TRSimulationMgmtMCO::computePDR() {
    for (auto it = pdrAtChannel.begin(); it != pdrAtChannel.end();) {
        if ((simTime()-it->second.insertTime) >= pdrWindow) {
            for (auto v = it->second.vehicles.begin(); v != it->second.vehicles.end(); v++) {
                int received = it->second.received[v->first];
                int vehicles = v->second;
                if (vehicles != 0)
                    emit(pdrSignals[v->first], ((double)received/vehicles));
            }
            it = pdrAtChannel.erase(it);
        } else ++it;
    }
}

Packet* TRSimulationMgmtMCO::createMCOPacket(Packet* packet, int channel) {
    auto data = makeShared<MCOPacket>();
    auto p = packet->peekData<TrafficPacket>();

    data->setChunkLength(B(p->getChunkLength()));
    data->setAppIdentifier(p->getAppIdentifier());
    data->setSource(myId);
    data->setSequenceNumber(MCOSent);
    MCOSent++;
    data->setChannel(channel);
    Coord pos = mob->getCurrentPosition();
    data->setPosition(pos);

    Packet *newpacket = new Packet("MCOPacket", data);

    //Add SAP. I think we may remove this
    newpacket->addTagIfAbsent<Ieee802SapReq>()->setDsap(SapCode::SAP_IP);
    newpacket->addTagIfAbsent<MacAddressReq>()->setDestAddress(MacAddress::BROADCAST_ADDRESS);
    //Should put something sensible here. Keep this to prevent LlcEpd from complaining
    newpacket->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ipv4);
    //Next tag is necessary to have a Best Effort QoS
    newpacket->addTagIfAbsent<UserPriorityReq>()->setUserPriority(0);

    return newpacket;
}

void TRSimulationMgmtMCO::receiveMCOPacket(cMessage* msg) {
    Packet *pkt = static_cast<Packet*>(msg);

    updateVehicleInfo(pkt);

    auto p = pkt->peekAtFront<MCOPacket>(pkt->getDataLength());
    MCOReceivedInfo* ci = new MCOReceivedInfo();
    ci->id = myId;
    ci->appId = p->getAppIdentifier();
    ci->source = p->getSource();
    ci->sequenceNumber = p->getSequenceNumber();
    ci->channel = p->getChannel();
    ci->position = mob->getCurrentPosition();
    emit(MCOReceivedSignal, check_and_cast<const cObject*>(ci));
    delete ci;

    EV_INFO << "Received MCOPacket from node[" << p->getSource() << "] at channel " << p->getChannel()
            << " with sequence number equals to " << p->getSequenceNumber() << ".\n";

    send(msg, outAppId);
}

void TRSimulationMgmtMCO::updateVehicleInfo(Packet* pkt) {
    auto p = pkt->peekAtFront<MCOPacket>(pkt->getDataLength());

    info->id = p->getSource();
    info->appId = p->getAppIdentifier();
    info->pos = p->getPosition();
    vehicleTable->insertOrUpdate(info);
}

} /* namespace inet */
