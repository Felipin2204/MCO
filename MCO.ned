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

import inet.queueing.common.PacketMultiplexer;
import inet.queueing.contract.IPacketClassifier;
import inet.queueing.contract.IPacketQueue;
import inet.queueing.contract.IPacketScheduler;

module MCO like IMCO
{
    parameters:
        @display("i=block/cogwheel");
        
        int numApplications;
        int numChannels;

    gates:
        input inApp[numApplications];
        output outApp[numApplications];

        input inWLAN[numChannels];
        output outWLAN[numChannels];

    submodules:
        packetMultiplexer: PacketMultiplexer {
            parameters:
                @display("p=76,58");
        }
        classifier: <> like IPacketClassifier {
            parameters:
                @display("p=76,157");
        }
        mgmt: <default("MgmtMCO")> like IMgmtMCO {
            parameters:
                @display("p=199,289");
                numChannels = parent.numChannels;
                numApplications = parent.numApplications;
        }
        interferenceMonitor: <default("AdjacentChannelInterferenceMonitor")> like IInterferenceMonitor if typename != "" {
            parameters:
                @display("p=300,200");
                numChannels = parent.numChannels;
        }
        vehicleTable: <default("VehicleTable")> like IVehicleTable {
              parameters:
                @display("p=450,200");
        }

    connections allowunconnected:
        for i=0..numApplications-1 {
            inApp[i] --> { @display("m=n"); } --> packetMultiplexer.in++;
            mgmt.outApp[i] --> { @display("m=n"); } --> outApp[i];
        }
        packetMultiplexer.out --> classifier.in;
        for i=0..numChannels-1 {
            mgmt.outWLAN[i] --> { @display("m=s"); } --> outWLAN[i];
            inWLAN[i] --> { @display("m=s"); } --> mgmt.inWLAN[i];
        }
}
