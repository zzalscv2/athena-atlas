/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "SCT_RodDecoder.h"

#include "ByteStreamData/RawEvent.h"
#include "InDetIdentifier/SCT_ID.h"
#include "SCT_ReadoutGeometry/SCT_DetectorManager.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "SCT_ConditionsTools/ISCT_ByteStreamErrorsTool.h"

#include "Identifier/IdentifierHash.h"

#include <algorithm>
#include <array>
#include <utility>

union RawWord {
  uint32_t word32;
  uint16_t word16[2];
};

// Constructor

SCT_RodDecoder::SCT_RodDecoder(const std::string& type, const std::string& name,
                               const IInterface* parent) :
  base_class(type, name, parent)
{
}

// Initialize

StatusCode SCT_RodDecoder::initialize()
{
  ATH_CHECK(AlgTool::initialize());

  // Retrieve cabling tool
  ATH_CHECK(m_cabling.retrieve());
  ATH_MSG_DEBUG("Retrieved tool " << m_cabling);

  ATH_CHECK(detStore()->retrieve(m_sctID,"SCT_ID"));
  m_contextSCT = m_sctID->wafer_context();
  m_swapPhiReadoutDirection.resize(m_sctID->wafer_hash_max(), false);

  ATH_CHECK(m_configTool.retrieve());

  // See if strip numbers go from 0 to N_STRIPS_PER_SIDE-1(=767) or vice versa for all the wafers.
  // swapPhiReadoutDirection will not change during a run.
  // Since this is access to SiDetectorElement during initialization,
  // condition object of SiDetectorElementCollection is not accessible.
  // SCT_DetectorManager has to be used.
  const InDetDD::SCT_DetectorManager* sctDetManager{nullptr};
  ATH_CHECK(detStore()->retrieve(sctDetManager, "SCT"));

  const InDetDD::SiDetectorElementCollection* sctDetElementColl{sctDetManager->getDetectorElementCollection()};
  for (const InDetDD::SiDetectorElement* element: *sctDetElementColl) {
    if (element->swapPhiReadoutDirection()) {
      m_swapPhiReadoutDirection[element->identifyHash()] = true;
    }
  }

  return StatusCode::SUCCESS;
}

// Finalize

StatusCode SCT_RodDecoder::finalize()
{
  // Print out summaries of data and errors decoded
  ATH_MSG_INFO("SCT BytestreamCnv summary: " << m_headNumber  <<" link headers found");
  ATH_MSG_INFO("SCT BytestreamCnv summary: " << m_trailerNumber << " link trailers found");

  ATH_MSG_INFO("SCT decoding bytestream summary: "
               << m_singleCondHitNumber << " single strips with hit in condensed mode");
  ATH_MSG_INFO("SCT decoding bytestream summary: "
               << m_pairedCondHitNumber << " paired strips with hit in condensed mode");
  ATH_MSG_INFO("SCT decoding bytestream summary: "
               << m_firstExpHitNumber   << " first strips with hit in expanded mode");
  ATH_MSG_INFO("SCT decoding bytestream summary: "
               << m_evenExpHitNumber    << " consecutive paired strips with hit in expanded mode");
  ATH_MSG_INFO("SCT decoding bytestream summary: "
               << m_lastExpHitNumber    << " last consecutive strips with hit in expanded mode");

  if (m_headErrorBCID > 0) {
     ATH_MSG_INFO("SCT BytestreamCnv summary: header-> "
                  << m_headErrorLvl1ID    << " LVL1d errors found");
  }
  if (m_headErrorTimeout > 0) {
     ATH_MSG_INFO("SCT BytestreamCnv summary: header-> "
                  << m_headErrorTimeout   << " timeout errors found");
  }
  if (m_headErrorFormatter > 0) {
     ATH_MSG_INFO("SCT BytestreamCnv summary: header-> "
                  << m_headErrorFormatter << " formatter errors found");
  }
  if (m_headErrorPreamble > 0) {
     ATH_MSG_INFO("SCT BytestreamCnv summary: header-> "
                  << m_headErrorPreamble  << " preamble errors found");
  }
  if (m_maskedLinkNumber > 0) {
     ATH_MSG_INFO("SCT BytestreamCnv summary: header-> "
                  << m_maskedLinkNumber   << " masked links found");
  }

  if (m_trailerErrorOverflow > 0) {
     ATH_MSG_INFO("SCT BytestreamCnv summary: trailer-> "
                  << m_trailerErrorOverflow << " trailer data overflow errors found");
  }
  if (m_trailerErrorLimit > 0) {
     ATH_MSG_INFO("SCT BytestreamCnv summary: trailer-> "
                  << m_trailerErrorLimit    << " header trailer limit errors found");
  }
  if (m_trailerErrorBit > 0) {
     ATH_MSG_INFO("SCT BytestreamCnv summary: trailer-> "
                  << m_trailerErrorBit      << " trailer bit errors found");
  }

  if (m_configDataBit > 0) {
     ATH_MSG_INFO("SCT BytestreamCnv summary: raw Data-> "
                  << m_configDataBit << " raw data found: Config data mode");
  }

  if (m_flagErrorBit > 0) {
     ATH_MSG_INFO("SCT BytestreamCnv summary: flag-> "
                  << m_flagErrorBit << " module link flag bit errors found");
  }

  if (m_condHit1Error > 0) {
     ATH_MSG_INFO("SCT BytestreamCnv summary: hit-> "
                  << m_condHit1Error   << " 1st hit error found in condensed mode");
  }
  if (m_condHit2Error > 0) {
     ATH_MSG_INFO("SCT BytestreamCnv summary: hit-> "
                  << m_condHit2Error   << " 2nd hit error found in condensed mode");
  }
  if (m_chipNumberError > 0) {
     ATH_MSG_INFO("SCT BytestreamCnv summary: hit-> "
                  << m_chipNumberError << " Chip number > 5 error found");
  }

  if (m_unknownDataFormat > 0) {
     ATH_MSG_INFO("SCT BytestreamCnv summary: unknown data-> "
                  << m_unknownDataFormat << " Unknown data format found");
  }

  if (m_rodClockErrorNumber > 0) {
     ATH_MSG_INFO("SCT BytestreamCnv summary: ROD status word-> "
                  << m_rodClockErrorNumber << " ROD clock errors found");
  }
  if (m_maskedRODNumber > 0) {
     ATH_MSG_INFO("SCT BytestreamCnv summary: ROB status word-> "
                  << m_maskedRODNumber     << " masked RODs found");
  }
  if (m_truncatedRODNumber > 0) {
     ATH_MSG_INFO("SCT BytestreamCnv summary: ROB status word-> "
                  << m_truncatedRODNumber  << " truncated ROBFragments");
  }

  ATH_MSG_INFO("Number of SCT hits in ByteStream-> " << m_nHits);
  ATH_MSG_INFO("Number of SCT RDOs created->       " << m_nRDOs);

  if (m_numMissingLinkHeader > 0) ATH_MSG_WARNING("SCT Missing Link Headers found " << m_numMissingLinkHeader);

  ATH_CHECK(AlgTool::finalize());
  ATH_MSG_DEBUG("SCT_RodDecoder::finalize()");

  return StatusCode::SUCCESS;
}


/**
 * @brief allows to accumulate errors in one fillColection call
 *
 * Errors information is scattered across this code.
 * To be sure that all of the errors are saved this helper class provides add method allowing to update/accumulate erorr.
 * The IDC, for a very good reasons (MT safety) do not allow for that.
 **/
struct SCT_RodDecoderErrorsHelper {
  SCT_RodDecoderErrorsHelper( IDCInDetBSErrContainer& idcContainer )
    : errorsIDC{ idcContainer } {}
  ~SCT_RodDecoderErrorsHelper() {
    for ( auto [id, err]: accumulatedErrors ) {
      errorsIDC.setOrDrop( id, err );
    }
  }
  void noerror(const IdentifierHash id) {
    accumulatedErrors[id]; // this adds 0 (no error) for an ID
  }

  void add(const IdentifierHash id, SCT_ByteStreamErrors::ErrorType etype) {
    SCT_ByteStreamErrors::addError(accumulatedErrors[id], etype);
  }

  std::map<IdentifierHash, IDCInDetBSErrContainer::ErrorCode> accumulatedErrors;
  IDCInDetBSErrContainer& errorsIDC;
};


// fillCollection method
StatusCode SCT_RodDecoder::fillCollection(const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment& robFrag,
                                          ISCT_RDO_Container& rdoIDCont,
                                          IDCInDetBSErrContainer& errorsIDC,
                                          const std::vector<IdentifierHash>* vecHash) const
{
  SCT_RodDecoderErrorsHelper errs( errorsIDC ); // on destruction will fill the IDC
  const uint32_t robID{robFrag.rod_source_id()};
  // Determine whether this data was generated using the ROD simulator
  const uint32_t rodDataType{robFrag.rod_detev_type()};
  const bool rodSimulatedData{static_cast<bool>((rodDataType >> 20) & 0x1)};
  if (rodSimulatedData) ATH_CHECK(addRODError(robID, SCT_ByteStreamErrors::RODSimulatedData, errs));

  // Look for the bit that denotes "Super-condensed" mode
  const bool superCondensedMode{static_cast<bool>((rodDataType >> 21) & 0x1)};

  bool condensedMode{true};

  int strip{0};
  int oldStrip{-1};
  int oldSide{-1};
  int chip{0};
  int side{0};
  int nStripsInWord{0};
  int linkNumber{0}; // Determined from header and may be changed for links using Rx redundancy
  uint32_t onlineID{0}; // Determined from header and may be changed for links using Rx redundancy
  IdentifierHash currentLinkIDHash; // Determined from header and changed for links using Rx redundancy
  int timeBin{0};
  int groupSize{0};

  std::array<bool, N_STRIPS_PER_SIDE*N_SIDES> saved;
  saved.fill(false);
  RawWord robData;
  robData.word32=0;
  int errors{0}; // Encodes the errors on the header (bit 4: error in condensed mode 1st hit, bit 5: error in condensed mode 2nd hit)

  // These are for the trigger
  CacheHelper cache;
  cache.vecHash = vecHash;

  std::vector<int> errorHit;

  // For MissingLinkHeaderError
  bool foundMissingLinkHeaderError{false};
  std::unordered_set<IdentifierHash> foundHashes;

  StatusCode sc{StatusCode::SUCCESS, true};

  // Look at ROB status word
  if (robFrag.nstatus()!=0) {
    const uint32_t* robStatus;
    robFrag.status(robStatus);
    if ((*robStatus)!=0) {
      ATH_MSG_DEBUG("ROB status word for robID " << std::hex << robID
                    << " is non-zero " << (*robStatus) << std::dec);
      // First store generic "ROBFragmentError" error type.
      ATH_CHECK(addRODError(robID, SCT_ByteStreamErrors::ROBFragmentError, errs));
      sc = StatusCode::RECOVERABLE;
      // Now look for specific problems, e.g. truncated or masked-off RODs
      if (((*robStatus) >> 27) & 0x1) {
        ATH_MSG_DEBUG("ROB status word for robID " << std::hex << robID
                      << std::dec << " indicates data truncation.");
        ATH_CHECK(addRODError(robID, SCT_ByteStreamErrors::TruncatedROD, errs));
        m_truncatedRODNumber++;
        return sc;
      }
      if ((((*robStatus) >> 29) & 0x1) or (((*robStatus) >> 31) & 0x1)) {
        ATH_MSG_DEBUG("ROB status word for robID " << std::hex << robID
                      << std::dec << " indicates resource was masked off.");
        ATH_CHECK(addRODError(robID, SCT_ByteStreamErrors::MaskedROD, errs));
        m_maskedRODNumber++;
        return sc;
      }
    }
  }

  // Look at ROD status words
  OFFLINE_FRAGMENTS_NAMESPACE::PointerType vecRODStatus;
  const long unsigned int vecRODStatusSize{robFrag.rod_nstatus()};

  robFrag.rod_status(vecRODStatus);
  for (long unsigned int i{0}; i<vecRODStatusSize; i++) {
    const uint32_t statusWord{vecRODStatus[i]};
    // Check for clock errors in second ROD status word
    if (i==1) {
      const int timClockError{static_cast<int>((statusWord >> 16) & 0x1)};
      const int bocClockError{static_cast<int>((statusWord >> 17) & 0x1)};
      if (timClockError or bocClockError) {
        ATH_MSG_DEBUG(" Clock error in ROD status word: " << timClockError << " " << bocClockError);
        ATH_CHECK(addRODError(robID, SCT_ByteStreamErrors::RODClockError, errs));
        m_rodClockErrorNumber++;
        sc=StatusCode::RECOVERABLE;
      }
      // Look at bits 20-23 for DCS HV
      // const int hvBits{static_cast<int>((statusWord >> 20) & 0xF)};
      // const bool hvOn{hvBits==0xF};
    }
  }

  // Now look at the data words
  bool foundHeader{false};

  OFFLINE_FRAGMENTS_NAMESPACE::PointerType vecROBData;
  const unsigned long int vecROBDataSize{robFrag.rod_ndata()};
  robFrag.rod_data(vecROBData);

  // Loop over header, hit element, flagged ABCD error, raw data, trailer words
  for (unsigned long int i{0}; i<vecROBDataSize; i++) {
    robData.word32 = vecROBData[i];
    // The data is 16-bits wide packed to a 32-bit word (rob_it1). So we unpack it here.
    uint16_t data16[2];
    data16[1] = robData.word16[0];
    data16[0] = robData.word16[1];

    for (int n{0}; n<2; n++) {
      // Header
      if (((data16[n]>>13)&0x7) == 0x1) {
        foundHeader=true;
        m_headNumber++;

        // Create the last RDO of the previous link if any
        if (saved[side*N_STRIPS_PER_SIDE+strip]==false and oldStrip>=0) {
	  errs.noerror(currentLinkIDHash);
          const int rdoMade{makeRDO(strip, groupSize, timeBin, currentLinkIDHash, errors, rdoIDCont, cache, errorHit)};
          if (rdoMade == -1) {
            sc=StatusCode::RECOVERABLE;
            ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ByteStreamParseError, errs));
          }
          else {
            saved[side*N_STRIPS_PER_SIDE+strip] = rdoMade;
          }
        }

        // Everything is set to default for a new hunt of RDO
        strip =0;
        oldStrip = -1;
        oldSide = -1;
        groupSize = 0;
        errors = 0;
        saved.fill(false);
        errorHit.clear();

        // Link Number (or stream) in the ROD fragment
        const int rodlinkNumber{static_cast<int>(data16[n] & 0x7F)};

        // This is the real calculation for the offline
        linkNumber = (((rodlinkNumber >>4)&0x7)*12+(rodlinkNumber &0xF));
        onlineID = ((robID & 0xFFFFFF) | (linkNumber << 24));
        if ((onlineID ==0) or (linkNumber > 95)) {
          ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ByteStreamParseError, errs));
          sc=StatusCode::RECOVERABLE;
          ATH_MSG_DEBUG("Header: xxx Link number out of range (skipping following data)"
                        << std::dec << linkNumber);
          break;
        }
        else {
          currentLinkIDHash = m_cabling->getHashFromOnlineId(onlineID);
          errs.noerror( currentLinkIDHash );
          foundHashes.insert(currentLinkIDHash);
        }
        // Look for masked off links - bit 7
        if ((data16[n] >> 7) & 0x1) {
          ATH_MSG_DEBUG("Masked link " << onlineID << " " << currentLinkIDHash);
          ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::MaskedLink, errs));
          sc=StatusCode::RECOVERABLE;
        }
        if (data16[n]&0x800) {
          ATH_MSG_DEBUG("    Header: xxx TimeOut Error " << currentLinkIDHash);
          m_headErrorTimeout++;
          ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::TimeOutError, errs));
          sc=StatusCode::RECOVERABLE;
        }

        if (data16[n]&0x1000) {
          ATH_MSG_DEBUG("    Header: xxx Preamble Error " << currentLinkIDHash);
          m_headErrorPreamble++;
          ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::PreambleError, errs));
          sc=StatusCode::RECOVERABLE;
        }

        if (data16[n]&0x400) {
          ATH_MSG_DEBUG("    Header: xxx LVL1 ID Error " << currentLinkIDHash);
          m_headErrorLvl1ID++;
          ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::LVL1IDError, errs));
          sc=StatusCode::RECOVERABLE;
        }

        if (data16[n]&0x200) {
          ATH_MSG_DEBUG("    Header: xxx BCID Error " << currentLinkIDHash);
          m_headErrorBCID++;
          ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::BCIDError, errs));
          sc=StatusCode::RECOVERABLE;
        }

        if ((data16[n]&0xF) > 11) {
          ATH_MSG_DEBUG("    Header: xxx Error in formatter " << currentLinkIDHash);
          m_headErrorFormatter++;
          ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::FormatterError, errs));
          sc=StatusCode::RECOVERABLE;
        }

        condensedMode = static_cast<bool>(data16[n]&0x100);

        continue;

      } // End header

      // Hit element
      else if (data16[n]&0x8000) {
        if (not foundHeader) {
          ATH_MSG_INFO(" Missing link header in ROD " << std::hex << robID << std::dec);
          foundMissingLinkHeaderError = true;
          m_numMissingLinkHeader++;
          sc = StatusCode::RECOVERABLE;
          continue;
        }

        m_nHits++;
        if (superCondensedMode) {
          // Super-condensed mode:
          // Chip info: 4 bits  data16[n]>>11)&0xF
          // Chip number == (data16[n]>>11)&0x7
          // Chip side == (data16[n]>>14)&0x1
          // For example if (data16[n]>>11)&0xF = 0101 => chip 5 or chip5 on side0, (data16[n]>>11)&0xF = 1101 => chip13 or chip5 on side1
          chip = ((data16[n]>>11)&0x7);
          side = ((data16[n]>>14)&0x1);
          strip = chip*N_STRIPS_PER_CHIP + ((data16[n]>>4)&0x7F);
          timeBin = 0x2; // Assuming timeBin is 010 in super-condensed mode
          nStripsInWord = (data16[n]&0xF)+1;
          if (chip>=N_CHIPS_PER_SIDE) {
            ATH_MSG_DEBUG("    Hit super-condensed : xxx Chip number = " << chip << " >= "<< N_CHIPS_PER_SIDE << " for hit "
                          << std::hex << data16[n]);
            m_chipNumberError++;
            ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ByteStreamParseError, errs));
            sc=StatusCode::RECOVERABLE;
            continue;
          }

          // Search for redundancy only for the master chip
          bool secondSide{false};
          if ((side==1) and ((linkNumber%2)==0)) {
            if (((strip!=oldStrip) or (side!=oldSide)) and (groupSize>0)) {
              // If it is a new cluster, make RDO with the previous cluster
	      errs.noerror(currentLinkIDHash);
              const int rdoMade{makeRDO(oldStrip, groupSize, timeBin, currentLinkIDHash, errors, rdoIDCont, cache, errorHit)};
              if (rdoMade == -1) {
                sc=StatusCode::RECOVERABLE;
                ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ByteStreamParseError, errs));
              }
              else {
                saved[oldSide*N_STRIPS_PER_SIDE+oldStrip] = rdoMade;
              }
              oldStrip = strip;
              oldSide = side;
              groupSize = 0;
            }
            linkNumber++;
            secondSide = true;
          }
          else if ((side==0) and ((linkNumber%2)!=0)) {
            if (((strip!=oldStrip) or (side!=oldSide)) and (groupSize>0)) {
              // If it is a new cluster, make RDO with the previous cluster
	      errs.noerror(currentLinkIDHash);	      
              const int rdoMade{makeRDO(oldStrip, groupSize, timeBin, currentLinkIDHash, errors, rdoIDCont, cache, errorHit)};
              if (rdoMade == -1) {
                sc=StatusCode::RECOVERABLE;
                ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ByteStreamParseError, errs));
              }
              else {
                saved[oldSide*N_STRIPS_PER_SIDE+oldStrip] = rdoMade;
              }
              oldStrip = strip;
              oldSide = side;
              groupSize = 0;
            }
            linkNumber--;
            secondSide = true;
          }
          if (secondSide) {
            onlineID = ((robID & 0xFFFFFF) | (linkNumber << 24));
            currentLinkIDHash = m_cabling->getHashFromOnlineId(onlineID);
            foundHashes.insert(currentLinkIDHash);
          }

          if (groupSize == 0)  {
            oldStrip = strip; // If it's the first super-condensed word
            oldSide = side;
          }

          if ((strip!=oldStrip) or (side!=oldSide)) {
            // If it is a new cluster, make RDO with the previous cluster
	    errs.noerror(currentLinkIDHash);	    
            const int rdoMade{makeRDO(oldStrip, groupSize, timeBin, currentLinkIDHash, errors, rdoIDCont, cache, errorHit)};
            if (rdoMade == -1) {
              sc=StatusCode::RECOVERABLE;
              ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ByteStreamParseError, errs));
            }
            else {
              saved[oldSide*N_STRIPS_PER_SIDE+oldStrip] = rdoMade;
            }
            oldStrip = strip;
            oldSide = side;
            groupSize = 0;
          }
          groupSize+=nStripsInWord; // Split clusters have the same strip number.

        }
        else if (condensedMode) {
          // Super-condensed mode:
          // Chip info: 4 bits  data16[n]>>11)&0xF
          // Chip number == (data16[n]>>11)&0x7
          // Chip side == (data16[n]>>14)&0x1
          // For example if (data16[n]>>11)&0xF = 0101 => chip5 or chip5 on side0, (data16[n]>>11)&0xF = 1101 => chip13 or chip5 on side1
          chip = ((data16[n]>>11)&0x7);
          side = ((data16[n]>>14)&0x1);
          strip = chip*N_STRIPS_PER_CHIP + ((data16[n]>>4)&0x7F);
          timeBin = 0x2; // Assuming timeBin is 010 in condensed mode
          if (chip>=N_CHIPS_PER_SIDE) {
            ATH_MSG_DEBUG("    Hit condensed : xxx Chip number = " << chip << " >= " << N_CHIPS_PER_SIDE << " for hit "
                          << std::hex << data16[n]);
            m_chipNumberError++;
            ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ByteStreamParseError, errs));
            sc=StatusCode::RECOVERABLE;
            continue;
          }

          // Search for redundancy only for the master chip
          bool secondSide{false};
          if ((side==1) and ((linkNumber%2)==0)) {
            if (((strip!=oldStrip) or (side!=oldSide)) and (groupSize>0)) {
              // If it is a new cluster, make RDO with the previous cluster
	      errs.noerror(currentLinkIDHash);
              const int rdoMade{makeRDO(oldStrip, groupSize, timeBin, currentLinkIDHash, errors, rdoIDCont, cache, errorHit)};
              if (rdoMade == -1) {
                sc=StatusCode::RECOVERABLE;
                ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ByteStreamParseError, errs));
              }
              else {
                saved[oldSide*N_STRIPS_PER_SIDE+oldStrip] = rdoMade;
              }
              oldStrip = strip;
              oldSide = side;
              groupSize = 0;
            }
            linkNumber++;
            secondSide = true;
          }
          else if ((side==0) and ((linkNumber%2)!=0)) {
            if (((strip!=oldStrip) or (side!=oldSide)) and (groupSize>0)) {
              // If it is a new cluster, make RDO with the previous cluster
	      errs.noerror(currentLinkIDHash);
              const int rdoMade{makeRDO(oldStrip, groupSize, timeBin, currentLinkIDHash, errors, rdoIDCont, cache, errorHit)};
              if (rdoMade == -1) {
                sc=StatusCode::RECOVERABLE;
                ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ByteStreamParseError, errs));
              }
              else {
                saved[oldSide*N_STRIPS_PER_SIDE+oldStrip] = rdoMade;
              }
              oldStrip = strip;
              oldSide = side;
              groupSize = 0;
            }
            linkNumber--;
            secondSide = true;
          }
          if (secondSide) {
            onlineID = ((robID & 0xFFFFFF) | (linkNumber << 24));
            currentLinkIDHash = m_cabling->getHashFromOnlineId(onlineID);
            foundHashes.insert(currentLinkIDHash);
          }

          if (groupSize == 0)  {
            oldStrip = strip; // If it's the first condensed word
            oldSide = side;
          }

          if (not (data16[n]&0x1)) { // 1-hit
            m_singleCondHitNumber++;
            if ((strip!=oldStrip) or (side!=oldSide)) {
              // If it is a new cluster, make RDO with the previous cluster
	      errs.noerror(currentLinkIDHash);
              const int rdoMade{makeRDO(oldStrip, groupSize, timeBin, currentLinkIDHash, errors, rdoIDCont, cache, errorHit)};
              if (rdoMade == -1) {
                sc=StatusCode::RECOVERABLE;
                ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ByteStreamParseError, errs));
              }
              else {
                saved[oldSide*N_STRIPS_PER_SIDE+oldStrip] = rdoMade;
              }
              oldStrip = strip;
              oldSide = side;
              groupSize = 0;
            }

            if (data16[n]&0x4) { // Error in the hit
              ATH_MSG_DEBUG("    Hit condensed : xxx ERROR in 1-hit " << std::hex << data16[n]);
              errorHit.push_back(groupSize);
              errors = (errors | 0x10);
              m_condHit1Error++;
              sc=StatusCode::RECOVERABLE;
            }
            groupSize = (groupSize>=2 ? groupSize : 1);
          }
          else { // 2-hits
            if (strip >= N_STRIPS_PER_SIDE) {
              ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ByteStreamParseError, errs));
              sc=StatusCode::RECOVERABLE;

              ATH_MSG_DEBUG("Condensed mode - strip number out of range");

              continue;
            }
            m_pairedCondHitNumber++;
            if ((strip!=oldStrip) or (side!=oldSide)) {
              // If it is a new cluster, make RDO with the previous cluster
	      errs.noerror(currentLinkIDHash);
              const int rdoMade{makeRDO(oldStrip, groupSize, timeBin, currentLinkIDHash, errors, rdoIDCont, cache, errorHit)};
              if (rdoMade == -1) {
                sc=StatusCode::RECOVERABLE;
                ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ByteStreamParseError, errs));
              }
              else {
                saved[oldSide*N_STRIPS_PER_SIDE+oldStrip] = rdoMade;
              }
              oldStrip = strip;
              oldSide = side;
              groupSize = 0;
            }
            if (data16[n]&0x4) { // Error in the first hit
              ATH_MSG_DEBUG("    Hit condensed : xxx ERROR in 1st hit" << std::hex << data16[n]);
              errorHit.push_back(groupSize);
              m_condHit1Error++;
              errors = (errors | 0x10);
              sc=StatusCode::RECOVERABLE;
            }
            if (data16[n]&0x8) { // Error in the second hit
              ATH_MSG_DEBUG("    Hit condensed : xxx ERROR in 2nd hit" << std::hex << data16[n]);
              errorHit.push_back(groupSize);
              m_condHit2Error++;
              errors = (errors | 0x20);
              sc=StatusCode::RECOVERABLE;
            }
            groupSize = 2;
          }
        }
        else {
          // Expanded mode:
          // Chip info from the first word of expanded cluster : 4 bits (data16[n]>>11)&0xF
          // Chip number == (data16[n]>>11)&0x7
          // Chip side == (data16[n]>>14)&0x1
          // For example if (data16[n]>>11)&0xF = 0101 => chip5 or chip5 on side0, (data16[n]>>11)&0xF = 1101 => chip13 or chip5 on side1
          if (not (data16[n]&0x8)) {  // 1st hit cluster expanded
            m_firstExpHitNumber++;
            chip = ((data16[n]>>11)&0x7);
            side = ((data16[n]>>14)&0x1);
            strip = chip*N_STRIPS_PER_CHIP + ((data16[n]>>4)&0x7F);
            timeBin = data16[n]&0x7; // Real way for obtaining timeBin info

            if (chip>=N_CHIPS_PER_SIDE) {
              ATH_MSG_DEBUG("Expanded hit: First hit xxx ERROR chip Nb = " << chip << " >= " << N_CHIPS_PER_SIDE);
              m_chipNumberError++;
              ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ByteStreamParseError, errs));
              continue;
            }

            // Search for redundancy only for the master chip
            bool secondSide{false};
            if ((side==1) and ((linkNumber%2)==0))  {
              linkNumber++;
              secondSide = true;
            }
            else if ((side==0) and ((linkNumber%2)!=0)) {
              linkNumber--;
              secondSide = true;
            }
            if (secondSide) {
              onlineID = ((robID & 0xFFFFFF) | (linkNumber << 24));
              currentLinkIDHash = m_cabling->getHashFromOnlineId(onlineID);
              foundHashes.insert(currentLinkIDHash);
            }
            groupSize = 1;
	    errs.noerror(currentLinkIDHash);
            const int rdoMade{makeRDO(strip, groupSize, timeBin, currentLinkIDHash, errors, rdoIDCont, cache, errorHit)};
            if (rdoMade == -1) {
              sc=StatusCode::RECOVERABLE;
              ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ByteStreamParseError, errs));
            }
            else {
              saved[side*N_STRIPS_PER_SIDE+strip] = rdoMade;
            }
            groupSize = 0;
          }
          else { // Next hits cluster expanded
            if (data16[n]&0x80) { // Paired hits
              if (strip >= N_STRIPS_PER_SIDE) {
                ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ByteStreamParseError, errs));
                sc=StatusCode::RECOVERABLE;
                ATH_MSG_DEBUG("Expanded mode - strip number out of range");
                continue;
              }
              m_evenExpHitNumber++;
              if (chip>=N_CHIPS_PER_SIDE) {
                ATH_MSG_DEBUG("Expanded Hit: paired hits xxx ERROR chip Nb = " << chip << " >= " << N_CHIPS_PER_SIDE);
                m_chipNumberError++;
                ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ByteStreamParseError, errs));
                continue;
              }
              // First hit from the pair
              strip++;
              timeBin = data16[n]&0x7;
              groupSize = 1;
	      errs.noerror(currentLinkIDHash);
              const int rdoMade1{makeRDO(strip, groupSize, timeBin, currentLinkIDHash, errors, rdoIDCont, cache, errorHit)};
              if (rdoMade1 == -1) {
                sc=StatusCode::RECOVERABLE;
                ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ByteStreamParseError, errs));
              }
              else {
                saved[side*N_STRIPS_PER_SIDE+strip] = rdoMade1;
              }
              // Second hit from the pair
              strip++;
              timeBin = ((data16[n] >> 4) & 0x7);
	      errs.noerror(currentLinkIDHash);
              const int rdoMade2{makeRDO(strip, groupSize, timeBin, currentLinkIDHash, errors, rdoIDCont, cache, errorHit)};
              if (rdoMade2 == -1) {
                sc=StatusCode::RECOVERABLE;
                ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ByteStreamParseError, errs));
              }
              else {
                saved[side*N_STRIPS_PER_SIDE+strip] = rdoMade2;
              }
              groupSize = 0;
            }
            else { // Last hit of the cluster
              m_lastExpHitNumber++;
              if (chip>=N_CHIPS_PER_SIDE) {
                ATH_MSG_DEBUG("Expanded Hit: last hit xxx ERROR chip Nb = " << chip << " >= " << N_CHIPS_PER_SIDE);
                m_chipNumberError++;
                ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ByteStreamParseError, errs));
                continue;
              }
              strip++;
              timeBin = (data16[n]&0x7);
              groupSize = 1;
	      errs.noerror(currentLinkIDHash);
              const int rdoMade{makeRDO(strip, groupSize, timeBin, currentLinkIDHash, errors, rdoIDCont, cache, errorHit)};
              if (rdoMade == -1) {
                sc=StatusCode::RECOVERABLE;
                ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ByteStreamParseError, errs));
              }
              else {
                saved[side*N_STRIPS_PER_SIDE+strip] = rdoMade;
              }
              groupSize = 0;
            }
          }
        } // End expanded mode
      } // End of hit element

      // FlaggedABCD error
      else if (((data16[n]>>13)&0x7) == 0x0) {
        // 000xxxxxxFFFFEEE
        // 000: FlaggedABCD error: xxxxxxx not used, FFFF: chip, EEE: error code

        if (not foundHeader) {
          ATH_MSG_INFO(" Missing link header in ROD " << std::hex << robID << std::dec);
          foundMissingLinkHeaderError = true;
          m_numMissingLinkHeader++;
          sc = StatusCode::RECOVERABLE;
          continue;
        }

        chip = ((data16[n]>>3)&0xF);
        int abcError{data16[n]&0x7};
        ATH_MSG_DEBUG(" xxx Flagged ABCD ERROR in chip " << chip
                      << " Error code abcError " << abcError << " Link Number (or Stream) "<<linkNumber);
        m_flagErrorBit++;
        // Error code of ABCD error should be 1, 2, 4 or 7.
        if (abcError!=0x1 and abcError!=0x2 and abcError!=0x4 and abcError!=0x7) {
          ATH_MSG_DEBUG("ABCD error has an invalid error code " << abcError
                        << " the 16-bit word is 0x" << std::hex << data16[n] << std::dec
                        << " for hash " << currentLinkIDHash);
          ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ABCDError_Invalid, errs));
        }
        else {
          // Chip is 4 bits. The highest bit 3 represents side. Chip 0-5 on side 0 and chip 8-13 on side 1.
          const unsigned int sideABCDError{static_cast<unsigned int>(chip/8)};
          if (currentLinkIDHash.value()%2!=sideABCDError) {
            // If the sides from the ABCD error and online ID are different,
            // the module is expected to read side 0 via link 1 and side 1 and via link 0.
            // Hash Id is flipped.
            ATH_MSG_DEBUG("ABCD error and online ID have different side information for hash " << currentLinkIDHash << ". "
                          << sideABCDError << " from ABCD error and " << currentLinkIDHash.value()%2 << " from online ID");
            currentLinkIDHash = (currentLinkIDHash.value()/2)*2+sideABCDError;
          }
          // Chip should be 0-5 or 8-13.
          if (chip%8>=N_CHIPS_PER_SIDE) {
            ATH_MSG_DEBUG("ABCD error has an invalid chip 0x" << std::hex << chip << std::dec
                          << " the 16-bit word is 0x" << std::hex << data16[n] << std::dec
                          << " for hash " << currentLinkIDHash.value());
            ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ABCDError_Invalid, errs));
          }
          else {
            if (     abcError==0x1) ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ABCDError_Error1, errs));
            else if (abcError==0x2) ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ABCDError_Error2, errs));
            else if (abcError==0x4) ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ABCDError_Error4, errs));
            else if (abcError==0x7) ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ABCDError_Error7, errs));
            if (     chip%8==0) ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ABCDError_Chip0, errs));
            else if (chip%8==1) ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ABCDError_Chip1, errs));
            else if (chip%8==2) ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ABCDError_Chip2, errs));
            else if (chip%8==3) ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ABCDError_Chip3, errs));
            else if (chip%8==4) ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ABCDError_Chip4, errs));
            else if (chip%8==5) ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ABCDError_Chip5, errs));
          }
        }
        ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ABCDError, errs));
        sc=StatusCode::RECOVERABLE;
        continue;
      }

      // Raw Data
      else if (((data16[n]>>13)&0x7) == 0x3) {
        if (not foundHeader) {
          ATH_MSG_INFO(" Missing link header in ROD " << std::hex << robID << std::dec);
          foundMissingLinkHeaderError = true;
          m_numMissingLinkHeader++;
          sc = StatusCode::RECOVERABLE;
          continue;
        }

        ATH_MSG_DEBUG(" xxx Raw Data Mode " << std::hex << data16[n] << std::dec << ": Config Data Mode ");
        // Too many errors in the BS for the ROD to decode the data
        m_configDataBit++;
        ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::RawError, errs));
        sc=StatusCode::RECOVERABLE;
        continue;
      }

      // Trailer
      else if (((data16[n]>>13)&0x7) == 0x2) {
        foundHeader=false;

        m_trailerNumber++;

        if (data16[n]&0x1000) {
          ATH_MSG_DEBUG("    Trailer: xxx Trailer ERROR " << std::hex << data16[n]);
          m_trailerErrorBit++;
          ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::TrailerError, errs));
          sc=StatusCode::RECOVERABLE;
        }

        if (data16[n]&0x800) {
          // No data should appear between header and trailer
          // See 1.2.2 Formatter FPGA - Serial Data Decoding and Formatting of
          // http://www-eng.lbl.gov/~jmjoseph/Atlas-SiROD/Manuals/usersManual-v164.pdf
          ATH_MSG_DEBUG("    Trailer: xxx Header-Trailer limit ERROR " << std::hex << data16[n]);
          m_trailerErrorLimit++;
          ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::HeaderTrailerLimitError, errs));
          sc=StatusCode::RECOVERABLE;
        }

        if (data16[n]&0x400) {
          // Not sure if there are hit elements before (probably yes but in principle they are fine)
          ATH_MSG_DEBUG("    Trailer: xxx Data Overflow ERROR " << std::hex << data16[n]);
          m_trailerErrorOverflow++;
          ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::TrailerOverflowError, errs));
          sc=StatusCode::RECOVERABLE;
        }
        if (data16[n] & 0xF) {
          // First temporarily masked chip information
          // 0 means no masked chip (always has been 0 until April 2017)
          //
          // If Rx redundacy is not used,
          // 1 means chips 0-5 are temporarily masked.
          // 6 means chip 5 is temporarily masked.
          // 7 means chips 6-11 are temporarily masked.
          // 12 means chip 11 is temporarily masked.
          //
          // If Rx redundacy is used and link-1 is not used,
          // 1 means chips 0-11 are temporarily masked.
          // 6 means chips 5-11 are temporarily masked.
          // 7 means chips 6-11 are temporarily masked.
          // 12 means chip 11 is temporarily masked.
          //
          // If Rx redundacy is used and link-0 is not used,
          // 1 means chips 0-5 are temporarily masked.
          // 6 means chip 5 is temporarily masked.
          // 7 means chips 6-11, 0-5 are temporarily masked.
          // 12 means chips 11, 0-5 are temporarily masked.
          setFirstTempMaskedChip(currentLinkIDHash, (data16[n] & 0xF), errs).ignore();
        }
        continue;
      }

      /// Unknown
      else {
        ATH_MSG_DEBUG("Data word format unknown ");
        m_unknownDataFormat++;
        ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ByteStreamParseError, errs));
        sc=StatusCode::RECOVERABLE;
      }
    } // End of 16-bit word loop
  } // End of 32-bit word loop

  // Create the last RDO of the last link of the event
  if (saved[side*N_STRIPS_PER_SIDE+strip]==false and oldStrip>=0) {
    errs.noerror(currentLinkIDHash);
    const int rdoMade{makeRDO(strip, groupSize, timeBin, currentLinkIDHash, errors, rdoIDCont, cache, errorHit)};
    if (rdoMade == -1) {
      sc=StatusCode::RECOVERABLE;
      ATH_CHECK(addSingleError(currentLinkIDHash, SCT_ByteStreamErrors::ByteStreamParseError, errs));
    }
    else {
      saved[side*N_STRIPS_PER_SIDE+strip] = rdoMade;
    }
  }

  // MissingLinkHeaderError is filled in only FE-lins of the ROD whose headers are not found.
  // We cannot know which FE-link does not have header. However, we should not add the error to found ones.
  if (foundMissingLinkHeaderError) {
    ATH_CHECK(addRODError(robID, SCT_ByteStreamErrors::MissingLinkHeaderError, errs, &foundHashes));
  }
  // auto hashes = rdoIDCont.GetAllCurrentHashes();
  // for ( auto dataID :  hashes ) {
  //   if ( errs.accumulatedErrors.find(dataID) == errs.accumulatedErrors.end()) {
  //     ATH_MSG_WARNING("Have data for ID " << std::hex << dataID << " but errors info missing");
  //   }
  // }

  if (sc.isFailure()) ATH_MSG_DEBUG("One or more ByteStream errors found ");
  return sc;
}

// makeRDO method

int SCT_RodDecoder::makeRDO(int strip, int groupSize, int timeBin,
                            const IdentifierHash& collIDHash,
                            int errors,
                            ISCT_RDO_Container& rdoIDCont,
                            CacheHelper& cache,
                            const std::vector<int>& errorHit) const
{
  if (((strip & 0x7f) + (groupSize-1) >= N_STRIPS_PER_CHIP) or (strip<0) or (strip>=N_STRIPS_PER_SIDE)) {
    ATH_MSG_WARNING("Cluster with " << groupSize << " strips, starting at strip " << strip
                    << " in collection " << collIDHash << " out of range. Will not make RDO");
    return -1;
  }

  // This option is for the trigger, if there is a vecHash* given, test it!
  if (cache.vecHash) {
    if (collIDHash == cache.skipHash) {
      ATH_MSG_VERBOSE("Collection for Hash not to be decoded, skip");
      return 0;
    }
    else if (collIDHash != cache.lastHash) {
      cache.lastHash = collIDHash;
      // Maybe the new hash is not in the list, so test it
      std::vector<IdentifierHash>::const_iterator hashIDIterator{find(cache.vecHash->begin(),
                                                                      cache.vecHash->end(),
                                                                      collIDHash)};
      if (hashIDIterator == cache.vecHash->end()) {
        ATH_MSG_VERBOSE("Collection for Hash not to be decoded, skip");
        // Remember this one, so that we do not find(...) forever
        cache.skipHash = collIDHash;
        return 0;
      }
    }
  }

  if(rdoIDCont.hasExternalCache() and rdoIDCont.tryAddFromCache(collIDHash)){
    ATH_MSG_DEBUG("Hash already in collection - cache hit " << collIDHash);
    return 0;
  }

  // See if strips go from 0 to N_STRIPS_PER_SIDE-1(=767) or vice versa
  if (m_swapPhiReadoutDirection[collIDHash]) {
    strip = N_STRIPS_PER_SIDE-1 - strip;
    strip = strip-(groupSize-1);
  }

  // Get identifier from the hash, this is not nice
  const Identifier collID{m_sctID->wafer_id(collIDHash)};
  const Identifier digitID{m_sctID->strip_id(collID, strip)};
  if (not m_sctID->is_sct(digitID)) {
    ATH_MSG_WARNING("Cluster with invalid Identifier. Will not make RDO");
    return -1;
  }

  const unsigned int rawDataWord{static_cast<unsigned int>(groupSize | (strip << 11) | (timeBin <<22) | (errors << 25))};

  ATH_MSG_DEBUG("Output Raw Data " << std::hex << " Coll " << collID.getString()
                << ":-> " << m_sctID->print_to_string(digitID) << std::dec);



  SCT_RDO_Collection* sctRDOColl{nullptr};
  ATH_CHECK(rdoIDCont.naughtyRetrieve(collIDHash, sctRDOColl), 0); // Returns null if not present

  if (sctRDOColl==nullptr) {
    ATH_MSG_DEBUG(" Collection ID = " << collIDHash << " does not exist, create it ");
    // Create new collection
    sctRDOColl = new SCT_RDO_Collection(collIDHash);
    sctRDOColl->setIdentifier(collID);
    StatusCode sc{rdoIDCont.addCollection(sctRDOColl, collIDHash)};
    ATH_MSG_DEBUG("Adding " << collIDHash);
    if (sc.isFailure()){
      ATH_MSG_ERROR("failed to add SCT RDO collection to container");
    }
  }

  // Now the Collection is there for sure. Create RDO and push it into Collection.
  m_nRDOs++;
  sctRDOColl->push_back(std::make_unique<SCT3_RawData>(digitID, rawDataWord, &errorHit));
  return 1;
}

// addRODError method

StatusCode SCT_RodDecoder::addRODError(uint32_t rodID, SCT_ByteStreamErrors::ErrorType error,
                                       SCT_RodDecoderErrorsHelper& errs,
                                       const std::unordered_set<IdentifierHash>* foundHashes) const
{
  std::vector<IdentifierHash> hashIDs;
  m_cabling->getHashesForRod(hashIDs, rodID);
  for (const IdentifierHash& hash: hashIDs) {
    // MissingLinkHeaderError is filled in only FE-links of the ROD whose headers are not found.
    // We cannot know which FE-link does not have header. However, we should not add the error to found ones.
    if ((error==SCT_ByteStreamErrors::MissingLinkHeaderError) and
        foundHashes and foundHashes->count(hash)) {
      continue;
    }

    ATH_CHECK(addSingleError(hash, error, errs));
  }
  return StatusCode::SUCCESS;
}

// addSingleError method

StatusCode SCT_RodDecoder::addSingleError(const IdentifierHash& hashID,
                                          SCT_ByteStreamErrors::ErrorType error,
                                          SCT_RodDecoderErrorsHelper& errs) const
{
  if (not hashID.is_valid()) {
    ATH_MSG_INFO("addSingleError hashID " << hashID << " is invalid.");
    return StatusCode::SUCCESS;
  }

  errs.add(hashID, error);

  if ((not (error>=SCT_ByteStreamErrors::ABCDError_Chip0 and error<=SCT_ByteStreamErrors::ABCDError_Chip5)) and
      (not (error>=SCT_ByteStreamErrors::TempMaskedChip0 and error<=SCT_ByteStreamErrors::TempMaskedChip5))) {
    std::pair<bool, bool> badLinks{m_configTool->badLinks(hashID)};
    int side{m_sctID->side(m_sctID->wafer_id(hashID))};
    bool result{(side==0 ? badLinks.first : badLinks.second) and (badLinks.first xor badLinks.second)};
    if (result) {
      /// error in a module using RX redundancy - add an error for the other link as well!!
      /// However, ABCDError_Chip0-ABCDError_Chip5 and TempMaskedChip0-TempMaskedChip5 are not common for two links.
      IdentifierHash otherSide;
      m_sctID->get_other_side(hashID, otherSide);
      errs.add(otherSide, error);
      ATH_MSG_DEBUG("Adding error to side " << 1-side << " for module with RX redundancy " << otherSide);
    }
  }

  return StatusCode::SUCCESS;
}

// setFirstTempMaskedChip method

StatusCode SCT_RodDecoder::setFirstTempMaskedChip(const IdentifierHash& hashID,
                                                  unsigned int firstTempMaskedChip,
                                                  SCT_RodDecoderErrorsHelper& errs) const
{
  if (not hashID.is_valid()) {
    ATH_MSG_INFO("setFirstTempMaskedChip hashID " << hashID << " is invalid.");
    return StatusCode::RECOVERABLE;
  }
  if (firstTempMaskedChip==0) {
    ATH_MSG_WARNING("setFirstTempMaskedChip: firstTempMaskedChip should be greater than 0. firstTempMaskedChip is "
                    << firstTempMaskedChip);
    return StatusCode::RECOVERABLE;
  }

  // wafer hash -> wafer id -> module id -> wafer hash on side-0, wafer hash on side-1
  const Identifier waferID{m_sctID->wafer_id(hashID)};
  const Identifier moduleID{m_sctID->module_id(waferID)};

  // Side 0
  IdentifierHash hashSide0;
  m_sctID->get_hash(moduleID, hashSide0, &m_contextSCT);
  unsigned int firstTempMaskedChipSide0{0};
  if (hashID==hashSide0) firstTempMaskedChipSide0 = firstTempMaskedChip;

  // Side 1
  IdentifierHash hashSide1;
  m_sctID->get_other_side(hashSide0, hashSide1);
  unsigned int firstTempMaskedChipSide1{0};
  if (hashID==hashSide1) firstTempMaskedChipSide1 = firstTempMaskedChip;

  int type{0};
  // Check if Rx redundancy is used or not in this module
  const std::pair<bool, bool> badLinks{m_configTool->badLinks(hashID)};
  if (badLinks.first xor badLinks.second) {
    // Rx redundancy is used in this module.
    if (badLinks.first and not badLinks.second) {
      // link-1 is broken
      type = 1;
    }
    else if (badLinks.second and not badLinks.first) {
      // link-0 is broken
      type = 2;
    }
    else if (badLinks.first and badLinks.second) {
      // Both link-0 and link-1 are working
      ATH_MSG_WARNING("setFirstTempMaskedChip: Both link-0 and link-1 are working. But Rx redundancy is used... Why?");
      return StatusCode::RECOVERABLE;
    }
    else {
      // Both link-0 and link-1 are broken
      ATH_MSG_WARNING("setFirstTempMaskedChip: Both link-0 and link-1 are broken. But data are coming... Why?");
      return StatusCode::RECOVERABLE;
    }
  }

  // "Modified" module (using Rx redundancy) case
  // Information of modified modules are found in
  // modified0 and modified1 functions of SCT_ReadoutTool.cxx and
  // Table 3.8 of CERN-THESIS-2008-001 https://cds.cern.ch/record/1078223
  // However, there are two exceptions of the exceptions.
  const unsigned long long fullSerialNumber{m_cabling->getSerialNumberFromHash(hashID).to_ulonglong()};
  if (// Readout through link-0
      fullSerialNumber==20220170200183 or // hash=4662 bec=0 layer=2 eta= 6 phi=39
      fullSerialNumber==20220330200606 or // hash=5032 bec=0 layer=3 eta=-2 phi= 7
      fullSerialNumber==20220330200693    // hash=5554 bec=0 layer=3 eta=-5 phi=29
      ) {
    if (type!=1) ATH_MSG_WARNING("Link-0 is broken but modified module readingout link-0, inconsistent");
    type = 3;

    // An exception:
    // fullSerialNumber=20220170200941, hash=3560 bec=0 layer=1 eta=-6 phi=34
    // This module is a modified one. However, it is not using modified configuration.
    // Readout sequence is 0, 1, 2, 3, 4, 5, 7, 8, 9, 10, 11.
  }
  if (// Readout through link-1
      fullSerialNumber==20220170200653 or // hash=2786 bec=0 layer=1 eta= 4 phi= 1
      fullSerialNumber==20220330200117 or // hash=5516 bec=0 layer=3 eta= 1 phi=27
      fullSerialNumber==20220330200209 or // hash=5062 bec=0 layer=3 eta= 2 phi= 8
      fullSerialNumber==20220330200505 or // hash=5260 bec=0 layer=3 eta= 5 phi=16
      fullSerialNumber==20220330200637 or // hash=4184 bec=0 layer=2 eta=-6 phi=20
      fullSerialNumber==20220330200701    // hash=4136 bec=0 layer=2 eta=-6 phi=18
      ) {
    if (type!=2) ATH_MSG_WARNING("Link-1 is broken but modified module readingout link-1, inconsistent");
    type = 4;

    // Another exception:
    // fullSerialNumber=20220170200113, hash=3428 bec=0 layer=1 eta= 1 phi=28
    // This module is a modified one. However, it is not using modified configuration.
    // Readout sequence is 6, 7, 8, 9, 10, 11, 1, 2, 3, 4, 5.
  }

  static const int chipOrder[5][N_SIDES*N_CHIPS_PER_SIDE]{
    // type=0 not prepared for both link-0 and link-1 are working
    {},
    // type=1 link-1 is broken: chip 0 1 2 3 4 5 6 7 8 9 10 11
    {0, 1, 2, 3,  4,  5, 6, 7, 8,  9, 10, 11},
    // type=2 link-0 is broken: chip 6 7 8 9 10 11 0 1 2 3 4 5
    {6, 7, 8, 9, 10, 11, 0, 1, 2,  3,  4,  5},
    // type=3 "modified" modules and link-1 is broken: chip 0 1 2 3 4 5 7 8 9 10 11 6
    {0, 1, 2, 3,  4,  5, 7, 8, 9, 10, 11,  6},
    // type=4 "modified" modules and link-0 is broken: chip 6 7 8 9 10 11 1 2 3 4 5 0
    {6, 7, 8, 9, 10, 11, 1, 2, 3,  4,  5,  0}
  };

  if (type==0) {
    // Both link-0 and link-1 are working

    // Chips 0-5 are on side 0 and chips 6-11 are on side 1.
    // Normally, modules send hits on side 0 via link-0 and side 1 via link-1.
    // The first temporally masked chip value is the id of the chip that is
    // first masked in the readout chain "plus one".
    // If the value is in between 1 to  6, it indicates side 0.
    // If the value is in between 7 to 12, it indicates side 1.
    // However, some special modules send hits on side 0 via link-1 and hits on
    // side 1 via link-0. If the first masked chip value on side 1 (0) is
    // between 1 to 6 (7 to 12), it indicates the module is a special one.
    // In that case, information is swapped.
    if ((N_CHIPS_PER_SIDE<firstTempMaskedChipSide0 and firstTempMaskedChipSide0<=N_SIDES*N_CHIPS_PER_SIDE) or
        (               0<firstTempMaskedChipSide1 and firstTempMaskedChipSide1<=        N_CHIPS_PER_SIDE)) {
      const unsigned int swapFirstTempMaskedChipSide0{firstTempMaskedChipSide0};
      firstTempMaskedChipSide0 = firstTempMaskedChipSide1;
      firstTempMaskedChipSide1 = swapFirstTempMaskedChipSide0;
    }

    if (firstTempMaskedChipSide0>0) {
      for (unsigned int iChip{firstTempMaskedChipSide0-1}; iChip<N_CHIPS_PER_SIDE; iChip++) {
        ATH_CHECK(addSingleError(hashSide0, SCT_ByteStreamErrors::TempMaskedChipToBit(iChip), errs));
      }
    }
    if (firstTempMaskedChipSide1>N_CHIPS_PER_SIDE) {
      for (unsigned int iChip{firstTempMaskedChipSide1-1}; iChip<N_SIDES*N_CHIPS_PER_SIDE; iChip++) {
        ATH_CHECK(addSingleError(hashSide1, SCT_ByteStreamErrors::TempMaskedChipToBit(iChip-N_CHIPS_PER_SIDE), errs));
      }
    }
  }
  else {
    // type=1, 2, 3, 4: cases using Rx redundancy
    bool toBeMasked{false};
    for (int iChip{0}; iChip<N_SIDES*N_CHIPS_PER_SIDE; iChip++) {
      int jChip{chipOrder[type][iChip]};
      if (jChip==static_cast<int>(firstTempMaskedChip-1)) toBeMasked = true;
      if (toBeMasked) {
        if (jChip<N_CHIPS_PER_SIDE) {
          ATH_CHECK(addSingleError(hashSide0, SCT_ByteStreamErrors::TempMaskedChipToBit(jChip), errs));
        }
        else {
          ATH_CHECK(addSingleError(hashSide1, SCT_ByteStreamErrors::TempMaskedChipToBit(jChip-N_CHIPS_PER_SIDE), errs));
        }
      }
    }
  }

  ATH_MSG_VERBOSE("setFirstTempMaskedChip Hash " << hashID
                  << " SerialNumber " << m_cabling->getSerialNumberFromHash(hashID).str()
                  << " moduleID " << moduleID
                  << " barrel_ec " << m_sctID->barrel_ec(waferID)
                  << " layer_disk " << m_sctID->layer_disk(waferID)
                  << " eta_module " << m_sctID->eta_module(waferID)
                  << " phi_module " << m_sctID->phi_module(waferID)
                  << " side " << m_sctID->side(waferID)
                  << " firstTempMaskedChip " << firstTempMaskedChip);
  return StatusCode::SUCCESS;
}
