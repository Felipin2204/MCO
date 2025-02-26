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

#ifndef __INET4_5_MINOSTSOMGMTMCO_H_
#define __INET4_5_MINOSTSOMGMTMCO_H_

#include <omnetpp.h>

#include "OptimalMgmtMCOBase.h"
#include "tcpclient.h"
using namespace omnetpp;

namespace inet {

/**
 * TODO - Generated class
 */
class MinosTSOMgmtMCO : public OptimalMgmtMCOBase
{
public:
    MinosTSOMgmtMCO();
    virtual ~MinosTSOMgmtMCO();
protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;


    virtual void computeSubgradient();
    virtual void updateWeights();
    virtual void solveLagrangianWithCVX();

     //min load per channel. With TSO the min load is defined per vehicle, so it should be the sum of this
    TcpClient* cvx;
    std::string cvxHost;
    int cvxPort;
    bool useCVX; //Use CVX to solve inner problem
};

} //namespace

#endif
