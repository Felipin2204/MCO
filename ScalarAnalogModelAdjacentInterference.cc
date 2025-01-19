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

namespace physicallayer {

Define_Module(ScalarAnalogModelAdjacentInterference);

void ScalarAnalogModelAdjacentInterference::initialize(int stage)
{
    ScalarAnalogModel::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *vstr = par("adjacentLoss").stringValue();
        adjacentLoss = cStringTokenizer(vstr).asDoubleVector();
    }
}

const INoise *ScalarAnalogModelAdjacentInterference::computeNoise(const IListening *listening, const IInterference *interference) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    Hz commonCenterFrequency = bandListening->getCenterFrequency();
    Hz commonBandwidth = bandListening->getBandwidth();
    simtime_t noiseStartTime = SimTime::getMaxTime();
    simtime_t noiseEndTime = 0;
    std::map<simtime_t, W> powerChanges;
    powerChanges[math::getLowerBound<simtime_t>()] = W(0);
    powerChanges[math::getUpperBound<simtime_t>()] = W(0);
    const std::vector<const IReception *> *interferingReceptions = interference->getInterferingReceptions();
    for (auto reception : *interferingReceptions) {
        const ISignalAnalogModel *signalAnalogModel = reception->getAnalogModel();
        const INarrowbandSignal *narrowbandSignalAnalogModel = check_and_cast<const INarrowbandSignal *>(signalAnalogModel);
        Hz signalCenterFrequency = narrowbandSignalAnalogModel->getCenterFrequency();
        Hz signalBandwidth = narrowbandSignalAnalogModel->getBandwidth();
        if (commonCenterFrequency == signalCenterFrequency && commonBandwidth >= signalBandwidth) {
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
    const ScalarNoise *scalarBackgroundNoise = dynamic_cast<const ScalarNoise *>(interference->getBackgroundNoise());
    if (scalarBackgroundNoise) {
        if (commonCenterFrequency == scalarBackgroundNoise->getCenterFrequency() && commonBandwidth >= scalarBackgroundNoise->getBandwidth())
            addNoise(scalarBackgroundNoise, noiseStartTime, noiseEndTime, powerChanges);
        else if (!ignorePartialInterference && areOverlappingBands(commonCenterFrequency, commonBandwidth, scalarBackgroundNoise->getCenterFrequency(), scalarBackgroundNoise->getBandwidth()))
            throw cRuntimeError("Partially interfering background noise is not supported by ScalarAnalogModel, enable ignorePartialInterference to avoid this error!");
    }
    EV_TRACE << "Noise power begin " << endl;
    W power = W(0);
    for (auto & it : powerChanges) {
        power += it.second;
        it.second = power;
        EV_TRACE << "Noise at " << it.first << " = " << power << endl;
    }
    EV_TRACE << "Noise power end" << endl;
    const auto& powerFunction = makeShared<math::Interpolated1DFunction<W, simtime_t>>(powerChanges, &math::LeftInterpolator<simtime_t, W>::singleton);
    return new ScalarNoise(noiseStartTime, noiseEndTime, commonCenterFrequency, commonBandwidth, powerFunction);
}

void ScalarAnalogModelAdjacentInterference::addAdjacentReception(const IReception *reception, simtime_t &noiseStartTime, simtime_t &noiseEndTime, std::map<simtime_t, W> &powerChanges, int channelDistance) const
{
    W powersignal = check_and_cast<const IScalarSignal *>(reception->getAnalogModel())->getPower();

    //Apply ACS and ACL losses
    double dBW = 10.0*log10(powersignal.get());
    W power(pow(10.0, (dBW-adjacentLoss[channelDistance-1])/10.0));
    simtime_t startTime = reception->getStartTime();
    simtime_t endTime = reception->getEndTime();
    std::map<simtime_t, W>::iterator itStartTime = powerChanges.find(startTime);
    if (itStartTime != powerChanges.end())
        itStartTime->second += power;
    else
        powerChanges.insert(std::pair<simtime_t, W>(startTime, power));
    std::map<simtime_t, W>::iterator itEndTime = powerChanges.find(endTime);
    if (itEndTime != powerChanges.end())
        itEndTime->second -= power;
    else
        powerChanges.insert(std::pair<simtime_t, W>(endTime, -power));
    if (reception->getStartTime() < noiseStartTime)
        noiseStartTime = reception->getStartTime();
    if (reception->getEndTime() > noiseEndTime)
        noiseEndTime = reception->getEndTime();
    //std::cout<<simTime()<<"ScalarAnalogModelBase::addAdjacentReception "<<noiseStartTime<<","<<noiseEndTime<<"p="<<power<<std::endl;
}

}//namespace physicallayer

}//namespace inet
