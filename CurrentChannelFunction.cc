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

#include "inet/queueing/function/PacketSchedulerFunction.h"
#include "MgmtMCO.h"

namespace inet {
namespace queueing {

static int schedulePacketByCurrentChannel(const std::vector<IPassivePacketSource *>& sources)
{
    MgmtMCO *test = check_and_cast<MgmtMCO*>(getSimulation()->getSystemModule()->getSubmodule("node", 0)->getSubmodule("MCO")->getSubmodule("mgmt"));
    return test->getCurrentChannelNumber();
}

Register_Packet_Scheduler_Function(PacketCurrentChannel, schedulePacketByCurrentChannel);

} // namespace queueing
} /* namespace inet */
