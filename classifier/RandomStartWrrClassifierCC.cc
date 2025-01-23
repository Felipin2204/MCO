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

#include "RandomStartWrrClassifierCC.h"

#include <random>
#include <algorithm>

namespace inet {

Define_Module(RandomStartWrrClassifierCC);

void RandomStartWrrClassifierCC::initialize(int stage) {
    queueing::WrrClassifier::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *cstr = par("sequence").stringValue();
        sequence = cStringTokenizer(cstr).asIntVector();
        if (sequence.size() != consumers.size() - 1) { //consumers.size() - 1 because with CC we also have the binQueue
            throw cRuntimeError("The sequence size has to be equal to the number of queues/consumers");
        }

        //Shift the sequence a random number of times
        std::random_device rd;
        std::mt19937 g(rd());
        std::uniform_int_distribution<size_t> dist(0, sequence.size() - 1);
        size_t shift = dist(g);
        std::rotate(sequence.begin(), sequence.begin() + shift, sequence.end());
        WATCH_VECTOR(sequence);

        //Initialize the congested vector
        for (size_t i = 0; i < (int)sequence.size(); ++i){
            congested.push_back(false);
        }
        WATCH_VECTOR(congested);
    }
}

int RandomStartWrrClassifierCC::classifyPacket(Packet *packet)
{
    bool isEmpty = true;
    for (int i = 0; i < (int)sequence.size(); ++i) {
        if (consumers[sequence[i]]->canPushSomePacket(outputGates[sequence[i]]->getPathEndGate())) {
            if (congested[sequence[i]]) continue;
            isEmpty = false;
            if (buckets[sequence[i]] > 0) {
                buckets[sequence[i]]--;
                return sequence[i];
            }
        }
    }
    if (isEmpty){
        //If we are here, all channels are congested or can't push a packet, so we should probably drop the packet.
        //Return the index of the bin queue in order to drop the packet.
        return ((int)consumers.size() - 1);
    }

    int result = ((int)consumers.size() - 1);
    for (int i = 0; i < (int)sequence.size(); ++i) {
        if (!congested[sequence[i]]) {
            buckets[sequence[i]] = weights[sequence[i]];
            if (result == ((int)consumers.size() - 1) && buckets[sequence[i]] > 0 && consumers[sequence[i]]->canPushSomePacket(outputGates[sequence[i]]->getPathEndGate())) {
                buckets[sequence[i]]--;
                result = sequence[i];
            }
        }
    }
    //If we are here, not a single channel can't push a packet, so we should probably drop the packet.
    //Return the index of the bin queue in order to drop the packet.
    return result;
}

void RandomStartWrrClassifierCC::setState(int c, bool state) {
    congested[c] = state;
}

} //namespace
