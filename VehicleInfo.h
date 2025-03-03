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

//
// Data structure for storing comprehensive vehicle neighbor information.
//
class INET_API VehicleInfo : public cObject {
  public:
    VehicleInfo(int id, int appId, int channelNumberLastUpdate, Coord pos);
    VehicleInfo(const VehicleInfo &info);
    virtual ~VehicleInfo();

    //In INET4.4, the IEEE802.11p standard has 7 channels
    static const int maxNumberOfChannels = 7;

    int id;
    int appId;
    int channelNumberLastUpdate;
    Coord pos;

    std::vector<simtime_t> last_update;
    simtime_t init;
    simtime_t lastUpdatePeriodic;

    int beaconsReceived;

    friend std::ostream& operator<<(std::ostream& out, const VehicleInfo& inf);
    virtual std::string toString() const;
};

} /* namespace inet */

#endif /* INET_APPLICATIONS_VEHICULAR_VEHICLEINFO_H_ */
