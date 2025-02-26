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

#include "AdjacentChannelInterferenceMonitor.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IReception.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211ScalarTransmission.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Radio.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/FlatReceptionBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"
#include "inet/common/Simsignals.h"

using namespace omnetpp;

namespace inet {

Define_Module(AdjacentChannelInterferenceMonitor);

void AdjacentChannelInterferenceMonitor::initialize(int stage)
{
    if(stage == INITSTAGE_LOCAL) {
        numChannels = par("numChannels");
        maxChannelSeparation = par("maxChannelSeparation");
        if (numChannels <= maxChannelSeparation) {
            throw cRuntimeError("maxChannelSeparation has to be lower than numChannels");
        }
        const char *vstr = par("adjacentLoss").stringValue();
        adjacentLoss = cStringTokenizer(vstr).asDoubleVector();
        //Work in W, since all the powers below are provided in W
        for (unsigned int i = 0; i < adjacentLoss.size(); ++i) {
            adjacentLoss[i] = pow(10.0, adjacentLoss[i]/10.0);
            //std::cout << "adjacentLoss[i]" << adjacentLoss[i] << endl;
        }
        interferenceOnTransmission = par("interferenceOnTransmission");

        myNode = getParentModule()->getParentModule()->getIndex();
        createSignals();
    } else if(stage == INITSTAGE_LINK_LAYER) {
        for (int i = 0; i < numChannels; i++) {
            std::string r = "^.^.wlan[" + std::to_string(i) + "].radio";
            auto radio = check_and_cast<physicallayer::Radio*>(getModuleByPath(r.c_str()));
            radio->subscribe("receptionStarted", this); //TODO: Don't use in the receiveSignal function
            radios.push_back(radio);
        }
        //getModuleByPath("<root>.radioMedium")->subscribe("signalArrivalStarted", this);
        getModuleByPath("<root>.radioMedium")->subscribe("signalAdded", this);
    }
}

void AdjacentChannelInterferenceMonitor::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) {
    //TODO: Adjacent channel interference is not taken into account by the receiver when computing the SNIR
    //At least for scalar signals (packetlevel)
    //See inet/physicallayer/wireless/common/base/packetlevel/ScalarAnalogModelBase.cc computeNoise function
    //If we want to compute associated errors, we have to incorporate them
    //At the moment we are only interested in computing interference levels, not in higher level effects such as PER or inter beacon reception time

    //Pick up a new transmission
    if (signalID == physicallayer::IRadioMedium::signalAddedSignal) {
        auto transmission = check_and_cast<const physicallayer::Ieee80211ScalarTransmission*>(obj);
        auto txNode = transmission->getTransmitter()->getRadioGate()->getOwnerModule(); //Radio
        int txNodeIndex = txNode->getParentModule()->getParentModule()->getIndex(); //Node index
        int txChannel = transmission->getChannel()->getChannelNumber();
        auto medium = transmission->getTransmitter()->getMedium();
        //std::cout<<myNode<<"*"<<simTime()<<": Signal "<< transmission->getId()<<" transmitted on channel "<<txChannel<<" from node "<<txNodeIndex<<endl;

        //Consider only adjacent channels
        for (int channel = txChannel-maxChannelSeparation; channel < txChannel+maxChannelSeparation+1; ++channel) {
            if (channel<0 || channel==txChannel) {
                continue;
            } else if (channel>numChannels-1) {
                break;
            } else {
                int deltaChannel = std::abs(txChannel-channel);
                auto radio = check_and_cast<physicallayer::Ieee80211Radio*>(radios[channel]);

                //Get all the interfering packets (overlapped packet durations) for the transmission picked
                //auto interference = radio->getMedium()->getInterference(radio, transmission)->getInterferingReceptions();
                physicallayer::ICommunicationCache* comCache = const_cast<physicallayer::ICommunicationCache*>(radio->getMedium()->getCommunicationCache());
                auto interference = comCache->computeInterferingTransmissions(radio, transmission->getStartTime(), transmission->getEndTime());
                //std::cout<<myNode<<"*"<<simTime()<< ":"<<channel<<"interference size" <<(*interference).size()<<endl;

                //My own transmissions are never included in the interfering transmissions, so we include them here
                const physicallayer::ITransmission* txInProgress = nullptr;
                txInProgress = radio->getTransmissionInProgress(); //It returns an ITransmission
                if (txInProgress) {
                    interference->push_back(txInProgress);
                }

                for (const auto i : *interference) {
                    auto ieeeItransmission = check_and_cast<const physicallayer::Ieee80211ScalarTransmission*>(i);
                    // std::cout << myNode << "*" << simTime() << ":" << channel << ": ieeeItransmission " << ieeeItransmission->getId() << endl;
                    // auto ieeeItransmission=check_and_cast<const physicallayer::Ieee80211ScalarTransmission*>(i->getTransmission());
                    int channelI = ieeeItransmission->getChannel()->getChannelNumber();
                    //Consider only the interfering packets on the currently evaluated channel
                    if (channelI == channel) {
                        //Here we should compute the loss according to the transmit power, modulation and so on
                        double filterLoss = adjacentLoss[deltaChannel-1];
                        double interferencePower = -1.0;

                        //If the interfering packet it is being transmitted on my radio (channel) do not consider it if we do not consider interferences
                        //on packets being transmitted: the picked transmission cannot interfere an ongoing transmission
                        if (!interferenceOnTransmission && txInProgress && txInProgress->getId()==ieeeItransmission->getId()){
                            interferencePower = -1.0;
                        } else {
                            interferencePower = getInterferencePower(radio, transmission, medium, filterLoss);
                        }

                        //Record interference
                        if (interferencePower >= 0.0) {
                            auto overlapEnd = ieeeItransmission->getEndTime();
                            if (overlapEnd > transmission->getEndTime()) {
                                overlapEnd = transmission->getEndTime();
                            }
                            auto overlapStart = ieeeItransmission->getStartTime();
                            if (overlapStart <= transmission->getStartTime()) {
                                overlapStart = transmission->getStartTime();
                            }
                            auto overlap = overlapEnd-overlapStart;
                            auto fraction = overlap/transmission->getDuration();

                            //Fraction of the packet duration that is interfered
                            emit(interferenceFractionSignals[channel][txChannel], fraction);
                            //Absolute duration of the interference
                            emit(interferenceOverlapSignals[channel][txChannel], overlap.dbl());
                            //Number of packet interferred (if the model is not slotted the sum can be higher than the number of packets)
                            emit(interferenceCountSignals[channel][txChannel], 1);
                            //Power of the interferring signal
                            emit(interferencePowerSignals[channel][txChannel], interferencePower);
                        }

                        //Now the reciprocal
                        //Here we should compute the loss according to the transmit power, modulation and so on
                        interferencePower = -1.0;

                        //We have to create our own reception because the transmitted packet is not included in the interfering packets
                        if (i == txInProgress) {
                            auto arrival = medium->getPropagation()->computeArrival(transmission, radio->getAntenna()->getMobility());
                            auto reception = medium->getAnalogModel()->computeReception(radio, transmission, arrival);
                            const physicallayer::FlatReceptionBase *flatReception = check_and_cast<const physicallayer::FlatReceptionBase *>(reception);
                            //This is the received power from the new transmission after path loss, antenna gain and so on
                            W receptionPower = flatReception->computeMinPower(transmission->getStartTime(), transmission->getEndTime());

                            if (receptionPower < radio->getReceiver()->getMinReceptionPower() ) {
                                //This transmission on this channel is too weak to be received (no interference considered)
                                interferencePower = -1;
                            } else {
                                //Now we get the interfering power of this signal on that channel taking into account the filters. It actually depends on the transmit power of the signal and the modulation
                                W ip = receptionPower/filterLoss;

                                if (ip < radio->getReceiver()->getMinInterferencePower() ) {
                                    //Too weak to be considered interference
                                    interferencePower = 1.0;
                                } else {
                                    interferencePower = ip.get();
                                }
                            }
                            delete arrival;
                            delete reception;
                            //Otherwise just compute the reciprocal interference power
                        } else {
                            interferencePower = getInterferencePower(radio, ieeeItransmission, medium, filterLoss);
                        }

                        //Now if I have picked my own transmission, ignore interference.
                        if (!interferenceOnTransmission && txNodeIndex == myNode){
                            interferencePower = -1.0;
                        }

                        //Otherwise compute the interference that the overlapped transmission (ieeeItransmission) will have in the (future) reception of the picked transmission

                        //Now we have here a problem: maybe this transmission is below the sensitivity and will never be received
                        //We should not count the interference in this case
                        //The number of radios may not be equal for all vehicles
                        if (txChannel<radios.size()) {
                            if (!radios[txChannel]->getReceiver()->computeIsReceptionPossible(medium->getListening(radios[txChannel], transmission), transmission)){
                                //std::cout<<myNode<<"*"<<simTime()<<":"<<channel<<": transmission"<<transmission->getId()<<"will not be received"<<endl;
                                continue;
                            }
                        }

                        if (interferencePower < 0.0) {
                            continue;
                        } else {
                            auto overlapEnd = transmission->getEndTime();
                            if (overlapEnd > ieeeItransmission->getEndTime()) {
                                overlapEnd = ieeeItransmission->getEndTime();
                            }
                            auto overlapStart = transmission->getStartTime();
                            if (overlapStart <= ieeeItransmission->getStartTime()) {
                                overlapStart = ieeeItransmission->getStartTime();
                            }
                            auto overlap = overlapEnd-overlapStart;
                            auto fraction = overlap/ieeeItransmission->getDuration();

                            emit(interferenceFractionSignals[txChannel][channel], fraction);
                            emit(interferenceFractionSignals[txChannel][channel], overlap.dbl());
                            emit(interferenceCountSignals[txChannel][channel], 1);
                            emit(interferencePowerSignals[txChannel][channel], interferencePower);
                        }
                    }
                }
            }
        }
    }
}

double AdjacentChannelInterferenceMonitor::getInterferencePower(const physicallayer::IRadio* radio, const physicallayer::ITransmission* transmission, const physicallayer::IRadioMedium* medium, double loss) {
    const physicallayer::FlatReceptionBase *flatReception = check_and_cast<const physicallayer::FlatReceptionBase *>(medium->getReception(radio, transmission));
    //This is the received power from the new transmission after path loss, antenna gain and so on
    W receptionPower = flatReception->computeMinPower(transmission->getStartTime(), transmission->getEndTime());

    if (receptionPower < radio->getReceiver()->getMinReceptionPower() ) {
        //This transmission on this channel is too weak to be received (no interference considered)
        return -1.0;
    }
    //Now we get the interfering power of this signal on that channel taking into account the filters. It actually depends on the transmit power of the signal and the modulation
    W interferencePower = receptionPower/loss;

    if (interferencePower < radio->getReceiver()->getMinInterferencePower() ) {
        //Too weak to be considered interference
        return -1.0;
    }
    return interferencePower.get();
}

void AdjacentChannelInterferenceMonitor::createSignals() {
    //Here we compute all the interference that we want to capture
    //For all channel we can have interference from maxChannelSeparation channels, that is for channel i we have interference from i-maxChannelSeparation to i+maxChannelSeparation
    //Interference is computed from channel i to channel j E[I_i->j]
    for (int j = 0; j < numChannels; j++) {
        std::map<int,simsignal_t> ifv;
        std::map<int,simsignal_t> iov;
        std::map<int,simsignal_t> icv;
        std::map<int,simsignal_t> ipv;

        std::string iff("interferenceFraction");
        std::string ifo("interferenceOverlap");
        std::string ifc("interferenceCount");
        std::string ifp("interferencePower");

        if (j==0) {
            //On channel 0, we can only have interference from 1, 2, ...
            for (int i = 1; i < maxChannelSeparation+1; ++i) {
                std::string sname = iff + std::to_string(i) + "-" + std::to_string(j);
                //std::cout << "sname=" << sname << endl;
                ifv.insert({i,createAndRegisterSignal(sname,iff)});

                sname = ifc + std::to_string(i) + "-" + std::to_string(j);
                icv.insert({i,createAndRegisterSignal(sname,ifc)});

                sname = ifp + std::to_string(i) + "-" + std::to_string(j);
                ipv.insert({i,createAndRegisterSignal(sname,ifp)});

                sname = ifo + std::to_string(i) + "-" + std::to_string(j);
                iov.insert({i,createAndRegisterSignal(sname,ifo)});
            }
        } else if (j==numChannels-1) {
            //On the last channel we can only have interference from numChannels-1, ...
            for (int i = j-maxChannelSeparation; i < numChannels-1; ++i) {
                std::string sname = iff + std::to_string(i) + "-" + std::to_string(j);
                //std::cout << "sname=" << sname << endl;
                ifv.insert({i,createAndRegisterSignal(sname,iff)});

                sname = ifc + std::to_string(i) + "-" + std::to_string(j);
                icv.insert({i,createAndRegisterSignal(sname,ifc)});

                sname = ifp + std::to_string(i) + "-" + std::to_string(j);
                ipv.insert({i,createAndRegisterSignal(sname,ifp)});

                sname = ifo + std::to_string(i) + "-" + std::to_string(j);
                iov.insert({i,createAndRegisterSignal(sname,ifo)});
            }
        } else {
            for (int i = j-maxChannelSeparation; i < j+maxChannelSeparation+1; ++i) {
                if (i<0 || i==j) {
                    continue;
                } else if (i > numChannels-1) {
                    break;
                } else {
                    std::string sname = iff + std::to_string(i) + "-" + std::to_string(j);
                    //std::cout << "sname=" << sname << endl;
                    ifv.insert({i,createAndRegisterSignal(sname,iff)});

                    sname = ifc + std::to_string(i) + "-" + std::to_string(j);
                    icv.insert({i,createAndRegisterSignal(sname,ifc)});

                    sname = ifp + std::to_string(i) + "-" + std::to_string(j);
                    ipv.insert({i,createAndRegisterSignal(sname,ifp)});

                    sname = ifo + std::to_string(i) + "-" + std::to_string(j);
                    iov.insert({i,createAndRegisterSignal(sname,ifo)});
                }
            }
        }

        interferenceFractionSignals.insert({j,ifv});
        interferenceOverlapSignals.insert({j,iov});
        interferenceCountSignals.insert({j,icv});
        interferencePowerSignals.insert({j,ipv});
    }
    //    std::cout << "signals for " << myNode << endl;
    //    for(auto a: interferenceFractionSignals) {
    //        for (auto s: a.second) {
    //            std::cout << a.first << "," << s.first << endl;
    //        }
    //    }

}

simsignal_t AdjacentChannelInterferenceMonitor::createAndRegisterSignal(std::string sname, std::string tname) {
    auto signal = registerSignal(sname.c_str());
    cProperty *statisticTemplate = getProperties()->get("statisticTemplate", tname.c_str());
    getEnvir()->addResultRecorders(this, signal, sname.c_str(), statisticTemplate);
    return signal;
}

} //namespace inet
