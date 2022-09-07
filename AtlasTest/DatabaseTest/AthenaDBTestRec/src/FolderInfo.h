/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENADBTESTREC_FOLDERINFO_H
#define ATHENADBTESTREC_FOLDERINFO_H

#include <vector>
#include <string>

#include "DBConnection.h"
#include "CoolKernel/IFolder.h"
#include "CoolKernel/RecordSpecification.h"
#include "CxxUtils/checker_macros.h"
#include "PersistentDataModel/Placement.h"

class ATLAS_NOT_THREAD_SAFE FolderInfo {
 public:
  enum PayloadTime {
		    PTimeUndefined=0,
		    DCSC=1,
		    DCSP=2,
		    RUNC=3,
		    RUNP=4
		    };
  enum PayloadType {
		      PTypeUndefined=0,
		      Int=1,
		      Float=2,
		      String=3,
                      PoolRef=4,
		      Blob=5,
		      CoraCool=6,
		      CoolVector=7
		      };

  FolderInfo(const std::string name,DBConnection* dbconn,const int ndbconn,
	     const int nchan, PayloadTime ptime,PayloadType ptype, 
	     const int ncol, const int size, const int period,
	     const std::string tag="",const bool payloadTable=false);

  ~FolderInfo();

  bool createFolder();

  // accessor functions
  const std::string name() const;
  DBConnection* dbConnection();
  int ndbconn() const;
  int nchan() const;
  PayloadTime ptime() const;
  PayloadType ptype() const;
  int size() const;
  int period() const;
  const std::string tag() const;
  const Placement* poolplace() const;

  void setpoolplace(Placement* const poolplace);
  
  void printSpec(const cool::IRecordSpecification& catrspec) const;

 private:
  std::string m_name;
  DBConnection* m_dbconn;
  int m_ndbconn;
  int m_nchan;
  PayloadTime m_ptime;
  PayloadType m_ptype;
  int m_ncol;
  int m_size;
  int m_period;
  std::vector<int> m_noisychan;
  std::string m_tag;
  bool m_paytable;

  Placement* m_poolplace;

};

inline const std::string FolderInfo::name() const {return m_name;}
inline DBConnection* FolderInfo::dbConnection() {return m_dbconn;}
inline int FolderInfo::ndbconn() const { return m_ndbconn; }

inline int FolderInfo::nchan() const {return m_nchan;}
inline FolderInfo::PayloadTime FolderInfo::ptime() const {return m_ptime;}
inline FolderInfo::PayloadType FolderInfo::ptype() const {return m_ptype;}
inline int FolderInfo::size() const {return m_size;}
inline int FolderInfo::period() const {return m_period;}
inline const std::string FolderInfo::tag() const {return m_tag;}

inline const Placement* FolderInfo::poolplace() const {return m_poolplace;}
inline void FolderInfo::setpoolplace(Placement* const poolplace) {
  m_poolplace=poolplace; }

// helper functions
FolderInfo::PayloadTime string2ptime(const std::string str);
FolderInfo::PayloadType string2ptype(const std::string str);

#endif // ATHENADBTESTREC_FOLDERINFO_H
