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

#ifndef INET_APPLICATIONS_VEHICULAR_CURRENTCHANNELFUNCTION_H_
#define INET_APPLICATIONS_VEHICULAR_CURRENTCHANNELFUNCTION_H_

#include <omnetpp.h>
#include "inet/queueing/contract/IPacketSchedulerFunction.h"
#include "inet/queueing/contract/IPassivePacketSource.h"

using namespace omnetpp;

namespace inet {

class CurrentChannelFunction : public queueing::IPacketSchedulerFunction, public cObject {
  public:
    virtual int schedulePacket(const std::vector<queueing::IPassivePacketSource *>& sources) const override;
};

} /* namespace inet */

#endif /* INET_APPLICATIONS_VEHICULAR_CURRENTCHANNELFUNCTION_H_ */
