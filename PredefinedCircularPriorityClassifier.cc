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

#include "PredefinedCircularPriorityClassifier.h"

namespace inet {

Define_Module(PredefinedCircularPriorityClassifier);

void PredefinedCircularPriorityClassifier::initialize(int stage) {
    PredefinedPriorityClassifier::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        lastClassifiedChannelIndex = 0;
        WATCH(lastClassifiedChannelIndex);
    }
}

int PredefinedCircularPriorityClassifier::classifyPacket(Packet *packet) {
    for (size_t i = 0; i < sequence.size(); i++) {
        size_t nextChannelIndex = (lastClassifiedChannelIndex + i) % sequence.size();
        size_t outputGateIndex = getOutputGateIndex(sequence[nextChannelIndex]);
        if (congested[outputGateIndex] == false) {
            if (consumers[outputGateIndex]->canPushPacket(packet, outputGates[outputGateIndex])) {
                lastClassifiedChannelIndex = nextChannelIndex;
                return outputGateIndex;
            }
        }
    }
    //If we are here, all channels are congested, we should probably drop the packet.
    //Return the index of the bin queue in order to drop the packet
    return getOutputGateIndex(sequence.size());
}

} //namespace
