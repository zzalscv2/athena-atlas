/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
//								//
//			VolumeTreeNavigator.h			//
//	Class containing functions to allow a general   	//
//	navigation of uniquely specified GEANT4 volumes		//
//	and collection of their tree patterns and locations.	//
//								//
//	Written by 	Kevin Sapp				//
//			University of Pittsburgh		//
//			kcs34@pitt.edu				//
//	Last update	02.02.10				//
//                                                              //
//                                                              //
//////////////////////////////////////////////////////////////////

#ifndef VolumeTreeNavigator_H
#define VolumeTreeNavigator_H

#include <string>
#include <vector>
#include <stdarg.h>

#include <AthenaKernel/getMessageSvc.h>
#include <GaudiKernel/MsgStream.h>

#include <G4Track.hh>
#include <G4TouchableHistory.hh>
#include <G4VPhysicalVolume.hh>
#include <G4LogicalVolume.hh>

class G4Track;
class G4StepPoint;
class G4TouchableHistory;
class G4VPhysicalVolume;

typedef std::pair<G4VPhysicalVolume*,int> VolID;
typedef std::vector<VolID> VolTree;
typedef VolTree::iterator VolNav;

class VolumeTreeNavigator {
  public:
      VolumeTreeNavigator(const G4Step*);

      void               SetDepthCutSimple(const int, const int, const int, const int);
      void               SetDepthCutDetail(const char*);
      VolTree            Extract();
      bool               Ascend(unsigned int levels = 1);
      bool               Descend(unsigned int levels = 1);
      bool               KillProcesses(const int, const char*, ...);

      int                GetCurrentDepth();
      G4VPhysicalVolume* GetVolume(int rel = 0) const;
      int                GetCopyNumber(int rel = 0) const;
      G4Track*           GetTrack() const;
      G4StepPoint*       GetPreStepPoint() const;
      G4StepPoint*       GetPostStepPoint() const;
      int                GetStepNumber() const;
      VolTree            GetHistory() const;
      int                GetFullDepth() const;

  private:
      IMessageSvc* _msgSvc;

      G4Track* track;				//!< current track
      G4StepPoint* preStepPoint;		//!< volume/process/etc. info before the current step
      G4StepPoint* postStepPoint;		//!< volume/process/etc. info after the current step
      G4TouchableHistory* preHistory;		//!< touchable history containing current volume's history
      G4int preDepth, stepNo;			//!< depth of volume tree, step number

      VolTree history;				//!< full volume history, cast as const after preparation
      VolNav h_end;				//!< specifies cut termination in history
      VolNav h_nav;				//!< VolTree iterator with no access below cut
};

template <class T>
inline std::string stringify(T obj)
{
  std::ostringstream ss;
  ss << obj;
  return ss.str();
}

template<class T>
inline std::string cleanstr(T obj)
{
  std::string s = stringify(obj);
  std::string::size_type col = s.find_first_of(":");
  while (col != std::string::npos) {
      (s.substr(col,2) == "::") ? s.replace(col,2,"_") : s.replace(col,1,"-");
      col = s.find_first_of(":");
  }
  std::string::size_type sp = s.find_first_of(" ");
  while (sp != std::string::npos) {
      s.erase(sp,1);
      sp = s.find_first_of(" ");
  }
  return s;
}

#include "VolumeTreeNavigator.icc"
#endif
