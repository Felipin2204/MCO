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
        int numNIC;

		//Statistics

    gates:
        input inApp[];
        output outApp[];

        input inWLAN[];
        output outWLAN[];

    submodules:
        packetMultiplexer: PacketMultiplexer {
            parameters:
                @display("p=100,58");
        }
        classifier: <> like IPacketClassifier {
            parameters:
                @display("p=100,159");
        }
        mgmt: MgmtMCO {
            parameters:
                @display("p=320,306");
        }

    connections allowunconnected:
        for i=0..numApplications-1 {
            inApp[i] --> { @display("m=n"); } --> packetMultiplexer.in++;
            mgmt.outApp++ --> { @display("m=n"); } --> outApp[i];
        }
        packetMultiplexer.out --> classifier.in;
        for i=0..numNIC-1 {
            mgmt.outWLAN++ --> { @display("m=s"); } --> outWLAN++;
            inWLAN++ --> { @display("m=s"); } --> mgmt.inWLAN++;
        }
}
