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

#include "TRSimulationVehicleInfo.h"

namespace inet {

TRSimulationVehicleInfo::TRSimulationVehicleInfo(int id, int appId, Coord pos) {
  this->id = id;
  this->appId = appId;
  this->pos = pos;
  this->last_update = simTime();
  this->init = simTime();
  this->beaconsReceived = 0;
}

TRSimulationVehicleInfo::TRSimulationVehicleInfo(const TRSimulationVehicleInfo &info) {
    this->id = info.id;
    this->appId = info.appId;
    this->pos = info.pos;
    this->last_update = simTime();
    this->init = simTime();
    this->beaconsReceived = 0;
}

TRSimulationVehicleInfo::~TRSimulationVehicleInfo() {}

std::ostream& operator<<(std::ostream& out, const TRSimulationVehicleInfo& inf)
{
   std::string a = inf.toString();
   return out;
}

std::string TRSimulationVehicleInfo::toString() const {
   std::ostringstream s(std::ostringstream::out);
   s<<"id="<<id<<";appId="<<appId<<";(x,y)="<<pos<<";init="<<init<<";br="<<beaconsReceived;
   return s.str();
}

} /* namespace inet */
