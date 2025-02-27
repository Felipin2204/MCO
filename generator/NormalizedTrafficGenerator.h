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

#ifndef __INET4_5_NORMALIZEDTRAFFICGENERATOR_H_
#define __INET4_5_NORMALIZEDTRAFFICGENERATOR_H_

#include <omnetpp.h>
#include "TrafficGenerator.h"
#include "../MgmtMCO.h"
using namespace omnetpp;

namespace inet {

class NormalizedTrafficGenerator : public TrafficGenerator, public cListener
{
public:
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
    virtual void resetPeriod() override;

protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

    std::vector<int> messageSizes;
    double generatedLoad;
    double averagePacketDuration;
    double contentionTime;
    MCOChannelConfig channelConfig;
    static simsignal_t generatedLoadSignal;
    static simsignal_t packetDurationSignal;

    virtual void sendPacket() override;
};

} //namespace

#endif
