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
#include <random>
#include "inet/linklayer/common/Ieee802SapTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/common/ProtocolTag_m.h"

namespace inet {

Define_Module(TrafficGenerator);

TrafficGenerator::TrafficGenerator() {
    packetGenerationTimer = nullptr;
}

TrafficGenerator::~TrafficGenerator() {
    cancelAndDelete(packetGenerationTimer);
}

void TrafficGenerator::initialize(){
    //Getting the IDs of the gates
    lowerLayerIn = findGate("socketIn");
    lowerLayerOut = findGate("socketOut");

    //Getting NED parameters
    totalPacketsPerSecond = par("totalPacketsPerSecond");
    packetLength = par("packetLength");
    trafficDistribution = par("trafficDistribution").stdstringValue();

    generatedPackets = 0;
    WATCH(generatedPackets);

    timeBetweenPackets = getTimeBetweenPackets();
    packetGenerationTimer = new cMessage();
    scheduleAt(simTime()+timeBetweenPackets, packetGenerationTimer);
    generatedPackets++;
}

void TrafficGenerator::handleMessage(cMessage *packet){
    if(packet->isSelfMessage()) {
        auto data = makeShared<TrafficPacket>();
        data->setPacketLength(packetLength);
        data->setChunkLength(B(packetLength));
        data->enableImplicitChunkSerialization = true;

//        auto data = makeShared<ByteCountChunk>(B(100), 0);
//        data->enableImplicitChunkSerialization = true;

        char buffer [20];
        sprintf(buffer, "TrafficPacket-%d", generatedPackets);
        Packet *newpacket = new Packet(buffer, data);

        sendDown(newpacket);

        timeBetweenPackets = getTimeBetweenPackets();
        scheduleAt(simTime()+timeBetweenPackets, packet);
        generatedPackets++;
    } else {
        Packet *pkt = static_cast<Packet*>(packet);
        auto p = pkt->peekAtFront<TrafficPacket>(B(packetLength));
        int length = p->getPacketLength();
        EV << "Message " << packet->getName() << " (" << length << "B) arrived.\n";
        delete packet;
    }
}

void TrafficGenerator::sendDown(Packet* p){
    //Add SAP. I think we may remove this
    p->addTagIfAbsent<Ieee802SapReq>()->setDsap(SapCode::SAP_IP);
    p->addTagIfAbsent<MacAddressReq>()->setDestAddress(MacAddress::BROADCAST_ADDRESS);
    //Should put something sensible here. Keep this to prevent LlcEpd from complaining
    p->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ipv4);

    send(p,lowerLayerOut);
}

double TrafficGenerator::getTimeBetweenPackets(){
    //Assign time between packets depending on the selected traffic distribution
    //First we set a default value for the time between packets corresponding to a deterministic generation
    double packetTime = 1.0/totalPacketsPerSecond;

    std::random_device rd;
    std::mt19937 gen(rd());
    if(trafficDistribution == "uniform") {
        std::uniform_real_distribution<> dis(0.5*(1.0/totalPacketsPerSecond), 1.5*(1.0/totalPacketsPerSecond));
        packetTime = dis(gen);
    } else if (trafficDistribution == "exponential") {
        std::exponential_distribution<> dis(totalPacketsPerSecond);
        packetTime = dis(gen);
    } else if (generatedPackets == 0) {
        /*First time between packets for deterministic distribution is uniform in order to avoid that nodes send
        packets at the same time*/
        std::uniform_real_distribution<> dis(0.5*(1.0/totalPacketsPerSecond), 1.5*(1.0/totalPacketsPerSecond));
        packetTime = dis(gen);
    }
    return packetTime;
}

} /* namespace inet */
