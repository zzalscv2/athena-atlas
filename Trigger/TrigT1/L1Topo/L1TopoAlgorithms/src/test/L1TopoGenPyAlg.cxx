/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "CxxUtils/checker_macros.h"

#include "L1TopoInterfaces/AlgFactory.h"
#include "L1TopoInterfaces/ConfigurableAlg.h"

#include <fstream>

using namespace std;
using namespace TCS;

void generateCode(ofstream & f, const ConfigurableAlg* ca) {

   string cn(ca->className());

   const ParameterSpace & ps = ca->parameters();

   f << "class " << cn << "(" << (ca->isSortingAlg()?"SortingAlgo":(ca->isDecisionAlg()?"DecisionAlgo":"object")) << "):" << endl;
   f << "    _availableVars = [";
   bool first = true;
   for(const Parameter& p : ps) {
      if(first) { first=false; } else { f << ", "; }
      f << "'" << p.name() << "'";
   }
   f << "]" << endl;
   f << "    def __init__(self, name, inputs, outputs):" << endl;
   f << "        super(" << cn << ", self).__init__(classtype='" << cn << "', name=name, inputs=inputs, outputs=outputs)" << endl;
   f << endl << endl;

}

int main ATLAS_NOT_THREAD_SAFE (int argc, char** argv) {

   string pyname = "L1TopoAlgConfig.py";
   if(argc==2)
      pyname = argv[1];

   ofstream f(pyname, ios_base::out | ios_base::trunc);
   f << "'''This file is autogenerated by TrigConfL1TopoGenPyAlg'''" << endl << endl;
   f << "from TriggerMenuMT.L1.Base.TopoAlgos import SortingAlgo, DecisionAlgo" << endl << endl;
   for(const string & cn : AlgFactory::instance().getAllClassNames() ) {
      ConfigurableAlg * ca = AlgFactory::mutable_instance().create(cn, cn+"_Instance");
      generateCode(f, ca);
   }
   
   f.close();

   return 0;
}
