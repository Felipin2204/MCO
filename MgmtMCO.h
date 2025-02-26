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
#include "VehiclesNeighborCache.h"
#include "MCOPacket_m.h"

using namespace omnetpp;

namespace inet {

struct PDR {
    std::map<int,int> vehicles, received;
    simtime_t insertTime;
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

struct MCOChannelConfig {
    const physicallayer::Ieee80211ModeSet* mode;
    bps bitrate;
    Hz bw;
};

class MgmtMCO : public cSimpleModule, public cListener {
  public:
    MgmtMCO();
    virtual ~MgmtMCO();
    virtual MCOChannelConfig getChannelConfig(int channel);
    virtual int getCw(int channel);

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override {return NUM_INIT_STAGES;}
    virtual void handleMessage(cMessage *msg) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
    virtual void finish() override;

    //CBT (Channel Busy Time)
    virtual void receiveSignal(cComponent *source, simsignal_t signal, intval_t value, cObject *details) override;
    virtual double getMeasuredCBT(double period, int channel);
    virtual void setCbtWindow(const simtime_t& cbtWindow, double offset = -1);

    //PDR (Packet Delivery Ratio)
    virtual void processPDRSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details);
    virtual void computePDR();

    //MCOPacket
    virtual Packet* createMCOPacket(Packet* packet, int channel);
    virtual void receiveMCOPacket(cMessage *msg);
    virtual int getSequenceNumber(const Packet* pkt ) ;

    virtual void updateVehicleInfo(Packet* p);

    int numChannels;
    int numApplications;
    int myId;
    int MCOSent;
    VehicleInfo* info;
    IMobility* mob;
    VehicleTable* vehicleTable;

    static simsignal_t MCOReceivedSignal;
    static simsignal_t droppedPacketSourceSignal;

    std::vector<int> inWlanId;
    std::vector<int> outWlanId;
    std::map<int,int> outAppId;
    std::vector<cComponent*> radios;
    std::vector<cComponent*> queues;
    std::vector<ieee80211::Dcaf*> macDcaf;
    std::vector<MCOChannelConfig> ieeeModes;

    //CBT measurement
    simtime_t cbtWindow;
    std::vector<simsignal_t> cbtSignals;
    bool cbtFirstSample;
    cMessage* cbtSampleTimer;
    std::vector<simtime_t> cbt_idletime;
    std::vector<simtime_t> lu_idletime; //lu = last update
    std::vector<physicallayer::IRadio::ReceptionState> lastReceptionState;
    std::vector<physicallayer::IRadio::TransmissionState> lastTransmissionState;

    //PDR measurement
    simtime_t pdrWindow;
    simtime_t pdrUpdate;
    double pdrDistanceStep;
    unsigned int pdrNumberIntervals;
    std::vector<std::vector<simsignal_t>> pdrSignals;

    cMessage* pdrSampleTimer;
    std::vector<std::map<int,PDR>> pdrAtChannel;
    std::vector<std::vector<int>> nodesInPdrIntervals;
    VehiclesNeighborCache* neighborCache;

    std::vector<int> receivedPacketCount;
    std::vector<int> lastPacketCount;
    std::vector<simsignal_t> receivedPacketCountSignals;
};

} /* namespace inet */

#endif /* INET_APPLICATIONS_VEHICULAR_MGMTMCO_H_ */
