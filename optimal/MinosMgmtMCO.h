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

#ifndef __INET4_4_MINOSMCO_H_
#define __INET4_4_MINOSMCO_H_

#include <omnetpp.h>

#include "OptimalMgmtMCOBase.h"

using namespace omnetpp;

namespace inet {

/**
 * TODO - Generated class
 */
class MinosMgmtMCO : public OptimalMgmtMCOBase
{


protected:

    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void computeSubgradient();
    virtual void updateWeights();
    virtual void solveInnerProblem();
    virtual int getSequenceNumber(const Packet* pkt ) override;

    double p; //Minos use a probability of interference per slot
    //Configuration of the local minimization (inner problem of the dual);
    int maxInnerIterations;
    double innerGamma;
    double innerEpsilon;
    std::vector<double> normalizedRatePerChannel;

    double sqrRange;




};

} //namespace

#endif
