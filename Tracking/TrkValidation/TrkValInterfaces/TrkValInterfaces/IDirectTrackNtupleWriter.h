/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// IDirectTrackNtupleWriter.h
//   Header file for interface of ValidationNtupleTool in a flavour
//   that books its own tree
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Sebastian.Fleischmann -at- cern.ch
///////////////////////////////////////////////////////////////////

#ifndef TRK_IDIRECTTRACKNTUPLEWRITERTOOL_H
#define TRK_IDIRECTTRACKNTUPLEWRITERTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "TrkParameters/TrackParameters.h"
#include <vector>

class TrackTruth;
class TTree;
class HepMcParticleLink;

namespace Trk {
static const InterfaceID IID_IDirectTrackNtupleWriter("IDirectTrackNtupleWriter",1,0);

class TrackParticleBase;
class Track;


/** @class IDirectTrackNtupleWriter
    @brief Interface to set up and write a TrkValidation ntuple so that it can be
           called for validation action 'inside' any tracking tool.

    provides an interface of a wrapper tool around the various flavours of
    track validation tools in a similar way as the TrackValidationNtupleWriter
    works with storegate. In contrast to the track validation tools, this
    tools books its own tree and writes to it. It is therefore meant to
    fill track validation variables from inside a track reconstruction tool/alg.

    @author Wolfgang.Liebig -at- cern.ch
*/

class IDirectTrackNtupleWriter : virtual public IAlgTool {
public:
     /**Interface ID, declared here, and defined below*/
    static const InterfaceID& interfaceID();

    /** fill AND write ntuple data of a given track */
    virtual StatusCode writeTrackData (
        const Trk::Track&,
        const int iterationIndex,
        const unsigned int fitStatCode = 0) = 0;

     /** fill AND write ntuple data of a given track particle */
    virtual StatusCode writeTrackParticleData (
        const Trk::TrackParticleBase& ) = 0;
    
};

inline const InterfaceID& Trk::IDirectTrackNtupleWriter::interfaceID() {
    return IID_IDirectTrackNtupleWriter;
}

} // end of namespace

#endif // TRK_IDIRECTTRACKNTUPLEWRITERTOOL_H
