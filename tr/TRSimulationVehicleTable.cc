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

#include "TRSimulationVehicleTable.h"
#include "inet/common/ModuleAccess.h"

namespace inet {

#define UPDATE_TO 4001

Define_Module(TRSimulationVehicleTable);

simsignal_t TRSimulationVehicleTable::neighbors = SIMSIGNAL_NULL;
simsignal_t TRSimulationVehicleTable::irtSignal = SIMSIGNAL_NULL;

TRSimulationIRTHistogram::TRSimulationIRTHistogram(int cells, double size) : irts(cells+1, 0.0), samples(cells+1, 0) {
    cellsize = size;
    maxrange = size*cells;
}

void TRSimulationIRTHistogram::collect(double itime, double distance) {
    if (distance <= maxrange) {
        int k = (int)floor((distance-0.0)/cellsize);
        irts[k] += itime;
        samples[k]++;
    }
}

double TRSimulationIRTHistogram::meanAtCell(int k) {
    if (k < 0 || k >= irts.size()) {
        std::cout<<"k="<<k<<";irts=samples="<<irts.size()<<endl;
        throw cRuntimeError("mean");
    }

    if (samples[k] == 0) return 0.0;
    return (irts[k]/samples[k]);
}

double TRSimulationIRTHistogram::meanAtDistance(double distance) {
    int k = (int)floor((distance-0.0)/cellsize);

    if (samples[k] == 0) return 0.0;
    return (irts[k] / samples[k]);
}

double TRSimulationIRTHistogram::meanLessDistance(double distance) {
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

std::ostream& operator<<(std::ostream& out, const TRSimulationIRTHistogram& inf)
{
    out<<inf.toString();
    return out;
}

std::string TRSimulationIRTHistogram::toString() const {
    std::ostringstream s(std::ostringstream::out);

    s<<"cellsize="<<cellsize<<";maxrange="<<maxrange;
    for (uint i = 0; i < irts.size(); i++) {
        s<<"irts["<<i<<"]="<<irts[i]<<";samples["<<i<<"]="<<samples[i];
    }
    return s.str();
}

std::string TRSimulationIRTHistogram::info() const {
    return this->toString();
}

void TRSimulationVehicleTable::initialize()
{
    updateTime = par("updateTime");
    EV_INFO << "Initializing Vehicle Table with update time=" << updateTime << endl;
    persistent = par("persistent");
    irtRange = par("irtRange");
    updateTimer = new cMessage("TRVTable update", UPDATE_TO);
    if (!persistent) scheduleAt(simTime()+updateTime, updateTimer);
    irthist = new TRSimulationIRTHistogram(50, 10.0);
    WATCH_PTR(irthist);
    mob = check_and_cast<IMobility*>(getModuleByPath("^.^.mobility"));
    WATCH_PTRMAP(vt);

    neighbors = registerSignal("neighbors");
    irtSignal = registerSignal("irt");
}

void TRSimulationVehicleTable::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        refreshTable();
        scheduleAt(simTime()+updateTime, updateTimer);
    }
}

void TRSimulationVehicleTable::finish() {
    std::string aux = "irt_" + std::to_string(irtRange);
    recordScalar(aux.c_str(), irthist->meanLessDistance(irtRange));
}

TRSimulationVehicleTable::~TRSimulationVehicleTable() {
    cancelAndDelete(updateTimer);
    delete irthist;
}

int  TRSimulationVehicleTable::insertOrUpdate(TRSimulationVehicleInfo* info) {
    emit(neighbors, vt.size());
    TRVTable::iterator it = vt.find(info->id);
    if (it == vt.end()) {
        TRSimulationVehicleInfo* newNeigbor = new TRSimulationVehicleInfo(*info);
        newNeigbor->beaconsReceived++;
        vt.insert(it, std::pair<int, TRSimulationVehicleInfo*>(info->id, newNeigbor));
        return 1;
    } else {
        double irt_time = (simTime()-it->second->last_update).dbl();
        double distance = mob->getCurrentPosition().distance(info->pos);
        irthist->collect(irt_time, distance);

        //Emit IRT if the node is in the irtRange
        if (distance <= irtRange) emit(irtSignal, irt_time);

        (*it->second) = *info;
        it->second->beaconsReceived++;
        it->second->last_update = simTime();

        return 0;
    }
}

void TRSimulationVehicleTable::refreshTable() {
    emit(neighbors, vt.size());
    TRVTable::iterator it = vt.begin();
    while(it != vt.end()) {
        if (simTime()-it->second->last_update >= updateTime) {
            delete(it->second);
            it = vt.erase(it);
        } else {
            it->second->beaconsReceived = 0;
            ++it;
        }
    }
}

void TRSimulationVehicleTable::refreshTable(double ut) {
    emit(neighbors, vt.size());
    TRVTable::iterator it = vt.begin();
    while(it != vt.end()) {
        if (simTime()-it->second->last_update >= ut) {
            delete(it->second);
            it = vt.erase(it);
        } else {
            it->second->beaconsReceived = 0;
            ++it;
        }
    }
}

bool TRSimulationVehicleTable::cancelUpdateTimer() {
    if (updateTimer->isScheduled()) {
        cancelEvent(updateTimer);
        return true;
    } else return false;
}

} /* namespace inet */
