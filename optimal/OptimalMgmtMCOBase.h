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

#ifndef __INET4_5_OPTIMALMGMTMCOBASE_H_
#define __INET4_5_OPTIMALMGMTMCOBASE_H_

#include <omnetpp.h>
#include "../MgmtMCO.h"
#include "MinosVehicleInfo.h"
#include "MinosVehicleTable.h"
using namespace omnetpp;

namespace inet {

/**
 * TODO - Generated class
 */
class OptimalMgmtMCOBase :   public MgmtMCO
{
public:
    OptimalMgmtMCOBase();
    virtual ~OptimalMgmtMCOBase();
    static simsignal_t iterationsDoneSignal;
protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    //virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
    virtual Packet* createMCOPacket(Packet* packet, int channel) override;
    //  virtual void receiveMCOPacket(cMessage *msg) override;
    virtual void updateVehicleInfo(Packet* pkt) override;

    std::vector<cComponent*> applications;
    std::vector<double> channelWeights; //Lagrange multipliers per channel
    std::vector<double> channelLoads; //Measured load per channel (only used by certain algorithms)
    std::vector<double> subgradients; //Computed subgradient per channel
    std::vector<double> packetsPerChannel; //Computed data (packets) to transmit per channel per interval
    std::vector<double> currentDemandPerChannel; //Current demand (packets) to transmit per channel per interval
    std::vector<double> channelMaxCapacity; //MaxCapacity per channel as fraction
    std::vector<double> channelMaxCapacityPackets; //MaxCapacity per channel in packet/s
    std::vector<double> channelOmega; //Omega (priority) parameter per channel
    std::vector<double> tmin; //Minimum load per channel (it is set from the applications, not in NED parameters)
    double alpha; //Fairness parameter
    double gammaStep; //Gradient update step
    double initWeight; //Initial Lagrange weights

    bool useCBT; //Use CBT to measure the loads
    bool  updateWeight; //To alternate weights and lagrangian
    bool useCBTCorrection; //To correct the CBT due to idle time
    double cbtCorrection;
    double packetDuration;
    double minimumSlotDuration;
    bool regularized; //Use regularization
    double regParameter; //Regularization parameter

    cMessage* updateChannelTimer;
    cMessage* updateWeightTimer;
    MinosVehicleInfo* minfo;
    MinosVehicleTable* mvt;

    //Measurement time or time between algorithm iterations. It is  the double of this value, since
    //half the period is to measure the load and half to spread the Lagrange multipliers
    double syncPeriod;

    std::vector<double> powers; //Not used at the moment

    std::vector<simsignal_t> weightSignals;
    std::vector<simsignal_t> subgradientSignals;
    std::vector<simsignal_t> packetSignals;
    std::vector<simsignal_t> loadSignals;


    int seqNumber;

};

} //namespace

#endif
