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

#include "DCCTrafficGenerator.h"

namespace inet {

Define_Module(DCCTrafficGenerator);

DCCTrafficGenerator::DCCTrafficGenerator() {
    packetGenerationTimer = nullptr;
}

DCCTrafficGenerator::~DCCTrafficGenerator() {}

void DCCTrafficGenerator::initialize(int stage) {
    if (stage == INITSTAGE_LINK_LAYER) {
        getParentModule()->subscribe("cbt", this);

    } else if (stage == INITSTAGE_APPLICATION_LAYER) {
        timeBetweenPackets = par("timeBetweenPackets");
        emit(timeBetweenPacketsSignal, timeBetweenPackets);

        packetGenerationTimer = new cMessage();
        scheduleAt(simTime()+timeBetweenPackets, packetGenerationTimer);

    } else TrafficGenerator::initialize(stage);
}

void DCCTrafficGenerator::handleMessage(cMessage *msg) {
    if(msg->isSelfMessage()) {
        sendPacket();
        emit(timeBetweenPacketsSignal, timeBetweenPackets);
        scheduleAt(simTime()+timeBetweenPackets, msg);
    } else {
        receivePacket(msg);
    }
}

void DCCTrafficGenerator::receiveSignal(cComponent *source, simsignal_t signalID, double d, cObject *details) {
    timeBetweenPackets = par("timeBetweenPackets");
    if (d >= 0.62) {
        double aux = ((par("packetLength").intValue()*8)/6E+6)*(4000*((d-0.62)/d)-1);
        timeBetweenPackets = aux < 1 ? aux : 1;
    }
}

} /* namespace inet */
