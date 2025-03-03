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

package inet.applications.vehicular.generator;
import inet.applications.contract.IApp;

//
// Base class defining common methods and properties for all traffic generators.
//
simple TrafficGenerator like IApp
{
	parameters:
	    @display("i=block/app");
	    
	    //These parameters are illustrative in this module
	    int totalPacketsPerSecond = default(100);
	    int minPacketsPerSecond = default(0);
	    double minNormalizedLoad = default(0);
	    double packetRate = default(100);
	    double normalizedLoad = default(1);
	    double measurementPeriod = default(1);
	    int packetLength @unit(byte) = default(500B);
	    int appId = default(-1);
	    volatile double timeBetweenPackets = default(exponential(1/packetRate));
	    bool jitter = default(false);
	    
	    @signal[timeBetweenPackets](type=simtime_t);
	    @statistic[timeBetweenPackets](record=vector);
	    @signal[generatedPackets](type=int);
	    @statistic[generatedPackets](source=count(generatedPackets);record=last);
	    @signal[receivedPackets](type=int);
		@statistic[receivedPackets](source=count(receivedPackets);record=last);
	    
	gates:
		input socketIn;
        output socketOut;
}
