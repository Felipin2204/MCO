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
import inet.applications.vehicular.optimal.OptimalMgmtMCOBase;
//
// A variant of Minos
//
simple MinosTSOMgmtMCO extends OptimalMgmtMCOBase
{
    parameters:
        @class(MinosTSOMgmtMCO);
        bool useCVX=default(true); //Use CVX to solve inner problem. Requires providing port and host and running the python server
        string cvxHost=default("localhost");
        int cvxPort=default(9999);
}
