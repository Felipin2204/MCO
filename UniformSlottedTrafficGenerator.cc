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

#include "UniformSlottedTrafficGenerator.h"
#include <algorithm>
#include <random>
#include <numeric>

namespace inet {

Define_Module(UniformSlottedTrafficGenerator);

UniformSlottedTrafficGenerator::UniformSlottedTrafficGenerator() {
    generateSlotsTimer = nullptr;
    packetGenerationTimer = nullptr;
}

UniformSlottedTrafficGenerator::~UniformSlottedTrafficGenerator() {
    if (generateSlotsTimer) cancelAndDelete(generateSlotsTimer);
}

void UniformSlottedTrafficGenerator::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        TrafficGenerator::initialize(stage);

        minimumSlotDuration = par("minimumSlotDuration");
        maximumPacketsPerSecond = ceil(1/minimumSlotDuration);
        generatePacketsFraction = par("generatePacketsFraction");
        packetsToGenerate = floor(generatePacketsFraction*maximumPacketsPerSecond);
        generateSlotsPeriod = par("generateSlotsPeriod");
        //fractionDemand = par("fractionDemand");
        uniqueSlots = par("uniqueSlots");

        packetGenerationTimer = new cMessage("packetGenerationTimer");
    } else if (stage == INITSTAGE_APPLICATION_LAYER) {
        generateSlotsTimer = new cMessage("generateSlotsTimer");

        if (uniqueSlots) generateUniqueSlots();
        else generateSlots();

        //Already added simTime() when created the slots, generateSlots() will generate the first packet

        WATCH(currentSlot);
        WATCH_VECTOR(slots);
        WATCH(packetsToGenerate);
    } else {
        TrafficGenerator::initialize(stage);
    }
}

void UniformSlottedTrafficGenerator::generateUniqueSlots() {
    slots.clear();

    //Generate a number of slots equal to total capacity
    std::vector<int> genSlots(maximumPacketsPerSecond);
    std::iota(begin(genSlots), end(genSlots), 0);

    //Generate a random permutation
    std::mt19937 gen(intuniform(0, maximumPacketsPerSecond-1));
    std::shuffle(begin(genSlots), end(genSlots), gen);

    //Take only the first packetsToGenerate slots
    for (int i = 0; i < packetsToGenerate; i++) {
        slots.push_back(simTime()+genSlots[i]*minimumSlotDuration);
    }

    std::sort(slots.begin(),slots.end());
    scheduleAt(simTime()+generateSlotsPeriod, generateSlotsTimer);
    currentSlot = 0;
    if (!packetGenerationTimer->isScheduled()) {
        scheduleAt(slots[currentSlot], packetGenerationTimer);
        ++currentSlot;
    }
}

void UniformSlottedTrafficGenerator::generateSlots() {
    slots.clear();
    for (int i = 0; i < packetsToGenerate; i++) {
        int slot = intuniform(0, maximumPacketsPerSecond-1);
        //TODO: set the proper time according to the packet length
        slots.push_back(simTime()+slot*minimumSlotDuration);
    }
    std::sort(slots.begin(), slots.end());
    scheduleAt(simTime()+generateSlotsPeriod, generateSlotsTimer);
    currentSlot = 0;
    if (!packetGenerationTimer->isScheduled()) {
        scheduleAt(slots[currentSlot], packetGenerationTimer);
        ++currentSlot;
    }
}

void UniformSlottedTrafficGenerator::handleMessage(cMessage *msg)
{
    if(msg->isSelfMessage()) {
        if (msg == packetGenerationTimer) {
            sendPacket();
            if (currentSlot < slots.size()) {
                scheduleAt(slots[currentSlot], packetGenerationTimer);
                ++currentSlot;
            }
        } else if (msg == generateSlotsTimer) {
            if (uniqueSlots) generateUniqueSlots();
            else generateSlots();
        }
    } else {
        receivePacket(msg);
    }
}

void UniformSlottedTrafficGenerator::setPacketsToGenerate(int packets) {
    Enter_Method_Silent();
    packetsToGenerate = packets;

    if (packetGenerationTimer->isScheduled()) cancelEvent(packetGenerationTimer);
    if (uniqueSlots) generateUniqueSlots();
    else generateSlots();
}

void UniformSlottedTrafficGenerator::setPacketRate(double rate) {
    setPacketsToGenerate(round(rate));
}

} //namespace inet
