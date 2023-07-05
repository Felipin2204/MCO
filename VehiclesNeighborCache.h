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

#ifndef INET_APPLICATIONS_VEHICULAR_VEHICLESNEIGHBORCACHE_H_
#define INET_APPLICATIONS_VEHICULAR_VEHICLESNEIGHBORCACHE_H_

#include <set>
#include <vector>

#include "inet/common/ModuleRefByPar.h"
#include "inet/physicallayer/wireless/common/medium/RadioMedium.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"

namespace inet {

class VehiclesNeighborCache : public cSimpleModule, public physicallayer::INeighborCache
{
  public:
    struct RadioEntry {
      RadioEntry(const physicallayer::IRadio *radio) : radio(radio) {};
      const physicallayer::IRadio *radio;
      std::vector<const physicallayer::IRadio *> neighborVector;
      bool operator==(RadioEntry *rhs) const
      {
          return this->radio->getId() == rhs->radio->getId();
      }
    };
    typedef std::vector<RadioEntry *> RadioEntries;
    typedef std::vector<const physicallayer::IRadio *> Radios;
    typedef std::map<const physicallayer::IRadio *, RadioEntry *> RadioEntryCache;

  protected:
    ModuleRefByPar<physicallayer::RadioMedium> radioMedium;
    RadioEntries radios;
    RadioEntryCache radioToEntry;
    double range;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    void updateNeighborList(RadioEntry *radioEntry);
    void removeRadioFromNeighborLists(const physicallayer::IRadio *radio);

  public:
    VehiclesNeighborCache();
    ~VehiclesNeighborCache();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual void addRadio(const physicallayer::IRadio *radio) override;
    virtual void removeRadio(const physicallayer::IRadio *radio) override;
    virtual void sendToNeighbors(physicallayer::IRadio *transmitter, const physicallayer::IWirelessSignal *signal, double range) const override;

    virtual std::vector<const physicallayer::IRadio*> getNeighbors(physicallayer::IRadio *radio);
    void updateNeighborLists();
};

} /* namespace inet */

#endif /* INET_APPLICATIONS_VEHICULAR_VEHICLESNEIGHBORCACHE_H_ */
