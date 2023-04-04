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
        double initialX @unit(m);
        double initialY @unit(m);
        string mobilityType = default("inet.mobility.static.StationaryMobility");

        wlan*.opMode = "p";
        wlan*.interfaceTableModule = default(absPath(".interfaceTable"));
        
        //Bitrate
        wlan*.bitrate = 6Mbps;
        wlan*.mac.dcf.rateSelection.multicastFrameBitrate = 6Mbps;
        wlan*.mac.dcf.rateSelection.dataFrameBitrate = 6Mbps;
        wlan*.mac.dcf.rateSelection.dataFrameBandwidth = 20MHz;
        
//        wlan*.mac.dcf.rateControl.initialRate = 3Mbps;
//        wlan*.mac.dcf.rateControl.typename = "MCORateControl";

        wlan*.mgmt.typename = default("Ieee80211MgmtAdhoc");

        wlan*.agent.typename = default("");

        //wlan*.radio.centerFrequency = 5.9GHz; //opMode p works in the licensed ITS band of 5.9 GHz (5.85–5.925 GHz)
        //wlan*.radio.bandwidth = 10MHz; //opMode p already sets a channel spacing of 10MHz. TODO: inet4.4 insists in using a BW of 20MHz even though channel spacing is 10 MHz, why? Assume it is correct
        wlan*.radio.bandName = "5.9 GHz";
        //Radio channel number initilizated in the submodules block

        wlan*.radio.antenna.mobilityModule = default(absPath(".mobility"));

        mobility.initialX = this.initialX;
        mobility.initialY = this.initialY;

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
            radio.channelNumber = index;
        }
        mobility: <mobilityType> like IMobility if typename != "" {
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
