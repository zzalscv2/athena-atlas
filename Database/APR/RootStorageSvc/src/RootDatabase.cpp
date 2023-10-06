/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//        Root Database file implementation
//--------------------------------------------------------------------
//
//        Package    : RootStorageSvc (The POOL project)
//
//====================================================================
#include "RootDatabase.h"
#include "RootTreeContainer.h"
#include "StorageSvc/IOODatabase.h"
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbOption.h"
#include "StorageSvc/DbDomain.h"
#include "POOLCore/DbPrint.h"
#include "RootAuxDynIO/RootAuxDynIO.h"

#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IFileMgr.h"

#include <string>
#include <cerrno>

// Root include files
#include "TFile.h"
#include "TMemFile.h"
#include "TFileCacheWrite.h"
#include "TKey.h"
#include "TTree.h"
#include "TTreeCache.h"
#include "TSystem.h"

#include "ROOT/RNTuple.hxx"

using namespace pool;
using namespace std;

/// Standard Constuctor
RootDatabase::RootDatabase() :
        m_file(nullptr), 
        m_version ("2.0"),
        m_defCompression(1),
        m_defCompressionAlg(1),
        m_defSplitLevel(99),
        m_defAutoSave(16*1024*1024),
        m_defBufferSize(16*1024),
        m_maxBufferSize(16*1024*1024),
        m_minBufferEntries(-1),
        m_defWritePolicy(TObject::kOverwrite),   // On write create new versions
        m_branchOffsetTabLen(0),
        m_defTreeCacheLearnEvents(-1),
        m_indexMasterID(0),
        m_fileMgr(nullptr)
{
  m_counters[READ_COUNTER] = m_counters[WRITE_COUNTER] = m_counters[OTHER_COUNTER] = 0;
}

RootDatabase::~RootDatabase()  {
  deletePtr(m_file);
}

bool RootDatabase::exists(const std::string& nam)   {
  Bool_t result = gSystem->AccessPathName(nam.c_str(), kFileExists);
  return (result == kFALSE) ? true : false;
}

/// Access the size of the database: May be undefined for some technologies
long long int RootDatabase::size()  const   {
  if ( m_file )  {
    return m_file->GetSize();
  }
  return -1;
}

/// Callback after successful open of a database object
DbStatus RootDatabase::onOpen(DbDatabase& dbH, DbAccessMode mode)  {
  m_dbH = dbH;
  std::string par_val;
  DbPrint log("RootDatabase.onOpen");
  if ( !dbH.param("FORMAT_VSN", par_val).isSuccess() )  {
    if ( mode&pool::CREATE || mode&pool::UPDATE ) {
      return dbH.addParam("FORMAT_VSN", m_version);
    }
    log << DbPrintLvl::Warning << "No ROOT data format parameter present "
        << "and file not opened for update." << DbPrint::endmsg;
  }
  else {
    m_version = par_val;
  }
  if ( m_file )  {
    log << DbPrintLvl::Debug << dbH.name() << " File version:" << int(m_file->GetVersion())
        << DbPrint::endmsg;
  }
  else  {
    log << DbPrintLvl::Error << "Unknown Root file ..."
        << DbPrint::endmsg;
  }
  //patchStreamers(this, m_file);
  return Success;
}

// Open a new root Database: Access the TFile
DbStatus RootDatabase::open(const DbDomain& domH,const std::string& nam,DbAccessMode mode)
{
  DbPrint log("RootDatabase.open");
  const char* fname = nam.c_str();
  Bool_t result = ( mode == pool::READ ) ? kFALSE : gSystem->AccessPathName(fname, kFileExists);
  DbOption opt1("DEFAULT_COMPRESSION","");
  DbOption opt2("DEFAULT_COMPRESSIONALG","");
  DbOption opt3("DEFAULT_SPLITLEVEL","");
  DbOption opt4("DEFAULT_AUTOSAVE","");
  DbOption opt5("DEFAULT_BUFFERSIZE","");
  DbOption opt6("TREE_BRANCH_OFFSETTAB_LEN","");
  domH.getOption(opt1);
  domH.getOption(opt2);
  domH.getOption(opt3);
  domH.getOption(opt4);
  domH.getOption(opt5);
  domH.getOption(opt6);
  opt1._getValue(m_defCompression);
  opt2._getValue(m_defCompressionAlg);
  opt3._getValue(m_defSplitLevel);
  opt4._getValue(m_defAutoSave);
  opt5._getValue(m_defBufferSize);
  opt6._getValue(m_branchOffsetTabLen);
  //gDebug = 2;
  TDirectory::TContext dirCtxt(0);


  if (m_fileMgr == nullptr) {
    IService *is(nullptr);

    if (Gaudi::svcLocator()->getService("FileMgr",is,true).isFailure()) {
      log << DbPrintLvl::Error
	  << "unable to get the FileMgr, will not manage TFiles"
	  << DbPrint::endmsg;
    } else {
      m_fileMgr = dynamic_cast<IFileMgr*>(is);
    }
  }

  // FIXME: hack to avoid issue with setting up RecExCommon links
  if (m_fileMgr != nullptr && m_fileMgr->hasHandler(Io::ROOT).isFailure()) {
    log << DbPrintLvl::Info
	<< "Unable to locate ROOT file handler via FileMgr. "
	<< "Will use default TFile::Open" 
	<< DbPrint::endmsg;
    m_fileMgr = nullptr;
  }

  // Lock ROOT::gCoreMutex before loading any dictionaries to avoid deadlocks
  // see: https://its.cern.ch/jira/browse/ATR-20263
  ROOT::TReadLockGuard lock (ROOT::gCoreMutex);

  if ( mode == pool::READ )   {
    if (m_fileMgr == nullptr) {
      m_file = TFile::Open(fname);
    } else {
      void *vf(nullptr);
      int r =  m_fileMgr->open(Io::ROOT,"RootDatabase",fname,Io::READ,vf,"POOL",false);
      if (r < 0) {
	log << DbPrintLvl::Error << "unable to open \"" << fname 
	    << "\" for READ"
	    << DbPrint::endmsg;
      } else {      
	m_file = (TFile*)vf;
      }
    }
  }
  else if ( mode&pool::UPDATE && result == kFALSE )    {
    if (m_fileMgr == nullptr) {
      m_file = TFile::Open(fname, "UPDATE", fname);
    } else {
      void *vf(nullptr);
      int r =  m_fileMgr->open(Io::ROOT,"RootDatabase",fname,Io::APPEND,vf,"POOL",true);
      if (r < 0) {
	log << DbPrintLvl::Error << "unable to open \"" << fname 
	    << "\" for UPDATE"
	    << DbPrint::endmsg;
      } else {      
	m_file = (TFile*)vf;
      }
    }
  }
  else if ( pool::RECREATE == (mode&pool::RECREATE) )   {
    if (m_fileMgr == nullptr) {
      m_file = TFile::Open(fname, "RECREATE", fname);
    } else {
      void *vf(nullptr);
      int r =  m_fileMgr->open(Io::ROOT,"RootDatabase",fname,Io::WRITE|Io::CREATE,vf,"POOL",true);
      if (r < 0) {
	log << DbPrintLvl::Error << "unable to open \"" << fname 
	    << "\" for RECREATE"
	    << DbPrint::endmsg;
      } else {      
	m_file = (TFile*)vf;
      }
    }
  }
  else if ( mode&pool::CREATE && result == kTRUE )   {
    if (m_fileMgr == nullptr) {
      m_file = TFile::Open(fname, "RECREATE", fname);
    } else {
      void *vf(nullptr);
      int r =  m_fileMgr->open(Io::ROOT,"RootDatabase",fname,Io::WRITE|Io::CREATE,vf,"POOL",true);
      if (r < 0) {
	log << DbPrintLvl::Error << "unable to open \"" << fname 
	    << "\" for RECREATE"
	    << DbPrint::endmsg;
      } else {      
	m_file = (TFile*)vf;
      }
    }
  }
  else if ( mode&pool::CREATE && result == kFALSE )   {
    log << DbPrintLvl::Error << "You cannot open a ROOT file in mode CREATE"
        << " if it already exists. " << DbPrint::endmsg
        << "[" << nam << "]" << DbPrint::endmsg
        << "Use the RECREATE flag if you wish to overwrite."
        << DbPrint::endmsg;
  }
  else if ( mode&pool::UPDATE && result == kTRUE )   {
    log << DbPrintLvl::Error << "You cannot open a ROOT file in mode UPDATE"
        << " if it does not exists. " << DbPrint::endmsg
        << "[" << nam << "]" << DbPrint::endmsg
        << "Use the CREATE or RECREATE flag."
        << DbPrint::endmsg;
  }
  if ( m_file )   {
    log << DbPrintLvl::Info <<  fname << " File version:" << m_file->GetVersion() << DbPrint::endmsg;
    if ( !m_file->IsOpen() )   {
      log << DbPrintLvl::Error << "Failed to open file:" << nam << DbPrint::endmsg;
      deletePtr(m_file);
    }
  }
  else if ( mode == pool::READ )   {
    log << DbPrintLvl::Error << "You cannot open the ROOT file [" << nam << "] in mode READ"
        << " if it does not exists. " << DbPrint::endmsg;
  }

  if( !m_file ) return Error;

  if( mode != pool::READ ) {
     m_file->SetCompressionLevel(m_defCompression);
     m_file->SetCompressionAlgorithm(m_defCompressionAlg);
  }
  return Success;
}

/// Re-open database with changing access permissions
DbStatus RootDatabase::reopen(DbAccessMode mode)   {
  int result = -1;
  if ( m_file )   {
    TDirectory::TContext dirCtxt(0);
    if ( mode == pool::READ ) {
      result = m_file->ReOpen("READ");
    }
    else if ( mode == pool::UPDATE )  {
      result = m_file->ReOpen("UPDATE");
    }
    else  {
      const char* nam = (m_file) ? m_file->GetName() : "UNKNOWN";
      DbPrint log("RootDatabase.reopen");
      log << DbPrintLvl::Error << "Failed to reopen file: " << nam;
      log << " in mode " << accessMode(mode) << DbPrint::endmsg;
    }
  }
  return (0 == result) ? Success : Error;
}

static void printErrno(DbPrint& log, const char* nam, int err) {
  switch( err )  {
    case 0:      /// No error - all fine.
      break;
    case EXDEV:  /// Cross-device link. Attempt to move a file to a different device 
    case ENOENT: /// No such file or directory.
    case EINVAL: /// Invalid argument. An invalid value was given
    case EACCES: /// Permission denied. The permission setting does not allow 
    case EBADF:  /// file descriptor is invalid or file not open for writing
    case ENOSPC: /// not enough space left on the device for the operation.
    default:
      {
        char errbuf[256];
        log << DbPrintLvl::Error << "Failed to modify file: " << nam
            << " Errno=" << err << " " << strerror_r(err, errbuf, sizeof(errbuf)) << DbPrint::endmsg;
        break;
      }
  }
}

/// Close the root Database: in CREATE/Update mode write the file header...
DbStatus RootDatabase::close(DbAccessMode /* mode */ )  {
   int err(0);
   int fclose_rc = 0;
   if( m_file ) {
      if( m_file->IsOpen() ) {
         const char* nam = m_file->GetName();
         DbPrint log("RootDatabase.close");
         log << DbPrintLvl::Debug << "Closing DB " << nam << DbPrint::endmsg;
         bool closed(false);

         if( byteCount(WRITE_COUNTER) > 0 ) {
            for( auto& writer : m_ntupleWriterMap ) {
               writer.second->close();
               writer.second.reset();
            }
            TDirectory::TContext dirCtxt(0);
            m_file->ResetErrno();
            m_file->Write("0", m_defWritePolicy);
            if (errno != ENOENT) {
               // As of 6.20.02, ROOT may search for libraries during the Write()
               // call, possibly setting errno to ENOENT.  This isn't an error
               // of the write.
               printErrno(log, nam, m_file->GetErrno());
            }
            m_file->ResetErrno();

            if (m_fileMgr == nullptr) {
               m_file->Close();
            } else {
               fclose_rc = m_fileMgr->close(m_file,"RootDatabase");
            }
            closed = true;
            err = m_file->GetErrno();
            printErrno(log, nam, err);
            if (m_file->GetSeekKeys() == 0)   {
               log << DbPrintLvl::Error << "Failed to close file: " << nam
                   << " something wrong happened in Close." << DbPrint::endmsg;
               err = 1;
            }
         }
         log << DbPrintLvl::Debug
             << "I/O READ  Bytes: " << byteCount(READ_COUNTER)  << DbPrint::endmsg
             << "I/O WRITE Bytes: " << byteCount(WRITE_COUNTER) << DbPrint::endmsg
             << "I/O OTHER Bytes: " << byteCount(OTHER_COUNTER) << DbPrint::endmsg;

         if (!closed && m_fileMgr != nullptr) {
            fclose_rc = m_fileMgr->close(m_file,"RootDatabase");
         }      
      }    
   }
   if( fclose_rc == 0 ) {
      if( !deletePtr(m_file).isSuccess() ) err = 1;
   }
   return (err == 0 ? Success : Error);
}

/// Do some statistics: add number of bytes read/written/other
void RootDatabase::addByteCount(int which, long long int num_bytes)  {
  switch(which)   {
  case OTHER_COUNTER:
  case WRITE_COUNTER:
  case READ_COUNTER:
    m_counters[which] += num_bytes;
    break;
  default:
    break;
  }
}

/// Do some statistics: retrieve number of bytes read/written/other
long long int RootDatabase::byteCount(int which) const   {
  switch(which)   {
  case OTHER_COUNTER:
  case WRITE_COUNTER:
  case READ_COUNTER:
    return m_counters[which];
  default:
    return -1;
  }
}

/// Access options
DbStatus RootDatabase::getOption(DbOption& opt)  {
  const char* n = opt.name().c_str();
  switch( ::toupper(n[0]) )  {
    case 'C':
      if ( !m_file )
        return Error;
      else if ( !strcasecmp(n, "COMPRESSION_LEVEL") )     // int
        return opt._setValue(int(m_file->GetCompressionLevel()));
      else if ( !strcasecmp(n, "COMPRESSION_ALGORITHM") ) // int
        return opt._setValue(int(m_file->GetCompressionAlgorithm()));
      else if ( !strcasecmp(n, "COMPRESSION_FACTOR") )    // float
        return opt._setValue(double(m_file->GetCompressionFactor()));
      else if ( !strcasecmp(n, "CONTAINER_SPLITLEVEL") )  {
        if (!opt.option().size()) {
          DbPrint log("RootDatabase.getOption");
          log << DbPrintLvl::Error << "Must set option to container name to set CONTAINER_SPLITLEVEL" << DbPrint::endmsg;
          return Error;
        }
        const string containerName = opt.option();
        int containerSplitLevel=m_defSplitLevel;
        map< string, int >::const_iterator  cspit = m_customSplitLevel.find( containerName );
        if ( cspit != m_customSplitLevel.end() ) {
          containerSplitLevel = cspit->second;
        }
        return opt._setValue(int(containerSplitLevel));
      }
      break;
    case 'B':
      if ( !m_file )
        return Error;
      else if ( !strcasecmp(n,"BEST_BUFFER") )            // int
        return opt._setValue(int(m_file->GetBestBuffer()));
      else if ( !strcasecmp(n,"BYTES_WRITTEN") )          // double
        return opt._setValue(double(m_file->GetBytesWritten()));
      else if ( !strcasecmp(n,"BYTES_READ") )             // double
        return opt._setValue(double(m_file->GetBytesRead()));
      else if ( !strcasecmp(n,"BYTES_FREE") )             // int
        return opt._setValue(int(m_file->GetNbytesFree()));
      else if ( !strcasecmp(n,"BYTES_INFO") )             // int
        return opt._setValue(int(m_file->GetNbytesInfo()));
      else if ( !strcasecmp(n,"BYTES_KEYS") )             // int
        return opt._setValue(int(m_file->GetNbytesKeys()));
      break;
    case 'D':
      if ( !strcasecmp(n,"DEFAULT_COMPRESSION") )         // int
        return opt._setValue(int(m_defCompression));
      else if ( !strcasecmp(n,"DEFAULT_COMPRESSIONALG") ) // int
        return opt._setValue(int(m_defCompressionAlg));
      else if ( !strcasecmp(n, "DEFAULT_SPLITLEVEL") )    // int
        return opt._setValue(int(m_defSplitLevel));
      else if ( !strcasecmp(n, "DEFAULT_AUTOSAVE") )      // int
        return opt._setValue(int(m_defAutoSave));   
      else if ( !strcasecmp(n, "DEFAULT_BUFFERSIZE") )    // int
        return opt._setValue(int(m_defBufferSize));
      else if ( !strcasecmp(n, "DEFAULT_WRITEPOLICY") )   // int
        return opt._setValue(int(m_defWritePolicy));
      break;
    case 'F':
      if ( !m_file )
        return Error;
      else if ( !strcasecmp(n,"FILEBYTES_WRITTEN") )      // double
        return opt._setValue(double(m_file->GetFileBytesWritten()));
      else if ( !strcasecmp(n,"FILEBYTES_READ") )         // double
        return opt._setValue(double(m_file->GetFileBytesRead()));
      else if ( !strcasecmp(n,"FILE_READ_CALLS") )        // int
        return opt._setValue(int(m_file->GetFileReadCalls()));
      else if ( !strcasecmp(n,"FILE_DESCRIPTOR") )        // int
        return opt._setValue(int(m_file->GetFd()));
      else if ( !strcasecmp(n,"FILE_VERSION") )           // int
        return opt._setValue(int(m_file->GetVersion()));
      else if ( !strcasecmp(n,"FILE_SIZE") )              // int
        return opt._setValue((long long int)m_file->GetSize());
      else if ( !strcasecmp(n,"FILE_ERROR") )             // int
        return opt._setValue(int(m_file->GetErrno()));
      else if ( !strcasecmp(n,"FILE_KEYS") )              // void*
        return opt._setValue((void*)m_file->GetListOfKeys());
      else if ( !strcasecmp(n,"FILE_END") )               // int
        return opt._setValue((long long int)m_file->GetEND());
      break;
    case 'G':
      if ( !m_file )
        return Error;
      else if ( !strcasecmp(n,"GET_OBJECT") )  {          // void*
        const char* key = "";
        opt._getValue(key);
        return opt._setValue((void*)m_file->Get(key));
      }
      break;
    case 'I':
      if ( !strcasecmp(n,"IOBYTES_WRITTEN") )             // int
        return opt._setValue((long long int)(byteCount(WRITE_COUNTER)));
      else if ( !strcasecmp(n,"IOBYTES_READ") )           // int
        return opt._setValue((long long int)(byteCount(READ_COUNTER)));
      break;
    case 'M':
      if ( !strcasecmp(n, "MAXIMUM_BUFFERSIZE") )         // int
        return opt._setValue(int(m_maxBufferSize));
      else if ( !strcasecmp(n, "MINIMUM_BUFFERENTRIES") ) // int
        return opt._setValue(int(m_minBufferEntries));
      break;
    case 'N':
      if ( !m_file )
        return Error;
      else if ( !strcasecmp(n,"NKEYS") )                  // int
        return opt._setValue(int(m_file->GetNkeys()));
      break;
    case 'R':
      if ( !m_file )
        return Error;
      else if ( !strcasecmp(n,"READ_CALLS") )             // int
        return opt._setValue(int(m_file->GetReadCalls()));
      break;
    case 'T':
      if( !strcasecmp(n+5,"BRANCH_OFFSETTAB_LEN") )  {
        return opt._setValue(int(m_branchOffsetTabLen));
      } else if( !strcasecmp(n,"TFILE") )  {              // void*
          return opt._setValue((void*)m_file);
      } else if( !strcasecmp(n+5,"MAX_SIZE") )  {
          return opt._setValue((long long int)TTree::GetMaxTreeSize());
      } else if( !strcasecmp(n+5,"CACHE_SIZE") ) {
          if (!m_treeNameWithCache.size())
              return opt._setValue((int)0);
          if ( !m_file ) return Error;
          TTree* tr = (TTree*)m_file->Get(m_treeNameWithCache.c_str());
          if (tr) return opt._setValue((int)tr->GetCacheSize());
          return opt._setValue((int)0);
      } else if( !strcasecmp(n+5,"CACHE_LEARN_EVENTS") ) {
          return opt._setValue((int)TTreeCache::GetLearnEntries());
      } else if( !strcasecmp(n+5,"NAME_WITH_CACHE") ) {
          return opt._setValue(m_treeNameWithCache.c_str());
      }
      break;
    default:
      break;
  }
  return Error;  
}

/// Set options
DbStatus RootDatabase::setOption(const DbOption& opt)  {
  const char* n = opt.name().c_str();
  switch( ::toupper(n[0]) )  {
    case 'C':
      if ( !m_file )
        return Error;
      else if ( !strcasecmp(n, "CD") )  {
        const char* key = "";
        opt._getValue(key);
        m_file->cd(key);
        return Success;
      }
      else if ( !strcasecmp(n, "COMPRESSION_LEVEL") )  {
        int val=1;
        opt._getValue(val);
        m_file->SetCompressionLevel(val);
        return Success;
      }
      else if ( !strcasecmp(n, "COMPRESSION_ALGORITHM") )  {
        int val=1;
        opt._getValue(val);
        m_file->SetCompressionAlgorithm(val);
        return Success;
      }
      else if ( !strcasecmp(n, "CONTAINER_SPLITLEVEL") )  {
        if (!opt.option().size()) {
          DbPrint log("RootDatabase.setOption");
          log << DbPrintLvl::Error << "Must set option to container name to set CONTAINER_SPLITLEVEL" << DbPrint::endmsg;
          return Error;
        }
        string containerName = opt.option();
        int val=m_defSplitLevel;
        opt._getValue(val);
        m_customSplitLevel.insert(pair< std::string, int>(containerName, val) );
        return Success;
      }
      break;
    case 'D':
      if (      !strcasecmp(n, "DEFAULT_COMPRESSION") )   // int
        return opt._getValue(m_defCompression);
      else if ( !strcasecmp(n, "DEFAULT_COMPRESSIONALG") )// int
        return opt._getValue(m_defCompressionAlg);
      else if ( !strcasecmp(n, "DEFAULT_SPLITLEVEL") )    // int
        return opt._getValue(m_defSplitLevel);
      else if ( !strcasecmp(n, "DEFAULT_AUTOSAVE") )      // int
        return opt._getValue(m_defAutoSave);   
      else if ( !strcasecmp(n, "DEFAULT_BUFFERSIZE") )    // int
        return opt._getValue(m_defBufferSize);
      else if ( !strcasecmp(n, "DEFAULT_WRITEPOLICY") )   // int
        return opt._getValue(m_defWritePolicy);
      break;
    case 'F':
      if ( !m_file )
        return Error;
      else if ( !strcasecmp(n,"FILEBYTES_WRITTEN") )   {
        double val = 0;
        opt._getValue(val);
        Long64_t v = (Long64_t)val;
        m_file->SetFileBytesWritten(v);
        return Success;
      }
      else if ( !strcasecmp(n,"FILEBYTES_READ") )  {
        double val = 0;
        opt._getValue(val);
        Long64_t v = (Long64_t)val;
        m_file->SetFileBytesRead(v);
        return Success;
      }
      else if ( !strcasecmp(n,"FILECACHE_WRITE") )  {
        double val = 0;
        opt._getValue(val);
        Long64_t v = (Long64_t)val;
        new TFileCacheWrite(m_file, v); //TFile will take ownership and delete its TFileCacheWrite
        return Success;
      }
      else if ( !strcasecmp(n,"FILE_FLUSH") )  {
        m_file->Flush();
        return Success;
      }
      else if ( !strncasecmp(n, "FILE_READSTREAMERINFO",15) )  {
        m_file->ReadStreamerInfo();
        return Success;
      }
      else if ( !strcasecmp(n, "FRIEND_TREE") )  {
        DbPrint log("RootDatabase::setOption");
        char *s = nullptr;
        char *s2 = nullptr;
        if( opt._getValue(s).isSuccess() and s ) {
          for (s2 = s; *s2 != '\0'; s2++) {
            if (*s2 == ':') {
              *s2 = '\0';
              s2++;
              break;
            }
          }
          TTree* tree1 = (TTree*)m_file->Get(s);
          TTree* tree2 = (TTree*)m_file->Get(s2);
          if (!tree1 || !tree2) {
             return Error;
          }
          tree1->AddFriend(tree2);
          log << DbPrintLvl::Debug << "FRIEND_TREE set to " << s << " and " << s2 << DbPrint::endmsg;
          return Success;
        }
        return Error;
      }
      break;
    case 'I':
       if( !strcasecmp(n, "INDEX_MASTER") ) {
          DbPrint log("RootDatabase::setOption");
          char *s = nullptr;
          if( opt._getValue(s).isSuccess() and s ) {
             m_indexMaster = s;
             log << DbPrintLvl::Debug << "INDEX_MASTER set to " << m_indexMaster << DbPrint::endmsg;
             return Success;
          }
          log << DbPrintLvl::Debug << "INDEX_MASTER: s=" << (void*)s << DbPrint::endmsg;
          return Error;
       }
      break;
    case 'M':
      if ( !strcasecmp(n, "MAXIMUM_BUFFERSIZE") )         // int
        return opt._getValue(m_maxBufferSize);
      else if ( !strcasecmp(n, "MINIMUM_BUFFERENTRIES") ) // int
        return opt._getValue(m_minBufferEntries);
      break;
    case 'P':
      if ( !m_file )
        return Error;
      else if ( !strcasecmp(n,"PRINT") )  {
        m_file->Print();
        return Success;
      }
      break;
    case 'R':
      if ( !m_file )
        return Error;
      else if ( !strncasecmp(n, "READSTREAMERINFO",10) )  {
        m_file->ReadStreamerInfo();
        return Success;
      }
      break;
    case 'S':
      if ( !m_file )
        return Error;
      else if ( !strcasecmp(n,"SETMODIFIED") )  {
        m_file->SetModified();
        return Success;
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
       else if ( !strcasecmp(n+5,"MAX_VIRTUAL_SIZE") )  {
          DbPrint log("RootDatabase.setOption");
          log << DbPrintLvl::Debug << "Request virtual tree size" << DbPrint::endmsg;
          if ( !m_file ) return Error;
          log << DbPrintLvl::Debug << "File name " << m_file->GetName() << DbPrint::endmsg;

          int virtMaxSize = 0;
          opt._getValue(virtMaxSize);
          if (!opt.option().size()) {
             log << DbPrintLvl::Error << "Must set option to tree name to start TREE_MAX_VIRTUAL_SIZE " << DbPrint::endmsg;
             return Error;
          }
          TTree* tree = (TTree*)m_file->Get(opt.option().c_str());
          if (!tree) {
             log << DbPrintLvl::Error << "Could not find tree " << opt.option() << DbPrint::endmsg;
             return Error;
          }
          log << DbPrintLvl::Debug << "Got tree " << tree->GetName() << DbPrint::endmsg;
          tree->SetMaxVirtualSize(virtMaxSize);
          return Success;
       }
       else if ( !strcasecmp(n+5,"AUTO_FLUSH") )  {
          return setAutoFlush(opt);
       }
       else if ( !strcasecmp(n+5,"CACHE_LEARN_EVENTS") )  {
          DbStatus s = opt._getValue(m_defTreeCacheLearnEvents);
          if( s.isSuccess() ) {
             DbPrint log("RootDatabase.setOption");
             if ( m_file->GetListOfKeys()->Contains("CollectionTree") )  {
                TTree *tree = (TTree*)m_file->Get("CollectionTree");
                if (tree != nullptr && tree->GetAutoFlush() > 0) {
                   if (m_defTreeCacheLearnEvents < tree->GetAutoFlush()) {
                      log << DbPrintLvl::Info << n << ": Overwriting LearnEvents with CollectionTree AutoFlush" << DbPrint::endmsg;
                      m_defTreeCacheLearnEvents = tree->GetAutoFlush();
                   }
                }
             }
             TTreeCache::SetLearnEntries(m_defTreeCacheLearnEvents);
             log << DbPrintLvl::Debug << n << " = " << m_defTreeCacheLearnEvents << DbPrint::endmsg;
          }
          return s;
       }
       else if ( !strcasecmp(n+5,"CACHE") )  {
           DbPrint log("RootDatabase.setOption");
           log << DbPrintLvl::Debug << "Request tree cache " << DbPrint::endmsg;
           if ( !m_file ) return Error;
           log << DbPrintLvl::Debug << "File name " << m_file->GetName() << DbPrint::endmsg;

           int cacheSize = 0;
           opt._getValue(cacheSize);
           if (!opt.option().size()) {
               log << DbPrintLvl::Error << "Must set option to tree name to start TREE_CACHE " << DbPrint::endmsg;
               return Error;
           }
           m_treeNameWithCache = opt.option();
           TTree* tr = (TTree*)m_file->Get(m_treeNameWithCache.c_str());
           if (!tr) {
               log << DbPrintLvl::Error << "Could not find tree " << m_treeNameWithCache << DbPrint::endmsg;
               return Error;
           }
           else log << DbPrintLvl::Debug << "Got tree " << tr->GetName() 
                    << " read entry " << tr->GetReadEntry() << DbPrint::endmsg;
           tr->SetCacheSize(cacheSize);
           if (m_defTreeCacheLearnEvents < 0) {
              long long int autoFlush = tr->GetAutoFlush();
              if (autoFlush > 0) { // Tree was written flushing on number of events
                 TTreeCache::SetLearnEntries(-m_defTreeCacheLearnEvents * autoFlush);
              }
           } else {
              TTreeCache::SetLearnEntries(m_defTreeCacheLearnEvents);
           }
           TTreeCache* cache = (TTreeCache*)m_file->GetCacheRead();
           if (cache) {
               cache->SetEntryRange(0, tr->GetEntries());
               log << DbPrintLvl::Debug << "Using Tree cache. Size: " << cacheSize 
                   << " Nevents to learn with: " << m_defTreeCacheLearnEvents << DbPrint::endmsg;
           }
           else if (cacheSize != 0) {
               log << DbPrintLvl::Error << "Could not get cache " << DbPrint::endmsg;
               return Error;
           }
           return Success;
       }
      break;
    default:
      break;
  }
  return Error;  
}

/// Set TTree autoFlush value.  For Branch Containers enable TTree Fill mode
DbStatus RootDatabase::setAutoFlush(const DbOption& opt)
{
   DbPrint log("RootDatabase.setOption");
   log << DbPrintLvl::Debug << "Request TREE_AUTO_FLUSH " << DbPrint::endmsg;
   if ( !m_file ) return Error;
   log << DbPrintLvl::Debug << "File name " << m_file->GetName() << DbPrint::endmsg;

   if (!opt.option().size()) {
      log << DbPrintLvl::Error
          << "TREE_AUTO_FLUSH database option requires TTree name in option parameter"
          << DbPrint::endmsg;
      return Error;
   } 
   string treeName = opt.option();
   int val=0;
   DbStatus sc = opt._getValue(val);
   if( sc.isSuccess() )  {
      log << DbPrintLvl::Debug << "Demand to set AUTO_FLUSH for TTree: " << treeName
          << " with value: " << val << DbPrint::endmsg;
      map< string, int >::iterator  tafit = m_autoFlushTrees.find( treeName );
      if( tafit != m_autoFlushTrees.end() && tafit->second == val ) {
         log << DbPrintLvl::Debug << " -- AUTO_FLUSH already set, skipping " << DbPrint::endmsg;
         return Success;
      }
      m_autoFlushTrees[treeName] = val;
      // set Tree Fill mode for any branch containers already registered
      for( map< TTree*, ContainerSet_t >::iterator it = m_containersInTree.begin();
           it != m_containersInTree.end(); ++it ) {
         TTree* tree = it->first;
         if( tree->GetName() == treeName ) {              
            ContainerSet_t &contset = it->second;
            // already some branch containers created, set TTree AUTO_FLUSH
            // and mark them for TTree Fill mode
            log << DbPrintLvl::Debug << "Setting AUTO_FLUSH for TTree: "
                << treeName << " to value: " << val << DbPrint::endmsg; 
            tree->SetAutoFlush( val );
            for( ContainerSet_t::iterator it = contset.begin(); it != contset.end(); ++it ) {
               log << DbPrintLvl::Debug << "Setting TTree fill mode for container: " << (**it).getName()
                   << DbPrint::endmsg; 
               (**it).setTreeFillMode(true);
            }
            break;
         }
      } 
   } else {
      log << DbPrintLvl::Error << "Could not get AUTO_FLUSH value for TTree: " << treeName
          << DbPrint::endmsg;
   }
   return sc;
}


/* register creation (or opening for update) of a Branch Container.
   If it is located in a TTree with AUTO_FLUSH option, the whole TTree must be Filled
   (instead of the container branch, or the AUTO_FLUSH option will not work).
   TTree::Fill() is done in commit in transAct()
*/
void RootDatabase::registerBranchContainer(RootTreeContainer* cont)
{
   TTree* tree = cont->getTTree();
   // cout << "------ registerBranchContainer: " << cont->getName() << endl;
   map<string,int>::const_iterator flushIt = m_autoFlushTrees.find( tree->GetName() );
   if( !cont->isBranchContainer() ) {
      if( flushIt != m_autoFlushTrees.end() ) {
         // normal container option but set on the DB level
         DbPrint log(cont->getName());
         log << DbPrintLvl::Debug << "Setting AUTO_FLUSH for Container TTree: "
             << tree->GetName() << " to value: " << flushIt->second << DbPrint::endmsg; 
         tree->SetAutoFlush( flushIt->second );
      }
      return;
   }
   // group branch containers from the same TTree
   ContainerSet_t &contset = m_containersInTree[tree];
   contset.insert(cont);
   if( flushIt == m_autoFlushTrees.end() )
      return; // this branch container is not in auto_flushed tree
   if( contset.size() == 1 ) {
      // first container for a given tree -> set the tree option 
      DbPrint log(cont->getName());
      log << DbPrintLvl::Debug << "Setting AUTO_FLUSH for TTree: "
        << tree->GetName() << " to value: " << flushIt->second << DbPrint::endmsg; 
      tree->SetAutoFlush( flushIt->second );
   }
   cont->setTreeFillMode(true);
}


/// Execute Database Transaction action
DbStatus RootDatabase::transAct(Transaction::Action action)
{
   // We only care about writing actions here
   if( action != Transaction::TRANSACT_COMMIT and action != Transaction::TRANSACT_FLUSH )
      return Success;

   // MN: maybe !m_file should be an error
   if( m_file == nullptr or !m_file->IsWritable() )
      return Success;

   // Flush the RNTuples from the DB level, so every ntuple is flused only once
   for( auto& writer : m_ntupleWriterMap ) {
      auto wr = writer.second.get();
      if( wr->isGrouped() and wr->needsCommit() ) wr->commit();
   }

   if( fillBranchContainerTrees() != Success ) return Error;

   // process flush to write file
   if( action == Transaction::TRANSACT_FLUSH ) {
      m_file->Write();
      if (dynamic_cast<TMemFile*>(m_file) == nullptr) {
         TIter nextKey(m_file->GetListOfKeys());
         while (TKey* key = static_cast<TKey*>(nextKey())) {
            TClass* cl = TClass::GetClass(key->GetClassName());
            if (cl != nullptr && cl->InheritsFrom("TTree")) {
               TTree* tree = static_cast<TTree*>(m_file->Get(key->GetName()));
               DbPrint log( m_file->GetName() );
               if (tree != nullptr && tree->GetBranch("index_ref") != nullptr && tree->GetEntries() > 0) {
                  TList* friendTrees(tree->GetListOfFriends());
                  if (friendTrees != nullptr && !friendTrees->IsEmpty()) {
                     log << DbPrintLvl::Debug << "BuildIndex for index_ref to " << tree->GetName() << DbPrint::endmsg;
                     tree->BuildIndex("index_ref");
                     for (const auto&& obj: *friendTrees) {
                        TTree* friendTree = tree->GetFriend(obj->GetName());
                        if (friendTree != nullptr && friendTree->GetBranch("index_ref") != nullptr && friendTree->GetEntries() > 0) {
                           log << DbPrintLvl::Debug << "BuildIndex for index_ref to " << friendTree->GetName() << DbPrint::endmsg;
                           friendTree->BuildIndex("index_ref");
                        }
                     }
                  }
               }
            }
         }
      }
      // check all TTrees, if Branch baskets are below max, after explicit Write() call
      for( auto& el : m_containersInTree ) {
         reduceBasketsSize( el.first );
      }
   }
   // process commits only
   if( action != Transaction::TRANSACT_COMMIT )
      return Success;

   // process commits
   DbPrint log( m_file->GetName() );
   log << DbPrintLvl::Debug << "DB Action Commit" << DbPrint::endmsg; 

   for( auto& el : m_containersInTree ) {
      TTree *tree = el.first;
      if( tree->GetEntries() == m_minBufferEntries ) {
         increaseBasketsSize(tree);
      }
   }

   if( !m_indexMaster.empty() ) {
      log << DbPrintLvl::Debug << "Synchronizing indexes to master: " <<  m_indexMaster << DbPrint::endmsg;
      std::vector<IDbContainer*> containers;
      m_dbH.containers(containers);
      m_indexMasterID = 0;
      if( m_indexMaster == "*" ) {
         // find the biggest index ID
         for( auto c : containers ) {
            uint64_t nextID = c->nextRecordId() & 0xFFFFFFFF;
            if( nextID > m_indexMasterID ) m_indexMasterID = nextID;
         }
      } else {
         // look for the master by name
         for( auto c = containers.begin(); c !=  containers.end(); ++c ) {
            if( (*c)->name() == m_indexMaster ) {
               m_indexMasterID = (*c)->nextRecordId() & 0xFFFFFFFF;
               containers.erase(c);
               break;
            }
         }
      }
      // synchronize all container indices
      if( m_indexMasterID > 0 ) {
         // go back by one, so nextID in other indices is equal the last ID in master
         // this works if synchronizing in separate commits - but not in the same commit!
         --m_indexMasterID;
         for( auto c: containers ) {
            c->useNextRecordId( m_indexMasterID );
         }
      }
   }
   return Success;
}


void RootDatabase::reduceBasketsSize(TTree* tree)
{
   if( !tree ) return;
   TIter next(tree->GetListOfBranches());
   TBranch * b = nullptr;
   while((b = (TBranch*)next())){
      if (b->GetBasketSize() > m_maxBufferSize) {
         DbPrint log( m_file->GetName() );
         log << DbPrintLvl::Debug << b->GetName() << " Basket size = " << b->GetBasketSize()
             << " reduced to " << m_maxBufferSize
             << DbPrint::endmsg;
         b->SetBasketSize(m_maxBufferSize);
      }
   }
}


void RootDatabase::increaseBasketsSize(TTree* tree)
{
   if( !tree ) return;
   TIter next(tree->GetListOfBranches());
   TBranch * b = nullptr;
   while((b = (TBranch*)next())){
      if (b->GetBasketSize() < b->GetTotalSize()) {
         DbPrint log( m_file->GetName() );
         log << DbPrintLvl::Debug << b->GetName() << " Initial basket size = " << b->GetBasketSize()
             << " increased to " << b->GetBasketSize() * (1 + int(b->GetTotalSize()/b->GetBasketSize()))
             << DbPrint::endmsg;
         b->SetBasketSize(b->GetBasketSize() * (1 + int(b->GetTotalSize()/b->GetBasketSize())));
      }
   }
}


DbStatus RootDatabase::fillBranchContainerTrees()
{
   // check all TTrees with branch containers, if they need Filling
   for( map< TTree*, ContainerSet_t >::iterator treeIt = m_containersInTree.begin(),
           mapEnd = m_containersInTree.end(); treeIt != mapEnd; ++treeIt ) {
      TTree *tree = treeIt->first;
      // check all TTrees, if Branch baskets are below max, after flushing is triggered via ROOT auto-flush
      if (tree->GetEntriesFast() == tree->GetAutoFlush()) {
         TIter next(tree->GetListOfBranches());
         TBranch * b = nullptr;
         while((b = (TBranch*)next())){
            if (b->GetBasketSize() > m_maxBufferSize) {
               DbPrint log( m_file->GetName() );
               log << DbPrintLvl::Debug << b->GetName() << " Basket size = " << b->GetBasketSize()
                   << " reduced to " << m_maxBufferSize
                   << DbPrint::endmsg;
               b->SetBasketSize(m_maxBufferSize);
            }
         }
      }

      ContainerSet_t &containers = treeIt->second;
      if( (*containers.begin())->usingTreeFillMode() ) {
         // cout << "------- TTree " << tree->GetName() << " - checking containers for commit" << endl;
         int    clean = 0, dirty = 0;
         for( ContainerSet_t::const_iterator cIt = containers.begin(); cIt != containers.end(); ++cIt ) {
            if( (*cIt)->isDirty() ) dirty++;
            else clean++;
         }
         if( !dirty ) continue;   // nothing to write, go to the next TTree
         if( clean == 0 ) {
            // good - all containers were updated.  Fill the TTree and clean dirty status
            int num_bytes = tree->Fill();
            //cout << "-----MN---  Filled branch container TTree: " << m_file->GetName() << "::" << tree->GetName() << endl;
            if( num_bytes > 0 ) {
               addByteCount( RootDatabase::WRITE_COUNTER, num_bytes );
            } else {
               DbPrint err( m_file->GetName() );
               err << DbPrintLvl::Error << "Write to " <<  tree->GetName() << " failed"
                   << DbPrint::endmsg;
               return Error;
            }
            for( ContainerSet_t::iterator cIt = containers.begin(); cIt != containers.end(); ++cIt ) {
               (*cIt)->clearDirty();
            }
         } else {
            // error - some containers in this TTree were not updated
            DbPrint err( m_file->GetName() );
            for( ContainerSet_t::const_iterator cIt = containers.begin(); cIt != containers.end(); ++cIt ) {
               if( !(*cIt)->isDirty() ) {
                  err << DbPrintLvl::Error << "Branch Container " <<  (*cIt)->getName()
                      << " was not filled in this transaction. This is required by TREE_AUTO_FLUSH option."
                      << DbPrint::endmsg;
               }
            }
            return Error;
         }
      } else { // not TreeFillMode
         uint64_t maxbranchlen = 0;
         for( ContainerSet_t::iterator cIt = containers.begin(); cIt != containers.end(); ++cIt ) {
            maxbranchlen = max( maxbranchlen, (*cIt)->size() );
         }
         if( maxbranchlen > 0 )  tree->SetEntries( maxbranchlen );
      }
   }
   return Success;
}


std::unique_ptr<RootAuxDynIO::IRootAuxDynReader>
RootDatabase::getNTupleAuxDynReader(const std::string& ntuple_name, const std::string& field_name)
{
   auto reader_entry = m_ntupleReaderMap.find(ntuple_name);
   if( reader_entry == m_ntupleReaderMap.end() ) {
      return nullptr;
   }
   return RootAuxDynIO::getNTupleAuxDynReader(field_name, reader_entry->second.get());
}


RNTupleReader*
RootDatabase::getNTupleReader(std::string ntuple_name)
{
   auto reader_entry = m_ntupleReaderMap.find(ntuple_name);
   if( reader_entry != m_ntupleReaderMap.end() ) {
      return reader_entry->second.get();
   }
   const std::string file_name = m_file->GetName();
   auto native_reader = RNTupleReader::Open(string("RNT:")+ntuple_name, file_name);
   RNTupleReader *r = native_reader.get();
   m_ntupleReaderMap.emplace(ntuple_name, std::move(native_reader));
   return r;
}


RootAuxDynIO::IRNTupleWriter*
RootDatabase::getNTupleWriter(std::string ntuple_name, bool create)
{
   auto& writer = m_ntupleWriterMap[ntuple_name];
   if( !writer and create ) {
      writer = RootAuxDynIO::getNTupleAuxDynWriter(m_file, string("RNT:")+ntuple_name, m_file->GetCompressionSettings() );
   }
   if( writer and create ) {
      // treat the create flag as an indication of a new container client and count them
      writer->increaseClientCount();
   }
   return writer.get();
}


uint64_t RootDatabase::indexLookup([[maybe_unused]]RNTupleReader *reader, uint64_t idx_val) {
#if ROOT_VERSION_CODE >= ROOT_VERSION( 6, 29, 0 )
   if( m_ntupleIndexMap.find(reader) == m_ntupleIndexMap.end() ) {
      // first access to RNTuple, read and store the index
      DbPrint log( m_file->GetName() );
      log << DbPrintLvl::Debug << "Reading index" << DbPrint::endmsg;
      indexLookup_t &index = m_ntupleIndexMap[reader];
      ROOT::Experimental::REntry *entry = reader->GetModel()->GetDefaultEntry();
      auto val_i = entry->begin();
      for(; val_i != entry->end(); ++val_i ) {
         if( val_i->GetField()->GetName() == "index_ref" ) break;
      }
      if( val_i == entry->end() ) {
         return idx_val;
      }
      uint64_t idx_size = reader->GetNEntries();
      index.reserve(idx_size);
      uint64_t idx;
      auto rfv = val_i->GetField()->BindValue( &idx );
      for(uint64_t row=0; row < idx_size; row++) {
         rfv.Read(row);
         index[idx] = row;
      }
   }
   indexLookup_t &index = m_ntupleIndexMap[reader];
   auto it = index.find(idx_val);
   if( it != index.end() ) {
      // cout << "MN: remapped OID=" << hex << idx_val << " to " << it->second << endl;
      idx_val = it->second;
   }
#endif
   return idx_val;
}
