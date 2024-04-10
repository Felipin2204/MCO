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

#ifndef __INET4_3_VEHICLETABLE_H_
#define __INET4_3_VEHICLETABLE_H_

#include <omnetpp.h>
#include "VehicleInfo.h"
#include "inet/mobility/contract/IMobility.h"

using namespace omnetpp;

namespace inet {

class IRTHistogram : public cObject {
  public:
    IRTHistogram(int cells, double size);
    double cellsize;
    double maxrange;
    std::vector<double> irts;
    std::vector<int> samples;

    void collect(double itime, double distance);
    double meanAtCell(int k);
    double meanAtDistance(double distance);
    double meanLessDistance(double distance);

    friend std::ostream& operator<<(std::ostream& out, const IRTHistogram& inf);
    std::string toString() const;
    virtual std::string info() const;
};

class VehicleTable : public cSimpleModule {
  protected:
    typedef std::map<int, VehicleInfo*> VTable;

    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

    int numChannels;
    double updateTime;
    bool persistent;
    double irtRange;
    cMessage* updateTimer;
    IRTHistogram* irthist;
    IMobility* mob;

    static simsignal_t neighbors;
    std::vector<simsignal_t> irtSignals;

  public:
    virtual ~VehicleTable();
    virtual int insertOrUpdate(VehicleInfo* info);
    virtual void refreshTable();
    virtual void refreshTable(double ut);
    virtual bool cancelUpdateTimer();
    virtual int getNumberOfNeighbors() const {return vt.size();};
    VTable vt;
};

} //namespace

#endif
