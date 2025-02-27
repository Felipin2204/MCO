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

#include "MCOLoadBalancingCCMgmt.h"

namespace inet {

Define_Module(MCOLoadBalancingCCMgmt);

void MCOLoadBalancingCCMgmt::initialize(int stage)
{
    MgmtMCO::initialize(stage);
    if(stage == INITSTAGE_LOCAL) {
        const char *cstr = par("maxChannelCapacity").stringValue();
        maxChannelCapacity = cStringTokenizer(cstr).asDoubleVector();
        if (maxChannelCapacity.size() != numChannels) {
            throw cRuntimeError("maxChannelCapacity size has to be equal to numChannels");
        }
        WATCH_VECTOR(maxChannelCapacity);
    } else if(stage == INITSTAGE_LINK_LAYER) {
        classifier = check_and_cast<IWrrClassifierCC*>(getModuleByPath("^.classifier"));
    }
}

void MCOLoadBalancingCCMgmt::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        if (msg == cbtSampleTimer) {
            for (int i=0; i<numChannels; i++) {
                double cbt = getMeasuredCBT(cbtWindow.dbl(), i);
                if (cbt > maxChannelCapacity[i])
                    classifier->setState(i, true);
                else
                    classifier->setState(i, false);
            }
            if (cbtFirstSample) {
                cbtFirstSample = false;
                cbtWindow = par("cbtWindow");
            }
            scheduleAfter(cbtWindow, cbtSampleTimer);
        } else {
            MgmtMCO::handleMessage(msg);
        }
    } else {
        MgmtMCO::handleMessage(msg);
    }
}

} //namespace
