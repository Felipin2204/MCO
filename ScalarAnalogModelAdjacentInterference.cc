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

#include "ScalarAnalogModelAdjacentInterference.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/BandListening.h"

namespace inet {

Define_Module(ScalarAnalogModelAdjacentInterference);

void ScalarAnalogModelAdjacentInterference::initialize(int stage)
{
    ScalarAnalogModel::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *vstr = par("adjacentLoss").stringValue();
        adjacentLoss = cStringTokenizer(vstr).asDoubleVector();
    }
}

const physicallayer::INoise *ScalarAnalogModelAdjacentInterference::computeNoise(const physicallayer::IListening *listening, const physicallayer::IInterference *interference) const
{
    const physicallayer::BandListening *bandListening = check_and_cast<const physicallayer::BandListening *>(listening);
    Hz commonCenterFrequency = bandListening->getCenterFrequency();
    Hz commonBandwidth = bandListening->getBandwidth();
    simtime_t noiseStartTime = SimTime::getMaxTime();
    simtime_t noiseEndTime = 0;
    std::map<simtime_t, W> *powerChanges = new std::map<simtime_t, W>();
    const std::vector<const physicallayer::IReception *> *interferingReceptions = interference->getInterferingReceptions();
    for (auto reception : *interferingReceptions) {
        const physicallayer::ISignalAnalogModel *signalAnalogModel = reception->getAnalogModel();
        const physicallayer::INarrowbandSignal *narrowbandSignalAnalogModel = check_and_cast<const physicallayer::INarrowbandSignal *>(signalAnalogModel);
        Hz signalCenterFrequency = narrowbandSignalAnalogModel->getCenterFrequency();
        Hz signalBandwidth = narrowbandSignalAnalogModel->getBandwidth();
        //std::cout<<"Interfering reception with power"<<narrowbandSignalAnalogModel->computeMinPower(listening->getStartTime(),listening->getEndTime())<<std::endl;
        if (commonCenterFrequency == signalCenterFrequency && commonBandwidth >= signalBandwidth) {
            //std::cout<<"Added interfering reception with power"<<narrowbandSignalAnalogModel->computeMinPower(listening->getStartTime(),listening->getEndTime())<<std::endl;
            addReception(reception, noiseStartTime, noiseEndTime, powerChanges);
        } else {
            //std::cout<<"Interfering reception with frequency"<<signalCenterFrequency<<";BW="<<commonBandwidth<<";my freq="<<commonCenterFrequency<<std::endl;

            //if (!ignorePartialInterference && areOverlappingBands(commonCenterFrequency, commonBandwidth, narrowbandSignalAnalogModel->getCenterFrequency(), narrowbandSignalAnalogModel->getBandwidth()))
            //throw cRuntimeError("Partially interfering signals are not supported by ScalarAnalogModel, enable ignorePartialInterference to avoid this error!");

            //TODO BW is 22 Mhz for inet... so we use the correct one
            Hz itsG5BW(10e6);
            if ((signalCenterFrequency == commonCenterFrequency+itsG5BW) || (signalCenterFrequency == commonCenterFrequency-itsG5BW)) {
                //Adjacent channel signal
                addAdjacentReception(reception, noiseStartTime, noiseEndTime, powerChanges, 1);
            } else if ((signalCenterFrequency == commonCenterFrequency+itsG5BW+itsG5BW) || (signalCenterFrequency == commonCenterFrequency-itsG5BW-itsG5BW)) {
                addAdjacentReception(reception, noiseStartTime, noiseEndTime, powerChanges, 2);
            }
        }
    }
    const physicallayer::ScalarNoise *scalarBackgroundNoise = dynamic_cast<const physicallayer::ScalarNoise *>(interference->getBackgroundNoise());
    if (scalarBackgroundNoise) {
        if (commonCenterFrequency == scalarBackgroundNoise->getCenterFrequency() && commonBandwidth >= scalarBackgroundNoise->getBandwidth())
            addNoise(scalarBackgroundNoise, noiseStartTime, noiseEndTime, powerChanges);
        else if (!ignorePartialInterference && areOverlappingBands(commonCenterFrequency, commonBandwidth, scalarBackgroundNoise->getCenterFrequency(), scalarBackgroundNoise->getBandwidth()))
            throw cRuntimeError("Partially interfering background noise is not supported by ScalarAnalogModel, enable ignorePartialInterference to avoid this error!");
    }
    EV_TRACE << "Noise power begin " << endl;
    //std::cout << "Noise power begin " << endl;
    W noise = W(0);
    for (std::map<simtime_t, W>::const_iterator it = powerChanges->begin(); it != powerChanges->end(); it++) {
        noise += it->second;
        EV_TRACE << "Noise at " << it->first << " = " << noise << endl;
        //std::cout << "Noise at " << it->first << " = " << noise << endl;
    }
    EV_TRACE << "Noise power end" << endl;
    //std::cout << "Noise power end" << endl;
    return new physicallayer::ScalarNoise(noiseStartTime, noiseEndTime, commonCenterFrequency, commonBandwidth, powerChanges);
}

void ScalarAnalogModelAdjacentInterference::addAdjacentReception(const physicallayer::IReception *reception, simtime_t &noiseStartTime, simtime_t &noiseEndTime, std::map<simtime_t, W> *powerChanges, int channelDistance) const {
    W powersignal = check_and_cast<const physicallayer::IScalarSignal *>(reception->getAnalogModel())->getPower();
    //Apply ACS and ACL losses
    double dBW = 10.0*log10(powersignal.get());
    W power(pow(10.0, (dBW-adjacentLoss[channelDistance-1])/10.0));
    simtime_t startTime = reception->getStartTime();
    simtime_t endTime = reception->getEndTime();
    auto itStartTime = powerChanges->find(startTime);
    if (itStartTime != powerChanges->end())
        itStartTime->second += power;
    else
        powerChanges->insert(std::pair<simtime_t, W>(startTime, power));
    auto itEndTime = powerChanges->find(endTime);
    if (itEndTime != powerChanges->end())
        itEndTime->second -= power;
    else
        powerChanges->insert(std::pair<simtime_t, W>(endTime, -power));
    if (reception->getStartTime() < noiseStartTime)
        noiseStartTime = reception->getStartTime();
    if (reception->getEndTime() > noiseEndTime)
        noiseEndTime = reception->getEndTime();
}

} //namespace inet
