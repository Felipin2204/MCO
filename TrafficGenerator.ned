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
import inet.applications.contract.IApp;

simple TrafficGenerator like IApp
{
	parameters:
	    @display("i=block/app");
	    
	    int totalPacketsPerSecond = default(100);
	    int packetLength @unit(byte) = default(1000B);
	    volatile double timeBetweenPackets = default(exponential(1/totalPacketsPerSecond));
	    
	    @signal[timeBetweenPackets](type=double);
	    @signal[generatedPackets](type=int);
	    @signal[receivedPackets](type=int);
	    @statistic[timeBetweenPackets](title="Time between packets"; record=vector,stats,histogram);
		@statistic[generatedPackets](title="Generated packets"; source=count(generatedPackets); record=last);
		@statistic[receivedPackets](title="Received packets"; source=count(receivedPackets); record=last);
	    
	gates:
		input socketIn;
        output socketOut;
}
