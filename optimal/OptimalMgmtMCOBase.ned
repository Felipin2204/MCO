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
//
// Base module for MCO scheduler based on convex optimization
//
simple OptimalMgmtMCOBase extends MgmtMCO
{
        @class(OptimalMgmtMCOBase);
        string channelMaxCapacity=default("0.6 0.6 0.6 0.6 0.6 0.6 0.6"); //MaxCapacity per channel
        string channelOmega=default("1 1 1 1 1 1 1");  //Priority parameters
        double alpha=default(1); //Fairness parameter
        double gammaStep=default(1e-6); //Gradient upgrade weigth
        double initWeight=default(1e-3); //Initial weights
   
        bool useCBT=default(true); //Use CBT for computing channel loads
        bool useCBTCorrection=default(true);
        bool regularized=default(false); //Use regularization
        double regParameter=default(1e-3); //regularization parameter
        double syncPeriod @unit(s)= default(0.25s); //Period to update weights and controls, compute gradient steps
         //Slot is the effective time to transmit a packet (752us for 500B (537B with MAC and PHY) plus IFS (58us) and cw*slot (slot=13us, 15*13=195us))
        //This should be computed from the datarate, data size and physical layer parameters to be general
        //but it is complicated because the padding depends on the datasize and bitrate
        //To keep it simple we pass it as a parameter. Default corresponds to 500B packets at 6 Mbps
        
        double minimumSlotDuration @unit(s)=default(1005us);
        //Again, we set this to keep it simple 
        double packetDuration @unit(s)=default(752us);
        @signal[weight*](type=double);
        @signal[subgradient*](type=double);
        @signal[packets*](type=double);
        @signal[load*](type=double);
        @signal[iterationsDone](type=int);
        @signal[MCOPacketReceived];
  
        @statistic[iterationsDone](record=mean,vector);
        @statisticTemplate[weight](record=vector);
        @statisticTemplate[subgradient](record=vector);
        @statisticTemplate[packets](record=vector, mean);
        @statisticTemplate[load](record=vector, mean);
}
