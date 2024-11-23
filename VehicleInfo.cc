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

#include "VehicleInfo.h"

namespace inet {

VehicleInfo::VehicleInfo(int id, int appId, int channelNumberLastUpdate, Coord pos) {
    this->id = id;
    this->appId = appId;
    this->channelNumberLastUpdate = channelNumberLastUpdate;
    this->pos = pos;
    last_update.resize(maxNumberOfChannels);
    for (int i = 0; i < last_update.size(); i++) {
        if (i != channelNumberLastUpdate) this->last_update[i] = SIMTIME_ZERO;
        else this->last_update[channelNumberLastUpdate] = simTime(); 
    }
    this->init = simTime();
    this->beaconsReceived = 0;
    if (appId != 0) this->lastUpdatePeriodic = SIMTIME_ZERO;
    else this->lastUpdatePeriodic = simTime();
}

VehicleInfo::VehicleInfo(const VehicleInfo &info) {
    this->id = info.id;
    this->appId = info.appId;
    this->channelNumberLastUpdate = info.channelNumberLastUpdate;
    this->pos = info.pos;
    last_update.resize(maxNumberOfChannels);
    for (int i = 0; i < last_update.size(); i++) {
        if (i != info.channelNumberLastUpdate) this->last_update[i] = SIMTIME_ZERO;
        else this->last_update[info.channelNumberLastUpdate] = simTime();
    }
    this->init = simTime();
    this->beaconsReceived = 0;
    if (info.appId != 0) this->lastUpdatePeriodic = SIMTIME_ZERO;
    else this->lastUpdatePeriodic = simTime();
}

VehicleInfo::~VehicleInfo() {}

std::ostream& operator<<(std::ostream& out, const VehicleInfo& inf)
{
   std::string a = inf.toString();
   return out;
}

std::string VehicleInfo::toString() const {
   std::ostringstream s(std::ostringstream::out);
   s<<"id="<<id<<";appId="<<appId<<";channelNumberLastUpdate="<<channelNumberLastUpdate<<";(x,y)="<<pos<<";init="<<init<<";lu="<<last_update[channelNumberLastUpdate]<<";br="<<beaconsReceived;
   return s.str();
}

} /* namespace inet */
