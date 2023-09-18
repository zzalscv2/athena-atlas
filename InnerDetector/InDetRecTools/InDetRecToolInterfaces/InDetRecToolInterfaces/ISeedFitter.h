/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISeedFitter_H
#define ISeedFitter_H

#include <vector>
#include <memory>

#include "GaudiKernel/AlgTool.h"
#include "TrkSpacePoint/SpacePoint.h"
#include "TrkTrack/Track.h"

class MsgStream;

namespace InDet {

  /** 
   * @interface ISeedFitter
   * @brief 
   * @author xiangyang.ju@cern.ch
   */
  class ISeedFitter : virtual public IAlgTool
  {
    public:
    DeclareInterfaceID(ISeedFitter, 1, 0);

    ///////////////////////////////////////////////////////////////////
    /// Convert spacepoints to Trk::Track
    ///////////////////////////////////////////////////////////////////
    /** 
     * @brief Fit seed to get track parameters
     * @param spacepoints a list of spacepoints as inputs
     * @return Trk::Track pointer
     */
    virtual std::unique_ptr<const Trk::TrackParameters> fit(
        const std::vector<const Trk::SpacePoint*>& spacepoints
    ) const = 0;


    ///////////////////////////////////////////////////////////////////
    // Print internal tool parameters and status
    ///////////////////////////////////////////////////////////////////
  
    virtual MsgStream&    dump(MsgStream&    out) const=0;
    virtual std::ostream& dump(std::ostream& out) const=0;
  };

  ///////////////////////////////////////////////////////////////////
  // Overload of << operator for MsgStream and  std::ostream
  ///////////////////////////////////////////////////////////////////

  MsgStream&    operator << (MsgStream&   ,const ISeedFitter&);
  std::ostream& operator << (std::ostream&,const ISeedFitter&);

  ///////////////////////////////////////////////////////////////////
  // Overload of << operator MsgStream
  ///////////////////////////////////////////////////////////////////
    
  inline MsgStream& operator << (MsgStream& sl,const ISeedFitter& se) { 
      return se.dump(sl); 
  }
  ///////////////////////////////////////////////////////////////////
  // Overload of << operator std::ostream
  ///////////////////////////////////////////////////////////////////

  inline std::ostream& operator << (std::ostream& sl,const ISeedFitter& se) { 
      return se.dump(sl); 
  }  
}
#endif