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

package inet.applications.vehicular.optimal;
import inet.applications.vehicular.MgmtMCO;
//
// TODO auto-generated module
//
simple MinosMgmtMCO extends OptimalMgmtMCOBase
{
    parameters:
        @class(MinosMgmtMCO);
        double p=default(1); //Probability of slot interference
        int maxInnerIterations=default(1000);
        double innerGamma=default(1e-5);
        double innerEpsilon=default(1e-7);
       
        double range=default(150);
    
}
