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

package inet.applications.vehicular;
import inet.mobility.base.MovingMobilityBase;

//
// ETSI TR 103 439 mobility model for highway scenarios in vehicular networks.
//
simple HighwayMobility extends MovingMobilityBase
{
    parameters:
        //MobilityBase
        constraintAreaMinX = 0m;
        constraintAreaMinY = 0m;
        constraintAreaMinZ = 0m;
        constraintAreaMaxX = 8000m;
        constraintAreaMaxY = 24m;
        constraintAreaMaxZ = 100m;
        
        double initialX @unit(m) = uniform(this.constraintAreaMinX, this.constraintAreaMaxX);
        //If the highway has 6 lines and each of them is 4 meters wide (constraintAreaMaxY = 6*4 = 24),
        //we want the vehicles to be in the center of the line. So the possible initialY values are: 2, 6, 10, 14, 18, 22.
        double initialY @unit(m) = (floor(uniform(this.constraintAreaMinY, this.constraintAreaMaxY)/4m)*4m) + 2m;
        double initialZ @unit(m) = 2m;
        bool initFromDisplayString = false;
        double speed @unit(mps) = default(normal(30mps, 3mps));
        
        @class(HighwayMobility);
}
