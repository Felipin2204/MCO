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
    if (k < 0 || k >= irts.size()) {
        std::cout<<"k="<<k<<";irts=samples="<<irts.size()<<endl;
        throw cRuntimeError("mean");
    }

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
    for (int i = 0; i < k; i++) {
        aux += irts[i];
        sumc += samples[i];
    }

    if (sumc == 0) return 0.0;
    else return (aux/sumc);
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

void VehicleTable::initialize()
{
    numChannels = par("numChannels");
    updateTime = par("updateTime");
    EV_INFO << "Initializing Vehicle Table with update time=" << updateTime << endl;
    persistent = par("persistent");
    irtRange = par("irtRange");
    updateTimer = new cMessage("VTable update", UPDATE_TO);
    if (!persistent) scheduleAt(simTime()+updateTime, updateTimer);
    irthist = new IRTHistogram(50, 10.0);
    WATCH_PTR(irthist);
    mob = check_and_cast<IMobility*>(getModuleByPath("^.^.mobility"));
    WATCH_PTRMAP(vt);

    neighbors = registerSignal("neighbors");
    irtSignals.resize(numChannels);
    //Statistics recording for dynamically registered signals
    for (int i = 0; i < numChannels; i++) {
        std::string tname("irt");
        tname += std::to_string(i);
        irtSignals[i] = registerSignal(tname.c_str());
        cProperty *statisticTemplate = getProperties()->get("statisticTemplate", "irt");
        getEnvir()->addResultRecorders(this, irtSignals[i], tname.c_str(), statisticTemplate);
    }
}

void VehicleTable::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        refreshTable();
        scheduleAt(simTime()+updateTime, updateTimer);
    }
}

void VehicleTable::finish() {
    recordScalar("irt_irtRange", irthist->meanLessDistance(irtRange));
}

VehicleTable::~VehicleTable() {
    cancelAndDelete(updateTimer);
    delete irthist;
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
        double irt_time = (simTime()-it->second->last_update[info->channelNumberLastUpdate]).dbl();
        double distance = mob->getCurrentPosition().distance(info->pos);
        irthist->collect(irt_time, distance);

        //Emit IRT if the node is in the irtRange
        if (distance <= irtRange)
            emit(irtSignals[info->channelNumberLastUpdate], irt_time);

        (*it->second) = *info;
        int brp = it->second->beaconsReceived;
        it->second->beaconsReceived = brp++;
        it->second->last_update[info->channelNumberLastUpdate] = simTime();

        return 0;
    }
}

void VehicleTable::refreshTable() {
    emit(neighbors, vt.size());
    VTable::iterator it = vt.begin();
    while(it != vt.end()) {
        bool remove = true;
        for (int i = 0; i < it->second->last_update.size(); i++) {
            if(simTime()-it->second->last_update[i] <= updateTime) {
                remove = false;
                break;
            }
        }
        if (remove) {
            delete(it->second);
            it = vt.erase(it);
        } else {
            it->second->beaconsReceived = 0;
            ++it;
        }
    }
}

void VehicleTable::refreshTable(double ut) {
    emit(neighbors, vt.size());
    VTable::iterator it = vt.begin();
    while(it != vt.end()) {
        bool remove = true;
        for (int i = 0; i < it->second->last_update.size(); i++) {
            if(simTime()-it->second->last_update[i] <= ut) {
                remove = false;
                break;
            }
        }
        if (remove) {
            delete(it->second);
            it = vt.erase(it);
        } else {
            it->second->beaconsReceived = 0;
            ++it;
        }
    }
}

bool VehicleTable::cancelUpdateTimer() {
    if (updateTimer->isScheduled()) {
        cancelEvent(updateTimer);
        return true;
    } else return false;
}

} //namespace
