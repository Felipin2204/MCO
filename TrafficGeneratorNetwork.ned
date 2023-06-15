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

import inet.physicallayer.wireless.common.contract.packetlevel.IRadioMedium;


network TraficGeneratorNetwork
{
    parameters:
        int nodes;

    submodules:
        radioMedium: <default("Ieee80211ScalarRadioMedium")> like IRadioMedium {
            @display("p=0,0;is=vs");
            analogModel.ignorePartialInterference = true;
            mediumLimitCache.centerFrequency = 5.9GHz;
        }
        node[nodes]: TrafficGeneratorHost {
            id = index;
            @display("b=100,100,oval,,black,1");
        }
        nodeManager: <default("")> like INodeManager if typename != "" {
            @display("p=0,0;is=vs");
        }
}
