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
import inet.applications.vehicular.TrafficGenerator;

module TrafficGeneratorHost
{
    parameters:
        @networkNode;        
        int id @mutable = default(0);
        string mobilityType = default("inet.mobility.static.StationaryMobility");
        ///string vtType = default("VehicleTable");
        
        wlan.mgmt.typename = default("Ieee80211MgmtAdhoc");
        wlan.agent.typename = default("");
        wlan.opMode = "p";
        wlan.bitrate = default(6Mbps);
        //wlan.radio.bandwidth=default(10 MHz); opMode p already sets a channel spacing of 10MHz. TODO: inet4.3 insists in using a BW of 20MHz even though channel spacing is 10 MHz, why? Assume it is correct
        wlan.radio.channelNumber = 4;  //4 gives 5.9 GHz center frequency, which is the control channel G5-CCH according to ETSI EN 302 663
        //wlan.radio.carrierFrequency = 5.9 GHz;
        
        ///application.mobilityModule=".mobility";
        ///vehicleTable.mobilityModule=".mobility";
        ///application.vehicleTable=".vehicleTable";
       
        ///wlan.**.mobilityModule = default(exists(mobility) ? absPath(".mobility") : "");
        //wlan.interfaceTableModule = default("InterfaceTable");
        
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        wlan.radio.antenna.mobilityModule = default("^.^.^.mobility");
        //wlan.radio.antenna.typename=default("OpalLinearPolarizationAntenna");
        
    gates:
        input radioIn @directIn;
        
    submodules:
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=125,240;is=s");
        }
        mobility: <mobilityType> like IMobility if typename != "" {
            parameters:
                @display("p=125,160;is=s");
        }
        application: TrafficGenerator {
            parameters:
                @display("p=366,422;is=s");
        }
///        vehicleTable: <vtType>  like IVehicleTable {
///                @display("p=100,200;is=s");
///        }
        wlan: <default("Ieee80211Interface")> like IWirelessInterface {
            parameters:
                @display("p=375,616,row,150;q=queue");
        }
        
    connections allowunconnected:
        application.socketOut --> { @display("m=s"); } --> wlan.upperLayerIn;
        wlan.upperLayerOut --> { @display("m=s"); } --> application.socketIn;
        radioIn --> { @display("m=s"); } --> wlan.radioIn;
}
