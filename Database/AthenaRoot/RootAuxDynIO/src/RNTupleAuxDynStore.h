// This file is really -*- C++ -*-

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RNTUPLEAUXDYNSTORE_H
#define RNTUPLEAUXDYNSTORE_H

/**
 * @author Marcin Nowak
 * @brief Specialization of RootAuxDynStore for reading Aux Dynamic attributes from RNTuple
 */

#include "RootAuxDynStore.h"
namespace RootAuxDynIO { class RNTupleAuxDynReader; }

namespace ROOT { namespace Experimental { class RNTupleReader; } }
using RNTupleReader   = ROOT::Experimental::RNTupleReader;


class RNTupleAuxDynStore : public RootAuxDynStore
{
public:
   RNTupleAuxDynStore( RootAuxDynIO::RNTupleAuxDynReader& aux_reader,
                         RNTupleReader *reader,
                         const std::string& base_branch, 
                         long long entry, bool standalone,
                         std::recursive_mutex* iomtx = nullptr);
  
   virtual ~RNTupleAuxDynStore() {}

protected:
   /// read data from ROOT and store it in m_vecs. Returns False on error
   virtual bool readData(SG::auxid_t auxid) override final;

   RootAuxDynIO::RNTupleAuxDynReader& m_reader;
   RNTupleReader*               m_ntupleReader = nullptr;
   std::string                  m_baseBranchName;
};

#endif

