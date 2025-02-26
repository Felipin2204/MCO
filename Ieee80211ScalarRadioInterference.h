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

#ifndef __INET4_4_IEEE80211SCALARRADIOINTERFERENCE_H_
#define __INET4_4_IEEE80211SCALARRADIOINTERFERENCE_H_

#include <omnetpp.h>
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Radio.h"
using namespace omnetpp;

namespace inet {
namespace physicallayer {

class Ieee80211ScalarRadioInterference : public Ieee80211Radio
{
public:
    static simsignal_t receptionAttemptedSignal;
    static simsignal_t receptionNotAttemptedSignal;
protected:
    virtual void startReception(cMessage *timer, IRadioSignal::SignalPart part) override;

};

} //namespace
}
#endif
