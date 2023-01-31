/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKINGGEOMETRY_BEAMPIPEBUILDERIMPL_H
#define INDETTRACKINGGEOMETRY_BEAMPIPEBUILDERIMPL_H

// Athena
#include "AthenaBaseComps/AthAlgTool.h"
// Gaudi
#include "GaudiKernel/SystemOfUnits.h"
// STL
#include <vector>

namespace Trk {
  class CylinderLayer;
  class DiscLayer;
  class PlaneLayer;
}

class BeamPipeDetectorManager;

namespace InDet {

  /** @class BeamPipeBuilderImpl
      Base LayerBuilder for the BeamPipe,
      can be configured through jobOptions:
      - radius
      - halflength
      - thickness
      - MaterialProperties

      later on the slight shift/rotation of the BeamPipe can be implemented
      - make a binding to the database afterwards

      Used by derived classes BeamPipeBuilder and BeamPipeBuilderCond.
      @author Andreas.Salzburger@cern.ch
  */
  class BeamPipeBuilderImpl : public AthAlgTool {


  public:
    /** Destructor */
    virtual ~BeamPipeBuilderImpl() = default;

    /** AlgTool initialize method */
    virtual StatusCode initialize() override;

  protected:
    /** Constructor */
    BeamPipeBuilderImpl(const std::string& t, const std::string& n, const IInterface* p) :
      AthAlgTool(t,n,p) { }

    std::unique_ptr<const std::vector<Trk::CylinderLayer*> > cylindricalLayersImpl() const;
    BooleanProperty m_beamPipeFromDb{this, "BeamPipeFromGeoModel", true};    //!< steer beam pipe parameters from DataBase
    const BeamPipeDetectorManager *m_beamPipeMgr{};       //!< the beam pipe manager
    StringProperty m_beamPipeMgrName{this, "BeamPipeManager", "BeamPipe"};   //!< the name of the beam pipe manager to be configured
    DoubleProperty m_beamPipeEnvelope{this, "BeamPipeEnvelope", 1.*Gaudi::Units::mm};  //!< radial envelope when taking the Top volume radius

    DoubleProperty m_beamPipeOffsetX{this, "BeamPipeOffsetX", 0.*Gaudi::Units::mm};    //!< beam pipe offset in x
    DoubleProperty m_beamPipeOffsetY{this, "BeamPipeOffsetY", 0.*Gaudi::Units::mm};    //!< beam pipe offset in y
    DoubleProperty m_beamPipeRadius{this, "BeamPipeRadius", 33.1*Gaudi::Units::mm};     //!< radius of the beam pipe
    DoubleProperty m_beamPipeHalflength{this, "BeamPipeHalflength", 2.7*Gaudi::Units::m}; //!< halflength of the beampipe
    DoubleProperty m_beamPipeThickness{this, "BeamPipeThickness", 1.*Gaudi::Units::mm};  //!< thickness of the beam pipe
    DoubleProperty m_beamPipeX0{this, "BeamPipeX0", 352.8*Gaudi::Units::mm};         //!< X0 of the beam pipe
    double m_beamPipeL0{407.*Gaudi::Units::mm};         //!< X0 of the beam pipe
    //DoubleProperty m_beamPipedEdX{this, , 0.2945*Gaudi::Units::MeV/Gaudi::Units::mm};       //!< dEdX of the beam pipe
    DoubleProperty m_beamPipeA{this, "BeamPipeAverageA", 9.012};          //!< averageA of the beam pipe
    DoubleProperty m_beamPipeZ{this, "BeamPipeAverageZ", 4.};          //!< averageZ of the beam pipe
    DoubleProperty m_beamPipeRho{this, "BeamPipeAverageRho", 1.848e-3};        //!< averageRho of the beam pipe

    UnsignedIntegerProperty m_beamPipeBinsZ{this, "BeamPipeMaterialBinsZ", 1};      //!< number of bins in the beam pipe

    StringProperty m_identification{this, "Identification", "BeamPipe"};     //!< string identification


  };

} // end of namespace


#endif // INDETTRACKINGGEOMETRY_BEAMPIPEBUILDERIMPL_H
