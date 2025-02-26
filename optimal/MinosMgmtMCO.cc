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

#include "MinosMgmtMCO.h"

#include "MinosMCOPacket_m.h"
#include "../TrafficGenerator.h"
#include "../UniformSlottedTrafficGenerator.h"




namespace inet {

Define_Module(MinosMgmtMCO);



void MinosMgmtMCO::initialize(int stage) {
    OptimalMgmtMCOBase::initialize(stage);
    if(stage == INITSTAGE_LOCAL) {
        p= par("p");

        double range=par("range");
        sqrRange=range*range;
        maxInnerIterations=par("maxInnerIterations");
        innerGamma=par("innerGamma");
        innerEpsilon=par("innerEpsilon");
        std::cout<<myId<<":p="<<p<<";innerGamma="<<innerGamma<<";innerEpsilon="<<innerEpsilon<<";maxInnerIterations="<<maxInnerIterations<<endl;
    } else if(stage == INITSTAGE_APPLICATION_LAYER) {

        for ( int i=0; i<numApplications; i++) {
            //With Minos we use normalized rates.
            //To simplify, for now let us assume that that each application sends on its own channel
            //So each application has to generate the traffic computed by Minos per channel
            //A more realistic approach requires to create a packet classifier to distribute the app traffic between the channels
            //tmin[i]= tmin[i]*minimumSlotDuration;
            //std::cout<<myId<<":tmin["<<i<<"]="<<tmin[i]<<endl;
            //We get the initial packet rates from the applications
            TrafficGenerator* app= check_and_cast<TrafficGenerator*>(applications[i]);
            app->setMeasurementPeriod(2.0*syncPeriod);
            packetsPerChannel[i]=app->getPacketRate();
            //Normalize
            normalizedRatePerChannel.push_back(packetsPerChannel[i]*minimumSlotDuration);
        }

        WATCH_VECTOR(normalizedRatePerChannel);
    }
}

void MinosMgmtMCO::handleMessage(cMessage *msg)
{

    if (msg==updateChannelTimer) {
        if (updateWeight) {
            //We split the computation of the subgradient of dual (basically the relaxed constraint) in a different function
            //from the update of weights because alternative methods to compute the subgradient can be used
            computeSubgradient();
            updateWeights();
            updateWeight=false;
            //std::cout<<simTime()<<":"<<myId<<":updateWeight"<<endl;
        } else {
            //Compute CBT here to reset the measurement period but we do not use it to update weights
            for (int i=0; i<numChannels; i++) {
                if (useCBT) {
                    double measuredCbt =getMeasuredCBT(syncPeriod,i);
                    channelLoads[i]=measuredCbt*cbtCorrection;
                    //std::cout<<simTime()<<":"<<myId<<":t["<<i<<"]="<<channelLoads[i]<<endl;
                }
            }

            //Solve inner minimization problem to get the new packet rates
            solveInnerProblem();

            updateWeight=true;

            //To simplify, for now let us assume that that each application sends on its own channel
            //So each application has to generate the traffic computed by Minos per channel
            //A more realistic approach requires to create a packet classifier to distribute the app traffic between the channels
            for ( int i=0; i<numApplications; i++) {
                //UniformSlottedTrafficGenerator* app=check_and_cast<UniformSlottedTrafficGenerator*>(applications[i]);
                TrafficGenerator* app=check_and_cast<TrafficGenerator*>(applications[i]);

                //This is for fixed packet size app (ExponentialTrafficGenerator)
                packetsPerChannel[i]= normalizedRatePerChannel[i]/minimumSlotDuration;
                double rate=packetsPerChannel[i];
                //This is much more general, for NormalizedTrafficGenerator
                double load= normalizedRatePerChannel[i];
                if (load<=0) {
                    load=1e-5;
                }
                if (rate<=0) { //The exponential generator cannot have a zero rate, an alternative is to disable generation
                    rate=1e-3;
                }
                app->setNormalizedLoad(load);
                app->resetPeriod();
                app->setPacketRate(rate);
                emit(packetSignals[i], rate);
                emit(loadSignals[i],load);
            }
        }
        scheduleAt(simTime()+syncPeriod, updateChannelTimer);


    } else {
        OptimalMgmtMCOBase::handleMessage(msg);
    }
}

void MinosMgmtMCO::computeSubgradient() {
    for (int i=0; i<numChannels; i++) {
        if (useCBT) {
            double measuredCbt =getMeasuredCBT(syncPeriod,i);
            //Update measured loads to be sent on the packets
            channelLoads[i]=measuredCbt*cbtCorrection;
            subgradients[i]=(measuredCbt*cbtCorrection)-channelMaxCapacity[i];//Normalized

            //std::cout<<simTime()<<":"<<myId<<":cbt="<<measuredCbt<<"cbt corrected="<<(measuredCbt*cbtCorrection)<<"channelMaxCapacity["<<i<<"]="<<channelMaxCapacity[i]<<"sg="<<subgradients[i]<<"time_slot"<<minimumSlotDuration<<"syncPeriod="<<syncPeriod<<std::endl;

            emit(subgradientSignals[i],subgradients[i]);
        }

    }
}
void MinosMgmtMCO::updateWeights() {
    for (int i=0; i<numChannels; i++) {
        //here we use a gradient descent for the lagrange multipliers of the dual
        //we use + because we are maximizing the dual, since we have formulated the problem in standard minimization form
        double aux=channelWeights[i] + gammaStep*subgradients[i];
        if (aux<0) {
            aux=0;
        }
        channelWeights[i]=aux;
        //std::cout<<simTime()<<":"<<myId<<"weight["<<i<<"]="<< channelWeights[i] <<std::endl;
        emit(weightSignals[i], channelWeights[i]);
    }

}



void MinosMgmtMCO::solveInnerProblem() {
    //Compute the sum of the lagrange multipliers (weights) of each vehicle per channel
    std::vector<double> sumWeights(numChannels,0.0);
    std::vector<double> sumLoads(numChannels,0.0);
    std::vector<double> current_rates(numChannels,0.0);
    std::vector<double> log_current(numChannels,0.0); // log-rates
    for (int i=0; i<numChannels; i++) {
        current_rates[i]=normalizedRatePerChannel[i];
        sumWeights[i]=channelWeights[i]; //Start with my weight;
        sumLoads[i]=channelLoads[i];
        log_current[i]=log(current_rates[i]);
        //Now add neighbor weights
        for (auto v: mvt->vt) {
            MinosVehicleInfo* vi = check_and_cast<MinosVehicleInfo*>(v.second);
            //std::cout<<myId<<"vi->id="<<vi->id<<"loads="<<vi->channelLoads.size()<<endl;
            //Not all neighbors may use the same number of channels
            if (i<vi->channelWeigths.size()) {
                sumWeights[i] +=vi->channelWeigths[i];
            }


            if (i<vi->channelLoads.size()) {
                sumLoads[i] += vi->channelLoads[i];
            }

            //std::cout<<simTime()<<":"<<myId<<":sum_weights["<<i<<"]="<<sumWeights[i]<<"channel we="<<vi->channelWeigths[i]<<std::endl;
            //std::cout<<simTime()<<":"<<myId<<":sum_loads["<<i<<"]="<<sumLoads[i]<<"channel load="<<vi->channelLoads[i]<<std::endl;
        }
    }
    std::vector<double> grad_c(numChannels,0.0);
    std::vector<double> grad_prev(numChannels,1e10); //To compare with previous iteration, start with a very large number

    int iterationsDone=0;
    for (int i=0; i <maxInnerIterations; ++i) {


        //Compute grad of Lagrangian function: -wij +e^tij(sum_v in n(j) lambda_iv + 2p (T_i-1,v+T_i+1,v) //e^tij=rate
        for (int k=0; k<numChannels; ++k) {
            if (regularized) {
                grad_c[k]= -channelOmega[k] + regParameter*2*current_rates[k]+ current_rates[k]*(sumWeights[k]);
            } else {
                grad_c[k]= -channelOmega[k] + current_rates[k]*(sumWeights[k]);
                //std::cout<<"channelOmega[k]="<<channelOmega[k]<<";current_rates[k]="<<current_rates[k]<<";sumWeights[k]="<<sumWeights[k]<<";grad="<<grad_c[k]<<endl;
            }
            //Border channels
            if (k==0) {
                grad_c[k] += current_rates[k]*2*p*sumLoads[k+1];
                //std::cout<<"channelOmega[k]="<<channelOmega[k]<<";current_rates[k]="<<current_rates[k]<<";sumWeights[k]="<<sumLoads[k+1]<<";grad="<<grad_c[k]<<endl;

            } else if (k==numChannels-1) {
                grad_c[k] += current_rates[k]*2*p*sumLoads[k-1];
            } else {
                grad_c[k] += current_rates[k]*2*p*(sumLoads[k-1] + sumLoads[k+1]);
            }
            if (std::isnan(grad_c[k])) {
                throw cRuntimeError("grad_c is nan");
            }
        }

        //Inner problem is minimization so we use - (We have put it in standard minimization form for the Lagrangian)
        //grad is with respect to tij so we use log_rates
        bool done =true;
        for (int k=0; k<numChannels; ++k) {
            double aux = log_current[k]-innerGamma*grad_c[k]; //log rate: aux=tij
            if (aux<log(tmin[k])) {
                aux=log(tmin[k]);
            }
            //Update rates
            log_current[k]=aux;
            current_rates[k]=exp(aux);
            if (std::isnan(current_rates[k])) {
                throw cRuntimeError("current_rates is nan");
            }
            //Check if we are done: all elements of grad have to be below innerEpsilon
            double delta_grad=fabs(grad_c[k]-grad_prev[k]);
            //std::cout<<simTime()<<":"<<myId<<":grad_prev["<<k<<"]="<<grad_prev[k]<<"; grad_c="<<grad_c[k]<<";delta_grad="<<delta_grad<<endl;
            if (delta_grad>innerEpsilon) {
                done=false;
                //std::cout<<simTime()<<":"<<myId<<"done="<<done<<endl;
            }
            //Store grad
            grad_prev[k]=grad_c[k];
        }
        if (done) {
            std::cout<<simTime()<<":"<<myId<<"Inner problem solved at iteration "<<i<<endl;
            break;
        }
        ++iterationsDone;

    }

    emit(iterationsDoneSignal,iterationsDone);
    //We have finished: update final rates, applications will be notified outside the function
    for (int i=0; i<numChannels; i++) {
        normalizedRatePerChannel[i]=current_rates[i];

        //std::cout<<simTime()<<":"<<myId<<": new rate["<<i<<"]="<<normalizedRatePerChannel[i]<<endl;
    }
}

int MinosMgmtMCO::getSequenceNumber(const Packet *pkt) {

    auto p =  pkt->peekAt<MinosMCOPacket>(B(31));
    return p->getSequenceNumber();

}

} //namespace
