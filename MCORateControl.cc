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

#include "MCORateControl.h"

namespace inet {

using namespace inet::physicallayer;

Define_Module(MCORateControl);

void MCORateControl::initialize(int stage)
{
    RateControlBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        interval = par("interval");
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        updateDisplayString();
    }
}

void MCORateControl::updateDisplayString() const
{
    getDisplayString().setTagArg("t", 0, currentMode->getName());
}

void MCORateControl::frameTransmitted(Packet *frame, int retryCount, bool isSuccessful, bool isGivenUp)
{
    increaseRateIfTimerIsExpired();

    // Here comes the code to increase or decrease the rate
}

void MCORateControl::resetTimer()
{
    timer = simTime();
}

void MCORateControl::increaseRateIfTimerIsExpired()
{
    if (simTime() - timer >= interval) {
        currentMode = increaseRateIfPossible(currentMode);
        emitDatarateChangedSignal();
        updateDisplayString();
        EV_DETAIL << "Increased rate to " << *currentMode << endl;
        resetTimer();
    }
}

void MCORateControl::frameReceived(Packet *frame)
{
}

const IIeee80211Mode *MCORateControl::getRate()
{
    Enter_Method("getRate");
    increaseRateIfTimerIsExpired();
    EV_INFO << "The current mode is " << currentMode << " the net bitrate is " << currentMode->getDataMode()->getNetBitrate() << std::endl;
    return currentMode;
}

} /* namespace inet */
