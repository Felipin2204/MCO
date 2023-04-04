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

VehicleInfo::VehicleInfo(int id, int appId, Coord pos) {
  this->id=id;
  this->appId=appId;
  this->pos=pos;
  this->last_update = simTime();
  this->init = simTime();
  this->beaconsReceived = 0;
}

VehicleInfo::VehicleInfo(const VehicleInfo &info) {
    this->id = info.id;
    this->appId = info.appId;
    this->pos = info.pos;
    this->last_update = simTime();
    this->init = simTime();
    this->beaconsReceived = 0;
}

VehicleInfo::~VehicleInfo() {}

std::ostream& operator<<(std::ostream& out, const VehicleInfo& inf)
{
   //std::string a=inf.toString();
   out << inf.description;
   return out;
}

std::string VehicleInfo::toString() const {
   std::ostringstream s(std::ostringstream::out);
   s <<"id="<<id<<";appId="<<appId<<";(x,y)="<<pos<<";init="<<init<<";lu="<<last_update<<";br="<<beaconsReceived;
   return s.str();
}

} /* namespace inet */
