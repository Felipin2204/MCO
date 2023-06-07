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

#ifndef INET_APPLICATIONS_VEHICULAR_ECC68RURALPATHLOSS_H_
#define INET_APPLICATIONS_VEHICULAR_ECC68RURALPATHLOSS_H_

#include "inet/physicallayer/wireless/common/base/packetlevel/PathLossBase.h"

namespace inet {

class ECC68RuralPathLoss : public physicallayer::PathLossBase
{
  protected:
    double n0;
    double n1;
    double breakpointDistance0;
    double breakpointDistance1;

    bool useFading;
    double shapeFactor;

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override {return NUM_INIT_STAGES;}

  public:
    ECC68RuralPathLoss();
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual double computePathLoss(mps propagationSpeed, Hz frequency, m distance) const override;
    virtual m computeRange(mps propagationSpeed, Hz frequency, double loss) const override;
};

} /* namespace inet */

#endif /* INET_APPLICATIONS_VEHICULAR_ECC68RURALPATHLOSS_H_ */
