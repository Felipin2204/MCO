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

#include <string>
#include "MCO.h"
#include "TrafficPacket_m.h"
#include "inet/common/packet/Packet.h"

namespace inet {

Define_Module(MCO);

MCO::MCO() {

}

MCO::~MCO() {

}

void MCO::initialize(){
    indexOutGateWLAN = findGate("outWLAN");
}

void MCO::handleMessage(cMessage *msg){
    if((std::string(msg->getSenderModule()->getName()).compare("application")) == 0) {
        send(msg, indexOutGateWLAN);
    } else {
        Packet *pkt = static_cast<Packet*>(msg);
        auto p = pkt->peekData<TrafficPacket>();
        send(msg, findGate("outApp", p->getAppIdentifier()));
    }
}

} /* namespace inet */
