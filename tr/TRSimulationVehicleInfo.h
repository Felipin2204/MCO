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

#ifndef INET_APPLICATIONS_VEHICULAR_TRSIMULATIONVEHICLEINFO_H_
#define INET_APPLICATIONS_VEHICULAR_TRSIMULATIONVEHICLEINFO_H_

#include "inet/common/INETDefs.h"
#include "inet/common/geometry/common/Coord.h"

namespace inet {

class INET_API TRSimulationVehicleInfo : public cObject {
  public:
    TRSimulationVehicleInfo(int id, int appId, Coord pos);
    TRSimulationVehicleInfo(const TRSimulationVehicleInfo &info);
    virtual ~TRSimulationVehicleInfo();

    int id;
    int appId;
    Coord pos;

    simtime_t last_update;
    simtime_t init;

    int beaconsReceived;

    friend std::ostream& operator<<(std::ostream& out, const TRSimulationVehicleInfo& inf);
    virtual std::string toString() const;
};

} /* namespace inet */

#endif /* INET_APPLICATIONS_VEHICULAR_TRSIMULATIONVEHICLEINFO_H_ */
