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
// Computes interference from adjacent channels in IEEE 802.11p networks. It monitors a number of NICs (numChannels), each one operating on a different channel.
// It only computes the interference from channels which are at maxChannelSeparation (it is assumed that further away channel interference is neglectable).
// Separation is given by channel number, not by actual delta frequency.
//
simple AdjacentChannelInterferenceMonitor like IInterferenceMonitor
{  
    parameters:
    int numChannels;
    
    //Maximum channel (number) separation to compute interference from: 1 means that only adjacent (left and right) channels are used to compute the interference
    int maxChannelSeparation = default(1);
    
    //Set of losses due to ACL and ACS adjacent channels
    string adjacentLoss = default("32.1 46.4");
    //For simplicity we only take the values in dB for first and second adjacent channels. The actual losses depend on 
	//frequency, channel separation, tx power etc. (see ETSI TR 103 439 v2.1.1 appendix A, for instance)
	//Since it depends on transmission power, it should be changed in .ini if tx power changes
	//It is given as positive value in dB
       						
     bool interferenceOnTransmission = default(false);
       						
     @signal[interferenceFraction*](type=double);
     @signal[interferenceOverlap*](type=double);
     @signal[interferenceCount*](type=int);
     @signal[interferencePower*](type=double);
     @statisticTemplate[interferenceFraction](record=mean,vector); //Average fraction of a packet interferred
     @statisticTemplate[interferenceOverlap](record=sum,vector); //To be divided by total sim time to get the fraction of total time that interference has occurred
     @statisticTemplate[interferenceCount](record=sum,vector); //Number of packets interferred
     @statisticTemplate[interferencePower](record=mean,vector); //Interferencer power
}
