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

#include "PredefinedPriorityClassifier.h"

namespace inet {

Define_Module(PredefinedPriorityClassifier);

void PredefinedPriorityClassifier::initialize(int stage) {
    queueing::PriorityClassifier::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *cstr = par("sequence").stringValue();
        sequence = cStringTokenizer(cstr).asIntVector();
        if (sequence.size() != consumers.size()) {
            throw cRuntimeError("The  sequence size has to be equal to the number of queues/consumers");
        }
        for (size_t i = 0; i < sequence.size(); ++i){
            congested.push_back(false);
        }
        WATCH_VECTOR(congested);
    }
}

int PredefinedPriorityClassifier::classifyPacket(Packet *packet) {
    for (size_t i = 0; i < sequence.size(); i++) {
        size_t outputGateIndex = getOutputGateIndex(sequence[i]);
        if (congested[outputGateIndex] == false) {
            if (consumers[outputGateIndex]->canPushPacket(packet, outputGates[outputGateIndex])) {
                return outputGateIndex;
            }
        }
    }
    //If we are here, all channels are congested, we should probably drop the packet, but it is left to the queue.
    //FIXME: The queue doesn't drop the packet unless it's full (dropTailQueue).
    //TODO: queues should have a capacity equal to the max load but the inet API does not allow to set dynamically the max capacity
    //so we have to do a custom queue. In any case we would have to adapt it to the measured load...difficult

    //Just return a random channel
    return getOutputGateIndex(intuniform(0,sequence.size()-1)); //The packet is going to be transmitted by a random channel, although this channel is congested
}

void PredefinedPriorityClassifier::setState(int c, bool state) {
    congested[c] = state;
}

} //namespace
