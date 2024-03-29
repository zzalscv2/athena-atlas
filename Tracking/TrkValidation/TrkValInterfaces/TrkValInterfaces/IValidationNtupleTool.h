/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// IValidationNtupleTool.h
//   Header file for interface of ValidationNtupleTools
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Sebastian.Fleischmann@cern.ch
///////////////////////////////////////////////////////////////////

#ifndef TRK_IVALIDATIONNTUPLETOOL_H
#define TRK_IVALIDATIONNTUPLETOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "TrkEventPrimitives/FitQualityOnSurface.h"
#include "TrkParameters/TrackParameters.h"
//#include "GeneratorObjects/HepMcParticleLink.h"
#include <vector>

class TrackTruth;
class TTree;
class HepMcParticleLink;

namespace Trk {
static const InterfaceID IID_IValidationNtupleTool("IValidationNtupleTool",1,0);

class TrackParticleBase;
class MeasurementBase;     //!< measurement base
class Track;
class TrackStateOnSurface;


/** @class IValidationNtupleTool
    provides the interface for validation tools which write special information
    about generated tracks into ntuples.
*/

class IValidationNtupleTool : virtual public IAlgTool {
public:
     /**Interface ID, declared here, and defined below*/
    static const InterfaceID& interfaceID();

    /** @brief add branches to the tree
      Should be called once (per track collection and tree) dunring the initialisation phase by the calling algorithm
      (usually Trk::TrackValidationNtupleWriter) */
    virtual StatusCode addNtupleItems (
        TTree* tree ) = 0;

    /** fill AND write ntuple data of a given track */
    virtual StatusCode writeTrackData (
        const Trk::Track&,
        const int iterationIndex,
        const unsigned int fitStatCode = 0 ) = 0;

     /** fill AND write ntuple data of a given track particle */
    virtual StatusCode writeTrackParticleData (
        const Trk::TrackParticleBase& ) = 0;
    
    /** fill ntuple data of a given track without writing the record.
    - if this method is called twice without writing the ntuple inbetween the first data will be lost! */
    virtual StatusCode fillTrackData (
        const Trk::Track&,
        const int iterationIndex,
        const unsigned int fitStatCode = 0 ) = 0;

    /** fill ntuple data of a given track particle without writing the record.
    - if this method is called twice without writing the ntuple inbetween the first data will be lost! */
    virtual StatusCode fillTrackParticleData (
        const Trk::TrackParticleBase&) = 0;
        
    /** fill ntuple data of a given track without writing the record.
    - if this method is called twice without writing the ntuple inbetween the first data will be lost! */
    virtual StatusCode fillTrackParameter (
        const Trk::TrackParameters*,
        const int iterationIndex ) = 0;
    
    /** fill ntuple data of given measurement and track parameters without writing the record*/
    virtual StatusCode fillMeasurementData (
        const Trk::MeasurementBase*,
        const Trk::TrackParameters* ) = 0;
        
    /** fill ntuple data of an outlier measurement (without writing the
        record yet). The TrackParameters and FQoS are optional, i.e.
        NULL pointers should be handled by the implementation */
    virtual StatusCode fillOutlierData (
        const Trk::MeasurementBase*,
        const Trk::TrackParameters*,
        const Trk::FitQualityOnSurface* ) = 0;

    /** fill ntuple data of holes on track without writing the record */
    virtual StatusCode fillHoleData (
        const Trk::TrackStateOnSurface&) = 0;

    virtual StatusCode fillTrackTruthData ( const TrackParameters*&,
                                            const TrackTruth&, 
                                            const int truthIndex = -1 ) = 0;
    
    /** write the filled data into the ntuple */
    virtual StatusCode writeRecord( TTree* tree ) = 0;
};

inline const InterfaceID& Trk::IValidationNtupleTool::interfaceID() {
    return IID_IValidationNtupleTool;
}

} // end of namespace

#endif // TRK_IVALIDATIONNTUPLETOOL_H
