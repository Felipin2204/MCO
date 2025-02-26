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

#include "NormalizedTrafficGenerator.h"
#include "inet/common/Simsignals.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ITransmission.h"
#include "inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeSet.h"
#include "TrafficPacket_m.h"

namespace inet {

Define_Module(NormalizedTrafficGenerator);
simsignal_t NormalizedTrafficGenerator::generatedLoadSignal = SIMSIGNAL_NULL;
simsignal_t NormalizedTrafficGenerator::packetDurationSignal=SIMSIGNAL_NULL;

void NormalizedTrafficGenerator::initialize(int stage)
{
    TrafficGenerator::initialize(stage);
    if(stage == INITSTAGE_LOCAL) {
        generatedLoad=0.0;
        const char *cstr = par("messageSizes").stringValue();
        messageSizes= cStringTokenizer(cstr).asIntVector();
        generatedLoadSignal=registerSignal("generatedLoad");
        packetDurationSignal=registerSignal("packetDuration");
    } else if(stage == INITSTAGE_LAST) {
        //Again, we are assuming that this app is sending only to its corresponding channel, for simplicity
        //To make it general we would need to subscribe to all radios and check that the transmitted packet is ours
        std::string r = "^.MCO.mgmt";
        auto mco = check_and_cast<MgmtMCO*>(getModuleByPath(r.c_str()));

        int cw=mco->getCw(appId);
        channelConfig=mco->getChannelConfig(appId);
        averagePacketDuration=0.0;
        auto mode=channelConfig.mode->getMode(channelConfig.bitrate,channelConfig.bw);
        double ifs=(mode->getSifsTime()+2*mode->getSlotTime()).dbl();
        contentionTime=ifs+ cw*mode->getSlotTime().dbl();
        for (auto d : messageSizes) {

            std::cout<<"ct="<<contentionTime<<"slot="<<mode->getSlotTime()<<"cw="<<cw<<endl;
            double pd=mode->getDuration(B(d+30)).dbl();
            std::cout<<"d="<<d<<";pd="<<pd<<endl;
            averagePacketDuration += pd ;
        }

        //Compute the average duration to set the corresponding packet rate later on
        averagePacketDuration = (contentionTime+averagePacketDuration)/messageSizes.size();
        WATCH(averagePacketDuration);
        WATCH(measurementPeriod);

        //Subscribe to transmissions to get the duration of packets
        std::string ra="^.wlan["+std::to_string(appId)+"].radio";
        auto radio=getModuleByPath(ra.c_str());
        radio->subscribe(transmissionStartedSignal, this);
    }
}

void NormalizedTrafficGenerator::receiveSignal(cComponent *source,
        simsignal_t signalID, cObject *obj, cObject *details) {
    if (signalID == transmissionStartedSignal) {
        const physicallayer::ITransmission * transmission = check_and_cast<const physicallayer::ITransmission *>(obj);
        double duration=transmission->getDuration().dbl();
        emit(packetDurationSignal, duration );
        generatedLoad += duration;
    }

}

void NormalizedTrafficGenerator::handleMessage(cMessage *msg)
{
    if(msg->isSelfMessage()) {

        //We just check the generated load so far, we could compute the next packet duration first ...
        if (generatedLoad<(normalizedLoad*measurementPeriod)) {
            sendPacket();
        }

        //Set the packet rate according to the normalized load set by MCO
        packetRate=normalizedLoad/averagePacketDuration;
        timeBetweenPackets = simtime_t(exponential(1/packetRate));
        emit(timeBetweenPacketsSignal, timeBetweenPackets);
        scheduleAt(simTime()+timeBetweenPackets, packetGenerationTimer);

    } else {
        receivePacket(msg);

    }
}

void NormalizedTrafficGenerator::sendPacket() {

    int index=intuniformexcl(0, messageSizes.size());
    int size=messageSizes[index];

    auto data = makeShared<TrafficPacket>();
    data->setChunkLength(B(size));
    data->setAppIdentifier(appId);

    //    auto data = makeShared<ByteCountChunk>(B(packetLength), 0);

    //data->enableImplicitChunkSerialization = true;

    char buffer [20];
    sprintf(buffer, "App%d-Packet%d", appId, generatedPackets);
    Packet *newpacket = new Packet(buffer, data);

    sendDown(newpacket);

    generatedPackets++;
    emit(generatedPacketsSignal, generatedPackets);
}

void NormalizedTrafficGenerator::resetPeriod() {
    emit(generatedLoadSignal,generatedLoad);
    generatedLoad=0.0;
}

} //namespace
