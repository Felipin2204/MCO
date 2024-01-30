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
        if (sequence.size() != consumers.size()-1) {
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
    //If we are here, all channels are congested, we should probably drop the packet.
    //Return the index of the bin queue in order to drop the packet
    return getOutputGateIndex(sequence.size());
}

void PredefinedPriorityClassifier::setState(int c, bool state) {
    congested[c] = state;
}

} //namespace
