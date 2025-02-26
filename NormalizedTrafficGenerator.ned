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
// A particularization of the TrafficGenerator 
// With this implementation we let the MCO set the maximum normalized load that the generator can send to the channel
// In addition, we select message sizes from a set of fixed sizes
//

simple NormalizedTrafficGenerator extends TrafficGenerator
{
     @class(NormalizedTrafficGenerator);
     string messageSizes=default("200 300 360 455");  //Message sizes in bytes
     @signal[generatedLoad](type=double);
     @statistic[generatedLoad](record=vector);
     @signal[packetDuration](type=double);
     @statistic[packetDuration](record=mean);
}
