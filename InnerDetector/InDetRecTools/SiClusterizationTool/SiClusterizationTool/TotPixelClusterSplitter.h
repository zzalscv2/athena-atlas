/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TotPixelClusterSplitter.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef INDETRECTOOLS_TOTPIXELCLUSTERSPLITTER_H
#define INDETRECTOOLS_TOTPIXELCLUSTERSPLITTER_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "InDetRecToolInterfaces/IPixelClusterSplitter.h"
#include "InDetPrepRawData/PixelClusterParts.h"
#include "InDetPrepRawData/PixelClusterSplitProb.h"
#include "InDetIdentifier/PixelID.h"
#include "PixelConditionsData/PixelChargeCalibCondData.h"
#include "PixelReadoutGeometry/IPixelReadoutManager.h"
#include "StoreGate/ReadCondHandleKey.h"

template <class T> class ServiceHandle;

namespace InDet
{
  class PixelCluster;
    
  /** @class TotPixelClusterSplitter


      @author Kurt Barry, Tim Nelsen, Andreas Salzburger
  */
  class TotPixelClusterSplitter final : public extends<AthAlgTool, IPixelClusterSplitter>
  {
    public :
      /** Constructor*/
      TotPixelClusterSplitter(const std::string & type,
        const std::string & name,
        const IInterface * parent);
          
      /** Destructor*/
      ~TotPixelClusterSplitter() = default;
          
      /** AthAlgTool interface methods */
      virtual StatusCode initialize() override;            
      virtual StatusCode finalize() override;

      /** take one, give zero or many */
      virtual std::vector<InDet::PixelClusterParts> splitCluster(
          const InDet::PixelCluster& OrigCluster) const override;

      /** take one, give zero or many - with split probability object */
      virtual std::vector<InDet::PixelClusterParts> splitCluster(
          const InDet::PixelCluster& OrigCluster,
          const InDet::PixelClusterSplitProb& spo) const override;

      /** Set the lower and upper bounds for the number of pixels in clusters to be considered. */
      inline void setMinPixels(unsigned int minPix);
      inline void setMaxPixels(unsigned int maxPix);

    private:
      /** Determine a pixel's type. Even numbers have normal eta-pitch, odd numbers have long eta pitch.

          Arguments:
            phiIdx : sensor phi index, in the range [0, 327]
            etaIdx : sensor eta index, in the range [0, 17]

          Return Values:
           -1 : one or both indicies are out-of-range
            0 : short normal pixel
            1 : long normal pixel
            2 : short ganged pixel
            3 : long ganged pixel
            4 : short inter-ganged pixel
            5 : long inter-ganged pixel
      */
      static int pixelType(const int PhiIdx, const int EtaIdx) ;

      enum SplitType { PhiSplit = 0, EtaSplit = 1, NoSplit = 2 };

      ServiceHandle<InDetDD::IPixelReadoutManager> m_pixelReadout
      {this, "PixelReadoutManager", "PixelReadoutManager", "Pixel readout manager" };

      SG::ReadCondHandleKey<PixelChargeCalibCondData> m_chargeDataKey
         {this, "PixelChargeCalibCondData", "PixelChargeCalibCondData", "Pixel charge calibration data"};

      /** Minimum number of pixels in cluster to consider splitting. */
      unsigned int m_minPixels;

      /** Maximum size of cluster for which splitting will be attempted. */
      unsigned int m_maxPixels;

  };

  inline void TotPixelClusterSplitter::setMinPixels(unsigned int minPix)
  {
    m_minPixels = minPix;
    return;
  }

  inline void TotPixelClusterSplitter::setMaxPixels(unsigned int maxPix)
  {
    m_maxPixels = maxPix;
    return;
  }

}

#endif
