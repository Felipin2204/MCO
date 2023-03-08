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

#include "FileNodeManager.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace inet {

Define_Module(FileNodeManager);

void FileNodeManager::initialize(int stage)
{
    if (stage == INITSTAGE_LAST) {
        moduleType = par("moduleType").stdstringValue();
        moduleName = par("moduleName").stdstringValue();
        moduleDisplayString = par("moduleDisplayString").stdstringValue();
        const char* filename = par("filename");

        nodeVectorIndex = 0;
        std::string line;
        std::ifstream in(filename, std::ios::in);

        if (in.fail()) {
            throw cRuntimeError("Cannot open file '%s'", filename);
        }

        cModule* parentmod = getParentModule();
        if (!parentmod) error("Parent Module not found");

        while (std::getline(in, line))
        {
            double xc;
            double yc;
            std::string delimiters("\t");
            std::istringstream iline;
            std::string val;

            iline.str(line);

            getline(iline,val,'\t');
            xc = std::stof(val);

            getline(iline,val,'\t');
            yc = std::stof(val);

            cModuleType* nodeType = cModuleType::get(moduleType.c_str());
            if (!nodeType) error("Module Type \"%s\" not found", moduleType.c_str());

            parentmod->setSubmoduleVectorSize("node", nodeVectorIndex + 1);
            cModule* mod = nodeType->create(moduleName.c_str(), parentmod, nodeVectorIndex);

            cPar& id = mod->par("id");
            id.setIntValue(nodeVectorIndex);

            cPar& y = mod->par("initialY");
            y.setDoubleValue(yc);

            cPar& x = mod->par("initialX");
            x.setDoubleValue(xc);

            mod->finalizeParameters();
            mod->getDisplayString().parse(moduleDisplayString.c_str());
            mod->buildInside();
            mod->scheduleStart(simTime());
            mod->callInitialize();

            nodeVectorIndex++;
        }
        in.close();
    }
}

int FileNodeManager::getNumberOfNodes() {
    return (nodeVectorIndex+1);
}
} /* namespace inet */
