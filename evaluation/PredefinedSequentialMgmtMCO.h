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

#ifndef __INET4_4_PREDEFINEDSEQUENTIALMCO_H_
#define __INET4_4_PREDEFINEDSEQUENTIALMCO_H_

#include <omnetpp.h>
#include "../MgmtMCO.h"
#include "../classifier/PredefinedPriorityClassifier.h"
using namespace omnetpp;

namespace inet {

class PredefinedSequentialMgmtMCO : public MgmtMCO
{
  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    std::vector<double> maxChannelCapacity;
    PredefinedPriorityClassifier* classifier;
    static simsignal_t currentUsedChannelSignal;
};

} //namespace

#endif
