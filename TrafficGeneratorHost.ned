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

import inet.linklayer.contract.IWirelessInterface;
import inet.mobility.contract.IMobility;
import inet.networklayer.common.InterfaceTable;
import inet.applications.contract.IApp;

module TrafficGeneratorHost
{
    parameters:
        @networkNode;

        int id @mutable = default(0);
        int numApplications = default(1);
        int numChannels = default(7);
        
        //These parameters are used when we use the FileNodeManager
        double initialX @unit(m) = default(0m);
        double initialY @unit(m) = default(0m);

		wlan*.agent.typename = "";
        wlan*.mgmt.typename = "Ieee80211MgmtAdhoc";

        wlan*.interfaceTableModule = absPath(".interfaceTable");
        wlan*.opMode = "p";
        wlan*.bitrate = 6Mbps;
        
        //Settings for a Best Effort Acess Category
        wlan*.mac.qosStation = true;
        wlan*.mac.hcf.edca.edcaf[1].aifsn = 6; //AIFS = AIFSN[AC=BE] * SlotTime + SIFS = 6 * 13us + 32us = 110us. 
        wlan*.mac.hcf.edca.edcaf[1].cwMin = 15;
        wlan*.mac.hcf.edca.edcaf[1].cwMax = 1023;
        
        wlan*.mac.hcf.rateSelection.multicastFrameBitrate = 6Mbps;
        wlan*.mac.hcf.rateSelection.dataFrameBitrate = 6Mbps;
        wlan*.mac.hcf.rateSelection.dataFrameBandwidth = 20MHz;
        
        //IEEE 802.11 Frame Aggregation is turned off
        wlan*.mac.hcf.originatorMacDataService.msduAggregationPolicy.typename = "";
        
        //Backoff optimization is also turned off
        wlan*.mac.hcf.edca.contention.edcaf*.contention.backoffOptimization = false;
        
//        wlan*.mac.dcf.rateSelection.multicastFrameBitrate = 6Mbps;
//        wlan*.mac.dcf.rateSelection.dataFrameBitrate = 6Mbps;
//        wlan*.mac.dcf.rateSelection.dataFrameBandwidth = 20MHz;
//        wlan*.mac.dcf.rateControl.initialRate = 3Mbps;
//        wlan*.mac.dcf.rateControl.typename = "MCORateControl";

//        wlan*.radio.centerFrequency = 5.9GHz; //opMode p works in the licensed ITS band of 5.9 GHz (5.85–5.925 GHz)
//        wlan*.radio.bandwidth = 10MHz; //opMode p already sets a channel spacing of 10MHz. TODO: inet4.4 insists in using a BW of 20MHz even though channel spacing is 10 MHz, why? Assume it is correct
        wlan*.radio.bandName = "5.9 GHz";
        //Radio channel number initilizated in the submodules block
        wlan*.radio.antenna.typename = "ConstantGainAntenna";
        wlan*.radio.antenna.mobilityModule = absPath(".mobility");
        //The antenna gain value should be 3 dB only at the receiver (transmitter power -3dB)
        wlan*.radio.antenna.gain = 3dB;
        //For a QPSK modulation the SNR threshold is 6 dB
        wlan*.radio.receiver.snirThreshold = 6 dB;
        wlan*.radio.*.modulation = "QPSK"; //Transmitter and receiver
		
		//In the HighwayMobility case, these values don't overwrite the predefined ones.
        mobility.initialX = this.initialX;
        mobility.initialY = this.initialY;
        mobility.initialZ = 2m;
        mobility.initFromDisplayString = false;

        MCO.numApplications = this.numApplications;
        MCO.numChannels = this.numChannels;

    gates:
        input radioIn[numChannels] @directIn;

    submodules:
        application[numApplications]: <default("TrafficGenerator")> like IApp {
            @display("p=375,403");
        }
        wlan[numChannels]: <default("Ieee80211Interface")> like IWirelessInterface {
            @display("p=375,616");
            radio.channelNumber = default(index);
        }
        mobility: <default("StationaryMobility")> like IMobility {
            @display("p=125,160");
        }
        interfaceTable: InterfaceTable {
            @display("p=125,240");
        }
        MCO: <> like IMCO {
            @display("p=375,512");
        }

    connections allowunconnected:
        for i=0..numApplications-1 {
            application[i].socketOut --> MCO.inApp[i];
            MCO.outApp[i] --> application[i].socketIn;
        }
        for i=0..numChannels-1 {
            MCO.outWLAN[i] --> wlan[i].upperLayerIn;
            wlan[i].upperLayerOut --> MCO.inWLAN[i];
            radioIn[i] --> { @display("m=s"); } --> wlan[i].radioIn;
        }
}
