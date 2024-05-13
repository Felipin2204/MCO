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

#include "WrrClassifierCC.h"

namespace inet {

Define_Module(WrrClassifierCC);

void WrrClassifierCC::initialize(int stage) {
    queueing::WrrClassifier::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        for (size_t i = 0; i < (int)consumers.size(); ++i){
            congested.push_back(false);
        }
        WATCH_VECTOR(congested);
    }
}

int WrrClassifierCC::classifyPacket(Packet *packet)
{
    bool isEmpty = true;
    for (int i = 0; i < (int)consumers.size(); ++i) {
        if (consumers[i]->canPushSomePacket(outputGates[i]->getPathEndGate())) {
            if (congested[i]) continue;
            isEmpty = false;
            if (buckets[i] > 0) {
                buckets[i]--;
                return i;
            }
        }
    }
    //If we are here, all channels are congested or can't push a packet, so we should probably drop the packet.
    //Return the index of the bin queue in order to drop the packet.
    if (isEmpty){
        return ((int)consumers.size() - 1);
    }

    int result = ((int)consumers.size() - 1);
    for (int i = 0; i < (int)consumers.size(); ++i) {
        if (!congested[i]) {
            buckets[i] = weights[i];
            if (result == ((int)consumers.size() - 1) && buckets[i] > 0 && consumers[i]->canPushSomePacket(outputGates[i]->getPathEndGate())) {
                buckets[i]--;
                result = i;
            }
        }
    }
    //If we are here, not a single channel can't push a packet, so we should probably drop the packet.
    //Return the index of the bin queue in order to drop the packet.
    return result;
}

void WrrClassifierCC::setState(int c, bool state) {
    congested[c] = state;
}

} //namespace
