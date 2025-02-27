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
import inet.applications.vehicular.MCO;
import inet.queueing.queue.DropTailQueue;

module PredefinedSequentialMCO extends MCO
{
    parameters:
        mgmt.typename = default("PredefinedSequentialMgmtMCO");
        classifier.typename = default("PredefinedPriorityClassifier");

    submodules:
        queue[numChannels]: DropTailQueue {
            parameters:
                @display("p=76,289");
        }
        //In this module we are going to use one queue to drop leftover packets
        binQueue: DropTailQueue {
            parameters:
                @display("p=37,228");
                packetCapacity = 0;
                
        }

    connections allowunconnected:
        for i=0..numChannels-1 {
            classifier.out++ --> queue[i].in;
            queue[i].out --> mgmt.inQueue[i]; //Symbolic connections
        }
        //Connection for the bin queue
        classifier.out++ --> binQueue.in;
}
