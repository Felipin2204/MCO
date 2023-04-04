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
import inet.applications.vehicular.TrafficGenerator;

simple UniformSlottedTrafficGenerator extends TrafficGenerator
{
    parameters:
        @class(UniformSlottedTrafficGenerator);
        @display("i=block/app");
    
	    int maximumPacketsPerSecond = default(995);
	    double generatedPacketsFraction = default(0.5);
	    double generateSlotsPeriod @unit(s) = default(1s);
	    //minimumPacketDuration = generateSlotsPeriod/maximumPacketsPerSecond
	    double minimumPacketDuration @unit(s) = default(1005us); //Time to transmit a packet, including IFS (58us) and E[cw]*slot (slot=13us)
	    bool uniqueSlots = default(true); //Generate a set of unique slots
}
