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

#include "MinosVehicleInfo.h"

namespace inet {

MinosVehicleInfo::MinosVehicleInfo(int id, int appId,  int channelNumberLastUpdate, Coord pos, std::vector<double> channelWeigths, std::vector<double> channelLoads, std::vector<double> power) : VehicleInfo(id,appId, channelNumberLastUpdate, pos) {
    // TODO Auto-generated constructor stub

    this->channelWeigths=channelWeigths;
    this->power=power;
    this->channelLoads=channelLoads;

}

MinosVehicleInfo::~MinosVehicleInfo() {
    // TODO Auto-generated destructor stub
}

MinosVehicleInfo::MinosVehicleInfo(const MinosVehicleInfo &info) : VehicleInfo(info)  {


    this->channelWeigths=info.channelWeigths;
    this->power=info.power;
    this->channelLoads=info.channelLoads;
}

std::ostream& operator<<(std::ostream& out, const MinosVehicleInfo& inf)
{
    std::string a=inf.toString();
    out<<a;
    return out;


}
std::string  MinosVehicleInfo::toString() const {
    std::ostringstream s(std::ostringstream::out);

    s <<"id="<<id<<";(x,y)="<<pos<<";weigths=[";
    for (auto w : channelWeigths) {
        s<<w<<",";
    }
    s<<"];loads=[";
    for (auto l : channelLoads) {
           s<<l<<",";
       }
    s<<"];powers=[";
    for (auto p : power) {
            s<<p<<",";
        }

    s<<"];init="<<init<<";lu="<<last_update[channelNumberLastUpdate]<<";br="<<beaconsReceived;

    return s.str();
}

} /* namespace inet */
