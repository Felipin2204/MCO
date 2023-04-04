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

#ifndef INET_APPLICATIONS_VEHICULAR_MGMTMCO_H_
#define INET_APPLICATIONS_VEHICULAR_MGMTMCO_H_

#include <omnetpp.h>
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Radio.h"
#include "inet/linklayer/ieee80211/mac/channelaccess/Dcaf.h"
#include "VehicleInfo.h"
#include "VehicleTable.h"

using namespace omnetpp;

namespace inet {

struct PDR {
    unsigned int vehicles, received;
};

class MCOReceivedInfo: public cObject, noncopyable {
  public:
    int id;
    int appId;
    int source;
    int sequenceNumber;
    int channel;
    Coord position;
};

class MgmtMCO : public cSimpleModule, public cListener {
  public:
    MgmtMCO();
    virtual ~MgmtMCO();

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override {return NUM_INIT_STAGES;}
    virtual void handleMessage(cMessage *msg) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    //CBT (Channel Busy Time)
    virtual void receiveSignal(cComponent *source, simsignal_t signal, intval_t value, cObject *details) override;
    virtual double getMeasuredCBT(double period, int channel);
    virtual void setCbtWindow(const simtime_t& cbtWindow, double offset = -1);

    //PDR (Packet Delivery Ratio)
    virtual void processPDRSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details);
    virtual void computePDR(int channel);

    //MCOPacket
    virtual Packet* createMCOPacket(Packet* packet, int channel);
    virtual void receiveMCOPacket(cMessage *msg);

    int numChannels;
    int numApplications;
    int myId;
    int MCOSent;
    IMobility* mob;
    VehicleInfo* info;
    VehicleTable* vehicleTable;

    static simsignal_t MCOReceivedSignal;

    std::map<int,int> outAppId;
    std::vector<int> inWlanId;
    std::vector<int> outWlanId;
    std::vector<cComponent*> queues;
    std::vector<cComponent*> radios;
    std::vector<ieee80211::Dcaf*> macDcaf;

    //CBT measurement
    simtime_t cbtWindow;
    std::vector<simsignal_t> cbtSignals;
    cMessage* cbtSampleTimer;
    std::vector<simtime_t> cbt_rtime;
    std::vector<simtime_t> lu_rtime; //lu = last update
    std::vector<simtime_t> cbt_txtime;
    std::vector<simtime_t> lu_txtime;
    std::vector<simtime_t> cbt_idletime;
    std::vector<simtime_t> lu_idletime;
    std::vector<physicallayer::IRadio::ReceptionState> lastReceptionState;
    std::vector<physicallayer::IRadio::TransmissionState> lastTransmissionState;
    std::vector<simsignal_t> subgradientSignals;

    //PDR measurement
    double pdrRange;
    std::vector<simsignal_t> pdrSignals;
    std::vector<PDR> pdrAtChannel;
    std::vector<const physicallayer::ITransmission*> currentTransmission;
    std::vector<int> currentTransmissionSeqNumber;
    std::vector<int> MCOSignalArrival;
    std::vector<int> numberOfNodes;
};

} /* namespace inet */

#endif /* INET_APPLICATIONS_VEHICULAR_MGMTMCO_H_ */
