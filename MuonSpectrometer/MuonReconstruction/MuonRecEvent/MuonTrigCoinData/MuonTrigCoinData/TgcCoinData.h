/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TgcCoinData.h
//   Header file for class TgcCoinData
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef MUONTRIGCOINDATA_TGCCOINDATA_H
#define MUONTRIGCOINDATA_TGCCOINDATA_H

#include "AthLinks/tools/IdentContIndex.h"
#include "CxxUtils/CachedUniquePtr.h"
#include "EventPrimitives/EventPrimitives.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "MuonReadoutGeometry/TgcReadoutElement.h"
#include "TrkSurfaces/Surface.h"

#include "GaudiKernel/MsgStream.h"

#include <inttypes.h>
#include <iostream>

class TgcCoinDataContainerCnv;

namespace Muon
{
  class TgcCoinDataContainerCnv_p1;
  class TgcCoinDataContainerCnv_p2;
  class TgcCoinDataContainerCnv_p3;

  class TgcCoinData {

    ///////////////////////////////////////////////////////////////////
    // Public methods:
    ///////////////////////////////////////////////////////////////////
  public:

    friend class ::TgcCoinDataContainerCnv;
    friend class Muon::TgcCoinDataContainerCnv_p1;
    friend class Muon::TgcCoinDataContainerCnv_p2;
    friend class Muon::TgcCoinDataContainerCnv_p3;

    typedef enum {
      TYPE_TRACKLET,
      TYPE_HIPT,
      TYPE_SL,
      TYPE_UNKNOWN,
      TYPE_TRACKLET_EIFI
    } CoinDataType;
    
    /** Public, Copy, operator=, constructor*/
    TgcCoinData();
    TgcCoinData(const TgcCoinData &);
    TgcCoinData &operator=(const TgcCoinData &);

    /** Overload of << operator for MsgStream for debug output*/
    friend MsgStream& operator << (MsgStream& sl, const TgcCoinData& coin);

    /** Overload of << operator for std::ostream for debug output*/
    friend std::ostream& operator << (std::ostream& sl, const TgcCoinData& coin);

    /// Constructor: 
	
    //for HIPT and TRACKLET
    TgcCoinData( const Identifier& channelIdIn,
		 const Identifier& channelIdOut,
		 const IdentifierHash &collectionIdHash,
		 const MuonGM::TgcReadoutElement* detElIn,
		 const MuonGM::TgcReadoutElement* detElOut,
		 const CoinDataType type,
		 const bool isAside,
		 const int phi,
		 const bool isInner,
		 const bool isForward,
		 const bool isStrip,
		 const int trackletId,
		 const Amg::Vector2D *posIn,
		 const Amg::Vector2D *posOut,
		 const double widthIn,
		 const double widthOut,
		 const int delta, 
		 const int sub,
                 const int inner,
                 const int roi = 0,
                 const int pt = 0,
                 const int trackletIdStrip = 0,
                 const Amg::MatrixX* errMat = nullptr,
                 const bool isPositiveDeltaR = false,
                 const bool veto = false);

    //for TYPE_TRACKLET_EIFI 
    TgcCoinData( const Identifier& channelIdIn,
		 const IdentifierHash &collectionIdHash,
		 const MuonGM::TgcReadoutElement* detElIn,
		 const CoinDataType type,
		 const bool isAside,
		 const int phi,
		 const bool isForward,
		 const bool isStrip,
		 const Amg::Vector2D *posIn,
		 const double widthIn,
		 const int sub);

    //for SL
    TgcCoinData( const Identifier& channelIdOut,
		 const IdentifierHash &collectionIdHash, 
		 const MuonGM::TgcReadoutElement* detElOut,
		 const CoinDataType type,
		 const bool isAside,
		 const int phi,
		 const bool isForward,
		 const int trackletId,
		 const int trackletIdStrip,
		 const Amg::Vector2D *posOut,
		 const Amg::MatrixX *errMat,
		 const int roi,
		 const int pt, 
                 const bool veto,
		 const bool isPositiveDeltaR);

    /// Destructor:
      ~TgcCoinData();
      
      /** return OfflineID of Outer Hit */
      Identifier identify() const;

      /** return the identifierHash of Outer Hit*/
      IdentifierHash identifyHash() const;

      /** set the OfflineID of Inner and Outer Hits*/
      void setIdentifier(Identifier in, Identifier out);


      void setHashAndIndex(unsigned short collHash, unsigned short objIndex);
      void setHashAndIndex(unsigned int indexAndHash);
      const IdentContIndex& getHashAndIndex() const;

      /** return offlineID on TGC1(HIPT) or TGC2(TRACKLET) */
      Identifier channelIdIn() const;

      /** return offlineID on track on TGC3 */
      Identifier channelIdOut() const;

      /** return the coincidence type (HIPT, LPT, SL) */      
      CoinDataType type() const;

      /** Aside or Cside */
      bool isAside() const;

      /** return phi number of trigger sector */
      int phi() const;

      /** Inner layers or BW */
      bool isInner() const;

      /** Forward region or Endcap region */
      bool isForward() const;

      /** Strip hit or Wire hit? */
      bool isStrip() const;

      /** return trackletId (Identifier for each track) */
      int trackletId() const;

      /** return trackletId for strip (Identifier for each track) */
      int trackletIdStrip() const;

      /** return Local position of track on TGC1(HIPT) or TGC2(TRACKLET) */
      const Amg::Vector2D &posIn() const;
      const Amg::Vector2D *posInPtr() const;

      /** return Local position of track on TGC3 */
      const Amg::Vector2D &posOut() const;
      const Amg::Vector2D *posOutPtr() const;

      /** Test to see if the error matrix is present. */
      bool hasErrMat() const;

      /** return 2d width of ROI for SL: errMat[0][0]==(width for R), errMat[1][1]==(width for phi) */
      const Amg::MatrixX &errMat() const;

      /** return region width of hit on TGC1 */
      double widthIn() const;

      /** return region width of hit on TGC2(TRACKLET) or TGC3(HIPT) */
      double widthOut() const;

      /** return the deviation from Pt=oo track */
      int delta() const;

      /** return ROI number */
      int roi() const;

      /** return pt threshold value */
      int pt() const;

      /** return veto bit */
      bool veto() const;

      /** return subMatrix of Tracklet or hsub of HiPt */
      int sub() const;

      /** return InnerCoincidecne trigger bits */
      int inner() const;
      
      /** return isPositiveDeltaR (isMuplus) of SL */
      bool isPositiveDeltaR() const;

      /** return TgcReadoutElement of track on TGC1(HIPT) or TGC2(TRACKLET) */
      const MuonGM::TgcReadoutElement* detectorElementIn() const;

      /** return TgcReadoutElement of track on TGC3 */
      const MuonGM::TgcReadoutElement* detectorElementOut() const;

      /** return global position of track on TGC1(HIPT) or TGC2(TRACKLET) */
      const Amg::Vector3D& globalposIn() const;

      /** return global position of track on TGC3 */
      const Amg::Vector3D& globalposOut() const;

      /** Bit info in int inner for the NSW inner-coincidence */
      static constexpr uint32_t INNER_NSW_R_BITSHIFT = 0;
      static constexpr uint32_t INNER_NSW_R_BIT = 0xFF;
      static constexpr uint32_t INNER_NSW_PHI_BITSHIFT = 8;
      static constexpr uint32_t INNER_NSW_PHI_BIT = 0x3F;
      static constexpr uint32_t INNER_NSW_DTHETA_BITSHIFT = 14;
      static constexpr uint32_t INNER_NSW_DTHETA_BIT = 0x1F;
      static constexpr uint32_t INNER_NSW_PHIRES_BITSHIFT = 19;
      static constexpr uint32_t INNER_NSW_PHIRES_BIT = 0x1;
      static constexpr uint32_t INNER_NSW_LOWRES_BITSHIFT = 20;
      static constexpr uint32_t INNER_NSW_LOWRES_BIT = 0x1;
      static constexpr uint32_t INNER_NSW_ID_BITSHIFT = 21;
      static constexpr uint32_t INNER_NSW_ID_BIT = 0xF;
      static constexpr uint32_t INNER_NSW_BCID_BITSHIFT = 25;
      static constexpr uint32_t INNER_NSW_BCID_BIT = 0xF;
      static constexpr uint32_t INNER_NSW_INPUT_BITSHIFT = 29;
      static constexpr uint32_t INNER_NSW_INPUT_BIT = 0x7;

      /** Bit info in int inner for the RPC inner-coincidence */
      static constexpr uint32_t INNER_RPC_ETA_BITSHIFT = 0;
      static constexpr uint32_t INNER_RPC_ETA_BIT = 0x3F;
      static constexpr uint32_t INNER_RPC_PHI_BITSHIFT = 6;
      static constexpr uint32_t INNER_RPC_PHI_BIT = 0x3F;
      static constexpr uint32_t INNER_RPC_DETA_BITSHIFT = 12;
      static constexpr uint32_t INNER_RPC_DETA_BIT = 0x7;
      static constexpr uint32_t INNER_RPC_DPHI_BITSHIFT = 15;
      static constexpr uint32_t INNER_RPC_DPHI_BIT = 0x7;
      static constexpr uint32_t INNER_RPC_FLAG_BITSHIFT = 18;
      static constexpr uint32_t INNER_RPC_FLAG_BIT = 0x3;
      static constexpr uint32_t INNER_RPC_BCID_BITSHIFT = 20;
      static constexpr uint32_t INNER_RPC_BCID_BIT = 0xF;

      /** Bit info in int inner for the EI/FI inner-coincidence */
      static constexpr uint32_t INNER_EIFI_EI_BITSHIFT = 0;
      static constexpr uint32_t INNER_EIFI_EI_BIT = 0xFF;
      static constexpr uint32_t INNER_EIFI_FI_BITSHIFT = 8;
      static constexpr uint32_t INNER_EIFI_FI_BIT = 0xFF;
      static constexpr uint32_t INNER_EIFI_CID_BITSHIFT = 16;
      static constexpr uint32_t INNER_EIFI_CID_BIT = 0x7;

      /** Bit info in int inner for the Tile inner-coincidence */
      static constexpr uint32_t INNER_TILE_MODULE_BITSHIFT = 0;
      static constexpr uint32_t INNER_TILE_MODULE_BIT = 0xFFF;
      static constexpr uint32_t INNER_TILE_BCID_BITSHIFT = 12;
      static constexpr uint32_t INNER_TILE_BCID_BIT = 0xF;

  private:

      Identifier m_channelIdIn;
      Identifier m_channelIdOut;
      IdentifierHash m_collectionIdHash;

      IdentContIndex m_indexAndHash;

      const MuonGM::TgcReadoutElement* m_detElIn;
      const MuonGM::TgcReadoutElement* m_detElOut;
      
      CoinDataType m_type;
      bool m_isAside;
      int m_phi;
      bool m_isInner;
      bool m_isForward;
      bool m_isStrip;
      int m_trackletId;
      int m_trackletIdStrip;
      const Amg::Vector2D *m_posIn;
      const Amg::Vector2D *m_posOut;
      const Amg::MatrixX *m_errMat;
      double m_widthIn;
      double m_widthOut;

      int m_delta;
      int m_roi;
      int m_pt;
      bool m_veto;

      int m_sub;
      int m_inner;
      bool m_isPositiveDeltaR;

      CxxUtils::CachedUniquePtr<const Amg::Vector3D> m_globalposIn;
      CxxUtils::CachedUniquePtr<const Amg::Vector3D> m_globalposOut;
  };
  ////////////////////////////////////////////////
  // inline methods:
  ////////////////////////////////////////////////

  inline Identifier TgcCoinData::identify() const { 
    if(m_type==TYPE_TRACKLET_EIFI) return m_channelIdIn;
    return m_channelIdOut; 
  }

  inline IdentifierHash TgcCoinData::identifyHash() const { return m_collectionIdHash; }

  inline void TgcCoinData::setIdentifier(Identifier in, Identifier out)
    {
      m_channelIdIn = in; m_channelIdOut = out;
    }

  inline void TgcCoinData::setHashAndIndex(unsigned short collHash, unsigned short objIndex)
    {
      m_indexAndHash.setCollHash(collHash);
      m_indexAndHash.setObjIndex(objIndex);
    }

  inline void TgcCoinData::setHashAndIndex(unsigned int indexAndHash)
    {
      m_indexAndHash.setHashAndIndex(indexAndHash);
    }

  inline const IdentContIndex& TgcCoinData::getHashAndIndex() const
    {
      return m_indexAndHash;
    }

  inline Identifier TgcCoinData::channelIdIn() const{ return m_channelIdIn; }

  inline Identifier TgcCoinData::channelIdOut() const{ return m_channelIdOut; }

  inline TgcCoinData::CoinDataType TgcCoinData::type() const{ return m_type; }

  inline bool TgcCoinData::isAside() const{ return m_isAside; }

  inline int TgcCoinData::phi() const{ return m_phi; }

  inline bool TgcCoinData::isInner() const{ return m_isInner; }

  inline bool TgcCoinData::isForward() const{ return m_isForward; }

  inline bool TgcCoinData::isStrip() const{ return m_isStrip; }

  inline int TgcCoinData::trackletId() const{ return m_trackletId; }

  inline int TgcCoinData::trackletIdStrip() const{ return m_trackletIdStrip; }

  inline const Amg::Vector2D& TgcCoinData::posIn() const{ return *m_posIn; }
  inline const Amg::Vector2D* TgcCoinData::posInPtr() const{ return m_posIn; }

  inline const Amg::Vector2D& TgcCoinData::posOut() const{ return *m_posOut; }
  inline const Amg::Vector2D* TgcCoinData::posOutPtr() const{ return m_posOut; }

  inline bool TgcCoinData::hasErrMat() const{ return m_errMat != nullptr; }
  
  inline const Amg::MatrixX& TgcCoinData::errMat() const{ return *m_errMat; }

  inline double TgcCoinData::widthIn() const{ return m_widthIn; }

  inline double TgcCoinData::widthOut() const{ return m_widthOut; }

  inline int TgcCoinData::delta() const{ return m_delta; }
  
  inline int TgcCoinData::roi() const{ return m_roi; }

  inline int TgcCoinData::pt() const{ return m_pt; }

  inline bool TgcCoinData::veto() const{ return m_veto; }

  inline int TgcCoinData::sub() const{ return m_sub; }

  inline int TgcCoinData::inner() const{ return m_inner; }

  inline bool TgcCoinData::isPositiveDeltaR() const{ return m_isPositiveDeltaR; }

  inline const MuonGM::TgcReadoutElement* TgcCoinData::detectorElementIn() const
    {
      return m_detElIn;
    }

  inline const MuonGM::TgcReadoutElement* TgcCoinData::detectorElementOut() const
    {
      return m_detElOut;
    }
  
  inline const Amg::Vector3D& TgcCoinData::globalposIn() const
    {
      if(not m_globalposIn) {
        if (m_detElIn) {
          m_globalposIn.set(std::make_unique<const Amg::Vector3D>(
            m_detElIn->surface(m_channelIdIn)
              .Trk::Surface::localToGlobal(*m_posIn)));
        } else {
          m_globalposIn.set(std::make_unique<const Amg::Vector3D>(0., 0., 0.));
        }
      }
      return *m_globalposIn;
    }
      
  inline const Amg::Vector3D& TgcCoinData::globalposOut() const
    {
      if(not m_globalposOut) {
        if (m_detElOut && m_posOut) {
          m_globalposOut.set(std::make_unique<const Amg::Vector3D>(
            m_detElOut->surface(m_channelIdOut)
              .Trk::Surface::localToGlobal(*m_posOut)));
        } else {
          m_globalposOut.set(std::make_unique<const Amg::Vector3D>(0., 0., 0.));
        }
      }
      return *m_globalposOut;
    }
}

#endif // MUONTRIGCOINDATA_TGCCOINDATA_H
