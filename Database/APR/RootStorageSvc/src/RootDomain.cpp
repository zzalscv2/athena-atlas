/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//
//        Package    : RootStorageSvc (The POOL project)
//
//        Author     : M.Frank
//====================================================================

// Framework include files
#include "StorageSvc/DbOption.h"
#include "RootDomain.h"
#include "RootDatabase.h"

#include "TSystem.h"
#include "TFile.h"
#include "TROOT.h"
#include "TTree.h"
#include "TVirtualStreamerInfo.h"

using namespace pool;

/// Standard Constuctor
RootDomain::RootDomain() :
  m_defCompression(1),
  m_defCompressionAlg(1),
  m_defSplitLevel(99),
  m_defAutoSave(16*1024*1024),
  m_defBufferSize(16*1024),
  m_branchOffsetTabLen(0)
{
}

/// Standard destructor
RootDomain::~RootDomain()   {
}

/// Check for Database existence
bool RootDomain::existsDbase(const std::string& nam)  {
  return RootDatabase::exists(nam);
}

/// Set domain specific options
DbStatus RootDomain::setOption(const DbOption& opt)  {
  const char* n = opt.name().c_str();
  switch( ::toupper(n[0]) )  {
    case 'D':
      if ( !strcasecmp(n, "DEFAULT_COMPRESSION") )  {
        return opt._getValue(m_defCompression);
      }
      else if ( !strcasecmp(n, "DEFAULT_COMPRESSIONALG") )  {
        return opt._getValue(m_defCompressionAlg);
      }
      else if ( !strcasecmp(n, "DEFAULT_SPLITLEVEL") )  {
        return opt._getValue(m_defSplitLevel);
      }
      else if ( !strcasecmp(n, "DEFAULT_AUTOSAVE") )  {
        return opt._getValue(m_defAutoSave);
      }
      else if ( !strcasecmp(n, "DEFAULT_BUFFERSIZE") )  {
        return opt._getValue(m_defBufferSize);
      }
      break;
    case 'E':
      if ( !strcasecmp(n, "ENABLE_THREADSAFETY") )  {
        bool multithreaded = false;
        DbStatus sc = opt._getValue(multithreaded);
        if ( sc.isSuccess() && multithreaded )  {
           ROOT::EnableThreadSafety();
        }
        return sc;
      }
      else if ( !strcasecmp(n, "ENABLE_IMPLICITMT") )  {
        int implicitMT = -1;
        DbStatus sc = opt._getValue(implicitMT);
        if ( sc.isSuccess() )  {
           if ( implicitMT == 0 )  {
              ROOT::EnableImplicitMT();
           } else if ( implicitMT > 0 )  {
              ROOT::EnableImplicitMT(implicitMT);
           }
        }
        return sc;
      }
      break;
    case 'F':
      if ( !strncasecmp(n+5, "READSTREAMERINFO",15) )  {
        int val = 1;
        DbStatus sc = opt._getValue(val);
        if ( sc.isSuccess() )  {
          TFile::SetReadStreamerInfo(val != 0 ? kTRUE : kFALSE);
        }
        return sc;
      }
      break;
    case 'S':
      if ( !strcasecmp(n,"STREAM_MEMBER_WISE") ) {       // int
        int val = 1;
        DbStatus sc = opt._getValue(val);
        if ( sc.isSuccess() )  {
            TVirtualStreamerInfo::SetStreamMemberWise(val);
        }
        return sc;
      }
      break;
    case 'T':
       if ( !strcasecmp(n+5,"BRANCH_OFFSETTAB_LEN") )  {
	  return opt._getValue(m_branchOffsetTabLen);
       }
       else if ( !strcasecmp(n+5,"MAX_SIZE") )  {
	  long long int max_size = TTree::GetMaxTreeSize();
	  DbStatus sc = opt._getValue(max_size);
	  if ( sc.isSuccess() )  {
	     TTree::SetMaxTreeSize(max_size);
        }
        return sc;
      }
    default:
      break;
  }
  return Error;  
}

/// Access domain specific options
DbStatus RootDomain::getOption(DbOption& opt) const   {
  const char* n = opt.name().c_str();
  switch( ::toupper(n[0]) )  {
    case 'C':
      if ( !strcasecmp(n, "CLASS") )  {
        return opt._setValue((void*)ROOT::GetROOT()->GetClass(opt.option().c_str(), kTRUE));
      }
      break;
    case 'D':
      if ( !strcasecmp(n, "DEFAULT_COMPRESSION") )  {
        return opt._setValue(int(m_defCompression));
      }
      else if ( !strcasecmp(n, "DEFAULT_COMPRESSIONALG") )  {
        return opt._setValue(int(m_defCompressionAlg));
      }
      else if ( !strcasecmp(n, "DEFAULT_SPLITLEVEL") )  {
        return opt._setValue(int(m_defSplitLevel));
      }
      else if ( !strcasecmp(n, "DEFAULT_AUTOSAVE") )  {
        return opt._setValue(int(m_defAutoSave));
      }
      else if ( !strcasecmp(n, "DEFAULT_BUFFERSIZE") )  {
        return opt._setValue(int(m_defBufferSize));
      }
      break;
    case 'E':
      if ( !strcasecmp(n, "ERRNO") )  {
        return opt._setValue(int(gSystem->GetErrno()));
      }
      break;
    case 'N':
      if ( !strcasecmp(n, "NUM_CLASSES") )  {
        return opt._setValue(int(ROOT::GetROOT()->GetNclasses()));
      }
      if ( !strcasecmp(n, "NUM_TYPES") )  {
        return opt._setValue(int(ROOT::GetROOT()->GetNtypes()));
      }
      break;
    case 'S':
      if ( !strcasecmp(n,"STREAM_MEMBER_WISE") )        // int
        return opt._setValue(int(TVirtualStreamerInfo::GetStreamMemberWise()));
      break;
    case 'T':
       if ( !strcasecmp(n+5,"BRANCH_OFFSETTAB_LEN") )  {
	  return opt._setValue(int(m_branchOffsetTabLen));
       }
       if ( !strcasecmp(n+5,"MAX_SIZE") )  {
	  long long int val = TTree::GetMaxTreeSize();
	  return opt._setValue(val);
       }
    default:
      break;
  }
  return Error;  
}

