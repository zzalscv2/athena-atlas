/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JIVEXML_TRUTHTRACKRETRIEVER_H
#define JIVEXML_TRUTHTRACKRETRIEVER_H

#include "JiveXML/IDataRetriever.h"
#include "GaudiKernel/IPartPropSvc.h"

#include "AthenaBaseComps/AthAlgTool.h"

#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

//Forward declarations
namespace Trk{ class IExtrapolator; }
namespace HepPDT{ class ParticleDataTable; }

namespace JiveXML{

  /**
   * @class TruthTrackRetriever
   * @brief Retrieves the @c McEventCollection or the @c TrackRecordCollection in simulated cosmics
   *
   *  - @b Properties
   *    - <em> StoreGateKey </em><tt> = TruthEvent </tt>: @copydoc m_McEvtCollName
   *    - <em> UnstableMinPtCut </em><tt> = 100*MeV </tt> @copydoc m_MinPtCut
   *    - <em> UnstableMinRhoCut </em><tt> = 40*mm </tt> @copydoc m_MinRhoCut
   *
   *  - @b Retrieved @b Data
   *    - <em>code</em> : the PDG ID of the particle
   *    - <em>id</em> : the particle barcode
   *    - <em>pt</em> : transverse momentum
   *    - <em>eta, phi</em> : @f$\eta@f$ and @f$\phi@f$ of the momentum vector
   *    - <em>rhoVertex,phiVertex,zVertex</em> : position of the production vertex in @f$\rho@f$, @f$\phi@f$ and @f$z@f$
   *    - <em>rhoEndVertex,phiEndVertex,zEndVertex</em> : position of the end-vertex in @f$\rho@f$, @f$\phi@f$ and @f$z@f$
   *    .
   */

  class TruthTrackRetriever : public extends<AthAlgTool, IDataRetriever> {

  public:
  
    /// Standard Constructor
    TruthTrackRetriever(const std::string& type, const std::string& name, const IInterface* parent);
  
    /// Retrieve all the data
    virtual StatusCode retrieve(ToolHandle<IFormatTool> &FormatTool);

    /// Return the name of the data type
    virtual std::string dataTypeName() const { return typeName; };

    ///Default AthAlgTool methods
    StatusCode initialize();

  private:
    ///The data type that is generated by this retriever
    const std::string typeName;

    /// Storegate key for the McEventCollection (different between RDO/ESD and AOD)
    std::string m_McEvtCollName;
    /// Minimum pT for a particle to get accepted
    double m_MinPtCut;
    /// Minium radius of the end-vertex for the particle to get accepted
    double m_MinRhoCut;
  };

}
#endif
