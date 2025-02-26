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

#include "MinosVehicleTable.h"

namespace inet {

Define_Module(MinosVehicleTable);



int MinosVehicleTable::insertOrUpdate(MinosVehicleInfo *info) {
    emit(neighbors,vt.size());
    VTable::iterator it = vt.find(info->id);
    if (it==vt.end()) {
        MinosVehicleInfo* newNeigbor=new MinosVehicleInfo(*info);
        newNeigbor->beaconsReceived++;
        vt.insert(it,std::pair<int,MinosVehicleInfo*>(info->id,newNeigbor));
        return 1;
    } else {
        double irt_time = (simTime()-it->second->last_update[info->channelNumberLastUpdate]).dbl();
        double distance = mob->getCurrentPosition().distance(info->pos);
        irthist->collect(irt_time,distance);
        //Uncomment if  to obtain the irt due to only the moving cluster with priority
        //if (info->id>=255 && info->id<=280 && distance>=200 && distance <=300) {
        //emit(irt,irt_time);
        //}
        //Emit IRT if the node is in the irtRange
        if (distance <= irtRange)
            emit(irtSignals[info->channelNumberLastUpdate], irt_time);
        int brp=it->second->beaconsReceived;


        auto mvi=check_and_cast<MinosVehicleInfo*>(it->second);
        *mvi=*info;


        it->second->beaconsReceived=brp+1;
        it->second->appId = info->appId;
        it->second->channelNumberLastUpdate = info->channelNumberLastUpdate;
        it->second->pos = info->pos;
        it->second->last_update[info->channelNumberLastUpdate] = simTime();
        it->second->beaconsReceived++;

        return 0;
    }
}

} //namespace
