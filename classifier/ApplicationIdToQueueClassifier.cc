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

#include "ApplicationIdToQueueClassifier.h"
#include "../TrafficPacket_m.h"

namespace inet {

Register_Class(ApplicationIdToQueueClassifier);

ApplicationIdToQueueClassifier::ApplicationIdToQueueClassifier() {

}

ApplicationIdToQueueClassifier::~ApplicationIdToQueueClassifier() {

}

int ApplicationIdToQueueClassifier::classifyPacket(Packet *packet) const {
    Packet *pkt = static_cast<Packet*>(packet);
    auto p = pkt->peekData<TrafficPacket>();
    EV << packet->getName() << "(" << pkt->getDataLength() << ") arrived from application with index="
            << p->getAppIdentifier() <<". \n";
    return p->getAppIdentifier();
}

} /* namespace inet */
