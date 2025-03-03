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

package inet.applications.vehicular.classifier;
import inet.queueing.classifier.WrrClassifier;

//
// This classifier pushes packets to consumers in a weighted round robin mode. The initial consumer is chosen randomly for every instance and then uses the predefined sequence.
//
simple RandomStartWrrClassifier extends WrrClassifier
{
    parameters:
        @class(RandomStartWrrClassifier);
        //Use the channels in order of: SCH0, SCH1, SCH2, SCH5, SCH4, SCH3 for V2X and SCH6 only for I2V 
        //and testing in a predefined area non-interfering with Urban-Rail.
        string sequence = default("4 2 3 5 0 1 6"); //7 channels
}
