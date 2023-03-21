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

#ifndef INET_APPLICATIONS_VEHICULAR_TRAFFICGENERATOR_H_
#define INET_APPLICATIONS_VEHICULAR_TRAFFICGENERATOR_H_

#include <omnetpp.h>
#include <string>
#include "inet/common/packet/Packet.h"

using namespace omnetpp;

namespace inet {

class TrafficGenerator: public cSimpleModule {
public:
    TrafficGenerator();
    virtual ~TrafficGenerator();
    virtual int getAppId() const { return appId;};
protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override {return NUM_INIT_STAGES;}
    virtual void handleMessage(cMessage *msg) override;
    virtual void sendDown(Packet* p);
    virtual void sendPacket();
    virtual void receivePacket(cMessage *packet);

    int lowerLayerIn;
    int lowerLayerOut;

    int totalPacketsPerSecond;
    int packetLength;
    int appId;
    simtime_t timeBetweenPackets;

    int generatedPackets;
    int receivedPackets;
    cMessage *packetGenerationTimer;

    static simsignal_t timeBetweenPacketsSignal;
    static simsignal_t generatedPacketsSignal;
    static simsignal_t receivedPacketsSignal;

};

} /* namespace inet */

#endif /* INET_APPLICATIONS_VEHICULAR_TRAFFICGENERATOR_H_ */
