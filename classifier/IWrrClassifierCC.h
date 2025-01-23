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

#ifndef INET_APPLICATIONS_VEHICULAR_CLASSIFIER_IWRRCLASSIFIERCC_H_
#define INET_APPLICATIONS_VEHICULAR_CLASSIFIER_IWRRCLASSIFIERCC_H_

namespace inet {

//Interface for the Wrr classifiers of the congestion control case.

class IWrrClassifierCC {
public:
    virtual ~IWrrClassifierCC() {};
    virtual bool isCongested(int c) const = 0;
    virtual void setState(int c, bool state) = 0;
};

} /* namespace inet */

#endif /* INET_APPLICATIONS_VEHICULAR_CLASSIFIER_IWRRCLASSIFIERCC_H_ */
