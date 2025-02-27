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

DCCTrafficGenerator::DCCTrafficGenerator() : DCCMode(false) {}

void DCCTrafficGenerator::initialize(int stage) {
    if (stage == INITSTAGE_LINK_LAYER) {
        //To be used with multiple channels, the application can only use the cbt from a given channel, at least for a very simple implementation
        //So we  subscribe to the channel with our own id
        //getParentModule()->subscribe("cbt", this);
        std::string tname("cbt");
        tname += std::to_string(appId);
        getParentModule()->subscribe(tname.c_str(), this);


    } else if (stage == INITSTAGE_APPLICATION_LAYER) {
        timeBetweenPackets = par("timeBetweenPackets");
        emit(timeBetweenPacketsSignal, timeBetweenPackets);

        packetGenerationTimer = new cMessage();
        scheduleAfter(timeBetweenPackets, packetGenerationTimer);

    } else TrafficGenerator::initialize(stage);
}

void DCCTrafficGenerator::handleMessage(cMessage *msg) {
    if(msg->isSelfMessage()) {
        sendPacket();
        if (!DCCMode)
            timeBetweenPackets = par("timeBetweenPackets");
        emit(timeBetweenPacketsSignal, timeBetweenPackets);
        scheduleAfter(timeBetweenPackets, msg);
    } else {
        receivePacket(msg);
    }
}

void DCCTrafficGenerator::receiveSignal(cComponent *source, simsignal_t signalID, double d, cObject *details) {

    DCCMode = false;
    if (d >= 0.62) {
        DCCMode = true;
        double tOn = (par("packetLength").intValue()*8)/6E+6;
        timeBetweenPackets = tOn*(4000*((d-0.62)/d)-1);
        if (timeBetweenPackets < 0.025)
            timeBetweenPackets = 0.025;
        else if (timeBetweenPackets > 1.0)
            timeBetweenPackets = 1.0;

    }
}

} /* namespace inet */
