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

#ifndef INET_APPLICATIONS_VEHICULAR_VEHICLEINFO_H_
#define INET_APPLICATIONS_VEHICULAR_VEHICLEINFO_H_
#include "inet/common/INETDefs.h"
#include "inet/common/geometry/common/Coord.h"

namespace inet {

class INET_API VehicleInfo : public cObject {
  public:
    VehicleInfo(int id, int appId, Coord pos);
    VehicleInfo(const VehicleInfo &info);
    virtual ~VehicleInfo();

    int id;
    int appId;
    Coord pos;

    simtime_t last_update;
    simtime_t init;

    int beaconsReceived;
    std::string description; //This is a hack to be able to see the description in GUI with polymorphism

    friend std::ostream& operator<<(std::ostream& out, const VehicleInfo& inf);
    virtual std::string  toString() const;
};

} /* namespace inet */

#endif /* INET_APPLICATIONS_VEHICULAR_VEHICLEINFO_H_ */
