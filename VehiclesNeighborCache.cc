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

#include "VehiclesNeighborCache.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Radio.h"

namespace inet {

Define_Module(VehiclesNeighborCache);

VehiclesNeighborCache::VehiclesNeighborCache() : range(NaN)
{
}

void VehiclesNeighborCache::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        radioMedium.reference(this, "radioMediumModule", true);
        range = par("range");
    }
    else if (stage == INITSTAGE_PHYSICAL_LAYER_NEIGHBOR_CACHE)
        updateNeighborLists();
}

std::ostream& VehiclesNeighborCache::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "VehiclesNeighborCache";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(range);
    return stream;
}

void VehiclesNeighborCache::sendToNeighbors(physicallayer::IRadio *transmitter, const physicallayer::IWirelessSignal *signal, double range) const
{
    if (this->range < range)
        throw cRuntimeError("The transmitter's (id: %d) range is bigger then the cache range", transmitter->getId());

    RadioEntryCache::const_iterator it = radioToEntry.find(transmitter);
    if (it == radioToEntry.end())
        throw cRuntimeError("Transmitter is not found");

    RadioEntry *radioEntry = it->second;
    Radios& neighborVector = radioEntry->neighborVector;

    for (auto& elem : neighborVector)
        radioMedium->sendToRadio(transmitter, elem, signal);
}

void VehiclesNeighborCache::updateNeighborList(RadioEntry *radioEntry)
{
    IMobility *radioMobility = radioEntry->radio->getAntenna()->getMobility();
    Coord radioPosition = radioMobility->getCurrentPosition();
    auto radio = check_and_cast<const physicallayer::Ieee80211Radio*>(radioEntry->radio);
    int channelNumber = radio->par("channelNumber");
    radioEntry->neighborVector.clear();

    for (auto& elem : radios) {
        const physicallayer::IRadio *otherRadio = elem->radio;
        Coord otherEntryPosition = otherRadio->getAntenna()->getMobility()->getCurrentPosition();
        radio = check_and_cast<const physicallayer::Ieee80211Radio*>(otherRadio);
        int otherChannelNumber = radio->par("channelNumber");

        if (otherRadio->getId() != radioEntry->radio->getId() &&
            otherEntryPosition.sqrdist(radioPosition) <= range * range &&
            otherChannelNumber == channelNumber)
            radioEntry->neighborVector.push_back(otherRadio);
    }
}

void VehiclesNeighborCache::addRadio(const physicallayer::IRadio *radio)
{
    RadioEntry *newEntry = new RadioEntry(radio);
    radios.push_back(newEntry);
    radioToEntry[radio] = newEntry;
    updateNeighborLists();
}

void VehiclesNeighborCache::removeRadio(const physicallayer::IRadio *radio)
{
    auto it = find(radios.begin(), radios.end(), radioToEntry[radio]);
    if (it != radios.end()) {
        removeRadioFromNeighborLists(radio);
        radios.erase(it);
    }
    else {
        throw cRuntimeError("You can't remove radio: %d because it is not in our radio vector", radio->getId());
    }
}

void VehiclesNeighborCache::updateNeighborLists()
{
    EV_DETAIL << "Updating the neighbor lists" << endl;
    for (auto& elem : radios)
        updateNeighborList(elem);
}

void VehiclesNeighborCache::removeRadioFromNeighborLists(const physicallayer::IRadio *radio)
{
    for (auto& elem : radios) {
        Radios neighborVector = elem->neighborVector;
        auto it = find(neighborVector.begin(), neighborVector.end(), radio);
        if (it != neighborVector.end())
            neighborVector.erase(it);
    }
}

VehiclesNeighborCache::~VehiclesNeighborCache()
{
    for (auto& elem : radios)
        delete elem;
}

std::vector<const physicallayer::IRadio*> VehiclesNeighborCache::getNeighbors(physicallayer::IRadio *radio) {
    for (unsigned int i = 0; i < radios.size(); i++) {
        if (radios[i]->radio->getId() == radio->getId()) {
            return radios[i]->neighborVector;
        }
    }
    return std::vector<const physicallayer::IRadio*>();
}

} /* namespace inet */
