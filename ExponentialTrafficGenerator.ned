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

//
// A particularization of the TrafficGenerator for exponential interpacket generation time
// A simple version could have been implemented just using a exponential function in the ini
// But with this implementation we let the MCO set the packet rate
//

simple ExponentialTrafficGenerator extends TrafficGenerator
{
    parameters:
        @class(ExponentialTrafficGenerator);    
}
