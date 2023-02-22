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
        int numApplications;

		scheduler.weights = "1 1 1 1 1 1 1";

		//Statistics

    gates:
        input inApp[];
        output outApp[];

        input inWLAN;
        output outWLAN;

    submodules:
        packetMultiplexer: PacketMultiplexer {
            parameters:
                @display("p=100,100");
        }
        classifier: <> like IPacketClassifier {
            parameters:
                @display("p=250,100");
        }
        queue[7]: <> like IPacketQueue {
            parameters:
                @display("p=400,100");
        }
        scheduler: <default("MyScheduler")> like IPacketScheduler {
            parameters:
                @display("p=550,100");
        }
        mgmt: MgmtMCO {
            parameters:
                @display("p=550,300");
        }

    connections allowunconnected:
        for i=0..numApplications-1 {
            inApp[i] --> { @display("m=w"); } --> packetMultiplexer.in++;
            mgmt.outApp++ --> { @display("m=w"); } --> outApp[i];
        }
        packetMultiplexer.out --> classifier.in;
        for i=0..6 {
            classifier.out++ --> queue[i].in;
            queue[i].out --> scheduler.in++;
        }
        scheduler.out --> mgmt.inScheduler;
        mgmt.outWLAN --> { @display("m=e"); } --> outWLAN;
        inWLAN --> mgmt.inWLAN;
}
