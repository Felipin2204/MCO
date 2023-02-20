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

#ifndef INET_APPLICATIONS_VEHICULAR_MCORATECONTROL_H_
#define INET_APPLICATIONS_VEHICULAR_MCORATECONTROL_H_

#include "inet/linklayer/ieee80211/mac/ratecontrol/RateControlBase.h"

namespace inet {

class MCORateControl : public ieee80211::RateControlBase
{
  protected:
    simtime_t timer = SIMTIME_ZERO;
    simtime_t interval = SIMTIME_ZERO;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    virtual void resetTimer();
    virtual void increaseRateIfTimerIsExpired();
    virtual void updateDisplayString() const;

  public:
    virtual const physicallayer::IIeee80211Mode *getRate() override;
    virtual void frameTransmitted(Packet *frame, int retryCount, bool isSuccessful, bool isGivenUp) override;
    virtual void frameReceived(Packet *frame) override;
};

} /* namespace inet */

#endif /* INET_APPLICATIONS_VEHICULAR_MCORATECONTROL_H_ */
