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
#include "TrafficGenerator.h"
#include "inet/linklayer/common/Ieee802SapTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/common/ProtocolTag_m.h"

#define CBT_TO 1003

namespace inet {

Define_Module(MgmtMCO);
simsignal_t MgmtMCO::MCOReceivedSignal = SIMSIGNAL_NULL;

MgmtMCO::MgmtMCO(): mob(nullptr), info(nullptr), vehicleTable(nullptr), cbtSampleTimer(nullptr) {}

MgmtMCO::~MgmtMCO() {
    if (cbtSampleTimer) cancelAndDelete(cbtSampleTimer);
}

void MgmtMCO::initialize(int stage)
{
    if(stage == INITSTAGE_LOCAL) {
        numChannels = par("numChannels");
        numApplications = par("numApplications");
        myId = getParentModule()->getParentModule()->par("id");
        MCOSent = 0;
        mob = check_and_cast<IMobility*>(getModuleByPath("^.^.mobility"));
        Coord pos(-1, -1);
        info = new VehicleInfo(-1, -1, pos);
        vehicleTable = check_and_cast<VehicleTable*>(getModuleByPath("^.vehicleTable"));

        MCOReceivedSignal = registerSignal("MCOPacketReceived");

        int baseIdIn = gateBaseId("inWLAN");
        int baseIdOut = gateBaseId("outWLAN");
        for (int i = 0; i < gateSize("inWLAN"); i++) {
            inWlanId.push_back(gate(baseIdIn+i)->getId());
            outWlanId.push_back(gate(baseIdOut+i)->getId());
        }

        cbtWindow = par("cbtWindow");
        cbtSignals.resize(numChannels);

        pdrRange = par("pdrRange");
        pdrRange = pdrRange*pdrRange;
        pdrSignals.resize(numChannels);

        for (int i = 0; i < numChannels; i++) {
            std::string tname("cbt");
            tname += std::to_string(i);
            cbtSignals[i] = registerSignal(tname.c_str());
            cProperty *statisticTemplate = getProperties()->get("statisticTemplate", "cbt");
            getEnvir()->addResultRecorders(this, cbtSignals[i], tname.c_str(), statisticTemplate);

            std::string sname("pdr");
            sname += std::to_string(i);
            pdrSignals[i] = registerSignal(sname.c_str());
            statisticTemplate = getProperties()->get("statisticTemplate", "pdr");
            getEnvir()->addResultRecorders(this, pdrSignals[i], sname.c_str(), statisticTemplate);
        }

        cbtSampleTimer = new cMessage("CBT Timer", CBT_TO);
        setCbtWindow(cbtWindow);

        pdrAtChannel.resize(numChannels);

    } else if(stage == INITSTAGE_LINK_LAYER) {
        int baseId = gateBaseId("outApp");
        for (int i = 0; i < gateSize("outApp"); i++) {
            int gateId = gate(baseId+i)->getId();
            int appId = check_and_cast<TrafficGenerator*>(findConnectedModule<TrafficGenerator>(gate(gateId)))->getAppId();
            outAppId.insert({appId, gateId});
        }
        WATCH_MAP(outAppId);

        for (int i = 0; i < numChannels; i++) {
            std::string r = "^.^.wlan[" + std::to_string(i) + "].radio";
            auto radio = check_and_cast<cComponent*>(getModuleByPath(r.c_str()));
            radios.push_back(radio);

            //Radio module is subscribed to this other signal which is emitted by himself when a transmission is finished
            radio->subscribe(transmissionEndedSignal, this);

            //Changed to collect all the queues in the MCOSequencialFilling case
            queues.push_back(findConnectedModule<queueing::PacketQueue>(gate("inQueue", i)));

            std::string d = "^.^.wlan[" + std::to_string(i) + "].mac.dcf.channelAccess";
            macDcaf.push_back(check_and_cast<ieee80211::Dcaf*>(getModuleByPath(d.c_str())));

            //CBT measurement
            radio->subscribe(physicallayer::IRadio::receptionStateChangedSignal, this);
            radio->subscribe(physicallayer::IRadio::transmissionStateChangedSignal, this);

            cbt_rtime.push_back(0.0);
            lu_rtime.push_back(simTime());
            cbt_txtime.push_back(0.0);
            lu_txtime.push_back(simTime());
            cbt_idletime.push_back(0.0);
            lu_idletime.push_back(simTime());
            lastReceptionState.push_back(check_and_cast<physicallayer::IRadio*>(radio)->getReceptionState());
            lastTransmissionState.push_back(check_and_cast<physicallayer::IRadio*>(radio)->getTransmissionState());

            //PDR measurement
            radio->subscribe(transmissionStartedSignal, this);

            currentTransmission.push_back(nullptr);
            currentTransmissionSeqNumber.push_back(-1);
            MCOSignalArrival.push_back(0);
            numberOfNodes.push_back(0);
        }

        //PDR measurement. Subscribe to all radios
        getSimulation()->getSystemModule()->subscribe(physicallayer::IRadioMedium::signalArrivalStartedSignal,this);
        getSimulation()->getSystemModule()->subscribe(physicallayer::IRadioMedium::signalRemovedSignal, this);
        getSimulation()->getSystemModule()->subscribe(MCOReceivedSignal, this);

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
            scheduleAt(simTime()+cbtWindow, cbtSampleTimer);
        }
    } else receiveMCOPacket(msg);
}

void MgmtMCO::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) {
    Enter_Method("checking queues");
    if (signalID == packetPushEndedSignal) {
        int qIndex = -1;
        for (auto c : queues) {
            if (source == c) {
                queueing::PacketQueue* queue = check_and_cast<queueing::PacketQueue*>(c);

                //Necessary for the MCOSequencialFilling case
                if(queue->isVector()) qIndex = queue->getIndex();
                else qIndex = numChannels-1;

                //Dequeue packet if the Dcf don't have a frame to transmit(Dcf::hasFrameToTransmit)
                if (macDcaf[qIndex]->getPendingQueue()->isEmpty() && !macDcaf[qIndex]->getInProgressFrames()->hasInProgressFrames()){
                    auto packet = queue->dequeuePacket();
                    //take(packet);
                    Packet *newpacket = createMCOPacket(packet, qIndex);
                    EV_INFO << "Collecting packet" << EV_FIELD(packet) << EV_ENDL;
                    delete packet;
                    send(newpacket, outWlanId[qIndex]);
                }
                break;
            }
        }
    } else  if (signalID == transmissionEndedSignal) {
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
                    EV_INFO << "Collecting packet" << EV_FIELD(packet) << EV_ENDL;
                    delete packet;
                    send(newpacket, outWlanId[wIndex]);
                }
                break;
            }
        }
    } else {
        processPDRSignal(source, signalID, obj, details);
    }
}

void MgmtMCO::receiveSignal(cComponent *source, simsignal_t signal, intval_t value, cObject *details) {
    //CBT measurement
    if (signal == physicallayer::IRadio::receptionStateChangedSignal) {
        for (int i = 0; i < numChannels; i++) {
            if (source == radios[i]) {
                //It is actually easier to compute the CBT with the IDLE state. Anything that is not idle must be busy somehow (transmitting, receiving or busy)
                physicallayer::IRadio::ReceptionState newState = static_cast<physicallayer::IRadio::ReceptionState>(value);
                if (newState != physicallayer::IRadio::ReceptionState::RECEPTION_STATE_IDLE && lastReceptionState[i] == physicallayer::IRadio::ReceptionState::RECEPTION_STATE_IDLE) {
                    cbt_idletime[i] += (simTime() - lu_idletime[i]);
                }
                if (newState == physicallayer::IRadio::ReceptionState::RECEPTION_STATE_IDLE) {
                    lu_idletime[i] = simTime();
                }

                //if (newState!= physicallayer::IRadio::ReceptionState::RECEPTION_STATE_RECEIVING && lastReceptionState ==physicallayer::IRadio::ReceptionState::RECEPTION_STATE_RECEIVING) {
                if ((newState != physicallayer::IRadio::ReceptionState::RECEPTION_STATE_RECEIVING || newState != physicallayer::IRadio::ReceptionState::RECEPTION_STATE_BUSY) && (lastReceptionState[i] == physicallayer::IRadio::ReceptionState::RECEPTION_STATE_RECEIVING || newState == physicallayer::IRadio::ReceptionState::RECEPTION_STATE_BUSY)) {
                    //We were receiving
                    cbt_rtime[i] += (simTime()-lu_rtime[i]);
                }
                if (newState == physicallayer::IRadio::ReceptionState::RECEPTION_STATE_RECEIVING || newState == physicallayer::IRadio::ReceptionState::RECEPTION_STATE_BUSY) {
                    lu_rtime[i] = simTime();
                }
                lastReceptionState[i] = newState;
                //What about IDLE and BUSY
                break;
            }
        }
    }
    if (signal == physicallayer::IRadio::transmissionStateChangedSignal) {
        for (int i = 0; i < numChannels; i++) {
            if (source == radios[i]) {
                physicallayer::IRadio::TransmissionState newState = static_cast<physicallayer::IRadio::TransmissionState>(value);
                if (newState != physicallayer::IRadio::TransmissionState::TRANSMISSION_STATE_TRANSMITTING && lastTransmissionState[i] == physicallayer::IRadio::TransmissionState::TRANSMISSION_STATE_TRANSMITTING) {
                    //We were receiving
                    cbt_txtime[i] += (simTime()-lu_txtime[i]);
                }
                if (newState == physicallayer::IRadio::TransmissionState::TRANSMISSION_STATE_TRANSMITTING) {
                    lu_txtime[i] = simTime();
                }
                lastTransmissionState[i] = newState;
            }
            break;
        }
    }
}

double MgmtMCO::getMeasuredCBT(double period, int channel) {
        int i = channel;
        auto radio = check_and_cast<physicallayer::Ieee80211Radio*>(radios[i]);
        if (radio->getReceptionState() == physicallayer::IRadio::ReceptionState::RECEPTION_STATE_RECEIVING || radio->getReceptionState() == physicallayer::IRadio::ReceptionState::RECEPTION_STATE_BUSY) {
            //I am in the middle of reception or transmission, update CBT
            cbt_rtime[i] += (simTime() - lu_rtime[i]);
            //std::cout<<myId<<":"<<simTime().dbl()<<"--middle getMeasuredCBT:lurx="<<lu_rtime<<"cbt_rtime="<<cbt_rtime.dbl()<<std::endl;
            lu_rtime[i] = simTime();
        }
        if (radio->getTransmissionState() == physicallayer::IRadio::TransmissionState::TRANSMISSION_STATE_TRANSMITTING) {
            cbt_txtime[i] += (simTime() - lu_txtime[i]);
            //std::cout<<myId<<":"<<simTime().dbl()<<"--middle getMeasuredCBT:lutx="<<lu_txtime<<"cbt_txtime="<<cbt_txtime.dbl()<<std::endl;
            lu_txtime[i] = simTime();
        }
        if (radio->getReceptionState() == physicallayer::IRadio::ReceptionState::RECEPTION_STATE_IDLE) {
            cbt_idletime[i] += (simTime() - lu_idletime[i]);
            lu_idletime[i] = simTime();
        }

        //double mcbt = (cbt_rtime.dbl() + cbt_txtime.dbl()) / period;

        //Work with the idle state
        double mcbt = 1.0 - (cbt_idletime[i].dbl() / period);
        //std::cout<<myId<<":"<<simTime().dbl()<<"--getMeasuredCBT:"<<mcbt<<"lurx="<<lu_rtime<<"cbt_rtime="<<cbt_rtime.dbl()<<"cbt_txtime="<<cbt_txtime.dbl()<<"lutx="<<lu_txtime<<"idle="<<cbt_idletime<<"luidle="<<lu_idletime<<std::endl;
        cbt_rtime[i] = 0.0;
        cbt_txtime[i] = 0.0;
        cbt_idletime[i] = 0.0;
        emit(cbtSignals[i], mcbt);
        return mcbt;
}

void MgmtMCO::setCbtWindow(const simtime_t& cbtWindow, double offset) {
    Enter_Method_Silent();
    this->cbtWindow = cbtWindow;
    if (cbtSampleTimer->isScheduled()) cancelEvent(cbtSampleTimer);

    if (offset>0) scheduleAt(simTime()+offset+cbtWindow,cbtSampleTimer);
    else scheduleAt(simTime()+cbtWindow,cbtSampleTimer);
}

void MgmtMCO::processPDRSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) {
    //PDR Measurement
    if (signalID == transmissionStartedSignal) {
        for (int i = 0; i < numChannels; i++) {
            if (source == radios[i]) {
                //Our transmission has started
                currentTransmission[i] = check_and_cast< const physicallayer::ITransmission* >(obj);
                const Packet* pkt = currentTransmission[i]->getPacket();
                auto p = pkt->peekDataAt<MCOPacket>(B(31)); //PhyHeader(5)+MACHeader(24)+LLCEDP(2), there is a trail afterwards
                //auto p = pkt->peekAtFront<MCOPacket>(pkt->getDataLength());
                currentTransmissionSeqNumber[i] = p->getSequenceNumber();
                MCOSignalArrival[i] = 0;
                numberOfNodes[i] = getSimulation()->getSystemModule()->getSubmoduleVectorSize("node");
                //std::cout<<myId<<"tx started ctsn="<<currentTransmissionSeqNumber<<p->getSequenceNumber()<<std::endl;
            }
            break;
        }
    }
    if (signalID == physicallayer::IRadioMedium::signalRemovedSignal) {
        const physicallayer::ITransmission* t = check_and_cast<const physicallayer::ITransmission*>(obj);
        for (int i = 0; i < numChannels; i++) {
            if (currentTransmission[i] == t) {
                if (currentTransmission[i]->getId() == t->getId()) {
                    currentTransmission[i] = nullptr;
                    //currentTransmissionSeqNumber=-1;
                    MCOSignalArrival[i] = 0;
                }
            }
            break;
        }
    }
    if (signalID == physicallayer::IRadioMedium::signalArrivalStartedSignal) {
        const physicallayer::IReception* rec = check_and_cast<const physicallayer::IReception*>(obj);
        const physicallayer::ITransmission* t = rec->getTransmission();
        if (t) {
            for (int i = 0; i < numChannels; i++) {
                if (currentTransmission[i] == t) {
                    MCOSignalArrival[i]++;
                    int idt = t->getId();
                    int cid = currentTransmission[i]->getId();
                    if (idt == cid) {
                        double sqrd = mob->getCurrentPosition().sqrdist(rec->getStartPosition());
                        if (sqrd <= pdrRange) {
                            pdrAtChannel[i].vehicles++;
                            //std::cout<<myId<<":"<<simTime()<<"PDR-"<<i<<" arrivalStarted from"<<rec->getStartPosition()<<"d="<<sqrd<<"veh="<<pdrAtDistance[i].vehicles<<std::endl;
                        }
                    }

                    if (MCOSignalArrival[i] == numberOfNodes[i]) {
                        currentTransmission[i] = nullptr;
                    }
                    break;
                }
            }
        }
    }
    if (signalID == MCOReceivedSignal) { //FIXME: This if was inside the one before
        MCOReceivedInfo* info = check_and_cast<MCOReceivedInfo*>(obj);
        int i = info->channel;
        if (info->sequenceNumber == currentTransmissionSeqNumber[i] && info->source != myId) { //FIXME: Changed == for != in the last statement
            double sqrd = mob->getCurrentPosition().sqrdist(info->position);
            //std::cout<<myId<<"source="<<info->id<<"sn="<<info->sequenceNumber<<"ctsn="<<currentTransmissionSeqNumber<<"sqrd="<<sqrd<<std::endl;
            //std::cout<<myId<<"pdrRanges"<<pdrRanges[i]<<std::endl;
            if (sqrd <= pdrRange) {
                pdrAtChannel[i].received++;
                //std::cout<<myId<<":"<<simTime()<<"PDR-packet received from"<<info->id<<" at d="<<sqrd<<" rec="<<pdrAtDistance[i].received<<std::endl;
            }
        }
        //else {
        //    std::cout<<myId<<"source="<<info->id<<"sn="<<info->sequenceNumber<<"ctsn="<<currentTransmissionSeqNumber<<std::endl;
        //}
    }
}

void MgmtMCO::computePDR(int channel) {
    if (pdrAtChannel[channel].vehicles > 0) {
        emit(pdrSignals[channel], ((double)pdrAtChannel[channel].received / pdrAtChannel[channel].vehicles));
    }
    pdrAtChannel[channel].vehicles = 0;
    pdrAtChannel[channel].received = 0;

    currentTransmission[channel] = nullptr;
    currentTransmissionSeqNumber[channel] = -1;
    MCOSignalArrival[channel] = 0;
}

Packet* MgmtMCO::createMCOPacket(Packet* packet, int channel) {
    computePDR(channel);
    auto data = makeShared<MCOPacket>();
    auto p = packet->peekData<TrafficPacket>();
    //std::cout<<myId<<":"<<simTime()<<":Create MCO "<<p->getChunkLength()<<std::endl;
    data->setChunkLength(B(p->getChunkLength()));
    data->setAppIdentifier(p->getAppIdentifier());
    data->setSource(myId);
    data->setSequenceNumber(MCOSent);
    MCOSent++;
    data->setChannel(channel);
    Coord pos = mob->getCurrentPosition();
    data->setPosition(pos);

    //data->enableImplicitChunkSerialization = true;

    Packet *newpacket = new Packet("MCOPacket", data);
    //std::cout<<myId<<":"<<simTime()<<":packet Create MCO "<<newpacket->getDataLength()<<std::endl;

    //Add SAP. I think we may remove this
    newpacket->addTagIfAbsent<Ieee802SapReq>()->setDsap(SapCode::SAP_IP);
    newpacket->addTagIfAbsent<MacAddressReq>()->setDestAddress(MacAddress::BROADCAST_ADDRESS);
    //Should put something sensible here. Keep this to prevent LlcEpd from complaining
    newpacket->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ipv4);
    return newpacket;
}

void MgmtMCO::receiveMCOPacket(cMessage* msg) {
    Packet *pkt = static_cast<Packet*>(msg);

    //auto p = pkt->peekDataAt<MCOPacket>(B(31)); //PhyHeader(5)+MACHeader(24)+LLCEDP(2)
    //auto p = pkt->peekData<MCOPacket>(); //PhyHeader(5)+MACHeader(24)+LLCEDP(2)
    //std::cout<<myId<<":"<<simTime()<<":Received MCO "<<pkt->getDataLength()<<std::endl;

    auto p = pkt->peekAtFront<MCOPacket>(pkt->getDataLength());

    //VehicleInfo
    info->pos = p->getPosition();
    info->id = p->getSource();
    info->appId = p->getAppIdentifier();
    info->description = info->toString();
    vehicleTable->insertOrUpdate(info);

    //MCOReceivedInfo
    MCOReceivedInfo* ci = new MCOReceivedInfo();
    ci->id = myId;
    ci->appId = p->getAppIdentifier();
    ci->source = p->getSource();
    ci->sequenceNumber = p->getSequenceNumber();
    ci->channel = p->getChannel();
    ci->position = mob->getCurrentPosition();
    emit(MCOReceivedSignal, check_and_cast<const cObject*>(ci));
    delete ci;

    //send(msg, outAppId[0]+ p->getAppIdentifier());
    if (auto s = outAppId.find(p->getAppIdentifier()); s != outAppId.end()){
        send(msg, s->second);
    } else {
        //drop packet
        EV_INFO << "Application " << p->getAppIdentifier() << " not found. Dropping packet" << endl;
        delete pkt;
    }
}

} /* namespace inet */
