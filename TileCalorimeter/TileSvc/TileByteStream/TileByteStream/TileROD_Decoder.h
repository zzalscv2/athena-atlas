/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILEBYTESTREAM_TILEROD_DECODER_H
#define TILEBYTESTREAM_TILEROD_DECODER_H


// Tile includes
#include "TileByteStream/TileOFC.h"
#include "TileByteStream/TileRawChannel2Bytes.h" 
#include "TileByteStream/TileRawChannel2Bytes2.h"
#include "TileByteStream/TileRawChannel2Bytes4.h"
#include "TileByteStream/TileRawChannel2Bytes5.h"
#include "TileByteStream/TileDigits2Bytes.h"
#include "TileCalibBlobObjs/TileCalibUtils.h"

#include "TileEvent/TileBeamElem.h"
#include "TileEvent/TileBeamElemContainer.h"
#include "TileEvent/TileBeamElemCollection.h"
#include "TileEvent/TileDigits.h"
#include "TileEvent/TileDigitsContainer.h"
#include "TileEvent/TileDigitsCollection.h"
#include "TileEvent/TileRawChannel.h"
#include "TileEvent/TileFastRawChannel.h"
#include "TileEvent/TileRawChannelContainer.h"
#include "TileEvent/TileRawChannelCollection.h"
#include "TileEvent/TileLaserObject.h"                                          
#include "TileEvent/TileCell.h"
#include "TileEvent/TileCellIDC.h"
#include "TileEvent/TileCellCollection.h"
#include "TileEvent/TileL2.h"
#include "TileEvent/TileContainer.h"
#include "TileConditions/TileCondToolOfcCool.h"
#include "TileConditions/TileCondToolTiming.h"
#include "TileConditions/TileCondToolEmscale.h"
#include "TileConditions/ITileBadChanTool.h"
#include "TileL2Algs/TileL2Builder.h"
#include "TileByteStream/TileHid2RESrcID.h"

// Atlas includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "ByteStreamData/RawEvent.h"
#include "eformat/ROBFragment.h"

// Gaudi includes
#include "GaudiKernel/ToolHandle.h"

#include "CxxUtils/checker_macros.h"


#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <cassert>
#include <atomic>
#include <mutex>
#include <stdint.h>


class TileRawChannelBuilder;
class TileCellBuilder;
class TileHid2RESrcID;

namespace TileROD_Helper {


// Helper to find the Tile container type corresponding to a collection.
template <class COLLECTION>
struct ContainerForCollection {};

template <>
struct ContainerForCollection<TileBeamElemCollection>
{
  typedef TileBeamElemContainer type;
};

template <>
struct ContainerForCollection<TileDigitsCollection>
{
  typedef TileDigitsContainer type;
};

template <>
struct ContainerForCollection<TileRawChannelCollection>
{
  typedef TileRawChannelContainer type;
};


} // namespace TileROD_Helper


/**
 *
 * @class TileROD_Decoder
 * @brief Decodes the different TileCal ROD subfragment types in
 bytestream data and fills TileDigitsContainer, TileRawChannelContainer
 or TileL2Container.
 
 * @author A. Solodkov
 * @version  0-0-1 , Oct 17, 2002
 *

 * Modified, Dec 20, 2002
 Create either TileRawChannel or TileCell.
 
 * Modified, Jan 15, 2003
 Moved Encoding part to TileROD_Encoder. 
 
 * Modified, Sep 7, 2003
 Decoding of Testbeam data added
 */

class TileROD_Decoder: public AthAlgTool {

  public:

    struct D0CellsHLT {
      D0CellsHLT() { clear(); }
      void clear();

      bool m_D0Existneg[64];
      bool m_D0Existpos[64];
      bool m_D0Maskneg[64];
      bool m_D0Maskpos[64];
      TileFastRawChannel m_D0chanpos[64];
      TileFastRawChannel m_D0channeg[64];
      TileCell* m_cells[64];
    };

    /** constructor
     */
    TileROD_Decoder(const std::string& type, const std::string& name, const IInterface* parent);

    /** destructor
     */
    virtual ~TileROD_Decoder();

    /** AlgTool InterfaceID
     */
    static const InterfaceID& interfaceID();

    virtual StatusCode initialize();
    virtual StatusCode finalize();

    /** convert ROD Data words into either TileCell or TileRawChannel.
     */
    typedef eformat::ROBFragment<const uint32_t*> ROBData;

    template<class COLLECTION>
    /** This method calls the unpacking methods to decode the ROD data and fills
     the TileDigitsContainer and/or the TileRawChannelContainer
     */
    void fillCollection(const ROBData * rob,
                        COLLECTION& v,
                        typename TileROD_Helper::ContainerForCollection<COLLECTION>::type* container = nullptr) const;
    uint32_t fillCollectionHLT(const ROBData * rob,
                               TileCellCollection & v,
                               D0CellsHLT & d0cells,
                               TileCellCollection * MBTS) const;
    void fillCollectionL2(const ROBData * rob, TileL2Container & v) const;
    void fillCollectionL2ROS(const ROBData * rob, TileL2Container & v) const;
    void fillTileLaserObj(const ROBData * rob, TileLaserObject & v) const;
    void fillCollection_TileMuRcv_RawChannel(const ROBData* rob, TileRawChannelCollection& v) const;
    void fillCollection_TileMuRcv_Digi(const ROBData* rob, TileDigitsCollection& v) const;
    void fillContainer_TileMuRcv_Decision(const ROBData* rob, TileMuonReceiverContainer& v) const;
    void fillCollection_FELIX_Digi(const ROBData* rob,  TileDigitsCollection& v) const;

    void setLaserVersion(TileLaserObject & laserObject) const {
      laserObject.setVersion((m_runPeriod >= 2) ? -2 : -1);
    }

    void loadRw2Cell(const int section, const std::vector<int>& vec) {
      //    std::cout << vec.size() << std::endl;
      for (unsigned int i = 0; i < vec.size(); ++i) {
        //	std::cout << vec[i] << std::endl;
        m_Rw2Cell[section].push_back(vec[i]);
      }
    }
    void loadMBTS(std::map<unsigned int, unsigned int>& mapMBTS, int MBTS_channel);
    inline const TileHWID* getTileHWID() const { return m_tileHWID;}
    inline const TileFragHash* hashFunc() const { return &m_hashFunc; }

    StatusCode convert(const RawEvent* re, TileL2Container* L2Cnt) const;
    StatusCode convertLaser(const RawEvent* re, TileLaserObject* TileLaserObj) const;
    StatusCode convertTMDBDecision(const RawEvent* re, TileMuonReceiverContainer* tileMuRcv) const; 

    void mergeD0cellsHLT (const D0CellsHLT& d0cells, TileCellCollection&) const;

    void loadRw2Pmt(const int section, const std::vector<int>& vec) {
      for (unsigned int i = 0; i < vec.size(); ++i) {
        //	std::cout << vec[i] << std::endl;
        m_Rw2Pmt[section].push_back(vec[i]);
      }
    }

    // Error reporting
    void printErrorCounter(bool printIfNoError);
    int getErrorCounter();

    void printWarningCounter(bool printIfNoWarning);
    int getWarningCounter();

    const TileHid2RESrcID * getHid2reHLT() {
      return m_hid2reHLT;
    }

    const TileHid2RESrcID * getHid2re() {
      std::lock_guard<std::mutex> lock (m_HidMutex);
      if (!m_hid2re) initHid2re();
      return m_hid2re;
    }

    void setUseFrag0 (bool f) { m_useFrag0 = f; }
    void setUseFrag1 (bool f) { m_useFrag1 = f; }
    void setUseFrag4 (bool f) { m_useFrag4 = f; }
    void setUseFrag5Raw (bool f) { m_useFrag5Raw = f; }
    void setUseFrag5Reco (bool f) { m_useFrag5Reco = f; }

  enum TileFragStatus {ALL_OK=0, CRC_ERR=1, ALL_FF=0x10, ALL_00=0x20, NO_FRAG=0x40, NO_ROB=0x80};

  private:
    friend class TileHid2RESrcID;

    typedef std::vector<TileCell *> pCellVec;
    typedef std::vector<TileDigits *> pDigiVec;
    typedef std::vector<TileBeamElem *> pBeamVec;
    typedef std::vector<TileRawChannel *> pRwChVec;
    typedef std::vector<TileFastRawChannel> FRwChVec;

    // Save ROD metadata (size frag id,Chip CRC+Headers, Trailer DMUMask+CRC)
    // First element is vector with size,frag,BCID for whole fragment,
    // second is vector with all trailer words
    // third is 16 chip headers and forth is 16 chip CRC
    // In case of calib mode, there are 2 additional vectors, one header and one CRC
    typedef std::vector<std::vector<uint32_t> > DigitsMetaData_t;

    typedef std::vector<std::vector<uint32_t> > RawChannelMetaData_t;

    /** getOFW returns Optimal Filtering Weights for Frag5 decoder loaded from COOL
     for correspondent units. Coefficients automatically stored in memory,
     thus for next calls nothing loaded again. Once loaded in memory
     coefficients will be kept there and freed only by destructor method. */
    const uint32_t* getOFW(int fragId, int unit) const;

    /** unpack_frag0 decodes tile subfragment type 0x0. This subfragment contains the
     tile raw digits from the 48 read-out channels of a tilecal module. */
    void unpack_frag0(uint32_t version, uint32_t sizeOverhead,
                      DigitsMetaData_t& digitsMetaData,
                      const uint32_t* p, pDigiVec & pDigits, int fragID, int demoType) const;

    /** unpack_frag1 decodes tile subfragment type 0x1. This subfragment contains the
     tile raw digits ONLY from the existing channels of a tilecal module. <p>
     (not implemented yet). */
    void unpack_frag1(uint32_t version, uint32_t sizeOverhead,
                      DigitsMetaData_t& digitsMetaData,
                      const uint32_t* p, pDigiVec & pDigits, int fragID, int demoType) const;

    /** unpack_frag2 decodes tile subfragment type 0x2. This subfragment contains the
     reconstructed amplitude and phase from the tilecal digitized pulse and a
     quality factor of the reconstruction. <p>
     The amplitude is not calibrated and is encoded in ADC counts. <p>
     The phase is encoded in ns. <p>
     The subfragment type 0x2 contains the reconstructed parameters from the
     48 read-out channels of a tilecal module. */
    void unpack_frag2(uint32_t version, uint32_t sizeOverhead,
                      const uint32_t* p, pRwChVec & pChannel, int fragID, int demoType) const;

    /** unpack_frag3 decodes tile subfragment type 0x3. This subfragment contains the
     reconstructed amplitude and phase from the tilecal digitized pulse and a
     quality factor of the reconstruction. <p>
     The amplitude is not calibrated and it is encoded in ADC counts. <p>
     The phase is encoded in ns. <p>
     The subfragment type 0x3 contains the reconstructed parameters ONLY
     from the existing channels of a tilecal module. */
    void unpack_frag3(uint32_t version, uint32_t sizeOverhead,
                      const uint32_t* p, pRwChVec & pChannel, int fragID, int demoType) const;

    /** unpack_frag4 decodes tile subfragment type 0x4. This subfragment contains the
     reconstructed amplitude and phase from the tilecal digitized pulse and a
     quality factor of the reconstruction. <p>
     The amplitude is calibrated to the online units that can be:
     \li  TileRawChannelUnit::OnlineADCcounts
     \li  TileRawChannelUnit::OnlinePicoCoulombs
     \li  TileRawChannelUnit::OnlineCesiumPicoCoulombs
     \li  TileRawChannelUnit::OnlineMegaElectronVolts<p>
     The phase is encoded in ns. <p>
     The subfragment type 0x4 contains the reconstructed parameters from the
     48 read-out channels of a tilecal module. */
    void unpack_frag4(uint32_t version, uint32_t sizeOverhead, unsigned int unit,
                      RawChannelMetaData_t& rawchannelMetaData,
                      const uint32_t* p, pRwChVec & pChannel, int fragID, int demoType) const;


    /** unpack_frag5 decodes tile subfragment type 0x4. This subfragment contains the
     reconstructed amplitude and phase from the tilecal digitized pulse and a
     quality factor of the reconstruction. <p>
     The amplitude is calibrated to the online units that can be:
     \li  TileRawChannelUnit::OnlineADCcounts
     \li  TileRawChannelUnit::OnlinePicoCoulombs
     \li  TileRawChannelUnit::OnlineCesiumPicoCoulombs
     \li  TileRawChannelUnit::OnlineMegaElectronVolts<p>
     The phase is encoded in ns. <p>
     The subfragment type 0x5 contains the reconstructed parameters and residuals from the
     48 read-out channels of a tilecal module. */
    void unpack_frag5(uint32_t version, uint32_t sizeOverhead, unsigned int unit,
                      DigitsMetaData_t& digitsMetaData,
                      const uint32_t* p, pDigiVec & pDigits, pRwChVec & pChannel, int fragID, int demoType) const;

    /** unpack_frag6 decodes tile subfragment type 0x6. This subfragment contains the
     tile raw digits with 16 samples and 2 gains from the 48 read-out channels of a tilecal module. */
    void unpack_frag6(uint32_t version, uint32_t sizeOverhead,
                      DigitsMetaData_t& digitsMetaData,
                      const uint32_t* p, pDigiVec & pDigits, int fragID, int demoType) const;

    /** unpack_frag3HLT decodes tile subfragment type 0x3 for the high level trigger (HLT).
     This subfragment contains the
     reconstructed amplitude and phase from the tilecal digitized pulse and a
     quality factor of the reconstruction. <p>
     The amplitude is not calibrated and it is encoded in ADC counts. <p>
     The phase is encoded in ns. <p>
     The subfragment type 0x3 contains the reconstructed parameters ONLY
     from the existing channels of a tilecal module. */
    void unpack_frag3HLT(uint32_t version, uint32_t sizeOverhead,
                         const uint32_t* p, FRwChVec & pChannel, int fragID, int demoType) const;

    /** unpack_frag2HLT decodes tile subfragment type 0x2 for the high level trigger (HLT).
     This subfragment contains the
     reconstructed amplitude and phase from the tilecal digitized pulse and a
     quality factor of the reconstruction. <p>
     The amplitude is not calibrated and is encoded in ADC counts. <p>
     The phase is encoded in ns. <p>
     The subfragment type 0x2 contains the reconstructed parameters from the
     48 read-out channels of a tilecal module. */
    void unpack_frag2HLT(uint32_t version, uint32_t sizeOverhead,
                         const uint32_t* p, FRwChVec & pChannel, int fragID, int demoType) const;

    /** unpack_frag4HLT decodes tile subfragment type 0x4 for the high level trigger (HLT).
     This subfragment contains the
     reconstructed amplitude and phase from the tilecal digitized pulse and a
     quality factor of the reconstruction. <p>
     The amplitude is calibrated to the online units that can be:
     \li  TileRawChannelUnit::OnlineADCcounts
     \li  TileRawChannelUnit::OnlinePicoCoulombs
     \li  TileRawChannelUnit::OnlineCesiumPicoCoulombs
     \li  TileRawChannelUnit::OnlineMegaElectronVolts<p>
     The phase is encoded in ns. <p>
     The subfragment type 0x4 contains the reconstructed parameters from the
     48 read-out channels of a tilecal module. */
    void unpack_frag4HLT(uint32_t version, uint32_t sizeOverhead, unsigned int unit, const uint32_t* p, FRwChVec & pChannel, int fragID, int demoType) const;

    /** unpack_frag5HLT decodes tile subfragment type 0x5 for the high level trigger (HLT).
     This subfragment contains the
     reconstructed amplitude and phase from the tilecal digitized pulse<p>
     The amplitude is calibrated to the online units that can be:
     \li  TileRawChannelUnit::OnlineADCcounts
     \li  TileRawChannelUnit::OnlinePicoCoulombs
     \li  TileRawChannelUnit::OnlineCesiumPicoCoulombs
     \li  TileRawChannelUnit::OnlineMegaElectronVolts<p>
     The phase is encoded in ns. <p>
     The subfragment type 0x5 contains the reconstructed parameters and residuals from the
     48 read-out channels of a tilecal module. */
    void unpack_frag5HLT(uint32_t version, uint32_t sizeOverhead, unsigned int unit, const uint32_t* p, FRwChVec & pChannel, int fragID, int demoType) const;

    /** unpack_fragA decodes tile subfragment type 0XA. This subfragment contains
     data quality checks. */
    void unpack_fragA(uint32_t version,
                      RawChannelMetaData_t& rawchannelMetaData,
                      const uint32_t* p, pRwChVec & pChannel, int fragID, int demoType) const;
    /** unpack_fragAHLT decodes tile subfragment type 0XA. This subfragment contains
     data quality checks. */
    void unpack_fragAHLT(uint32_t version, const uint32_t* p, uint16_t rob_bcid,
        uint16_t& mask, int fragID, int demoType) const;

    /** unpack_frag10 decodes tile subfragment type 0x10. This subfragment contains
     the result of the MTag algorithm and the transverse energy calculation for
     two tilecal modules. <p>
     The MTag algorithm identifies low transverse momentum muons with eta projective
     pattern that traverse the tile calorimeter and calculates the energy deposited in
     the corresponding eta projective tower.<p>
     The total transverse energy is calculated for each of the two tilecal modules in the subfragment. */
    void unpack_frag10(uint32_t version, const uint32_t* p, TileL2Container & v, int fragID, int demoType) const;

    /** unpack_frag11 decodes tile subfragment type 0x11. This subfragment contains
     the result of the MTag algorithm and the transverse energy calculation ONLY for
     one tilecal module. <p>
     The MTag algorithm identifies low transverse momentum muons with eta projective
     pattern that traverse the tile calorimeter and calculates the energy deposited in
     the corresponding eta projective tower.<p>
     The total transverse energy is calculated for a tilecal module. */
    void unpack_frag11(uint32_t version, const uint32_t* p, TileL2Container & v, int fragID, int demoType) const;

    /** unpack_frag12 decodes tile subfragment type 0x12. This subfragment contains
     the result of the MTag algorithm for two tilecal modules. <p>
     The MTag algorithm identifies low transverse momentum muons with eta projective
     pattern that traverse the tile calorimeter and calculates the energy deposited in
     the corresponding eta projective tower. */
    void unpack_frag12(uint32_t version, const uint32_t* p, TileL2Container & v, int fragID, int demoType) const;

    /** unpack_frag13 decodes tile subfragment type 0x13. This subfragment contains
     the result of the MTag algorithm ONLY for
     one tilecal module. <p>
     The MTag algorithm identifies low transverse momentum muons with eta projective
     pattern that traverse the tile calorimeter and calculates the energy deposited in
     the corresponding eta projective tower.*/
    void unpack_frag13(uint32_t version, const uint32_t* p, TileL2Container & v, int fragID, int demoType) const;

    /** unpack_frag14 decodes tile subfragment type 0x14. This subfragment contains
     the transverse energy calculation for tilecal module. */
    void unpack_frag14(uint32_t version, const uint32_t* p, TileL2Container & v, int fragID, int demoType) const;

    /** unpack_frag15 decodes tile subfragment type 0x15. This subfragment contains
     the transverse energy calculation ONLY for
     one tilecal module. */
    void unpack_frag15(uint32_t version, const uint32_t* p, TileL2Container & v, int fragID, int demoType) const;

    /** unpack_frag4L2 decodes tile subfragment type 0x4
     and extract transverse energy from this fragment */
    bool unpack_frag4L2(uint32_t version, uint32_t sizeOverhead, const uint32_t* p, TileL2Container & v, int fragID, int demoType) const;

    /** unpack_frag5L2 decodes tile subfragment type 0x5
     and extract transverse energy from this fragment */
    bool unpack_frag5L2(uint32_t version, const uint32_t* p, TileL2Container & v, int fragID, int demoType) const;

    /** unpack_frag16 decodes tile subfragment type 0x16 or 0x20. This subfragment contains
     informations coming from the Laser box [calibration run] */
    void unpack_frag16(uint32_t version, const uint32_t* p, TileLaserObject & v) const; // LASERI

    /** unpack_frag17 decodes tile subfragment type 0x17 or 0x20. This subfragment contains
     informations coming from the Laser box [calibration run] */
    void unpack_frag17(uint32_t version, uint32_t sizeOverhead, const uint32_t* p, TileLaserObject & v) const; // LASERII

    /** unpack_brod decodes all ancillary tile subfragments coming from beam ROD
     at the testbeam or LASTROD in normal ATLAS configuration */
    void unpack_brod(uint32_t version, uint32_t sizeOverhead, const uint32_t* p, pBeamVec & pBeam, int fragID) const;

    /** unpacking methods dedicated to the TMDB ROD format sub-fragments 0x40 0x41 0x42 */
    void unpack_frag40(uint32_t collid,   uint32_t version, const uint32_t* p, int size, TileDigitsCollection &coll) const;
    void unpack_frag41(uint32_t collid,   uint32_t version, const uint32_t* p, int size, TileRawChannelCollection &coll) const;
    void unpack_frag42(uint32_t sourceid, uint32_t version, const uint32_t* p, int size, TileMuonReceiverContainer &v) const;

    /**/

    inline void make_copy(uint32_t bsflags,
                          TileFragHash::TYPE rChType,
                          TileRawChannelUnit::UNIT rChUnit,
                          DigitsMetaData_t& digitsMetaData,
                          RawChannelMetaData_t& rawchannelMetaData,
                          const ROBData * rob, pDigiVec & pDigits, pRwChVec & pChannel,
                          TileBeamElemCollection& v,
                          TileBeamElemContainer* container) const;
    inline void make_copy(uint32_t bsflags,
                          TileFragHash::TYPE rChType,
                          TileRawChannelUnit::UNIT rChUnit,
                          DigitsMetaData_t& digitsMetaData,
                          RawChannelMetaData_t& rawchannelMetaData,
                          const ROBData * rob, pDigiVec & pDigits, pRwChVec & pChannel,
                          TileDigitsCollection& v,
                          TileDigitsContainer* container) const;
    inline void make_copy(uint32_t bsflags,
                          TileFragHash::TYPE rChType,
                          TileRawChannelUnit::UNIT rChUnit,
                          DigitsMetaData_t& digitsMetaData,
                          RawChannelMetaData_t& rawchannelMetaData,
                          const ROBData * rob, pDigiVec & pDigits, pRwChVec & pChannel,
                          TileRawChannelCollection& v,
                          TileRawChannelContainer* container) const;

    uint32_t make_copyHLT(bool of2,
                          TileRawChannelUnit::UNIT rChUnit,
                          bool correctAmplitude,
                          const FRwChVec & pChannel,
                          TileCellCollection& v, const uint16_t DQuality,
                          D0CellsHLT& d0cells,
                          TileCellCollection * MBTS) const;
    std::vector<uint32_t> get_correct_data(const uint32_t* p) const;
    inline void make_copy(const ROBData * rob, pBeamVec & pBeam, TileBeamElemCollection& v) const;
    inline void make_copy(const ROBData * rob, pBeamVec & pBeam, TileDigitsCollection& v) const;
    inline void make_copy(const ROBData * rob, pBeamVec & pBeam, TileRawChannelCollection& v) const;
    inline void make_copy(const ROBData * rob, pBeamVec & pBeam, TileCellCollection& v) const;

    template<typename ELEMENT>
    inline void delete_vec(std::vector<ELEMENT *> & v) const;

    template<typename ELEMENT, class COLLECTION>
    inline void copy_vec(std::vector<ELEMENT *> & v, COLLECTION & coll) const;

    /** check the bitmap for a channel
     */
    bool checkBit(const uint32_t* p, int chan) const;

    TileRawChannel2Bytes5 m_rc2bytes5;
    TileRawChannel2Bytes4 m_rc2bytes4;
    TileRawChannel2Bytes2 m_rc2bytes2;
    TileRawChannel2Bytes m_rc2bytes;
    TileDigits2Bytes m_d2Bytes;

    const TileHWID* m_tileHWID = nullptr;

    Gaudi::Property<bool> m_useFrag0{this, "useFrag0", true, "Use frag0"};
    Gaudi::Property<bool> m_useFrag1{this, "useFrag1", true, "Use frag1"};
    Gaudi::Property<bool> m_useFrag4{this, "useFrag4", true, "User frag4"};
    Gaudi::Property<bool> m_useFrag5Raw{this, "useFrag5Raw", false, "Use frag5 raw"};
    Gaudi::Property<bool> m_useFrag5Reco{this, "useFrag5Reco", false, "Use frag5 reco"};
    Gaudi::Property<bool> m_ignoreFrag4HLT{this, "ignoreFrag4HLT", false, "Ignore frag4 HLT"};

    // Outside this time withdow, all amplitudes taken from Reco fragment will be set to zero
    Gaudi::Property<float> m_allowedTimeMin{this, "AllowedTimeMin", -50.0,
        "Set amplitude to zero if time is below allowed time minimum"};
    Gaudi::Property<float> m_allowedTimeMax{this, "AllowedTimeMax", 50.0,
        "Set amplitude to zero if time is above allowed time maximum"};

    // Thresholds for parabolic amplitude correction
    Gaudi::Property<float> m_ampMinThresh{this, "AmpMinForAmpCorrection", 15.0,
        "Correct amplitude if it's above amplitude threshold (in ADC counts)"};
    Gaudi::Property<float> m_timeMinThresh{this, "TimeMinForAmpCorrection", -12.5,
        "Correct amplitude is time is above time minimum threshold"};
    Gaudi::Property<float> m_timeMaxThresh{this, "TimeMaxForAmpCorrection", 12.5,
        "Correct amplitude is time is below time maximum threshold"};

    Gaudi::Property<unsigned int> m_fullTileRODs{this, "fullTileMode", 320000,
        "Run from which to take the cabling (for the moment, either 320000 - full 2017 mode (default) - or 0 - 2016 mode)"};

   Gaudi::Property<std::vector<int>> m_demoFragIDs{this,
         "DemoFragIDs", {}, "List of Tile frag IDs with new electronics (demonstrator)"};

    Gaudi::Property<bool> m_verbose{this, "VerboseOutput", false, "Print extra information"};
    Gaudi::Property<bool> m_calibrateEnergy{this, "calibrateEnergy", true, "Convert ADC counts to pCb for RawChannels"};
    Gaudi::Property<bool> m_suppressDummyFragments{this, "suppressDummyFragments", false, "Suppress dummy fragments"};
    Gaudi::Property<bool> m_maskBadDigits{this, "maskBadDigits", false,
        "Put -1 in digits vector for channels with bad BCID or CRC in unpack_frag0"};

    Gaudi::Property<int> m_maxWarningPrint{this, "MaxWarningPrint", 1000, "Maximum warning messages to print"};
    Gaudi::Property<int> m_maxErrorPrint{this, "MaxErrorPrint", 1000, "Maximum error messages to print"};

    ToolHandle<TileCondToolTiming> m_tileToolTiming{this,
        "TileCondToolTiming", "TileCondToolTiming", "Tile timing tool"};
    ToolHandle<TileCondToolOfcCool> m_tileCondToolOfcCool{this,
        "TileCondToolOfcCool", "TileCondToolOfcCool", "Tile OFC tool"};
    ToolHandle<TileCondToolEmscale> m_tileToolEmscale{this,
        "TileCondToolEmscale", "TileCondToolEmscale", "Tile EM scale calibration tool"};
    ToolHandle<ITileBadChanTool> m_tileBadChanTool{this,
        "TileBadChanTool", "TileBadChanTool", "Tile bad channel tool"};
    ToolHandle<TileL2Builder> m_L2Builder{this,
        "TileL2Builder", "TileL2Builder", "Tile L2 builder tool"};

    // thresholds for parabolic amplitude correction
    float m_ampMinThresh_pC; //!< correct amplitude if it's above amplitude threshold (in pC)
    float m_ampMinThresh_MeV; //!< correct amplitude if it's above amplitude threshold (in MeV)
    void updateAmpThreshold(int run = -1);

    // OFWeights for different units and different drawers:
    // every element contains OFC for single drawer and one of 4 different units
    mutable std::vector<uint32_t> m_OFWeights[4 * TileCalibUtils::MAX_DRAWERIDX] ATLAS_THREAD_SAFE;

    // Pointers to the start of the data for each vector.
    mutable std::atomic<const uint32_t*> m_OFPtrs[4 * TileCalibUtils::MAX_DRAWERIDX] ATLAS_THREAD_SAFE;

    // Mutex protecting access to weight vectors.
    mutable std::mutex m_OFWeightMutex;

    // Mutex protecting access to m_hid2re.
    mutable std::mutex m_HidMutex;

    // fast decoding
    std::vector<int> m_Rw2Cell[4];
    std::vector<int> m_Rw2Pmt[4];

    TileFragHash m_hashFunc;
    bool m_of2Default;

    // Map from frag id to MBTS idx
    std::map<unsigned int, unsigned int> m_mapMBTS;
    // index of the MBTS channel
    int m_MBTS_channel = 0;

    mutable std::atomic<int> m_WarningCounter;
    mutable std::atomic<int> m_ErrorCounter;

    TileHid2RESrcID * m_hid2re;
    TileHid2RESrcID * m_hid2reHLT;

    std::vector<int> m_list_of_masked_drawers;
    void initHid2re();
    void initHid2reHLT();

    unsigned int m_maxChannels;
    bool m_checkMaskedDrawers;
    int m_runPeriod;

    std::vector<int> m_demoChanLB;
    std::vector<int> m_demoChanEB;

    const uint32_t * get_data(const ROBData * rob) const {
      const uint32_t * p;
      if (rob->rod_status_position()==0 && 
          rob->rod_nstatus() + rob->rod_header_size_word() + rob->rod_trailer_size_word() >= rob->rod_fragment_size_word()) {
        rob->rod_status(p);
      } else {
        rob->rod_data(p);
      }
      return p;
    }
    
    uint32_t data_size(const ROBData * rob, uint32_t& error) const {
      uint32_t size = rob->rod_ndata();
      uint32_t max_allowed_size = rob->rod_fragment_size_word();
      uint32_t delta = rob->rod_header_size_word() + rob->rod_trailer_size_word();
      if (max_allowed_size > delta)
        max_allowed_size -= delta;
      else
        max_allowed_size = 0;
      if (size < 3 && size > 0) {
        if (rob->rod_source_id() > 0x50ffff) error |= 0x10000; // indicate error in frag size, but ignore error in laser ROD
        if (m_WarningCounter++ < m_maxWarningPrint) {
          ATH_MSG_WARNING("ROB " << MSG::hex << rob->source_id()
              << " ROD " << rob->rod_source_id() << MSG::dec
              << " has unexpected data size: " << size << " - assuming zero size " );
        }
        return 0;
      } else if (rob->rod_header_size_word() >= rob->rod_fragment_size_word()) {
        if (rob->rod_source_id() > 0x50ffff) error |= 0x10000; // indicate error in frag size, but ignore error in laser ROD
        if (m_WarningCounter++ < m_maxWarningPrint) {
          ATH_MSG_WARNING("ROB " << MSG::hex << rob->source_id()
              << " ROD " << rob->rod_source_id() << MSG::dec
              << " has unexpected header size: " << rob->rod_header_size_word()
              << " bigger than full size " << rob->rod_fragment_size_word()
              << " - assuming no data " );
        }
        return 0;
      } else if (size > max_allowed_size) {
        if (rob->rod_source_id() > 0x50ffff) error |= 0x10000; // indicate error in frag size, but ignore error in laser ROD

        if (size - rob->rod_trailer_size_word() < max_allowed_size) {
          if ((m_WarningCounter++) < m_maxWarningPrint) {
            ATH_MSG_WARNING("ROB " << MSG::hex << rob->source_id()
                            << " ROD " << rob->rod_source_id() << MSG::dec
                            << " data size " << size << " is longer than allowed size " << max_allowed_size
                            << " - assuming that ROD trailer is shorter: "
                            << rob->rod_trailer_size_word()-(size-max_allowed_size)
                            << " words instead of " << rob->rod_trailer_size_word());
          }
          max_allowed_size = size;
        } else if (size - rob->rod_trailer_size_word() == max_allowed_size) {
          if ((m_WarningCounter++) < m_maxWarningPrint) {
            ATH_MSG_WARNING("ROB " << MSG::hex << rob->source_id()
                            << " ROD " << rob->rod_source_id() << MSG::dec
                            << " data size " << size << " is longer than allowed size " << max_allowed_size
                            << " - assuming that ROD trailer ("
                            << rob->rod_trailer_size_word()
                            << " words) is absent");
          }
          max_allowed_size = size;
        } else {
          max_allowed_size += rob->rod_trailer_size_word();
          if ((m_WarningCounter++) < m_maxWarningPrint) {
            ATH_MSG_WARNING("ROB " << MSG::hex << rob->source_id()
                            << " ROD " << rob->rod_source_id() << MSG::dec
                            << " has unexpected data size: " << size
                            << " - assuming data size = " << max_allowed_size << " words and no ROD trailer at all" );
          }
        }
        return max_allowed_size;
      } else {
        return size;
      }
    }
};

template<typename ELEMENT>
inline void TileROD_Decoder::delete_vec(std::vector<ELEMENT *> & v) const {
  typedef typename std::vector<ELEMENT *>::const_iterator ELEMENT_const_iterator;

  ELEMENT_const_iterator iCh = v.begin();
  ELEMENT_const_iterator iEnd = v.end();
  for (; iCh != iEnd; ++iCh)  delete (*iCh);
  v.clear();
}

template<typename ELEMENT, class COLLECTION>
inline void TileROD_Decoder::copy_vec(std::vector<ELEMENT *> & v, COLLECTION & coll) const {
  typedef typename std::vector<ELEMENT *>::const_iterator ELEMENT_const_iterator;

  ELEMENT_const_iterator iCh = v.begin();
  ELEMENT_const_iterator iEnd = v.end();
  for (; iCh != iEnd; ++iCh) coll.push_back(*iCh);
}

inline
void TileROD_Decoder::make_copy(uint32_t /*bsflags*/,
                                TileFragHash::TYPE /*rChType*/,
                                TileRawChannelUnit::UNIT /*rChUnit*/,
                                DigitsMetaData_t& digitsMetaData,
                                RawChannelMetaData_t& /*rawchannelMetaData*/,
                                const ROBData * rob,
                                pDigiVec & pDigits,
                                pRwChVec & pChannel,
                                TileDigitsCollection & v,
                                TileDigitsContainer* /*container*/) const
{
  copy_vec(pDigits, v); // Digits stored

  if (pChannel.size() > 0) { // RawChannels deleted
    delete_vec(pChannel);
  }

  v.setLvl1Id(rob->rod_lvl1_id());
  v.setLvl1Type(rob->rod_lvl1_trigger_type());
  v.setDetEvType(rob->rod_detev_type());
  v.setRODBCID(rob->rod_bc_id());

  uint32_t status = TileFragStatus::ALL_OK;
  for (size_t j=0; j<digitsMetaData[6].size(); ++j) {
    status |= digitsMetaData[6][j];
  }

  if (v.size() > 0) {
    // Set meta data
    v.setFragSize(digitsMetaData[0][0]);
    v.setFragBCID(digitsMetaData[0][2] | (status<<16));

    v.setFragExtraWords(digitsMetaData[1]);

    v.setFragChipHeaderWords(digitsMetaData[2]);
    v.setFragChipCRCWords(digitsMetaData[3]);

    if (v.isCalibMode()) {
      v.setFragChipHeaderWordsHigh(digitsMetaData[4]);
      v.setFragChipCRCWordsHigh(digitsMetaData[5]);
    }
    if (m_verbose) v.printExtra();
  } else if ( digitsMetaData[0].size() == 0 ) {
    // no useful digi fragment or no data inside fragment
    status |= TileFragStatus::NO_FRAG;
    v.setFragBCID(0xDEAD | (status<<16));
    v.setFragSize(0);
  }

  if (status!=TileFragStatus::ALL_OK)
    ATH_MSG_DEBUG( "Status for drawer 0x" << MSG::hex << v.identify() << " in Digi frag is 0x" << status << MSG::dec);
}

inline
void TileROD_Decoder::make_copy(uint32_t bsflags,
                                TileFragHash::TYPE rChType,
                                TileRawChannelUnit::UNIT rChUnit,
                                DigitsMetaData_t& /*digitsMetaData*/,
                                RawChannelMetaData_t& rawchannelMetaData,
                                const ROBData * rob,
                                pDigiVec & pDigits,
                                pRwChVec & pChannel,
                                TileRawChannelCollection & v,
                                TileRawChannelContainer* container) const
{
  if (pChannel.size() > 0) { // take available raw channels
                             // and store in collection
    if (container) {
      ATH_MSG_VERBOSE( "RawChannel unit is " << rChUnit
                      << "  - setting unit in TileRawChannelContainer " );
      container->set_unit(rChUnit);
      container->set_type(rChType);
      container->set_bsflags(bsflags);
    } else {
      ATH_MSG_ERROR( "Can't set unit=" << rChUnit << " in TileRawChannelContainer" );
    }

    copy_vec(pChannel, v);

    if (pDigits.size() > 0) delete_vec(pDigits); // Digits deleted

  } else {

    if (pDigits.size() > 0) { // convert digits to raw channels
                              // and store directly in collection
      ATH_MSG_VERBOSE( " No reco frag in BS " );
      //m_RCBuilder->build(pDigits.begin(),pDigits.end(),(&v));

      delete_vec(pDigits); // Digits deleted

    } else {
      ATH_MSG_DEBUG( "data for drawer 0x" << MSG::hex << v.identify() << MSG::dec << " not found in BS" );
    }
    rawchannelMetaData[6].push_back(TileFragStatus::NO_FRAG);
    HWIdentifier drawerID = m_tileHWID->drawer_id(v.identify());
    for (unsigned int ch = 0; ch < m_maxChannels; ++ch) {
      HWIdentifier adcID = m_tileHWID->adc_id(drawerID, ch, 0);
      v.push_back(new TileRawChannel(adcID, 0.0, -100.0, 31));
    }

  }

  v.setLvl1Id(rob->rod_lvl1_id());
  v.setLvl1Type(rob->rod_lvl1_trigger_type());
  v.setDetEvType(rob->rod_detev_type());
  v.setRODBCID(rob->rod_bc_id());

  if (rChUnit < TileRawChannelUnit::OnlineOffset && rChType > TileFragHash::OptFilterDsp) { // set good status for BS from MC
    rawchannelMetaData[0].push_back(0);
    rawchannelMetaData[0].push_back(0xDEAD);
    rawchannelMetaData[5].push_back(0xFFFF);
    rawchannelMetaData[5].push_back(0xFFFF);
  }

  for (unsigned int i = 0; i < 6; ++i) {
    for (size_t j=rawchannelMetaData[i].size(); j<2; ++j) {
      rawchannelMetaData[i].push_back(0);
    }
  }

  uint32_t status = (rawchannelMetaData[0][0] & 0x1) ? TileFragStatus::CRC_ERR : TileFragStatus::ALL_OK ;
  for (size_t j=0; j<rawchannelMetaData[6].size(); ++j) {
    status |= rawchannelMetaData[6][j];
  }
  if (status>TileFragStatus::CRC_ERR)
    ATH_MSG_DEBUG( "Status for drawer 0x" << MSG::hex << v.identify() << " is 0x" << status << MSG::dec);

  v.setFragGlobalCRC(status);
  v.setFragDSPBCID(rawchannelMetaData[0][1]);
  v.setFragBCID(rawchannelMetaData[1][0]);
  v.setFragMemoryPar(rawchannelMetaData[1][1]);
  v.setFragSstrobe(rawchannelMetaData[2][0]);
  v.setFragDstrobe(rawchannelMetaData[2][1]);
  v.setFragHeaderBit(rawchannelMetaData[3][0]);
  v.setFragHeaderPar(rawchannelMetaData[3][1]);
  v.setFragSampleBit(rawchannelMetaData[4][0]);
  v.setFragSamplePar(rawchannelMetaData[4][1]);
  v.setFragFEChipMask(rawchannelMetaData[5][0]);
  if (std::binary_search(m_demoFragIDs.begin(), m_demoFragIDs.end(), v.identify()))
    v.setFragRODChipMask(rawchannelMetaData[5][0]);
  else
    v.setFragRODChipMask(rawchannelMetaData[5][1]);
}

inline
void TileROD_Decoder::make_copy(uint32_t /*bsflags*/,
                                TileFragHash::TYPE /*rChType*/,
                                TileRawChannelUnit::UNIT /*rChUnit*/,
                                DigitsMetaData_t& /*digitsMetaData*/,
                                RawChannelMetaData_t& /*rawchannelMetaData*/,
                                const ROBData * /* rob */, pDigiVec & pDigits,
                                pRwChVec & pChannel, TileBeamElemCollection &,
                                TileBeamElemContainer* /*container*/) const
{
  // do nothing
  delete_vec(pDigits);
  delete_vec(pChannel);
}

inline
void TileROD_Decoder::make_copy(const ROBData * rob, pBeamVec & pBeam,
    TileBeamElemCollection & v) const {
  copy_vec(pBeam, v);

  v.setLvl1Id(rob->rod_lvl1_id());
  v.setLvl1Type(rob->rod_lvl1_trigger_type());
  v.setDetEvType(rob->rod_detev_type());
  v.setRODBCID(rob->rod_bc_id());
}

inline
void TileROD_Decoder::make_copy(const ROBData * /* rob */, pBeamVec & pBeam,
    TileDigitsCollection &) const {
  // do nothing
  delete_vec(pBeam);
}

inline
void TileROD_Decoder::make_copy(const ROBData * /* rob */, pBeamVec & pBeam,
    TileRawChannelCollection &) const {
  // do nothing
  delete_vec(pBeam);
}

inline
void TileROD_Decoder::make_copy(const ROBData * /* rob */, pBeamVec & pBeam,
    TileCellCollection &) const {
  // do nothing
  delete_vec(pBeam);
}


// ----  Implement the template method: 

/**   fill either TileDigitsCollection or TileRawChannelCollection 
 or TileCellCollection or TileBeamElemCollection
 from a BLOCK of integers
 */
template<class COLLECTION>
void TileROD_Decoder::fillCollection(const ROBData * rob,
                                     COLLECTION & v,
                                     typename TileROD_Helper::ContainerForCollection<COLLECTION>::type* container /*= nullptr*/) const
{
  //
  // get info from ROD header
  //
  //  if (msgLvl(MSG::VERBOSE)) {
  //    msg(MSG::VERBOSE) << "ROD header info: " << endmsg
  //    msg(MSG::VERBOSE) << " Format Vers.  " << MSG::hex << "0x" << rob->rod_version() << MSG::dec << endmsg;
  //    msg(MSG::VERBOSE) << " Source ID     " << MSG::hex << "0x" << rob->rod_source_id() << MSG::dec << endmsg;
  //    msg(MSG::VERBOSE) << " Source ID str " << eformat::helper::SourceIdentifier(rob->source_id()).human().c_str() << endmsg;
  //    msg(MSG::VERBOSE) << " Run number    " << (int) rob->rod_run_no() << endmsg;
  //    msg(MSG::VERBOSE) << " Level1 ID     " << rob->rod_lvl1_id() << endmsg;
  //    msg(MSG::VERBOSE) << " BCID          " << rob->rod_bc_id() << endmsg;
  //    msg(MSG::VERBOSE) << " Lvl1 TrigType " << rob->rod_lvl1_trigger_type() << endmsg;
  //    msg(MSG::VERBOSE) << " Event Type    " << rob->rod_detev_type() << endmsg;
  //    msg(MSG::VERBOSE) << " Fragment size " << rob->rod_fragment_size_word() << endmsg;
  //    msg(MSG::VERBOSE) << " Header   size " << rob->rod_header_size_word() << endmsg;
  //    msg(MSG::VERBOSE) << " Trailer  size " << rob->rod_trailer_size_word() << endmsg;
  //    msg(MSG::VERBOSE) << " N data        " << rob->rod_ndata() << endmsg;
  //    msg(MSG::VERBOSE) << " N status      " << rob->rod_nstatus() << endmsg;
  //    msg(MSG::VERBOSE) << " Status pos    " << rob->rod_status_position() << endmsg;
  //  }

  uint32_t version = rob->rod_version() & 0xFFFF;

  bool isBeamROD = false;
  // figure out which fragment we want to unpack
  TileRawChannelCollection::ID frag_id = v.identify();
  const std::vector<uint32_t> & drawer_info = m_hid2re->getDrawerInfo(frag_id);
  int bs_frag_id = drawer_info.size()>1 ? drawer_info[1] : frag_id;
  int drawer_type = drawer_info.size()>2 ? drawer_info[2] : -1;

  uint32_t mask = 0xFFFF;

  // special conversion for sub-fragments in beam ROD
  if (frag_id < 0x100) {
    isBeamROD = true;
    mask = 0xFF; // this is needed for TB 2003, when beam frag_id >= 0x400
  }

  // find all fragments with given Fragment ID
  // and possibly with different types
  std::vector<const uint32_t *> pFrag;
  pFrag.reserve(5);

  uint32_t error = 0;
  uint32_t wc = 0;
  uint32_t size = data_size(rob, error);
  const uint32_t * p = get_data(rob);

  // 2 extra words in every frag by default (frag id + frag size)
  // but for all data after 2005 it is set to 3 later in the code
  uint32_t sizeOverhead = 2;

  // bool skipWords = ( ! isBeamROD && version == 0x1 );
  // std::cout << " *(p) = 0x" << std::hex << (*(p)) << std::dec << std::endl;
  if (size) {
    bool V3format = (*(p) == 0xff1234ff); // additional frag marker since Sep 2005
    V3format |= (*(p) == 0x00123400); // additional frag marker since Sep 2005 (can appear in buggy ROD frags)
    if (!V3format && version>0xff) {
      V3format = true;
      if ((m_WarningCounter++) < m_maxWarningPrint)
        ATH_MSG_WARNING("fillCollection: corrupted frag separator 0x" << MSG::hex << (*p) << " instead of 0xff1234ff in ROB 0x" << rob->rod_source_id() << MSG::dec );
    }
    if (V3format) {
      ++p; // skip frag marker
      sizeOverhead = 3;
    } else {
      sizeOverhead = 2;
    }
  }

  //std::cout << std::hex << " frag_id " << frag_id << " mask " << mask
  //          << " version " << version << " sizeOverhead " << sizeOverhead
  //          << " skipWords " << skipWords << " V3format " << V3format
  //          << std::dec << std::endl;
  //std::cout << " size is "<< size << std::endl;

  while (wc < size) { // iterator over all words in a ROD

    // first word is frag size
    uint32_t count = *(p);
    // second word is frag ID (16 bits) frag type (8 bits) and additional flags
    uint32_t idAndType = *(p + 1);
    int frag = (idAndType & mask);
    int type = (idAndType & 0xF00000) >> 16; // note special mask, we ignore one digit, keep only 0x10, 0x20, 0x30, ...

    if (count < sizeOverhead || count > size - wc) {
      int cnt = 0;
      for (; wc < size; ++wc, ++cnt, ++p) {
        if ((*p) == 0xff1234ff) {
          ++cnt;
          ++wc;
          ++p;
          break;
        }
      }
      if ((m_ErrorCounter++) < m_maxErrorPrint) {
        msg(MSG::WARNING) << "Frag 0x" << MSG::hex << frag << MSG::dec
                        << " has unexpected size: " << count;
        if (wc < size) {
          msg(MSG::WARNING) << "  skipping " << cnt << " words to the next frag" << endmsg;
        } else {
          msg(MSG::WARNING) << " ignoring " << cnt << " words till the end of ROD frag" << endmsg;
        }
      }
      continue;
    }

    if (type != 0x10 && frag == bs_frag_id) pFrag.push_back(p);

    p += count;
    wc += count;

    //std::cout << " frag ="<<frag<<" type = "<<type<<" count="<<count<<std::endl;

    // this is not needed anymore, since we can't read very old BS files anyhow.
    //if ( skipWords && type == 0 && wc < size ) { // special treatment of preROD output with digits
    //  wc+=7; p+=7; // skip extra header
    //}
  }

  if (wc != size) {
    // check word count
    if ((m_ErrorCounter++) < m_maxErrorPrint) {
      ATH_MSG_WARNING( "Incorrect ROD size: " << wc << " words instead of " << size );
    }
    assert(0);
    // return;
  }

  if (isBeamROD) {

    pBeamVec pBeam;
    pBeam.reserve(16);

    std::vector<const uint32_t *>::const_iterator it = pFrag.begin();
    std::vector<const uint32_t *>::const_iterator itEnd = pFrag.end();

    for (; it != itEnd; ++it) {

      p = (*it);

      unpack_brod(version, sizeOverhead, p, pBeam, frag_id);
    }

    make_copy(rob, pBeam, v);

  } else {

    pDigiVec pDigits;
    pRwChVec pChannel;
    pChannel.reserve(48);

    // initialize meta data storage
    DigitsMetaData_t digitsMetaData (7);
    RawChannelMetaData_t rawchannelMetaData (7);
    for (unsigned int i = 0; i < 7; ++i) {
      digitsMetaData[i].reserve(16);
      rawchannelMetaData[i].reserve(2);
    }

    // now unpack all channels in all fragments
    // and fill temporary array of TileDigits and/or TileRawChannels

    std::vector<const uint32_t *>::const_iterator it = pFrag.begin();
    std::vector<const uint32_t *>::const_iterator itEnd = pFrag.end();

    uint32_t bsflags = 0;
    TileFragHash::TYPE rChType = TileFragHash::Digitizer;
    TileRawChannelUnit::UNIT rChUnit = TileRawChannelUnit::ADCcounts;

    for (; it != itEnd; ++it) {

      p = (*it);
      // first word is frag size
      // uint32_t count = *(p);
      // second word is frag ID (16 bits) frag type (8 bits) and additional flags
      uint32_t idAndType = *(p + 1);
      int type = (idAndType & 0x00FF0000) >> 16;

      ATH_MSG_VERBOSE( "Unpacking frag: 0x" << MSG::hex << (idAndType & 0xFFFF)
                      << " type " << type << MSG::dec );

      switch (type) {
        case 0:
          if (m_useFrag0) unpack_frag0(version, sizeOverhead, digitsMetaData, p, pDigits, frag_id, drawer_type);
          break;
        case 1:
          if (m_useFrag1) unpack_frag1(version, sizeOverhead, digitsMetaData, p, pDigits, frag_id, drawer_type);
          break;
        case 2:
          if (m_useFrag4) unpack_frag2(version, sizeOverhead, p, pChannel, frag_id, drawer_type);
          break;
        case 3:
          if (m_useFrag4) unpack_frag3(version, sizeOverhead, p, pChannel, frag_id, drawer_type);
          break;
        case 4:
          if (m_useFrag4) {
            bsflags = idAndType & 0xFFFF0000; // ignore frag num, keep all the rest
            int unit = (idAndType & 0xC0000000) >> 30;

            int DataType = (idAndType & 0x30000000) >> 28;

            if (DataType < 3) { // real data

              // only one bit for type and next 2 bits for number of iterations
              //int AlgoType = (idAndType & 0x4000000) >> 26;
              //if (AlgoType == 0)      rChType = TileFragHash::OF1Filter;
              //else                    rChType = TileFragHash::OF2Filter;
              // always set special type, which means now that OF is done inside DSP
              rChType = TileFragHash::OptFilterDsp;

              // Attention! Switching to Online Units for release 14.2.0
              rChUnit = (TileRawChannelUnit::UNIT) (unit + TileRawChannelUnit::OnlineOffset); // Online units in real data
              // rChUnit = (TileRawChannelUnit::UNIT) ( unit );

            } else { // simulated data

              // all 3 bits for type
              int AlgoType = (idAndType & 0x7000000) >> 24;
              rChType = (TileFragHash::TYPE) AlgoType;

              rChUnit = (TileRawChannelUnit::UNIT) (unit); // Offline units in simulated data

              if (!m_demoFragIDs.empty()) {
                const_cast<Gaudi::Property<std::vector<int>> &> ( m_demoFragIDs ) = {}; // No demonstator cabling in MC
                ATH_MSG_INFO("Disable channel remapping for demonstrator in MC");
              }
            }

            unpack_frag4(version, sizeOverhead, unit, rawchannelMetaData, p, pChannel, frag_id, drawer_type);
          }
          break;

        case 5:
          if (m_useFrag5Raw || m_useFrag5Reco) {
            bsflags = idAndType & 0xFFFF0000; // ignore frag num, keep all the rest
            int unit = (idAndType & 0xC0000000) >> 30;

            // always set special type, which means now that OF is done inside DSP
            rChType = TileFragHash::OptFilterDspCompressed;

            rChUnit = (TileRawChannelUnit::UNIT) (unit + TileRawChannelUnit::OnlineOffset);
            unpack_frag5(version, sizeOverhead, unit,
                         digitsMetaData,
                         p, pDigits, pChannel, frag_id, drawer_type);
          }
          break;

        case 6:
          break;

        case 0xA:
          unpack_fragA(version, rawchannelMetaData, p, pChannel, frag_id, drawer_type);
          break;

        default:
          int frag = idAndType & 0xFFFF;
          ATH_MSG_WARNING( "Unknown frag type=" << type << " for frag=" << frag );
          assert(0);
          break;
      }
    } // end of all frags

    make_copy(bsflags, rChType, rChUnit, digitsMetaData, rawchannelMetaData,
              rob, pDigits, pChannel, v, container);
  }

  return;
}

#endif
