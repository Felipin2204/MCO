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

import inet.physicallayer.wireless.common.contract.packetlevel.IPathLoss;

module ECC68RuralPathLoss like IPathLoss
{
    parameters:
        double n0 = default(2.8);
        double n1 = default(3.3);
        double breakpointDistance0 @unit(m) = default(128m);
        double breakpointDistance1 @unit(m) = default(512m);
        
        bool useFading = default(false);
        double shapeFactor = default(1.0); 
        
        @class(ECC68RuralPathLoss);
        @display("i=block/control");
}
