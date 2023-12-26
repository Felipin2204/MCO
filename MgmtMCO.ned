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

simple MgmtMCO like IMgmtMCO
{
    parameters:
		@display("i=block/cogwheel");
		
        int numChannels;
        int numApplications;
        
        double cbtWindow @unit(s) = default(0.1s); //Tcbr-itsg5
        double pdrWindow @unit(s) = default(0.1s);
        double pdrUpdate @unit(s) = default(0.1s);
        double pdrDistanceStep @unit(m) = default(100m);
        int pdrNumberIntervals = default(10);
        
        @signal[cbt*](type=double);
        @signal[pdr*_*-*](type=double);
        @signal[receivedPacketCount*](type=int);
        @signal[MCOPacketReceived];
  
		@statisticTemplate[cbt](record=vector);
        @statisticTemplate[pdr](record=vector);
        @statisticTemplate[receivedPacketCount](record=last);
        
    gates:
        input inQueue[numChannels];
        output outApp[numApplications];
        
        input inWLAN[numChannels];
        output outWLAN[numChannels];
}
