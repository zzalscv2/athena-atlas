/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Author: Noel Dawe (Noel%dot%Dawe%at%cern%dot%ch)
 */

#ifndef METHODBASE_H
#define METHODBASE_H

#include <string>
#include <iostream>
#include <map>
#include <algorithm>
#include <deque>
#include "TauDiscriminant/Types.h"

#include "AsgTools/AsgMessaging.h"

#include "xAODTau/TauJet.h"

using namespace std;

namespace TauID
{
    class MethodBase : public asg::AsgMessaging
    {
        public:

            ////////////////////////////////////////////////////////////
            /// The name is an arbitrary name set by the user and should
            /// have no influence on the behaviour of the method
            ////////////////////////////////////////////////////////////
            MethodBase(const string& _name = ""):
                AsgMessaging(_name),
                name(_name)
            {}

            virtual ~MethodBase() {}

            ///////////////////////////////////////////////////////////
            /// Return the value of a continuous discriminant
            ///////////////////////////////////////////////////////////
            virtual float response(xAOD::TauJet& tau) =0;

            ///////////////////////////////////////////////////////////
            /// This method should only be used for cut-based
            /// methods for responses at different levels of
            /// "tightness." For continuous discriminants, this
            /// method should print a warning message and
            /// return the value of response() above.
            ///////////////////////////////////////////////////////////
            virtual float response(xAOD::TauJet& tau, unsigned int level) =0;

            string getName() const
            {
                return name;
            }

	    void registerVariable(const string& name, char type);

	    void registerVariables(vector<string>& names, char type);

	    bool updateVariables(xAOD::TauJet& tau);

	    const map<string, const float*>* getFloatPointers();

	    const map<string, const int*>* getIntPointers();

            ////////////////////////////////////////////////////////////
            /// Build the discriminant from an input file
            /// The first parameter is a filename. Your method
            /// should be saved in only one file.
            /// Specifying a list of files here separated
            /// by commas, for example, is not acceptable.
            /// The boolean parameter is optional and may be
            /// used to optionally validate your method after
            /// building from your input file.
            ////////////////////////////////////////////////////////////
            virtual bool build(const string&, bool = false) =0;

            virtual Types::MethodType getType() const =0;

        private:

            static int upper(int c)
            {
                return std::toupper((unsigned char)c);
            }

            string name;

	    deque<float> m_floatVariableStore;
            deque<int> m_intVariableStore;

        protected:

            map<string, const float*> m_floatVars;
            map<string, const int*> m_intVars;
	    map<string, SG::AuxElement::ConstAccessor<float>> m_floatAccessors;
	    map<string, SG::AuxElement::ConstAccessor<int>> m_intAccessors;
    };
}
#endif
