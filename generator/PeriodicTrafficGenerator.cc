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

#include "PeriodicTrafficGenerator.h"

namespace inet {

Define_Module(PeriodicTrafficGenerator);

PeriodicTrafficGenerator::PeriodicTrafficGenerator() {}

PeriodicTrafficGenerator::~PeriodicTrafficGenerator() {}

void PeriodicTrafficGenerator::initialize(int stage){
    if (stage == INITSTAGE_LOCAL) {
        TrafficGenerator::initialize(stage);

    } else if(stage == INITSTAGE_APPLICATION_LAYER) {
        timeBetweenPackets = par("timeBetweenPackets");
        emit(timeBetweenPacketsSignal, timeBetweenPackets);

        initialOffset = par("initialOffset");

        packetGenerationTimer = new cMessage();
        scheduleAfter(initialOffset, packetGenerationTimer);
    }
}

void PeriodicTrafficGenerator::handleMessage(cMessage *packet){
    if(packet->isSelfMessage()) {
        sendPacket();
        scheduleAfter(timeBetweenPackets, packet);
    } else {
        receivePacket(packet);
    }
}

} /* namespace inet */
