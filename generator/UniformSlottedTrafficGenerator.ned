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

package inet.applications.vehicular.generator;

//
// Sends packets in uniform time slots, useful for time-slotted transmission systems.
//
simple UniformSlottedTrafficGenerator extends TrafficGenerator
{
    parameters:
        @class(UniformSlottedTrafficGenerator);

        double minimumSlotDuration @unit(s) = default(1005us); //Time to transmit a packet (752us for 500B (537B with MAC and PHY), including IFS (58us) and cw*slot (slot=13us, 15*13=195us)
        double generatePacketsFraction = default(0.5);
        double generateSlotsPeriod @unit(s) = default(1s);
        //double fractionDemand = default(0.1);
        bool uniqueSlots = default(true); //Generate a set of unique slots
}
