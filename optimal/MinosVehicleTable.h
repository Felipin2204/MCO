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

#ifndef __INET4_4_MINOSVEHICLETABLE_H_
#define __INET4_4_MINOSVEHICLETABLE_H_

#include <omnetpp.h>

#include "MinosVehicleInfo.h"
#include "../VehicleTable.h"

using namespace omnetpp;

namespace inet {

/**
 * TODO - Generated class
 */
class MinosVehicleTable : public VehicleTable
{
  public:
    virtual int insertOrUpdate(MinosVehicleInfo* info);
};

} //namespace

#endif
