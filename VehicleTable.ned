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

package inet.applications.vehicular;

//
// Table implementation for storing neigbouring vehicle information.
//
simple VehicleTable like IVehicleTable
{
    parameters:
        int numChannels;
        double updateTime @unit(s) = default(1s);
        bool persistent = default(false);
        
        double irtRange @unit(m) = default(200m);
        
        bool enableIrtPeriodic = default(false);
        
        @signal[neighbors](type=unsigned int);
        @signal[irt*](type=double);
        @signal[irtPeriodic](type=double);
        
        @statistic[neighbors](record=mean,vector);
        @statisticTemplate[irt](record=vector);
		@statistic[irtPeriodic](record=vector);
}
