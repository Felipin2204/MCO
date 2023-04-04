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
    cancelAndDelete(packetGenerationTimer);
    if (generateSlotsTimer) {
        cancelAndDelete(generateSlotsTimer);
    }
}

void UniformSlottedTrafficGenerator::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        TrafficGenerator::initialize(stage);

        maximumPacketsPerSecond = par("maximumPacketsPerSecond");
        generatedPacketsFraction = par("generatedPacketsFraction");
        packetsToGenerate = floor(generatedPacketsFraction*maximumPacketsPerSecond);
        minimumPacketDuration = par("minimumPacketDuration");
        generateSlotsPeriod = par("generateSlotsPeriod");
        uniqueSlots = par("uniqueSlots");

        packetGenerationTimer = new cMessage("packetGenerationTimer");
    } else if (stage == INITSTAGE_APPLICATION_LAYER) {
        generateSlotsTimer = new cMessage("generateSlotsTimer");

        if (uniqueSlots) generateUniqueSlots();
        else generateSlots();

        //Already added simTime() when created the slots, generateSlots() will generate the first packet

        WATCH(currentSlot);
        WATCH_VECTOR(slots);
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
        slots.push_back(simTime()+genSlots[i]*minimumPacketDuration);
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
        slots.push_back(simTime()+slot*minimumPacketDuration);
    }
    std::sort(slots.begin(), slots.end());
    scheduleAt(simTime()+generateSlotsPeriod, generateSlotsTimer);
    currentSlot = 0;
    if (!packetGenerationTimer->isScheduled()) {
        scheduleAt(slots[currentSlot], packetGenerationTimer);
        ++currentSlot;

        ++generatedPackets;
        emit(generatedPacketsSignal, generatedPackets);
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

} //namespace inet
