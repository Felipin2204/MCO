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

#include "TrafficGenerator.h"
#include "TrafficPacket_m.h"
#include "inet/linklayer/common/Ieee802SapTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/common/ProtocolTag_m.h"

namespace inet {

Define_Module(TrafficGenerator);

simsignal_t TrafficGenerator::timeBetweenPacketsSignal = SIMSIGNAL_NULL;
simsignal_t TrafficGenerator::generatedPacketsSignal = SIMSIGNAL_NULL;
simsignal_t TrafficGenerator::receivedPacketsSignal = SIMSIGNAL_NULL;

TrafficGenerator::TrafficGenerator() {
    packetGenerationTimer = nullptr;
}

TrafficGenerator::~TrafficGenerator() {
    cancelAndDelete(packetGenerationTimer);
}

void TrafficGenerator::initialize(int stage){
    if (stage == INITSTAGE_LOCAL) {
        generatedPackets = 0;
        receivedPackets = 0;

        //Getting the IDs of the gates
        lowerLayerIn = findGate("socketIn");
        lowerLayerOut = findGate("socketOut");

        //Registering signals for statistics
        timeBetweenPacketsSignal = registerSignal("timeBetweenPackets");
        generatedPacketsSignal = registerSignal("generatedPackets");
        receivedPacketsSignal = registerSignal("receivedPackets");

        totalPacketsPerSecond = par("totalPacketsPerSecond");
        packetLength = par("packetLength");

        appId = par("appId");
        if(appId == -1) appId = getIndex();
    } else if(stage == INITSTAGE_APPLICATION_LAYER) {
        double aux = par("timeBetweenPackets");
        timeBetweenPackets = simtime_t(aux);
        emit(timeBetweenPacketsSignal, aux);

        packetGenerationTimer = new cMessage();
        scheduleAt(simTime()+timeBetweenPackets, packetGenerationTimer);
        generatedPackets++;
        emit(generatedPacketsSignal, generatedPackets);
    }
}

void TrafficGenerator::sendPacket() {
    auto data = makeShared<TrafficPacket>();
    data->setChunkLength(B(packetLength));
    data->setAppIdentifier(appId);

//    auto data = makeShared<ByteCountChunk>(B(packetLength), 0);

    data->enableImplicitChunkSerialization = true;

    char buffer [20];
    sprintf(buffer, "App%d-Packet%d", appId, generatedPackets);
    Packet *newpacket = new Packet(buffer, data);

    sendDown(newpacket);

    double aux = par("timeBetweenPackets");
    timeBetweenPackets = simtime_t(aux);
    emit(timeBetweenPacketsSignal, aux);

    generatedPackets++;
    emit(generatedPacketsSignal, generatedPackets);
}

void TrafficGenerator::receivePacket(cMessage *packet) {
    Packet *pkt = static_cast<Packet*>(packet);
    auto p = pkt->peekData<TrafficPacket>();
    EV << packet->getName() << "(" << pkt->getDataLength() << ") arrived from application with ID="
            << p->getAppIdentifier() <<". Current application ID is " << appId << ".\n";
    delete packet;
    receivedPackets++;
    emit(receivedPacketsSignal, receivedPackets);
}

void TrafficGenerator::handleMessage(cMessage *packet){
    if(packet->isSelfMessage()) {
        sendPacket();
        scheduleAt(simTime()+timeBetweenPackets, packet);

    } else {
        receivePacket(packet);
    }
}

void TrafficGenerator::sendDown(Packet* p){
    //Add SAP. I think we may remove this
    p->addTagIfAbsent<Ieee802SapReq>()->setDsap(SapCode::SAP_IP);
    p->addTagIfAbsent<MacAddressReq>()->setDestAddress(MacAddress::BROADCAST_ADDRESS);
    //Should put something sensible here. Keep this to prevent LlcEpd from complaining
    p->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ipv4);

    send(p, lowerLayerOut);
}

} /* namespace inet */
