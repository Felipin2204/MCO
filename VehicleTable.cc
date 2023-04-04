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

#include "VehicleTable.h"
#include "inet/common/ModuleAccess.h"

namespace inet {

#define UPDATE_TO 4001

Define_Module(VehicleTable);

simsignal_t VehicleTable::neighbors = SIMSIGNAL_NULL;
simsignal_t VehicleTable::irt = SIMSIGNAL_NULL;

IRTHistogram::IRTHistogram(int cells, double size) : irts(cells+1, 0.0), samples(cells+1, 0) {
    cellsize = size;
    maxrange = size*cells;
}

void IRTHistogram::collect(double itime, double distance) {
    if (distance <= maxrange) {
        int k = (int)floor((distance-0.0)/cellsize);
        irts[k] += itime;
        samples[k]++;
    }
}

double IRTHistogram::meanAtCell(int k) {
    if (k < 0 || k >= irts.size()) {std::cout<<"k="<<k<<";irts="<<irts.size()<<endl; throw cRuntimeError("mean");}
    if (k < 0 || k >= samples.size()) {std::cout<<"k="<<k<<";samples="<<samples.size()<<endl; throw cRuntimeError("mean");}

    if (samples[k] == 0) return 0.0;
    return (irts[k]/samples[k]);
}

double IRTHistogram::meanAtDistance(double distance) {
    int k = (int)floor((distance-0.0)/cellsize);

    if (samples[k] == 0) return 0.0;
    return (irts[k] / samples[k]);
}

double IRTHistogram::meanLessDistance(double distance) {
    int k = (int)floor((distance-0.0)/cellsize);

    double aux = 0.0;
    int sumc = 0.0;
    for (int i = 0; i <= k; i++) {
        sumc += samples[i];
        aux += irts[i];
    }
    if (sumc == 0) {
        return 0.0;
    } else {
        return (aux/sumc);
    }
}

std::ostream& operator<<(std::ostream& out, const IRTHistogram& inf)
{
    out<<inf.toString();
    return out;
}

std::string IRTHistogram::toString() const {
    std::ostringstream s(std::ostringstream::out);

    s<<"cellsize="<<cellsize<<";maxrange="<<maxrange;
    for (uint i = 0; i < irts.size(); i++) {
        s<<"irts["<<i<<"]="<<irts[i]<<";samples["<<i<<"]="<<samples[i];
    }
    return s.str();
}

std::string IRTHistogram::info() const {
    return this->toString();
}

VehicleTable::~VehicleTable() {
    cancelAndDelete(updateTimer);
    //std::cout<<"delete TRCRAdio"<<endl;
    delete irthist;
}

void VehicleTable::initialize()
{
    updateTime = par("updateTime");
    EV << "Initializing Vehicle Table with update time=" << updateTime << endl;
    persistent = par("persistent");
    updateTimer = new cMessage("VTable update", UPDATE_TO);
    if (!persistent) {
        scheduleAt(simTime()+updateTime, updateTimer);
    }
    irthist = new IRTHistogram(50, 10.0);
    WATCH_PTR(irthist);
    mob = check_and_cast<IMobility*>(getModuleByPath("^.^.mobility"));
    WATCH_PTRMAP(vt);

    neighbors = registerSignal("neighbors");
    irt = registerSignal("irt");
}

void VehicleTable::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        refreshTable();
        scheduleAt(simTime()+updateTime, updateTimer);
        //emit(neighbors, 0);
    }
}

void VehicleTable::finish() {
    recordScalar("irt_250", irthist->meanAtDistance(250.0));
    recordScalar("irt_500", irthist->meanAtDistance(500.0));
    //recordScalar("accirt_250",irthist->meanLessDistance(250.0));
    //recordScalar("accirt_500",irthist->meanLessDistance(500.0));
}

int  VehicleTable::insertOrUpdate(VehicleInfo* info) {
    emit(neighbors, vt.size());
    VTable::iterator it = vt.find(info->id);
    if (it == vt.end()) {
        VehicleInfo* newNeigbor = new VehicleInfo(*info);
        newNeigbor->beaconsReceived++;
        vt.insert(it, std::pair<int, VehicleInfo*>(info->id, newNeigbor));
        return 1;
    } else {
        double irt_time = (simTime()-it->second->last_update).dbl();
        double distance = mob->getCurrentPosition().distance(info->pos);
        irthist->collect(irt_time, distance);
        //Uncomment if to obtain the irt due to only the moving cluster with priority
        //if (info->id>=255 && info->id<=280 && distance>=200 && distance <=300) {
         emit(irt, irt_time);
        //}

        int brp = it->second->beaconsReceived;
        it->second->beaconsReceived = brp+1;

        (*it->second) = *info;
        it->second->last_update = simTime();

        return 0;
    }
}

void VehicleTable::refreshTable() {
    emit(neighbors, vt.size());
    VTable::iterator it = vt.begin();
    while(it != vt.end()) {
        //std::cout<<simTime()<<"--"<<it->first<<"last_update="<<it->second->last_update<<std::endl;
        if (simTime()-it->second->last_update > updateTime) {
            //WE HAVE NOT ACTUALLY RECEIVED A BEACON SO IF WE COLLECT HERE WE ARE ARTIFICIALLY DECREASING THE IRT
            //WHAT SHOULD WE DO, A HISTORIC WITH ALL THE KNOWN VEHICLES AND LAST UPDATE...?
            //double irt_time=(simTime()-it->second->last_update).dbl();
            //double distance = mob->getCurrentPosition().distance(pos);
            //irthist->collect(irt_time,distance);
            //emit(irt,irt_time);
            //emit(rrt,1/(irt_time*it->second->beaconRate));
            //std::cout<<simTime()<<"--Delete neighbor:"<<it->first<<"last_update="<<it->second->last_update<<std::endl;

            delete(it->second);
            it = vt.erase(it);
        } else {
            //std::cout<<it->second->id<<"br="<<it->second->beaconsReceived<<"t="<<simTime()<<"ut="<<updateTime<<"t-ut"<<(simTime()-it->second->last_update).dbl()<<"mbr=" <<(it->second->beaconsReceived/updateTime)<<endl;
            it->second->beaconsReceived = 0;
            ++it;
        }
    }
}

void VehicleTable::refreshTable(double ut) {
    emit(neighbors, vt.size());
    VTable::iterator it = vt.begin();
    while(it != vt.end()) {
        if (simTime()-it->second->last_update > ut) {
            delete(it->second);
            vt.erase(it);
        }else {
            it->second->beaconsReceived=0;
        }
        ++it;
    }
}

bool VehicleTable::cancelUpdateTimer() {
    if (updateTimer->isScheduled()) {
        cancelEvent(updateTimer);
        return true;
    } else {
        return false;
    }
}

} //namespace
