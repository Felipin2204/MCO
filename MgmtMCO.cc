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

#include "MgmtMCO.h"

#include "inet/common/ModuleAccess.h"
#include "inet/queueing/queue/PacketQueue.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
#include "inet/common/Simsignals.h"
#include "TrafficPacket_m.h"
#include "MCOPacket_m.h"
#include "generator/TrafficGenerator.h"
#include "inet/linklayer/common/Ieee802SapTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include <string>

#define CBT_TO 1003

namespace inet {

Define_Module(MgmtMCO);

simsignal_t MgmtMCO::MCOReceivedSignal = SIMSIGNAL_NULL;
simsignal_t MgmtMCO::droppedPacketSourceSignal = SIMSIGNAL_NULL;

MgmtMCO::MgmtMCO(): info(nullptr), mob(nullptr), vehicleTable(nullptr), cbtSampleTimer(nullptr), pdrSampleTimer(nullptr) {}

MgmtMCO::~MgmtMCO() {
    if (cbtSampleTimer) cancelAndDelete(cbtSampleTimer);
    if (pdrSampleTimer) cancelAndDelete(pdrSampleTimer);
    if (info) {delete info;}
}

void MgmtMCO::initialize(int stage)
{
    if(stage == INITSTAGE_LOCAL) {
        numChannels = par("numChannels");
        numApplications = par("numApplications");
        myId = getParentModule()->getParentModule()->par("id");
        MCOSent = 0;
        Coord pos(-1, -1);
        info = new VehicleInfo(-1, -1, -1, pos);

        MCOReceivedSignal = registerSignal("MCOPacketReceived");
        droppedPacketSourceSignal= registerSignal("droppedPacketSource");
        int baseIdIn = gateBaseId("inWLAN");
        int baseIdOut = gateBaseId("outWLAN");
        for (int i = 0; i < gateSize("inWLAN"); i++) {
            inWlanId.push_back(gate(baseIdIn+i)->getId());
            outWlanId.push_back(gate(baseIdOut+i)->getId());
        }

        cbtSignals.resize(numChannels);

        pdrWindow = par("pdrWindow");
        pdrUpdate = par("pdrUpdate");
        pdrDistanceStep = par("pdrDistanceStep");
        pdrNumberIntervals = par("pdrNumberIntervals");
        //We have a PDR signal for every channel and for every distance interval
        pdrSignals.resize(numChannels);
        ieeeModes.resize(numChannels);
        for (int i = 0; i < numChannels; i++)
            pdrSignals[i].resize(pdrNumberIntervals);

        receivedPacketCountSignals.resize(numChannels);
        receivedPacketCount.resize(numChannels);
        lastPacketCount.resize(numChannels);
        for (int i = 0; i < numChannels; i++) {
            receivedPacketCount[i] = 0;
            lastPacketCount[i] = 0;
        }

        //Statistics recording for dynamically registered signals
        for (int i = 0; i < numChannels; i++) {
            std::string tname("cbt");
            tname += std::to_string(i);
            cbtSignals[i] = registerSignal(tname.c_str());
            cProperty *statisticTemplate = getProperties()->get("statisticTemplate", "cbt");
            getEnvir()->addResultRecorders(this, cbtSignals[i], tname.c_str(), statisticTemplate);

            for (int j = 0; j < pdrNumberIntervals; j++) {
                std::string sname("pdr");
                sname += std::to_string(i) + "_" + std::to_string(j*(int)pdrDistanceStep) + "-" + std::to_string((j+1)*(int)pdrDistanceStep);
                pdrSignals[i][j] = registerSignal(sname.c_str());
                statisticTemplate = getProperties()->get("statisticTemplate", "pdr");
                getEnvir()->addResultRecorders(this, pdrSignals[i][j], sname.c_str(), statisticTemplate);
            }

            std::string pname("receivedPacketCount");
            pname += std::to_string(i);
            receivedPacketCountSignals[i] = registerSignal(pname.c_str());
            statisticTemplate = getProperties()->get("statisticTemplate", "receivedPacketCount");
            getEnvir()->addResultRecorders(this, receivedPacketCountSignals[i], pname.c_str(), statisticTemplate);
        }

        cbtSampleTimer = new cMessage("CBT Timer", CBT_TO);
        cbtWindow = par("cbtJitter");
        cbtFirstSample = true;
        setCbtWindow(cbtWindow);

        pdrSampleTimer = new cMessage("PDR Timer");
        scheduleAfter(pdrUpdate, pdrSampleTimer);
        pdrAtChannel.resize(numChannels);
        nodesInPdrIntervals.resize(numChannels);
        for(int i = 0; i < numChannels; i++) {
            nodesInPdrIntervals[i].resize(pdrNumberIntervals);
        }

    } else if(stage == INITSTAGE_PHYSICAL_ENVIRONMENT) {
        mob = check_and_cast<IMobility*>(getModuleByPath("^.^.mobility"));
        vehicleTable = check_and_cast<VehicleTable*>(getModuleByPath("^.vehicleTable"));
    } else if (stage == INITSTAGE_PHYSICAL_LAYER) {
        for (int i = 0; i < numChannels; i++) {
            //Collecting all the WLAN radios connected to this module
            std::string r = "^.^.wlan[" + std::to_string(i) + "].radio";
            auto radio = check_and_cast<cComponent*>(getModuleByPath(r.c_str()));
            radios.push_back(radio);

            //Radio module is subscribed to this other signal which is emitted by himself when a transmission is finished
            radio->subscribe(transmissionEndedSignal, this);

            radio->subscribe(transmissionStartedSignal, this); //Emitted by the radio
            //Needed to compute the bitrate and duration of packets in case it is necessary
            std::string m = "^.^.wlan[" + std::to_string(i) + "].mac";
            auto mac = check_and_cast<cComponent*>(getModuleByPath(m.c_str()));
            mac->subscribe(modesetChangedSignal, this);
            std::string rm = "^.^.wlan[" + std::to_string(i) + "].mac.rx";
            auto rxm = check_and_cast<cComponent*>(getModuleByPath(rm.c_str()));
            rxm->subscribe(packetDroppedSignal, this);
        }

    } else if(stage == INITSTAGE_LINK_LAYER) {
        int baseId = gateBaseId("outApp");
        for (int i = 0; i < gateSize("outApp"); i++) {
            int gateId = gate(baseId+i)->getId();
            int appId = check_and_cast<TrafficGenerator*>(findConnectedModule<TrafficGenerator>(gate(gateId)))->getAppId();
            outAppId.insert({appId, gateId});
        }
        WATCH_MAP(outAppId);

        for (int i = 0; i < numChannels; i++) {
            //Collecting all the queues connected to this module (this could be done the same way as we collected the radios, this is another way)
            queues.push_back(findConnectedModule<queueing::PacketQueue>(gate("inQueue", i)));

            //Collecting all the WLAN macDcafs connected to this module
            std::string d = "^.^.wlan[" + std::to_string(i) + "].mac.dcf.channelAccess";
            macDcaf.push_back(check_and_cast<ieee80211::Dcaf*>(getModuleByPath(d.c_str())));

            //CBT measurement
            radios[i]->subscribe(physicallayer::IRadio::receptionStateChangedSignal, this); //Emitted by the radio
            radios[i]->subscribe(physicallayer::IRadio::transmissionStateChangedSignal, this); //Emitted by the radio

            cbt_idletime.push_back(0.0);
            lu_idletime.push_back(simTime());
            lastReceptionState.push_back(check_and_cast<physicallayer::IRadio*>(radios[i])->getReceptionState());
            lastTransmissionState.push_back(check_and_cast<physicallayer::IRadio*>(radios[i])->getTransmissionState());

            //PDR measurement
            //std::string s = "<root>.radioMedium.communicationCache";
            std::string s = "^.^.^.radioMedium.neighborCache";
            neighborCache = check_and_cast<VehiclesNeighborCache*>(getModuleByPath(s.c_str()));
            //neighborCache->addRadio(radio); //All the radios are added to the neighborCache in Radio.cc
        }

        //PDR measurement
        getSimulation()->getSystemModule()->subscribe(MCOReceivedSignal, this); //Emitted in the receiveMCOPacket function

        //MCO module is subscribed to this signal which is emitted when a packet is pushed into one of the queues of the MCO
        getParentModule()->subscribe(packetPushEndedSignal, this);
    }
}

void MgmtMCO::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        if (msg == cbtSampleTimer) {
            for (int i = 0; i < numChannels; i++) {
                getMeasuredCBT(cbtWindow.dbl(), i);
            }
            if (cbtFirstSample) {
                cbtFirstSample = false;
                cbtWindow = par("cbtWindow");
            }
            scheduleAfter(cbtWindow, cbtSampleTimer);
        } else if (msg == pdrSampleTimer) {
            computePDR();
            scheduleAfter(pdrUpdate, pdrSampleTimer);
        }
    } else receiveMCOPacket(msg);
}

void MgmtMCO::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) {
    Enter_Method("checking queues");
    if (signalID == packetDroppedSignal) {
        Packet* pkt = check_and_cast<Packet*>(obj);
        //PacketPrinter printer;
        //printer.printPacket(std::cout, pkt);

        //const auto& ieee80211DataHeader = pkt->peekAtFront<ieee80211::Ieee80211DataHeader>();
        //std::cout<<"Data header="<<ieee80211DataHeader->getChunkLength()<<endl;

        //Since the packet comes from the MAC we still have the Ieee80211DataHeader (24B) plus the 80211Epd header (2B)
        auto p = pkt->peekDataAt<MCOPacket>(B(26));
        emit(droppedPacketSourceSignal, p->getSource());
        //if (myId==10) {
        //std::cout<<simTime()<<";"<<myId<<";source="<<p->getSource()<<";channel="<<p->getChannel()<<endl;
        //}
    }
    if (signalID == packetPushEndedSignal) {
        int qIndex = -1;
        for (auto c : queues) {
            if (source == c) {
                queueing::PacketQueue* queue = check_and_cast<queueing::PacketQueue*>(c);
                qIndex = queue->getIndex();

                //TODO: If we want to use Best Effort access category, we have to change this code.
                //Dequeue packet if the Dcf don't have a frame to transmit(Dcf::hasFrameToTransmit)
                if (macDcaf[qIndex]->getPendingQueue()->isEmpty() && !macDcaf[qIndex]->getInProgressFrames()->hasInProgressFrames()){
                    auto packet = queue->dequeuePacket();
                    //take(packet);
                    Packet *newpacket = createMCOPacket(packet, qIndex);
                    EV_INFO << "Collecting packet" << EV_FIELD(packet) << " and sending to NIC" << EV_ENDL;
                    delete packet;
                    send(newpacket, outWlanId[qIndex]);
                }
                break;
            }
        }
    } else if (signalID == transmissionEndedSignal) {
        int wIndex = -1;
        for (auto w : radios) {
            if (source == w) {
                auto radio = check_and_cast<physicallayer::Ieee80211Radio*>(source);
                wIndex = radio->getParentModule()->getIndex();
                queueing::PacketQueue* queue = check_and_cast<queueing::PacketQueue*>(queues[wIndex]);
                if (!queue->isEmpty()){
                    auto packet = queue->dequeuePacket();
                    //take(packet);
                    Packet *newpacket = createMCOPacket(packet, wIndex);
                    EV_INFO << "Collecting packet" << EV_FIELD(packet) << " and sending to NIC" << EV_ENDL;
                    delete packet;
                    send(newpacket, outWlanId[wIndex]);
                }
                break;
            }
        }
    } else if (signalID == modesetChangedSignal)  {
        const physicallayer::Ieee80211ModeSet* ms = check_and_cast<const physicallayer::Ieee80211ModeSet*>(obj);
        for (int i=0; i<radios.size(); ++i) {
            std::string m = "^.^.wlan[" + std::to_string(i) + "].mac";
            auto mac = check_and_cast<cComponent*>(getModuleByPath(m.c_str()));
            if (source == mac) {
                MCOChannelConfig conf;
                conf.mode = ms;
                conf.bitrate = bps(mac->getParentModule()->par("bitrate"));
                conf.bw = Hz(radios[i]->par("bandwidth"));
                //std::cout<<"Getting radio config: "<<i<<"; mode"<<ms<<";bitrate="<<conf.bitrate<<";BW="<<conf.bw<<endl;
                ieeeModes[i] = conf;
                break;
            }
        }
    } else {
        processPDRSignal(source, signalID, obj, details);
    }
}

void MgmtMCO::finish() {
    computePDR();
}

void MgmtMCO::receiveSignal(cComponent *source, simsignal_t signal, intval_t value, cObject *details) {
    //CBT measurement
    if (signal == physicallayer::IRadio::receptionStateChangedSignal) {
        for (int i = 0; i < numChannels; i++) {
            if (source == radios[i]) {
                physicallayer::IRadio::ReceptionState newState = static_cast<physicallayer::IRadio::ReceptionState>(value);
                //It is actually easier to compute the CBT with the IDLE state. Anything that is not idle must be busy somehow (transmitting, receiving or busy)
                if (newState == physicallayer::IRadio::ReceptionState::RECEPTION_STATE_IDLE) lu_idletime[i] = simTime();

                if (newState != physicallayer::IRadio::ReceptionState::RECEPTION_STATE_IDLE && lastReceptionState[i] == physicallayer::IRadio::ReceptionState::RECEPTION_STATE_IDLE) {
                    cbt_idletime[i] += (simTime() - lu_idletime[i]);
                }

                lastReceptionState[i] = newState;
                break;
            }
        }
    } else if (signal == physicallayer::IRadio::transmissionStateChangedSignal) {
        for (int i = 0; i < numChannels; i++) {
            if (source == radios[i]) {
                physicallayer::IRadio::TransmissionState newState = static_cast<physicallayer::IRadio::TransmissionState>(value);
                if (newState == physicallayer::IRadio::TransmissionState::TRANSMISSION_STATE_IDLE) lu_idletime[i] = simTime();

                if (newState != physicallayer::IRadio::TransmissionState::TRANSMISSION_STATE_IDLE && lastTransmissionState[i] == physicallayer::IRadio::TransmissionState::TRANSMISSION_STATE_IDLE) {
                    cbt_idletime[i] += (simTime() - lu_idletime[i]);
                }

                lastTransmissionState[i] = newState;
                break;
            }
        }
    }
}

double MgmtMCO::getMeasuredCBT(double period, int channel) {
    int i = channel;
    auto radio = check_and_cast<physicallayer::Ieee80211Radio*>(radios[i]);
    if (radio->getReceptionState() == physicallayer::IRadio::ReceptionState::RECEPTION_STATE_IDLE || radio->getTransmissionState() == physicallayer::IRadio::TransmissionState::TRANSMISSION_STATE_IDLE) {
        cbt_idletime[i] += (simTime() - lu_idletime[i]);
        lu_idletime[i] = simTime();
    }

    double mcbt = 1.0 - (cbt_idletime[i].dbl()/period);
    //std::cout<<simTime()<<":"<<myId<<":receivedPacketCount="<<receivedPacketCount[i]<<"; packets in period = "<<(receivedPacketCount[i]-lastPacketCount[i])<<endl;
    lastPacketCount[i] = receivedPacketCount[i];
    emit(cbtSignals[i], mcbt);
    cbt_idletime[i] = 0.0;
    return mcbt;
}

void MgmtMCO::setCbtWindow(const simtime_t& cbtWindow, double offset) {
    Enter_Method_Silent();
    this->cbtWindow = cbtWindow;
    if (cbtSampleTimer->isScheduled()) cancelEvent(cbtSampleTimer);

    if (offset>0) scheduleAfter(offset + cbtWindow, cbtSampleTimer);
    else scheduleAfter(cbtWindow, cbtSampleTimer);
}

int MgmtMCO::getSequenceNumber(const Packet* pkt ) {
    auto p= pkt->peekDataAt<MCOPacket>(B(31));
    return p->getSequenceNumber();


}

void MgmtMCO::processPDRSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) {
    //PDR Measurement
    if (signalID == transmissionStartedSignal) {
        for (int i = 0; i < numChannels; i++) {
            if (source == radios[i]) {
                //Clear nodesInPdrIntervals vector
                for (int j = 0; j < nodesInPdrIntervals[i].size(); j++)
                    nodesInPdrIntervals[i][j] = 0;

                //Refresh the number of nodes in each distance interval in nodesInPdrIntervals vector
                neighborCache->updateNeighborLists();
                auto neighborList = neighborCache->getNeighbors(check_and_cast<physicallayer::IRadio*>(radios[i]));
                for (int j = 0; j < neighborList.size(); j++) {
                    double sqrd = mob->getCurrentPosition().sqrdist(neighborList[j]->getAntenna()->getMobility()->getCurrentPosition());
                    unsigned int k = floor(pow(sqrd, 0.5) / pdrDistanceStep);
                    //TODO: for moving vehicles it is better to store the actual vehicle pointer
                    if (k < pdrNumberIntervals)
                        nodesInPdrIntervals[i][k]++;
                }

                //Our transmission has started
                auto ct = check_and_cast<const physicallayer::ITransmission*>(obj);
                const Packet* pkt = ct->getPacket();

                //auto p=getMCOPacket(pkt);
                int sn=getSequenceNumber(pkt);
                //auto p = pkt->peekDataAt<MCOPacket>(B(33)); //PhyHeader(5)+MACHeader(24+2(QoS))+LLCEDP(2), there is a trail afterwards

                //If the frame aggregation is activated, now in one transmission we have more than one packet and also one extra header for every packet.
                //So that if we want frame aggregation we have to change the following code to register all the packets.
                //auto p = pkt->peekDataAt<MCOPacket>(B(47)); //PhyHeader(5)+MACHeader(24+2(QoS))+MSDUSubFrameHeader(14)+LLCEDP(2), there is a trail afterwards

                //Details  can be found at Ieee80211Radio::encapsulate()
                PDR pdr;
                pdr.insertTime = simTime();

                for (int j = 0; j < nodesInPdrIntervals[i].size(); j++) {
                    if (nodesInPdrIntervals[i][j] != 0) {
                        pdr.vehicles[j] = nodesInPdrIntervals[i][j];
                        pdr.received[j] = 0;
                    }
                }

                //Insert in map
                //pdrAtChannel[i][p->getSequenceNumber()] = pdr;
                pdrAtChannel[i][sn] = pdr;

                break;
            }
        }
    } else if (signalID == MCOReceivedSignal) {
        //TODO: for moving vehicles, PDR may be > 1 because at the moment of reception (now) a
        // vehicle may have crossed the distance interval it was when the signal started (above)
        MCOReceivedInfo* info = check_and_cast<MCOReceivedInfo*>(obj);
        if (info->source == myId) {
            int i = info->channel;
            auto f = pdrAtChannel[i].find(info->sequenceNumber);
            if (f != pdrAtChannel[i].end()) {
                //This distance may be higher or lower that the distance we took for computing the number of vehicles for the PDR record
                //The alternative is to actually store the potential vehicle id at the beginning of the signal
                double sqrd = mob->getCurrentPosition().sqrdist(info->position);
                unsigned int k = floor(pow(sqrd, 0.5) / pdrDistanceStep);
                if (k < pdrNumberIntervals)
                    f->second.received[k]++;
            }
        }
    }
}

void MgmtMCO::computePDR() {
    for (int i = 0; i < pdrAtChannel.size(); ++i) {
        for (auto it = pdrAtChannel[i].begin(); it != pdrAtChannel[i].end();) {
            if ((simTime()-it->second.insertTime) >= pdrWindow) {
                for (auto v = it->second.vehicles.begin(); v != it->second.vehicles.end(); v++) {
                    int received = it->second.received[v->first];
                    int vehicles = v->second;
                    if (vehicles != 0)
                        emit(pdrSignals[i][v->first], ((double)received/vehicles));
                }
                it = pdrAtChannel[i].erase(it);
            } else ++it;
        }
    }
}

Packet* MgmtMCO::createMCOPacket(Packet* packet, int channel) {
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
    //newpacket->addTagIfAbsent<UserPriorityReq>()->setUserPriority(0);

    return newpacket;
}

void MgmtMCO::receiveMCOPacket(cMessage* msg) {
    Packet *pkt = static_cast<Packet*>(msg);

    updateVehicleInfo(pkt);

    auto p = pkt->peekAtFront<MCOPacket>(pkt->getDataLength());

    //Update the number of packets received per channel
    receivedPacketCount[p->getChannel()]++;
    //emit(receivedPacketCountSignals[p->getChannel()], receivedPacketCount[p->getChannel()]);
    emit(receivedPacketCountSignals[p->getChannel()], p->getSource());

    //MCOPacketReceived signal
    MCOReceivedInfo* ci = new MCOReceivedInfo();
    ci->id = myId;
    ci->appId = p->getAppIdentifier();
    ci->source = p->getSource();
    ci->sequenceNumber = p->getSequenceNumber();
    ci->channel = p->getChannel();
    ci->position = mob->getCurrentPosition();
    emit(MCOReceivedSignal, check_and_cast<const cObject*>(ci));
    delete ci;

    auto s = outAppId.find(p->getAppIdentifier());
    if (s != outAppId.end()){
        EV_INFO << "Received MCOPacket from node[" << p->getSource() << "] at channel " << p->getChannel() << " with sequence number equals to " << p->getSequenceNumber() << ".\n";
        send(msg, s->second);
    } else {
        //drop packet
        EV_INFO << "Application " << p->getAppIdentifier() << " not found. Dropping packet" << endl;
        delete pkt;
    }
}

MCOChannelConfig MgmtMCO::getChannelConfig(int channel) {
    if (channel<ieeeModes.size()) {
        return ieeeModes[channel];
    } else {
        MCOChannelConfig m;
        return m;
    }
}

int  MgmtMCO::getCw(int channel) {
    if (channel<macDcaf.size()) {
        return macDcaf[channel]->getCw();
    } else {
        return 1023;
    }
}

void MgmtMCO::updateVehicleInfo(Packet* pkt) {
    auto p = pkt->peekAtFront<MCOPacket>(pkt->getDataLength());

    info->id = p->getSource();
    info->appId = p->getAppIdentifier();
    info->channelNumberLastUpdate = p->getChannel();
    info->pos = p->getPosition();
    vehicleTable->insertOrUpdate(info);
}

} /* namespace inet */
