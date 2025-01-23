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

#include <random>
#include <algorithm>
#include "RandomSequenceWrrClassifier.h"

namespace inet {

Define_Module(RandomSequenceWrrClassifier);

void RandomSequenceWrrClassifier::initialize(int stage) {
    queueing::WrrClassifier::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        for (int i = 0; i < consumers.size(); ++i) {
            sequence.push_back(i);
        }

        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(sequence.begin(), sequence.end(), g);

        WATCH_VECTOR(sequence);
    }
}

int RandomSequenceWrrClassifier::classifyPacket(Packet *packet)
{
    bool isEmpty = true;
    for (int i = 0; i < (int)consumers.size(); ++i) {
        if (consumers[sequence[i]]->canPushSomePacket(outputGates[sequence[i]]->getPathEndGate())) {
            isEmpty = false;
            if (buckets[sequence[i]] > 0) {
                buckets[sequence[i]]--;
                return sequence[i];
            }
        }
    }

    if (isEmpty)
        return -1;

    int result = -1;
    for (int i = 0; i < (int)consumers.size(); ++i) {
        buckets[sequence[i]] = weights[sequence[i]];
        if (result == -1 && buckets[sequence[i]] > 0 && consumers[sequence[i]]->canPushSomePacket(outputGates[sequence[i]]->getPathEndGate())) {
            buckets[sequence[i]]--;
            result = sequence[i];
        }
    }
    return result;
}

} //namespace
