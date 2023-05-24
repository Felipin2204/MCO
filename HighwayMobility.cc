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

#include "HighwayMobility.h"

namespace inet {

Define_Module(HighwayMobility);

HighwayMobility::HighwayMobility()
{
    speed = 0;
}

void HighwayMobility::initialize(int stage)
{
    MovingMobilityBase::initialize(stage);

    EV_TRACE << "initializing HighwayMobility stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        speed = par("speed");
        stationary = (speed == 0);
        
        //The heading parameter depends on the Y position
        rad heading = par("initialY").doubleValue() > 12.0 ? deg(0) : deg(180);

        rad elevation = deg(0);
        Coord direction = Quaternion(EulerAngles(heading, -elevation, rad(0))).rotate(Coord::X_AXIS);

        lastVelocity = direction * speed;
    }
}

void HighwayMobility::move()
{
    double elapsedTime = (simTime() - lastUpdate).dbl();
    lastPosition += lastVelocity * elapsedTime;

    //Vehicles exiting in one direction will enter again the scenario in the same lane,
    //at the opposite side (wrap around approach).
    Coord dummyCoord;
    handleIfOutside(WRAP, dummyCoord, lastVelocity);
}

} /* namespace inet */
