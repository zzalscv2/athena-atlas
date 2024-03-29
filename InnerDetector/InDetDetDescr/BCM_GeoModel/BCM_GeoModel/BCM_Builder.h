/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BCMBCMBUILDER_H
#define BCMBCMBUILDER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GeoModelInterfaces/IGeoSubDetTool.h"
#include "AthenaKernel/IOVSvcDefs.h"
#include <vector>

class AtlasDetectorID;
class Identifier;
class GeoVPhysVol;
class StoreGateSvc;

namespace InDetDD
{

  /** @class BCMBuilder

  Beam Condition Monitor Builder
      @author  Bostjan Macek <bostjan.macek@cern.ch>
  */

  class BCM_Builder : public extends<AthAlgTool, IGeoSubDetTool>
    {
    public:
      BCM_Builder(const std::string&,const std::string&,const IInterface*);

       /** default destructor */
      virtual ~BCM_Builder () = default;

       /** standard Athena-Algorithm method */
      virtual StatusCode initialize();
       /** standard Athena-Algorithm method */
      virtual StatusCode finalize  ();
       /** build the BCM geometry */
      //virtual void build(GeoFullPhysVol*& Phys, StoredMaterialManager*& mat_mgr);
      virtual StatusCode build(GeoVPhysVol* parent);

      /** For alignment */
      // Register callback function on ConDB object
      virtual StatusCode registerCallback( StoreGateSvc* detStore );

      // Callback function itself
      virtual StatusCode align(IOVSVC_CALLBACK_ARGS);


    private:

      /** member variables for algorithm properties: */
      // int/double/bool  m_propertyName;
      std::vector<double> m_module0;
      std::vector<double> m_moduleI;
      std::vector<double> m_moduleII;
      std::vector<double> m_moduleIII;
      std::vector<double> m_moduleIV;
      std::vector<double> m_moduleV;
      std::vector<double> m_moduleVI;
      std::vector<double> m_moduleVII;
      unsigned int m_moduleon;
      bool m_bcmon;
      bool m_BDparameters;
    };
} // end of namespace

#endif
