// This file's extension implies that it's C, but it's really -*- C++ -*-.

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBRANCHAUXDYNSTORE_H
#define TBRANCHAUXDYNSTORE_H

/**
 * @author Marcin Nowak
 * @brief Specialization of RootAuxDynStore for reading Aux Dynamic attributes from TBranches
 */

#include "RootAuxDynStore.h"

class TBranchAuxDynReader;


class TBranchAuxDynStore : public RootAuxDynStore
{
public:
  TBranchAuxDynStore(TBranchAuxDynReader& reader, long long entry, bool standalone,
                       std::recursive_mutex* iomtx = nullptr);
  
  virtual ~TBranchAuxDynStore() {}

protected:
  /// read data from ROOT and store it in m_vecs. Returns False on error
  virtual bool readData(SG::auxid_t auxid) override final;

  TBranchAuxDynReader&  m_reader;

};

#endif
