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

#ifndef INET_APPLICATIONS_VEHICULAR_TRSIMULATIONVEHICLETABLE_H_
#define INET_APPLICATIONS_VEHICULAR_TRSIMULATIONVEHICLETABLE_H_

#include <omnetpp.h>
#include "TRSimulationVehicleInfo.h"
#include "inet/mobility/contract/IMobility.h"

using namespace omnetpp;

namespace inet {

typedef std::map<int, TRSimulationVehicleInfo*> VTable;

class TRSimulationIRTHistogram : public cObject {
  public:
    TRSimulationIRTHistogram(int cells, double size);
    double cellsize;
    double maxrange;
    std::vector<double> irts;
    std::vector<int> samples;

    void collect(double itime, double distance);
    double meanAtCell(int k);
    double meanAtDistance(double distance);
    double meanLessDistance(double distance);

    friend std::ostream& operator<<(std::ostream& out, const TRSimulationIRTHistogram& inf);
    std::string toString() const;
    virtual std::string info() const;
};

class TRSimulationVehicleTable : public cSimpleModule {
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

    double updateTime;
    bool persistent;
    double irtRange;
    cMessage* updateTimer;
    TRSimulationIRTHistogram* irthist;
    IMobility* mob;

    static simsignal_t neighbors;
    static simsignal_t irtSignal;

  public:
    virtual ~TRSimulationVehicleTable();
    virtual int insertOrUpdate(TRSimulationVehicleInfo* info);
    virtual void refreshTable();
    virtual void refreshTable(double ut);
    virtual bool cancelUpdateTimer();
    virtual int getNumberOfNeighbors() const {return vt.size();};
    VTable vt;
};

} /* namespace inet */

#endif /* INET_APPLICATIONS_VEHICULAR_TRSIMULATIONVEHICLETABLE_H_ */
