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
#include "inet/queueing/queue/PacketQueue.h"
#include "inet/queueing/scheduler/PacketScheduler.h"

using namespace omnetpp;

namespace inet {

class MgmtMCO : public cSimpleModule, public cListener {
  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override {return NUM_INIT_STAGES;}
    virtual void handleMessage(cMessage *msg) override;

    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    cGate *inSchedulerGate = nullptr;
    queueing::PacketScheduler *scheduler = nullptr;
    physicallayer::Ieee80211Radio *radio = nullptr;
    ieee80211::Dcaf *macDcaf = nullptr;

    cMessage *radioTimeDivisionTimer;
    queueing::PacketQueue *currentQueue;
    int numChannels;
    int currentChannelNumber;

  public:
    int getCurrentChannelNumber() const {return currentChannelNumber;}
};

} /* namespace inet */

#endif /* INET_APPLICATIONS_VEHICULAR_MGMTMCO_H_ */
