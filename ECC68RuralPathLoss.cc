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

#include "ECC68RuralPathLoss.h"

namespace inet {

Define_Module(ECC68RuralPathLoss);

ECC68RuralPathLoss::ECC68RuralPathLoss() :
        n0(0.0),
        n1(0.0),
        breakpointDistance0(0.0),
        breakpointDistance1(0.0),
        useFading(false),
        shapeFactor(1.0)
{
}

void ECC68RuralPathLoss::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        n0 = par("n0");
        n1 = par("n1");
        breakpointDistance0 = par("breakpointDistance0");
        breakpointDistance1 = par("breakpointDistance1");
        useFading = par("useFading");
        shapeFactor = par("shapeFactor");
    }
}

std::ostream& ECC68RuralPathLoss::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "ECC68RuralPathLoss";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(n0)
               << EV_FIELD(n1)
               << EV_FIELD(breakpointDistance0)
               << EV_FIELD(breakpointDistance1)
               << EV_FIELD(useFading)
               << EV_FIELD(shapeFactor);
    return stream;
}

double ECC68RuralPathLoss::computePathLoss(mps propagationSpeed, Hz frequency, m distance) const
{
    m waveLength = propagationSpeed / frequency;
    if (useFading) {
        double ECC68RuralPathLoss = 0.0;
        if (distance.get() <= breakpointDistance0)
        {
            if (distance.get() == 0.0) return 1.0;
            ECC68RuralPathLoss = (waveLength * waveLength).get() /
                                 (16 * M_PI * M_PI * distance.get() * distance.get());
        }
        else if (distance.get() <= breakpointDistance1)
        {
            ECC68RuralPathLoss = ((waveLength * waveLength).get() * pow(breakpointDistance0, n0)) /
                                 (16 * M_PI * M_PI * breakpointDistance0 * breakpointDistance0 * pow(distance.get(), n0));
        }
        else
        {
            ECC68RuralPathLoss = ((waveLength * waveLength).get() * pow(breakpointDistance0, n0) * pow(breakpointDistance1, n1)) /
                                 (16 * M_PI * M_PI * breakpointDistance0 * breakpointDistance0 * pow(breakpointDistance1, n0) * pow(distance.get(), n1));
        }
        return gamma_d(shapeFactor, ECC68RuralPathLoss / 1000.0 / shapeFactor) * 1000.0;

    } else {
        if (distance.get() <= breakpointDistance0)
        {
            return distance.get() == 0.0 ? 1.0 :
                   (waveLength * waveLength).get() /
                   (16 * M_PI * M_PI * distance.get() * distance.get());
        }
        else if (distance.get() <= breakpointDistance1)
        {
            return ((waveLength * waveLength).get() * pow(breakpointDistance0, n0)) /
                   (16 * M_PI * M_PI * breakpointDistance0 * breakpointDistance0 * pow(distance.get(), n0));
        }
        else
        {
            return ((waveLength * waveLength).get() * pow(breakpointDistance0, n0) * pow(breakpointDistance1, n1)) /
                   (16 * M_PI * M_PI * breakpointDistance0 * breakpointDistance0 * pow(breakpointDistance1, n0) * pow(distance.get(), n1));
        }
    }
}


m ECC68RuralPathLoss::computeRange(mps propagationSpeed, Hz frequency, double loss) const
{
    m waveLength = propagationSpeed / frequency;
    if (loss >= (waveLength * waveLength).get() /
                (16 * M_PI * M_PI * breakpointDistance0 * breakpointDistance0))
    {
        return loss == 1.0 ? m(0.0) :
               m(pow((waveLength * waveLength).get() /
                     (16.0 * M_PI * M_PI * loss), 0.5));
    }
    else if (loss >= ((waveLength * waveLength).get() * pow(breakpointDistance0, n0)) /
                     (16 * M_PI * M_PI * breakpointDistance0 * breakpointDistance0 * pow(breakpointDistance1, n0)))
    {
        return m(pow(((waveLength * waveLength).get() * pow(breakpointDistance0, n0)) /
                     (16.0 * M_PI * M_PI * breakpointDistance0 * breakpointDistance0 * loss), 1.0 / n0));
    }
    else
    {
        return loss == 0.0 ? m(INFINITY) :
                m(pow(((waveLength * waveLength).get() * pow(breakpointDistance0, n0) * pow(breakpointDistance1, n1)) /
                     (16.0 * M_PI * M_PI * breakpointDistance0 * breakpointDistance0 * pow(breakpointDistance1, n0) * loss), 1.0 / n1));
    }
}

} /* namespace inet */
