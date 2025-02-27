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

#include "OptimalMgmtMCOBase.h"

#include "inet/linklayer/common/Ieee802SapTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211ScalarTransmitter.h"
#include "MinosMCOPacket_m.h"
#include "../generator/TrafficGenerator.h"
#include "../generator/UniformSlottedTrafficGenerator.h"

#define UPDATE_WS_TO 1005
#define UPDATE_BR_TO 1006

namespace inet {

Define_Module(OptimalMgmtMCOBase);

simsignal_t OptimalMgmtMCOBase::iterationsDoneSignal =registerSignal("iterationsDone");

OptimalMgmtMCOBase::OptimalMgmtMCOBase() :    updateChannelTimer(nullptr), updateWeightTimer(nullptr), minfo(nullptr) {
}

OptimalMgmtMCOBase::~OptimalMgmtMCOBase() {
    cancelAndDelete(updateChannelTimer);
    cancelAndDelete(updateWeightTimer);
    if (minfo) {
        delete minfo;
    }

}

void OptimalMgmtMCOBase::initialize(int stage) {
    MgmtMCO::initialize(stage);
    if(stage == INITSTAGE_LOCAL) {
        weightSignals.resize(numChannels);
        subgradientSignals.resize(numChannels);
        packetSignals.resize(numChannels);
        loadSignals.resize(numChannels);
        initWeight =par("initWeight");
        useCBTCorrection=par("useCBTCorrection");
        seqNumber=0;

        for ( int i=0; i<numChannels;++i) {
            channelWeights.push_back(initWeight);
            channelLoads.push_back(0.0);
            subgradients.push_back(0);
            powers.push_back(0.0);
            packetsPerChannel.push_back(0);
            currentDemandPerChannel.push_back(0);
            tmin.push_back(0.0);

            //Signals for statistics
            std::string sname("weight");
            sname+=std::to_string(i);
            weightSignals[i]=registerSignal(sname.c_str());

            cProperty *statisticTemplate =
                    getProperties()->get("statisticTemplate", "weight");
            getEnvir()->addResultRecorders(this, weightSignals[i], sname.c_str(), statisticTemplate);
            std::string tname("subgradient");
            tname+=std::to_string(i);
            subgradientSignals[i]=registerSignal(tname.c_str());

            statisticTemplate =
                    getProperties()->get("statisticTemplate", "subgradient");
            getEnvir()->addResultRecorders(this, subgradientSignals[i], tname.c_str(), statisticTemplate);

            std::string pname("packets");
            pname+=std::to_string(i);
            packetSignals[i]=registerSignal(pname.c_str());

            statisticTemplate =
                    getProperties()->get("statisticTemplate", "packets");
            getEnvir()->addResultRecorders(this, packetSignals[i], pname.c_str(), statisticTemplate);

            std::string lname("load");
            lname+=std::to_string(i);
            loadSignals[i]=registerSignal(lname.c_str());

            statisticTemplate =
                    getProperties()->get("statisticTemplate", "load");
            getEnvir()->addResultRecorders(this, loadSignals[i], lname.c_str(), statisticTemplate);


        }

        alpha=par("alpha");
        gammaStep=par("gammaStep");
        useCBT=par("useCBT");

        regularized=par("regularized");
        regParameter=par("regParameter");
        syncPeriod = par("syncPeriod");

        minimumSlotDuration=par("minimumSlotDuration");
        packetDuration=par("packetDuration");
        const char *cstr = par("channelMaxCapacity").stringValue();
        channelMaxCapacity= cStringTokenizer(cstr).asDoubleVector();
        if (channelMaxCapacity.size()!=numChannels) {
            throw cRuntimeError("channelMaxCapacity size has to be equal to numChannels");
        }
        channelMaxCapacityPackets.resize(numChannels);
        for (unsigned int i=0; i<channelMaxCapacity.size(); ++i) {
            channelMaxCapacityPackets[i]=channelMaxCapacity[i]/minimumSlotDuration;
        }
        const char *ostr = par("channelOmega").stringValue();
        channelOmega= cStringTokenizer(ostr).asDoubleVector();
        if (channelOmega.size()!=numChannels) {
            throw cRuntimeError("channelOmega size has to be equal to numChannels");
        }
        updateWeightTimer=new cMessage("UPDATE_WS_TO",UPDATE_WS_TO);
        updateChannelTimer = new cMessage("UPDATE_CHANNEL_TO",UPDATE_BR_TO);
        //With this variable we alternate weight and lagrangian, to compute it with updated weights
        //otherwise we are solving the inner problem with previous weights
        updateWeight=true;
        //Synchronous update to compute gradients and new allocations
        simtime_t in=simTime().trunc(SIMTIME_S) + syncPeriod;
        scheduleAt(in, updateChannelTimer);
        if (useCBT) { //If using CBT, it has to be measured everytime the gradient is computed, so we cancel the current timer and let
            //our algorithm schedule it again when necessary
            if (cbtSampleTimer->isScheduled()) {
                cancelEvent(cbtSampleTimer);
            }
        }

        if (useCBTCorrection) {
            //We may want to correct the CBT to get the correct number of packets measured on the channel when it is compared
            //the load in packets to the MBL (Cmax) in packets. But for every transmitted packet there is some idle time (IFS, cw, etc.)
            //so we underestimate the actual load if we do not take it into account. That is for a fully loaded channel
            //in terms of packets, the CBT is not 1 but only 0.75 (for this particular packet length)
            //Of course, the algorithm works with CBT if we use the correction
            //The alternative is to count the actually sent and receive packets, but then the collisions are not counted...
            //It is usually better to disable this and let the algorithm adapt to the actual measured CBT

            cbtCorrection=minimumSlotDuration/packetDuration;
        } else {
            cbtCorrection=1.0;
        }

    } else if(stage == INITSTAGE_APPLICATION_LAYER) {
        mvt=check_and_cast<MinosVehicleTable*>(vehicleTable);
        for ( int i=0; i<numChannels; i++) {
            std::string r="^.^.wlan["+std::to_string(i)+"].radio.transmitter";
            auto tx=check_and_cast<physicallayer::Ieee80211ScalarTransmitter*>(getModuleByPath(r.c_str()));
            powers.push_back(tx->getPower().get());
        }
        for ( int i=0; i<numApplications; i++) {
            std::string r="^.^.application["+std::to_string(i)+"]";
            auto app=check_and_cast<cComponent*>(getModuleByPath(r.c_str()));
            applications.push_back(app);

            //A more general approach is to let applications inform of the packet size and compute duration
            //But still we may need to add one duration per channel... left as future work
            /*
            int cw=getCw(i);
            auto channelConfig=getChannelConfig(i);
            auto mode=channelConfig.mode->getMode(channelConfig.bitrate,channelConfig.bw);
            double ifs=(mode->getSifsTime()+2*mode->getSlotTime()).dbl();
            double contentionTime=ifs+ cw*mode->getSlotTime().dbl();
            int packetSize=check_and_cast<TrafficGenerator*>(app)->getPacketLength();
            packetDuration=mode->getDuration(B(packetSize+30)).dbl();
            minimumSlotDuration=contentionTime+packetDuration;
             */

            //Ask for demands to the applications
            currentDemandPerChannel[i]=check_and_cast<TrafficGenerator*>(app)->getTrafficDemand();
            double minLoad=check_and_cast<TrafficGenerator*>(app)->getMinimumNormalizedLoad();
            if (minLoad<0) {
                tmin[i]=check_and_cast<TrafficGenerator*>(app)->getMinimumTrafficDemand();
                tmin[i]= tmin[i]*minimumSlotDuration;
            } else {
                tmin[i]=minLoad;
            }


        }
        Coord pos(-1,-1);
        minfo=new MinosVehicleInfo(-1,-1,-1,pos, channelWeights,channelLoads,powers);


        WATCH_VECTOR(currentDemandPerChannel);
        WATCH_VECTOR(channelWeights);
        WATCH_VECTOR(subgradients);
        WATCH_VECTOR(packetsPerChannel);
        WATCH_VECTOR(channelMaxCapacityPackets);
        WATCH_VECTOR(channelMaxCapacity);


    }
}

void OptimalMgmtMCOBase::handleMessage(cMessage *msg)
{
    MgmtMCO::handleMessage(msg);
}

Packet* OptimalMgmtMCOBase::createMCOPacket(Packet *packet, int channel) {
    // computePDR(channel);
    auto data = makeShared<MinosMCOPacket>();
    auto p = packet->peekData<TrafficPacket>();
    Coord pos=mob->getCurrentPosition();
    data->setChunkLength(B(p->getChunkLength()));
    data->setAppIdentifier(p->getAppIdentifier());
    data->setPosition(pos);
    data->setSource(myId);
    data->setChannel(channel);
    data->setSequenceNumber(seqNumber);
    data->setChannelWeigthsArraySize(channelWeights.size());
    data->setChannelCBTArraySize(channelLoads.size());

    for (unsigned int i=0; i<channelWeights.size();++i) {

        data->setChannelWeigths(i, channelWeights[i]);
    }
    for (unsigned int i=0; i<channelLoads.size();++i) {

        data->setChannelCBT(i, channelLoads[i]);
    }


    //        auto data = makeShared<ByteCountChunk>(B(packetLength), 0);

    //data->enableImplicitChunkSerialization = true;


    Packet *newpacket = new Packet("MinosMCOPacket", data);
    //std::cout<<myId<<":"<<simTime()<<":packet Create MCO "<< newpacket->getDataLength() <<"sn="<<data->getSequenceNumber()<<std::endl;
    //Add SAP. I think we may remove this
    newpacket->addTagIfAbsent<Ieee802SapReq>()->setDsap(SapCode::SAP_IP);
    newpacket->addTagIfAbsent<MacAddressReq>()->setDestAddress(MacAddress::BROADCAST_ADDRESS);
    //Should put something sensible here. Keep this to prevent LlcEpd from complaining
    newpacket->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ipv4);
    MCOSent++;
    seqNumber++;
    return newpacket;
}

void OptimalMgmtMCOBase::updateVehicleInfo(Packet *pkt) {
    auto p=pkt->peekAtFront<MinosMCOPacket>(pkt->getDataLength());
    minfo->appId=p->getAppIdentifier();
    minfo->id =p->getSource();
    minfo->pos=p->getPosition();
    minfo->channelNumberLastUpdate = p->getChannel();
    std::vector<double> w(p->getChannelWeigthsArraySize());
    for (unsigned int i=0; i<p->getChannelWeigthsArraySize();++i) {
        w[i]=p->getChannelWeigths(i);
        //std::cout<<myId<<":id_n="<<minfo->id<<":w["<<i<<"]="<<w[i]<<std::endl;
    }
    std::vector<double> l(p->getChannelCBTArraySize());
    for (unsigned int i=0; i<p->getChannelCBTArraySize();++i) {
        l[i]=p->getChannelCBT(i);
        //std::cout<<myId<<":id_n="<<minfo->id<<";l["<<i<<"]="<<l[i]<<std::endl;
    }

    minfo->channelWeigths=w;
    minfo->channelLoads=l;
    minfo->power=powers;
    //minfo->description=minfo->toString();
    mvt->insertOrUpdate(minfo);
    //VehicleTable::VTable::iterator it = mvt->vt.find(minfo->id);
}

} //namespace
