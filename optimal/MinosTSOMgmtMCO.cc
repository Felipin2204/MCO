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

#include "MinosTSOMgmtMCO.h"

#include "../generator/TrafficGenerator.h"
#include "../generator/UniformSlottedTrafficGenerator.h"

namespace inet {

Define_Module(MinosTSOMgmtMCO);



MinosTSOMgmtMCO::MinosTSOMgmtMCO() :  cvx(nullptr)
{

}

MinosTSOMgmtMCO::~MinosTSOMgmtMCO() {
    if (cvx) {
        delete cvx;
    }
}

void MinosTSOMgmtMCO::initialize(int stage) {
    OptimalMgmtMCOBase::initialize(stage);
    if(stage == INITSTAGE_LOCAL) {
        cvxHost=par("cvxHost").stringValue();
        cvxPort=par("cvxPort");
        useCVX=par("useCVX");
    } else if(stage == INITSTAGE_APPLICATION_LAYER) {
        //Connect to CVX server
        if (useCVX) {
            cvx=new TcpClient();
            cvx->conn(cvxHost,cvxPort);
        }

    }
}

void MinosTSOMgmtMCO::handleMessage(cMessage *msg)
{
    if (msg==updateChannelTimer) {
        if (updateWeight) {
            computeSubgradient();
            updateWeights();
            updateWeight=false;
        } else {
            //Compute CBT here to reset the measurment period but we do not use it to update weights
            for (int i=0; i<numChannels; i++) {
                if (useCBT) {
                    double measuredCbt =getMeasuredCBT(syncPeriod,i);
                }
            }
            if (useCVX) {
                solveLagrangianWithCVX();
            }
            updateWeight=true;

            //To simplify, for now let us assume that that each application sends on its own channel
            //So each application has to generate the traffic computed by Minos per channel
            //A more realistic approach requires to create a packet classifier to distribute the app traffic between the channels
            for ( int i=0; i<numApplications; i++) {
                //UniformSlottedTrafficGenerator* app=check_and_cast<UniformSlottedTrafficGenerator*>(applications[i]);
                TrafficGenerator* app=check_and_cast<TrafficGenerator*>(applications[i]);
                double rate=packetsPerChannel[i];
                if (rate<=0) {
                    rate=1e-3;
                }
                app->setPacketRate(rate);
            }
        }
        scheduleAt(simTime()+syncPeriod, updateChannelTimer);


    } else {
        OptimalMgmtMCOBase::handleMessage(msg);
    }
}

void MinosTSOMgmtMCO::computeSubgradient() {
    for (int i=0; i<numChannels; i++) {
        if (useCBT) {
            double measuredCbt =getMeasuredCBT(syncPeriod,i);

            subgradients[i]=(measuredCbt*cbtCorrection-channelMaxCapacity[i])/minimumSlotDuration; //In packets/s

            //std::cout<<simTime()<<myId<<":cbt="<<measuredCbt<<"cbt corrected="<<(measuredCbt*cbtCorrection)<<"channelMaxCapacity["<<i<<"]="<<channelMaxCapacity[i]<<"sg="<<subgradients[i]<<"time_slot"<<minimumSlotDuration<<std::endl;

            emit(subgradientSignals[i],subgradients[i]);
        }

    }
}

void MinosTSOMgmtMCO::updateWeights() {
    for (int i=0; i<numChannels; i++) {
        //here we use a gradient descent for the lagrange multipliers of the dual
        //we use + because, even though we are minimizing the outer problem (dual is min (D)), we have not changed the sign of the constraints
        //that is, the subgradients are computed as (sum(r_i)-C_i)
        double aux=channelWeights[i] + gammaStep*subgradients[i];
        if (aux<0) {
            aux=0;
        }
        channelWeights[i]=aux;
        //std::cout<<simTime()<<":"<<myId<<"weight["<<i<<"]="<< channelWeights[i] <<std::endl;
        emit(weightSignals[i], channelWeights[i]);
    }
}

void MinosTSOMgmtMCO::solveLagrangianWithCVX() {
    //double CM=minimumSlotDuration; // C=max capacity in packets/s, CM = 1/C =minimumSlotDuration

    //nv is the number of neighbors of the vehicle plus itself
    double nv=1.0;
    nv += vehicleTable->getNumberOfNeighbors();
    //CVX problem expects the sum of the lagrange multipliers of each vehicle per channel
    std::vector<double> sumWeights(numChannels);
    for (int i=0; i<numChannels; i++) {
        sumWeights[i]=channelWeights[i]; //Start with my weight;
        //Now add neighbor weights
        for (auto v: mvt->vt) {
            MinosVehicleInfo* vi = check_and_cast<MinosVehicleInfo*>(v.second);
            sumWeights[i] +=vi->channelWeigths[i];
            //std::cout<<simTime()<<":"<<myId<<":sum_weights["<<i<<"]="<<sumWeights[i]<<"channel we="<<vi->channelWeigths[i]<<std::endl;
        }


    }
    //total demand should be updated if it changes over time
    double totalDemand=0;
    for (auto d :  currentDemandPerChannel) {
        totalDemand += d;
    }
    //tmin should also be updated here

    //Send data to CVXPY to solve the inner optimization problem
    auto t=cvx->request(myId,channelOmega, channelWeights, tmin, sumWeights, channelMaxCapacityPackets, totalDemand, minimumSlotDuration, alpha, nv, regularized, regParameter);
    if (t)
    {
        for (unsigned int i=0; i< (*t).size();  ++i) {
            packetsPerChannel[i]=(*t)[i];
            //std::cout<<simTime()<<myId<<":rates["<<i<<"]="<<packetsPerChannel[i]<<std::endl;
            emit(packetSignals[i], packetsPerChannel[i]);
        }
        delete t;
    } else {
        std::cout<<"Inner optimization not feasible"<<std::endl;
        //We just do not change the current traffic
        for (unsigned int i=0; i< packetsPerChannel.size();  ++i) {
            //std::cout<<simTime()<<myId<<":rates["<<i<<"]="<<packetsPerChannel[i]<<std::endl;
            emit(packetSignals[i], packetsPerChannel[i]);
        }
    }
}

} //namespace
