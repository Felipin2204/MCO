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

package inet.applications.vehicular.evaluation;
import inet.applications.vehicular.MgmtMCO;

//
// Management module for the MCOLoadBalancingCC implementation.
//
simple MCOLoadBalancingCCMgmt extends MgmtMCO
{
    parameters:
        @class(MCOLoadBalancingCCMgmt);
        string maxChannelCapacity = default("0.62 0.62 0.62 0.62 0.62 0.62 0.62"); //MaxCapacity per channel
}
