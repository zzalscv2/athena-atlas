/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 * Muon Calibration Utilities
 * -----------------------------------------
 *
 * Authors       : Martin Woudstra, Zdenko van Kesteren, Peter Kluit
 * Creation Date: 21 March 2005
 * Updated : 03 December 2007
 * Updated : 16 January  2015 E. Diehl Add BME/BOE/BMG chambers 
 *   Note: BOE=BOL in offline, so do not need to explicitly add BOE, it will be handled as BOL
 ***************************************************************************/


#ifndef MUONCALIBIDENTIFIER_MUONFIXEDLONGID_H
#define MUONCALIBIDENTIFIER_MUONFIXEDLONGID_H

#include "MuonCalibIdentifier/MuonFixedId.h"

// std
#include <iostream>
#include <iomanip>
#include <string>

#include <cstdlib>
#include <limits>

/** @namespace MuonCalib
* namespace to identify classes related only to muon calibration 
*/

namespace MuonCalib {

/** @class MuonFixedLonId
 Implements fixed identifiers not dependent upon Athena Identifier for internal
 use in the calibration framework.
 * Fixed identifiers are encoded words which contain some muon generic fields
 (for the technology, the station, eta and phi) and some technology specific
 fields for Mdt, Csc, Rpc and Tgc. To each field XXX correspond 
 * an offset (a private static constant XXXShift) 
 * a minimum and a maximum storable value (derived from private static constantsXXXMin and XXXMask) 
 * Encoding or decoding of a value num in a given field proceeds via encoding
 * or decoding and index, which is defined as num-XXXMin. 
 * This version implements the identifier in 64 bits, allowing for enough space for NSW hits
 */

class MuonFixedLongId {
public:

   /** constants for technology numbers */
   enum Technology {
     technologyMDT = 0,
     technologyCSC = 1,
     technologyTGC = 2,
     technologyRPC = 3,
     technologyMMG = 4,
     technologySTG = 5,
   };
   
   /** an enum with the station names */
   enum StationName {
     BIL =  1,
     BIS =  2,
     BML =  3,
     BMS =  4,
     BOL =  5,
     BOS =  6,
     BEE =  7,
     BIR =  8,
     BMF =  9,
     BOF = 10,
     BOG = 11,
     BME = 12,
     BIM = 13,
     EIC = 14,
     EIL = 15,
     EEL = 16,
     EES = 17,
     EMC = 18,
     EML = 19,
     EMS = 20,
     EOC = 21,
     EOL = 22,
     EOS = 23,
     EIS = 24,
     T1F = 25,
     T1E = 26,
     T2F = 27,
     T2E = 28,
     T3F = 29,
     T3E = 30,
     T4F = 31,
     T4E = 32,
     CSS = 33,
     CSL = 34,
     BMG = 35,
     MMS = 36,
     MML = 37,
     STS = 38,
     STL = 39,
     INVALID = -1,
   };

   /** an enum with the STGC channel types */
   enum StgChannelType {
     stgChannelPad   = 0,
     stgChannelStrip = 1,
     stgChannelWire  = 2,
   };

   /** default constructor */
   MuonFixedLongId(); 
   /** explicit constructor from an integer which corresponds to a MuonFixedId*/
   explicit MuonFixedLongId( uint64_t id );

   /** for backward compatibility */
   void initFromFixedId(MuonFixedId other);

   /** @return the fixed identifier in integer form */
   uint64_t getIdLong() const;
   /** clear the identifier (=set to invalid) */
   void clear();
   /** check validity of the identifier. @return false if invalid */
   bool isValid() const;

   /** @return true if the identifier corresponds to MDT technology */
   bool is_mdt() const;
   /** @return true if the identifier corresponds to TGC technology */
   bool is_tgc() const;
   /** @return true if the identifier corresponds to CSC technology */
   bool is_csc() const;
   /** @return true if the identifier corresponds to RPC technology */
   bool is_rpc() const;
   /** @return true if the identifier corresponds to MMG technology */
   bool is_mmg() const;
   /** @return true if the identifier corresponds to sTGC technology */
   bool is_stg() const;

   /** set identifier to MDT technology type */
   void set_mdt();
   /** set identifier to TGC technology type */
   void set_tgc();
   /** set identifier to CSC technology type */
   void set_csc();
   /** set identifier to RPC technology type */
   void set_rpc();
   /** set identifier to MMG technology type */
   void set_mmg();
   /** set identifier to sTGC technology type */
   void set_stg();


   /** comparison operator */
   bool operator==( const MuonFixedLongId& rhs ) const;
   /** comparison operator */
   bool operator!=( const MuonFixedLongId& rhs ) const;
   /** comparison operator */
   bool operator<( const MuonFixedLongId& rhs ) const;
   
   /** dump all the information corresponding to the identifier */
   std::ostream& dump(std::ostream& os) const;

//   friend std::istream& operator>>( std::istream& is, MuonCalib::MuonFixedLongId& id );

// Muon generic code

   /** set technology to num in the fixed identifier 
    * @return false if out of range */
   bool setTechnology( Technology num );
   /** set station name  to num in the fixed identifier
    * @return false if out of range */
   bool setStationName( StationName num );
   /** set station eta  to num in the fixed identifier
    * @return false if out of range */
   bool setStationEta( int num );
   /** set station phi  to num in the fixed identifier
    * @return false if out of range */
   bool setStationPhi( int num );

   /** @return technology */
   Technology technology() const;
   /** @return technology string */
   std::string_view technologyString() const;
   /** @return station name */
   StationName stationName() const;
   /** @return station name string */
   std::string_view stationNameString() const;
   /** @return station eta */
   int eta() const;
   /** @return station phi */
   int phi() const;
   
   /** @return the minimum index used to store technology */
   static int technologyMin();
   /** @return the minimum index used to store station name */
   static int stationNameMin();
   /** @return the minimum index used to store station eta */
   static int etaMin();
   /** @return the minimum index used to store station phi */
   static int phiMin();

   /** @return the maximum index used to store technology */
   static int technologyMax();
   /** @return the maximum index used to store station name */
   static int stationNameMax();
   /** @return the maximum index used to store station eta */
   static int etaMax();
   /** @return the maximum index used to store station phi */
   static int phiMax();

   /** @return the fixed station number for a station name 
     * @param[in] station the station name string
     */ 
   static StationName stationStringToFixedStationNumber(std::string_view station);
   /** @return the station name string given the fixed station number 
     * @param[in] station the station integer number 
     */ 
   static std::string_view stationNumberToFixedStationString(StationName station);


//  Mdt specific code

   /** Mdt specific: set tube to num 
    * @return false if out of range */
   bool setMdtTube( int num );
   /** Mdt specific: set layer to num 
    * @return false if out of range */
   bool setMdtTubeLayer( int num );
   /** Mdt specific: set multilayer to num 
    * @return false if out of range */
   bool setMdtMultilayer( int num );

   /** Mdt specific: @return tube number*/
   int mdtTube() const;
   /** Mdt specific: @return layer number*/
   int mdtTubeLayer() const;
   /** Mdt specific: @return multilayer number*/
   int mdtMultilayer() const;
   /** Mdt specific: compute the mezzanine number 
     * @return mezzanine number */
   int mdtMezzanine() const;

   /** Mdt specific: convert channel ID into MDT Chamber ID */
   MuonFixedLongId mdtChamberId() const;

   /** Mdt specific: convert channel ID into MDT Multilayer ID */
   MuonFixedLongId mdtMultilayerId() const;

   /** Mdt specific: @return the minimum index used to store the tube number */ 
   static int mdtTubeMin();
   /** Mdt specific: @return the minimum index used to store the layer number */
   static int mdtTubeLayerMin();
   /** Mdt specific: @return the minimum index used to store the multilayer number */ 
   static int mdtMultilayerMin();

   /** Mdt specific: @return the maximum index used to store the tube number */ 
   static int mdtTubeMax();
   /** Mdt specific: @return the maximum index used to store the layer number */ 
   static int mdtTubeLayerMax();
   /** Mdt specific: @return the maximum index used to store the multilayer number */ 
   static int mdtMultilayerMax();

   
//  Csc specific code

   /** Csc specific: set chamber layer to num
    * @return false if out of range */
   bool setCscChamberLayer( int num );
   /** Csc specific: set wire layer to num 
    * @return false if out of range */
   bool setCscWireLayer( int num );
   /** Csc specific: set measures_phi to num 
    * @return false if out of range */
   bool setCscMeasuresPhi( int num );
   /** Csc specific: set strip to num 
    * @return false if out of range */
   bool setCscStrip( int num );  

   /** Csc specific: @return chamber layer*/
   int cscChamberLayer() const;
   /** Csc specific: @return wire layer*/
   int cscWireLayer() const;
   /** Csc specific: @return measures_phi */
   int cscMeasuresPhi() const;
   /** Csc specific: @return strip */
   int cscStrip() const;

   /** Csc specific: @return the minimum index used to store the chamber layer*/
   static int cscChamberLayerMin(); 
   /** Csc specific: @return the minimum index used to store the wire layer*/
   static int cscWireLayerMin(); 
   /** Csc specific: @return the minimum index used to store measures_phi */
   static int cscMeasuresPhiMin(); 
   /** Csc specific: @return the minimum index used to store the strip number */
   static int cscStripMin();
   
   /** Csc specific: @return the maximum index used to store the chamber layer*/
   static int cscChamberLayerMax(); 
   /** Csc specific: @return the maximum index used to store the wire layer*/
   static int cscWireLayerMax(); 
   /** Csc specific: @return the maximum index used to store measures_phi */
   static int cscMeasuresPhiMax(); 
   /** Csc specific: @return the maximum index used to store the strip number */
   static int cscStripMax();

//  Rpc specific code 

   /** Rpc specific: set doublet_R to num 
    * @return false if out of range */
   bool setRpcDoubletR( int num );
   /** Rpc specific: set doublet_Z to num 
    * @return false if out of range */
   bool setRpcDoubletZ( int num );
   /** Rpc specific: set doublet_Phi to num 
    * @return false if out of range */
   bool setRpcDoubletPhi( int num );
   /** Rpc specific: set gas_gap to num 
    * @return false if out of range */
   bool setRpcGasGap( int num );
   /** Rpc specific: set measures_phi to num 
    * @return false if out of range */
   bool setRpcMeasuresPhi( int num );
   /** Rpc specific: set strip to num 
    * @return false if out of range */
   bool setRpcStrip( int num );
   
   /** Rpc specific:
    * @return doublet_R */
   int rpcDoubletR() const;
   /** Rpc specific:
    * @return doublet_Z */
   int rpcDoubletZ() const;
   /** Rpc specific:
    * @return doublet_Phi */
   int rpcDoubletPhi() const;
   /** Rpc specific:
    * @return gas_gap */
   int rpcGasGap() const;
   /** Rpc specific:
    * @return measures_phi */
   int rpcMeasuresPhi() const;
   /** Rpc specific:
    * @return strip */
   int rpcStrip() const;

//  Tgc specific code

   /** Tgc specific: set gas_gap to num 
    * @return false if out of range */
   bool setTgcGasGap( int num );
   /** Tgc specific: set is_strip to num 
    * @return false if out of range */
   bool setTgcIsStrip( int num );
   /** Tgc specific: set channel to num 
    * @return false if out of range */
   bool setTgcChannel( int num );

   /** Tgc specific:
    * @return gas_gap */
   int tgcGasGap() const;
   /** Tgc specific:
    * @return is_strip */
   int tgcIsStrip() const;
   /** Tgc specific:
    * @return channel */
   int tgcChannel() const;

//  Mmg specific code

   /** Mmg specific: set multilayer to num 
    * @return false if out of range */
   bool setMmgMultilayer( int num );
   /** Mmg specific: set gas_gap to num 
    * @return false if out of range */
   bool setMmgGasGap( int num );
   /** Mmg specific: set strip to num 
    * @return false if out of range */
   bool setMmgStrip( int num );  

   /** Mmg specific:
    * @return multilayer number*/
   int mmgMultilayer() const;
   /** Mmg specific:
    * @return gas_gap */
   int mmgGasGap() const;
   /** Mmg specific: @return strip */
   int mmgStrip() const;
   /** Mmg specific: check if layer is stereo */
   bool mmgIsStereo() const;

// Stg specific code

   /** Stg specific: set multilayer to num 
    * @return false if out of range */
   bool setStgMultilayer( int num );
   /** Stg specific: set gas_gap to num 
    * @return false if out of range */
   bool setStgGasGap( int num );
   /** Stg specific: set channel_type to num 
    * @return false if out of range */
   bool setStgChannelType( StgChannelType num );
   /** Stg specific: set channel to num 
    * @return false if out of range */
   bool setStgChannel( int num );

   /** Stg specific:
    * @return multilayer number*/
   int stgMultilayer() const;
   /** Stg specific:
    * @return gas_gap */
   int stgGasGap() const;
   /** Stg specific:
    * @return channel_type */
   StgChannelType stgChannelType() const;
   /** Stg specific:
    * @return channel */
   int stgChannel() const;


private:
   // Private member functions

// Muon generic code

   /** set technology index in the fixed identifier 
    * @return false if out of range */
   bool setTechnologyIndex( uint64_t idx );
   /** set station name index in the fixed identifier 
    * @return false if out of range */
   bool setStationNameIndex( uint64_t idx );
   /** set station eta index in the fixed identifier 
    * @return false if out of range */
   bool setStationEtaIndex( uint64_t idx );
   /** set station phi index in the fixed identifier 
    * @return false if out of range */
   bool setStationPhiIndex( uint64_t idx );

   /** @return technology index */
   unsigned int technologyIndex() const;
   /** @return station name index */
   unsigned int stationNameIndex() const;
   /** @return station eta index */
   unsigned int etaIndex() const;
   /** @return station phi index */
   unsigned int phiIndex() const;

//  Mdt specific code

   /** Mdt specific: set index idx for tube 
    * @return false if out of range */
   bool setMdtTubeIndex( uint64_t idx ) ;
   /** Mdt specific: set index idx for layer 
    * @return false if out of range */
   bool setMdtTubeLayerIndex( uint64_t idx ) ;
   /** Mdt specific: set index idx for multilayer 
    * @return false if out of range */
   bool setMdtMultilayerIndex( uint64_t idx ) ;

   /** Mdt specific: @return tube index */
   unsigned int mdtTubeIndex() const;
   /** Mdt specific: @return layer index */
   unsigned int mdtTubeLayerIndex() const;
   /** Mdt specific: @return multilayer index */
   unsigned int mdtMultilayerIndex() const;

//  Csc specific code

   /** Csc specific: set chamber layer index to idx
    * @return false if out of range */
   bool setCscChamberLayerIndex( uint64_t idx );
   /** Csc specific: set wire layer index to idx 
    * @return false if out of range */
   bool setCscWireLayerIndex( uint64_t idx );
   /** Csc specific: set measures_phi index to idx
    * @return false if out of range */
   bool setCscMeasuresPhiIndex( uint64_t idx );
   /** Csc specific: set strip index to idx 
    * @return false if out of range */
   bool setCscStripIndex( uint64_t idx );

   /** Csc specific: @return chamber layer index */ 
   unsigned int cscChamberLayerIndex() const;
   /** Csc specific: @return wire layer index */ 
   unsigned int cscWireLayerIndex() const;
   /** Csc specific: @return measures_phi index */ 
   unsigned int cscMeasuresPhiIndex() const;
   /** Csc specific: @return strip index */ 
   unsigned int cscStripIndex() const;

//  Rpc specific code 

   /** Rpc specific: set index for doublet_R to idx
    * @return false if out of range */
   bool setRpcDoubletRIndex( uint64_t idx );
   /** Rpc specific: set index for doublet_Z to idx
    * @return false if out of range */
   bool setRpcDoubletZIndex( uint64_t idx );
   /** Rpc specific: set index for doublet_Phi to idx
    * @return false if out of range */
   bool setRpcDoubletPhiIndex( uint64_t idx );
   /** Rpc specific: set index for gas_gap to idx
    * @return false if out of range */
   bool setRpcGasGapIndex( uint64_t idx );
   /** Rpc specific: set index for measures_phi to idx
    * @return false if out of range */
   bool setRpcMeasuresPhiIndex( uint64_t idx );
   /** Rpc specific: set index for strip to idx
    * @return false if out of range */
   bool setRpcStripIndex( uint64_t idx );
   
   /** Rpc specific:
    * @return doublet_R index*/
   unsigned int rpcDoubletRIndex() const;
   /** Rpc specific:
    * @return doublet_Z index*/
   unsigned int rpcDoubletZIndex() const;
   /** Rpc specific:
    * @return doublet_Phi index*/
   unsigned int rpcDoubletPhiIndex() const;
   /** Rpc specific:
    * @return gas_gap index*/
   unsigned int rpcGasGapIndex() const;
   /** Rpc specific:
    * @return measures_phi index*/
   unsigned int rpcMeasuresPhiIndex() const;
   /** Rpc specific:
    * @return strip index*/
   unsigned int rpcStripIndex() const;

//  Tgc specific code

   /** Tgc specific: set index for gas_gap to idx
    * @return false if out of range */
   bool setTgcGasGapIndex( uint64_t idx );  
   /** Tgc specific: set index for is_strip to idx
    * @return false if out of range */
   bool setTgcIsStripIndex( uint64_t idx );
   /** Tgc specific: set index for channel to idx
    * @return false if out of range */
   bool setTgcChannelIndex( uint64_t idx );  

   /** Tgc specific:
    * @return gas_gap index */
   unsigned int tgcGasGapIndex() const;
   /** Tgc specific:
    * @return is_strip index */
   unsigned int tgcIsStripIndex() const;
   /** Tgc specific:
    * @return channel index */
   unsigned int tgcChannelIndex() const;

//  Mmg specific code

   /** Mmg specific: set index idx for multilayer 
    * @return false if out of range */
   bool setMmgMultilayerIndex( uint64_t idx ) ;
   /** Mmg specific: set index for gas_gap to idx
    * @return false if out of range */
   bool setMmgGasGapIndex( uint64_t idx );
   /** Mmg specific: set strip index to idx 
    * @return false if out of range */
   bool setMmgStripIndex( uint64_t idx );

   /** Mmg specific: @return multilayer index */
   unsigned int mmgMultilayerIndex() const;
   /** Mmg specific:
    * @return gas_gap index*/
   unsigned int mmgGasGapIndex() const;
   /** Mmg specific: @return strip index */ 
   unsigned int mmgStripIndex() const;

// Stg specific code

   /** Stg specific: set index idx for multilayer 
    * @return false if out of range */
   bool setStgMultilayerIndex( uint64_t idx ) ;
   /** Stg specific: set index for gas_gap to idx
    * @return false if out of range */
   bool setStgGasGapIndex( uint64_t idx );
   /** Stg specific: set index for channel_type to idx
    * @return false if out of range */
   bool setStgChannelTypeIndex( uint64_t idx );  
   /** Stg specific: set index for channel to idx
    * @return false if out of range */
   bool setStgChannelIndex( uint64_t idx );  

   /** Stg specific: @return multilayer index */
   unsigned int stgMultilayerIndex() const;
   /** Stg specific:
    * @return gas_gap index*/
   unsigned int stgGasGapIndex() const;
   /** Stg specific:
    * @return channel_type index */
   unsigned int stgChannelTypeIndex() const;   
   /** Stg specific:
    * @return channel index */
   unsigned int stgChannelIndex() const;   



   // the member holding the packed fields
   uint64_t m_data;

   static const uint64_t kInvalidData = 0xFFFFFFFFFFFFFFFF;

//   Muon generic

   static const uint64_t     kTechnologyMask   = 15;
   static const unsigned int kTechnologyShift  = 60;
   static const int          kTechnologyMin    = 0;
   
   static const int kNumberOfTechnologies = 6;
   static const char kTechnologyStrings[kNumberOfTechnologies][4];

   static const uint64_t     kStationNameMask      = 127;
   static const unsigned int kStationNameShift     = 53;
   static const int          kStationNameMin       = 1;

   static const uint64_t     kStationEtaMask          = 63;
   static const unsigned int kStationEtaShift         = 47;
   static const int          kStationEtaMin           = -31;

   static const uint64_t     kStationPhiMask          = 127;
   static const unsigned int kStationPhiShift         = 40;
   static const int          kStationPhiMin           = 1;

   static const int kNumberOfStationNames = 39;
   static const char kStationNameStrings[kNumberOfStationNames][4];

   // the full station information for making a station identifier
   static const unsigned int kStationShift = kStationPhiShift; // the smallest shift
   static const uint64_t     kStationMask = 
     ( (kTechnologyMask  << kTechnologyShift)  |
       (kStationNameMask << kStationNameShift) |
       (kStationEtaMask  << kStationEtaShift)  |
       (kStationPhiMask  << kStationPhiShift)    ) >> kStationShift; // kStationShift for consistent meaning

//   Mdt specific code 

   static const uint64_t     kMdtMultilayerMask    = 7;
   static const unsigned int kMdtMultilayerShift   = 14;
   static const int          kMdtMultilayerMin     = 1;

   static const uint64_t     kMdtTubeLayerMask     = 15;
   static const unsigned int kMdtTubeLayerShift    = 10;
   static const int          kMdtTubeLayerMin      = 1;

   static const uint64_t     kMdtTubeMask          = 1023;
   static const unsigned int kMdtTubeShift         = 0;
   static const int          kMdtTubeMin           = 1;

//  Csc specific code

   static const uint64_t     kCscChamberLayerMask  = 7;
   static const unsigned int kCscChamberLayerShift = 15;
   static const int          kCscChamberLayerMin   = 1;

   static const uint64_t     kCscWireLayerMask     = 15;
   static const unsigned int kCscWireLayerShift    = 11;
   static const int          kCscWireLayerMin      = 1;

   static const uint64_t     kCscMeasuresPhiMask   = 1;
   static const unsigned int kCscMeasuresPhiShift  = 10;
   static const int          kCscMeasuresPhiMin    = 0;

   static const uint64_t     kCscStripMask         = 1023;
   static const unsigned int kCscStripShift        = 0;
   static const int          kCscStripMin          = 1;

//  Rpc specific code

   static const uint64_t     kRpcDoubletRMask      = 3;
   static const unsigned int kRpcDoubletRShift     = 20; 
   static const int          kRpcDoubletRMin       = 1;

   static const uint64_t     kRpcDoubletZMask      = 15;
   static const unsigned int kRpcDoubletZShift     = 16; 
   static const int          kRpcDoubletZMin       = 1;

   static const uint64_t     kRpcDoubletPhiMask    = 3;
   static const unsigned int kRpcDoubletPhiShift   = 14; 
   static const int          kRpcDoubletPhiMin     = 1;

   static const uint64_t     kRpcGasGapMask        = 3;
   static const unsigned int kRpcGasGapShift       = 12;  
   static const int          kRpcGasGapMin         = 1;

   static const uint64_t     kRpcMeasuresPhiMask   = 3;
   static const unsigned int kRpcMeasuresPhiShift  = 10;  
   static const int          kRpcMeasuresPhiMin    = 0;

   static const uint64_t     kRpcStripMask         = 1023;
   static const unsigned int kRpcStripShift        = 0; 
   static const int          kRpcStripMin          = 1;

// Tgc specific code 
   static const uint64_t     kTgcGasGapMask        = 7;
   static const unsigned int kTgcGasGapShift       = 11;
   static const int          kTgcGasGapMin         = 1;

   static const uint64_t     kTgcIsStripMask       = 1;
   static const unsigned int kTgcIsStripShift      = 10;
   static const int          kTgcIsStripMin        = 0;

   static const uint64_t     kTgcChannelMask       = 1023;
   static const unsigned int kTgcChannelShift      = 0;
   static const int          kTgcChannelMin        = 1;

// MM specific code
   static const uint64_t     kMmgMultilayerMask    = 7;
   static const unsigned int kMmgMultilayerShift   = 19;
   static const int          kMmgMultilayerMin     = 1;

   static const uint64_t     kMmgGasGapMask        = 15;
   static const unsigned int kMmgGasGapShift       = 15;
   static const int          kMmgGasGapMin         = 1;

   static const uint64_t     kMmgStripMask         = 32767;
   static const unsigned int kMmgStripShift        = 0;
   static const int          kMmgStripMin          = 0;

// sTGC specific code
   static const uint64_t     kStgMultilayerMask    = 7;
   static const unsigned int kStgMultilayerShift   = 21;
   static const int          kStgMultilayerMin     = 1;

   static const uint64_t     kStgGasGapMask        = 15;
   static const unsigned int kStgGasGapShift       = 17;
   static const int          kStgGasGapMin         = 1;

   static const uint64_t     kStgChannelTypeMask   = 3;
   static const unsigned int kStgChannelTypeShift  = 15;
   static const int          kStgChannelTypeMin    = 0;

   static const uint64_t     kStgChannelMask       = 32767;
   static const unsigned int kStgChannelShift      = 0;
   static const int          kStgChannelMin        = 0;
};

// Muon generic methods

inline MuonFixedLongId::MuonFixedLongId()
  : m_data(kInvalidData){
  }

inline MuonFixedLongId::MuonFixedLongId( uint64_t id ) {
  if (id <= std::numeric_limits<uint32_t>::max()) {
    // we were passed a 32 bit identifier, import values from MuonFixedId
    initFromFixedId(MuonFixedId(static_cast<uint32_t>(id)));
  } else {
    m_data = id;
  }
}

inline uint64_t MuonFixedLongId::getIdLong() const{
  return m_data;
}
/* void MuonFixedLongId::setMuonIdentifier( int id ){ */
/*   if( id>-1 || id<kInvalidData ) m_data = id; */
/*   else  m_data = kInvalidData; */
/* } */

inline void MuonFixedLongId::clear() {
   m_data = kInvalidData;
}

inline bool MuonFixedLongId::isValid() const {
  return m_data != kInvalidData;
}

inline bool MuonFixedLongId::is_mdt() const{
  return isValid() && technology() == Technology::technologyMDT;
}

inline bool MuonFixedLongId::is_tgc() const{
  return isValid() && technology() == Technology::technologyTGC;
}

inline bool MuonFixedLongId::is_csc() const{
  return isValid() && technology() == Technology::technologyCSC;
}

inline bool MuonFixedLongId::is_rpc() const{
  return isValid() && technology() == Technology::technologyRPC;
}

inline bool MuonFixedLongId::is_mmg() const{
  return isValid() && technology() == Technology::technologyMMG;
}

inline bool MuonFixedLongId::is_stg() const{
  return isValid() && technology() == Technology::technologySTG;
}

inline void MuonFixedLongId::set_mdt() {
  setTechnology( Technology::technologyMDT );
}

inline void MuonFixedLongId::set_csc() {
  setTechnology( Technology::technologyCSC );
}

inline void MuonFixedLongId::set_tgc() {
  setTechnology( Technology::technologyTGC );
}

inline void MuonFixedLongId::set_rpc() {
  setTechnology( Technology::technologyRPC );
}

inline void MuonFixedLongId::set_mmg() {
  setTechnology( Technology::technologyMMG );
}

inline void MuonFixedLongId::set_stg() {
  setTechnology( Technology::technologySTG );
}

inline bool MuonFixedLongId::operator==( const MuonFixedLongId& rhs ) const {
   return m_data == rhs.m_data;
}

inline bool MuonFixedLongId::operator!=( const MuonFixedLongId& rhs ) const {
   return m_data != rhs.m_data;
}

inline bool MuonFixedLongId::operator<( const MuonFixedLongId& rhs ) const {
   return m_data < rhs.m_data;
}

inline bool MuonFixedLongId::setTechnologyIndex( uint64_t idx ) { 
  if ( idx > kTechnologyMask ) {
      clear();
      return false;
   }
   m_data &= ~(kTechnologyMask << kTechnologyShift);
   m_data |= (idx & kTechnologyMask) << kTechnologyShift;
   return true;
}

inline bool MuonFixedLongId::setTechnology( Technology num ) { 
   return setTechnologyIndex( num - kTechnologyMin );
}

inline unsigned int MuonFixedLongId::technologyIndex() const { 
   return (m_data >> kTechnologyShift) & kTechnologyMask;
}

inline MuonFixedLongId::Technology MuonFixedLongId::technology() const {
   return static_cast<Technology>(technologyIndex() + kTechnologyMin);
}

inline std::string_view MuonFixedLongId::technologyString() const {
  int index = technologyIndex();
  if ( index >= 0 && index < kNumberOfTechnologies ) {
    return kTechnologyStrings[index];
  }
  return "???";
}

inline bool MuonFixedLongId::setStationNameIndex( uint64_t idx ) {
  if ( idx > kStationNameMask ) {
      clear();
      return false;
   }
   m_data &= ~(kStationNameMask << kStationNameShift);
   m_data |= (idx & kStationNameMask) << kStationNameShift;
   return true;
}

inline bool MuonFixedLongId::setStationName( StationName num ) {
   return setStationNameIndex( num - kStationNameMin );
}

inline unsigned int MuonFixedLongId::stationNameIndex() const {
   return (m_data >> kStationNameShift) & kStationNameMask;
}

inline MuonFixedLongId::StationName MuonFixedLongId::stationName() const {
   return static_cast<StationName>(stationNameIndex() + kStationNameMin);
}

inline std::string_view MuonFixedLongId::stationNameString() const {
  int index = stationNameIndex();
  if ( index >= 0 && index < kNumberOfStationNames ) {
    return kStationNameStrings[index];
  } 
  return "XXX";    
}

inline bool MuonFixedLongId::setStationEtaIndex( uint64_t idx ) {
  if ( idx > kStationEtaMask ) {
      clear();
      return false;
   }
   m_data &= ~(kStationEtaMask << kStationEtaShift);
   m_data |= (idx & kStationEtaMask) << kStationEtaShift;
   return true;
}

inline bool MuonFixedLongId::setStationEta( int num ) {
   return setStationEtaIndex( num - kStationEtaMin );
}

inline unsigned int MuonFixedLongId::etaIndex() const {
   return (m_data >> kStationEtaShift) & kStationEtaMask;
}

inline int MuonFixedLongId::eta() const {
   return etaIndex() + kStationEtaMin;
}


inline bool MuonFixedLongId::setStationPhiIndex( uint64_t idx ) {
  if ( idx > kStationPhiMask ) {
      clear();
      return false;
   }
   m_data &= ~(kStationPhiMask << kStationPhiShift);
   m_data |= (idx & kStationPhiMask) << kStationPhiShift;
   return true;
}

inline bool MuonFixedLongId::setStationPhi( int num ) {
   return setStationPhiIndex( num - kStationPhiMin );
}

inline unsigned int MuonFixedLongId::phiIndex() const {
   return (m_data >> kStationPhiShift) & kStationPhiMask;
}

inline int MuonFixedLongId::phi() const {
   return phiIndex() + kStationPhiMin;
}
  
inline int MuonFixedLongId::technologyMin(){
  return kTechnologyMin;
}

inline int MuonFixedLongId::stationNameMin(){
  return kStationNameMin;
}

inline int MuonFixedLongId::etaMin(){ 
  return kStationEtaMin;
}

inline int MuonFixedLongId::phiMin(){
  return kStationPhiMin;
}

inline int MuonFixedLongId::technologyMax(){
  return kTechnologyMin + kTechnologyMask;
}

inline int MuonFixedLongId::stationNameMax(){ 
  return kStationNameMin + kStationNameMask;
}

inline int MuonFixedLongId::etaMax(){ 
  return kStationEtaMin + kStationEtaMask;
}

inline int MuonFixedLongId::phiMax(){
   return kStationPhiMin + kStationPhiMask; 
}

inline MuonFixedLongId::StationName MuonFixedLongId::stationStringToFixedStationNumber(std::string_view station ) {
  for ( int i = 0; i < kNumberOfStationNames; ++i ) {
    if ( station == kStationNameStrings[i] ) return static_cast<StationName>(i + kStationNameMin);
  }
  return StationName::INVALID;  // signal error if not found
}

inline std::string_view MuonFixedLongId::stationNumberToFixedStationString(StationName station) {
  int index = station - kStationNameMin;
  if ( index >= 0 && index < kNumberOfStationNames ) {
    return kStationNameStrings[index];
  }
  return "XXX";   // signal error if not found
}

// Mdt specific methods

inline bool MuonFixedLongId::setMdtTubeIndex( uint64_t idx ) {
  if ( idx > kMdtTubeMask ) {
      clear();
      return false;
   }
   m_data &= ~(kMdtTubeMask << kMdtTubeShift);
   m_data |= (idx & kMdtTubeMask) << kMdtTubeShift;
   return true;
}

inline bool MuonFixedLongId::setMdtTube( int num ) {
   return setMdtTubeIndex( num - kMdtTubeMin );
}

inline unsigned int MuonFixedLongId::mdtTubeIndex() const {
   return (m_data >> kMdtTubeShift) & kMdtTubeMask;
}

inline int MuonFixedLongId::mdtTube() const {
   return mdtTubeIndex() + kMdtTubeMin;
}

inline int MuonFixedLongId::mdtMezzanine() const {
   StationName Ichamber =  stationName() ;
   int Ieta = eta() ;
   int Iphi = phi() ;
   int Iml  = mdtMultilayer() ;
   int Itube = mdtTube() ;
   if (Ieta < 0 ) Ieta = -Ieta*10 ;
   int ImezzTubes = 8 ;
   if (Ichamber==StationName::BIL || Ichamber==StationName::BIS || Ichamber==StationName::BIR
       || Ichamber==StationName::BIM || Ichamber==StationName::BEE || Ichamber==StationName::EIL
       || Ichamber==StationName::EIS) {
     ImezzTubes=6;
   }
//exception BIS eta=8 is a 3-layer chamber
   if(Ichamber==StationName::BIS && std::abs(Ieta)==8) ImezzTubes=8;
   int Imezz = ((Itube-1)/ImezzTubes)*2+(Iml-1) ;
   Imezz = 9*100000000 + Ichamber*1000000 + Ieta*10000 + Iphi*100 + Imezz ;
   return Imezz ;
}

inline bool MuonFixedLongId::setMdtTubeLayerIndex( uint64_t idx ) {
  if ( idx > kMdtTubeLayerMask ) {
      clear();
      return false;
   }
   m_data &= ~(kMdtTubeLayerMask << kMdtTubeLayerShift);
   m_data |= (idx & kMdtTubeLayerMask) << kMdtTubeLayerShift;
   return true;
}

inline bool MuonFixedLongId::setMdtTubeLayer( int num ) {
   return setMdtTubeLayerIndex( num - kMdtTubeLayerMin );
}

inline unsigned int MuonFixedLongId::mdtTubeLayerIndex() const {
   return (m_data >> kMdtTubeLayerShift) & kMdtTubeLayerMask;
}

inline int MuonFixedLongId::mdtTubeLayer() const {
   return mdtTubeLayerIndex() + kMdtTubeLayerMin;
}

inline bool MuonFixedLongId::setMdtMultilayerIndex( uint64_t idx ) {
  if ( idx > kMdtMultilayerMask ) {
      clear();
      return false;
   }
   m_data &= ~(kMdtMultilayerMask << kMdtMultilayerShift);
   m_data |= (idx & kMdtMultilayerMask) << kMdtMultilayerShift;
   return true;
}

inline bool MuonFixedLongId::setMdtMultilayer( int num ) {
   return setMdtMultilayerIndex( num - kMdtMultilayerMin );
}

inline unsigned int MuonFixedLongId::mdtMultilayerIndex() const {
   return (m_data >> kMdtMultilayerShift) & kMdtMultilayerMask;
}

inline int MuonFixedLongId::mdtMultilayer() const {
   return mdtMultilayerIndex() + kMdtMultilayerMin;
}

inline int MuonFixedLongId::mdtTubeMin(){
  return kMdtTubeMin;
}

inline int MuonFixedLongId::mdtTubeMax(){
  return kMdtTubeMin + kMdtTubeMask;
}

inline int MuonFixedLongId::mdtTubeLayerMin(){
  return kMdtTubeLayerMin;
}

inline int MuonFixedLongId::mdtTubeLayerMax(){
  return kMdtTubeLayerMin + kMdtTubeLayerMask;
}

inline int MuonFixedLongId::mdtMultilayerMin(){
  return kMdtMultilayerMin;
}

inline int MuonFixedLongId::mdtMultilayerMax(){
  return kMdtMultilayerMin + kMdtMultilayerMask;
}

inline MuonFixedLongId MuonFixedLongId::mdtChamberId() const {
  // mdt chamber id = muon station id
  return MuonFixedLongId( m_data & (kStationMask << kStationShift) );
}

inline MuonFixedLongId MuonFixedLongId::mdtMultilayerId() const {
  // mdt multilayer id = muon station id + multilayer field
  return MuonFixedLongId( m_data & ( (kStationMask << kStationShift) | (kMdtMultilayerMask << kMdtMultilayerShift) ) );
}


// Csc specific methods

inline bool MuonFixedLongId::setCscChamberLayerIndex( uint64_t idx ) {
  if ( idx > kCscChamberLayerMask ) {
      clear();
      return false;
   }
   m_data &= ~( kCscChamberLayerMask <<  kCscChamberLayerShift );
   m_data |= (idx &  kCscChamberLayerMask) << kCscChamberLayerShift;
   return true;
}

inline bool MuonFixedLongId::setCscChamberLayer( int num ) {
   return setCscChamberLayerIndex( num -  kCscChamberLayerMin );
}

inline unsigned int MuonFixedLongId::cscChamberLayerIndex() const {
   return (m_data >> kCscChamberLayerShift) & kCscChamberLayerMask;
}

inline int MuonFixedLongId::cscChamberLayer() const {
   return cscChamberLayerIndex() + kCscChamberLayerMin;
}


inline bool MuonFixedLongId::setCscWireLayerIndex( uint64_t idx ) {
  if ( idx > kCscWireLayerMask ) {
      clear();
      return false;
   }
   m_data &= ~( kCscWireLayerMask <<  kCscWireLayerShift );
   m_data |= (idx &  kCscWireLayerMask) << kCscWireLayerShift;
   return true;
}

inline bool MuonFixedLongId::setCscWireLayer( int num ) {
  return setCscWireLayerIndex( num -  kCscWireLayerMin );
}

inline unsigned int MuonFixedLongId::cscWireLayerIndex() const {
   return (m_data >> kCscWireLayerShift) & kCscWireLayerMask;
}

inline int MuonFixedLongId::cscWireLayer() const {
   return cscWireLayerIndex() + kCscWireLayerMin;
}

inline bool MuonFixedLongId::setCscMeasuresPhiIndex( uint64_t idx ) {
  if ( idx > kCscMeasuresPhiMask ) {
      clear();
      return false;
   }
   m_data &= ~( kCscMeasuresPhiMask <<  kCscMeasuresPhiShift );
   m_data |= (idx &  kCscMeasuresPhiMask) << kCscMeasuresPhiShift;
   return true;
}

inline bool MuonFixedLongId::setCscMeasuresPhi( int num ) {
  return setCscMeasuresPhiIndex( num -  kCscMeasuresPhiMin );
}

inline unsigned int MuonFixedLongId::cscMeasuresPhiIndex() const {
   return (m_data >> kCscMeasuresPhiShift) & kCscMeasuresPhiMask;
}

inline int MuonFixedLongId::cscMeasuresPhi() const {
   return cscMeasuresPhiIndex() + kCscMeasuresPhiMin;
}

inline bool MuonFixedLongId::setCscStripIndex( uint64_t idx ) {
  if ( idx > kCscStripMask ) {
      clear();
      return false;
   }
   m_data &= ~( kCscStripMask <<  kCscStripShift );
   m_data |= (idx &  kCscStripMask) << kCscStripShift;
   return true;
}

inline bool MuonFixedLongId::setCscStrip( int num ) {
  return setCscStripIndex( num -  kCscStripMin );
}

inline unsigned int MuonFixedLongId::cscStripIndex() const {
   return (m_data >> kCscStripShift) & kCscStripMask;
}

inline int MuonFixedLongId::cscStrip() const {
   return cscStripIndex() + kCscStripMin;
}

inline int MuonFixedLongId::cscChamberLayerMin(){
  return kCscChamberLayerMin;
}

inline int MuonFixedLongId::cscChamberLayerMax(){
  return kCscChamberLayerMin + kCscChamberLayerMask;
}

inline int MuonFixedLongId::cscWireLayerMin(){
  return kCscWireLayerMin;
}

inline int MuonFixedLongId::cscWireLayerMax(){
  return kCscWireLayerMin + kCscWireLayerMask;
}

inline int MuonFixedLongId::cscMeasuresPhiMin(){
  return kCscMeasuresPhiMin;
}

inline int MuonFixedLongId::cscMeasuresPhiMax(){
  return kCscMeasuresPhiMin + kCscMeasuresPhiMask;
}

inline int MuonFixedLongId::cscStripMin(){
  return kCscStripMin;
}

inline int MuonFixedLongId::cscStripMax(){
  return kCscStripMin + kCscStripMask;
}

// Rpc specific methods
inline bool MuonFixedLongId::setRpcDoubletRIndex( uint64_t idx ) {
  if ( idx > kRpcDoubletRMask ) {
      clear();
      return false;
   }
   m_data &= ~( kRpcDoubletRMask <<  kRpcDoubletRShift );
   m_data |= (idx &  kRpcDoubletRMask) << kRpcDoubletRShift;
   return true;
}

inline bool MuonFixedLongId::setRpcDoubletR( int num ) {
  return setRpcDoubletRIndex( num -  kRpcDoubletRMin );
}

inline unsigned int MuonFixedLongId::rpcDoubletRIndex() const {
   return (m_data >> kRpcDoubletRShift) & kRpcDoubletRMask;
}

inline int MuonFixedLongId::rpcDoubletR() const {
   return rpcDoubletRIndex() + kRpcDoubletRMin;
}

inline bool MuonFixedLongId::setRpcDoubletZIndex( uint64_t idx ) {
  if ( idx > kRpcDoubletZMask ) {
      clear();
      return false;
   }
   m_data &= ~( kRpcDoubletZMask <<  kRpcDoubletZShift );
   m_data |= (idx &  kRpcDoubletZMask) << kRpcDoubletZShift;
   return true;
}

inline bool MuonFixedLongId::setRpcDoubletZ( int num ) {
  return setRpcDoubletZIndex( num -  kRpcDoubletZMin );
}

inline unsigned int MuonFixedLongId::rpcDoubletZIndex() const {
   return (m_data >> kRpcDoubletZShift) & kRpcDoubletZMask;
}

inline int MuonFixedLongId::rpcDoubletZ() const {
   return rpcDoubletZIndex() + kRpcDoubletZMin;
}

inline bool MuonFixedLongId::setRpcDoubletPhiIndex( uint64_t idx ) {
  if ( idx > kRpcDoubletPhiMask ) {
      clear();
      return false;
   }
   m_data &= ~( kRpcDoubletPhiMask <<  kRpcDoubletPhiShift );
   m_data |= (idx &  kRpcDoubletPhiMask) << kRpcDoubletPhiShift;
   return true;
}

inline bool MuonFixedLongId::setRpcDoubletPhi( int num ) {
  return setRpcDoubletPhiIndex( num -  kRpcDoubletPhiMin );
}

inline unsigned int MuonFixedLongId::rpcDoubletPhiIndex() const {
   return (m_data >> kRpcDoubletPhiShift) & kRpcDoubletPhiMask;
}

inline int MuonFixedLongId::rpcDoubletPhi() const {
   return rpcDoubletPhiIndex() + kRpcDoubletPhiMin;
}

inline bool MuonFixedLongId::setRpcGasGapIndex( uint64_t idx ) {
  if ( idx > kRpcGasGapMask ) {
      clear();
      return false;
   }
   m_data &= ~( kRpcGasGapMask <<  kRpcGasGapShift );
   m_data |= (idx &  kRpcGasGapMask) << kRpcGasGapShift;
   return true;
}

inline bool MuonFixedLongId::setRpcGasGap( int num ) {
  return setRpcGasGapIndex( num -  kRpcGasGapMin );
}

inline unsigned int MuonFixedLongId::rpcGasGapIndex() const {
   return (m_data >> kRpcGasGapShift) & kRpcGasGapMask;
}

inline int MuonFixedLongId::rpcGasGap() const {
   return rpcGasGapIndex() + kRpcGasGapMin;
}

inline bool MuonFixedLongId::setRpcMeasuresPhiIndex( uint64_t idx ) {
  if ( idx > kRpcMeasuresPhiMask ) {
      clear();
      return false;
   }
   m_data &= ~( kRpcMeasuresPhiMask <<  kRpcMeasuresPhiShift );
   m_data |= (idx &  kRpcMeasuresPhiMask) << kRpcMeasuresPhiShift;
   return true;
}

inline bool MuonFixedLongId::setRpcMeasuresPhi( int num ) {
  return setRpcMeasuresPhiIndex( num -  kRpcMeasuresPhiMin );
}

inline unsigned int MuonFixedLongId::rpcMeasuresPhiIndex() const {
   return (m_data >> kRpcMeasuresPhiShift) & kRpcMeasuresPhiMask;
}

inline int MuonFixedLongId::rpcMeasuresPhi() const {
   return rpcMeasuresPhiIndex() + kRpcMeasuresPhiMin;
}

inline bool MuonFixedLongId::setRpcStripIndex( uint64_t idx ) {
  if ( idx > kRpcStripMask ) {
      clear();
      return false;
   }
   m_data &= ~( kRpcStripMask <<  kRpcStripShift );
   m_data |= (idx &  kRpcStripMask) << kRpcStripShift;
   return true;
}

inline bool MuonFixedLongId::setRpcStrip( int num ) {
  return setRpcStripIndex( num -  kRpcStripMin );
}

inline unsigned int MuonFixedLongId::rpcStripIndex() const {
   return (m_data >> kRpcStripShift) & kRpcStripMask;
}

inline int MuonFixedLongId::rpcStrip() const {
   return rpcStripIndex() + kRpcStripMin;
}


//  Tgc specific methods
inline bool MuonFixedLongId::setTgcGasGapIndex( uint64_t idx ) {
  if ( idx > kTgcGasGapMask ) {
      clear();
      return false;
   }
   m_data &= ~( kTgcGasGapMask <<  kTgcGasGapShift );
   m_data |= (idx &  kTgcGasGapMask) << kTgcGasGapShift;
   return true;
}

inline bool MuonFixedLongId::setTgcGasGap( int num ) {
  return setTgcGasGapIndex( num -  kTgcGasGapMin );
}

inline unsigned int MuonFixedLongId::tgcGasGapIndex() const {
   return (m_data >> kTgcGasGapShift) & kTgcGasGapMask;
}

inline int MuonFixedLongId::tgcGasGap() const {
   return tgcGasGapIndex() + kTgcGasGapMin;
}

inline bool MuonFixedLongId::setTgcIsStripIndex( uint64_t idx ) {
  if ( idx > kTgcIsStripMask ) {
      clear();
      return false;
   }
   m_data &= ~( kTgcIsStripMask <<  kTgcIsStripShift );
   m_data |= (idx &  kTgcIsStripMask) << kTgcIsStripShift;
   return true;
}

inline bool MuonFixedLongId::setTgcIsStrip( int num ) {
  return setTgcIsStripIndex( num -  kTgcIsStripMin );
}

inline unsigned int MuonFixedLongId::tgcIsStripIndex() const {
   return (m_data >> kTgcIsStripShift) & kTgcIsStripMask;
}

inline int MuonFixedLongId::tgcIsStrip() const {
   return tgcIsStripIndex() + kTgcIsStripMin;
}


inline bool MuonFixedLongId::setTgcChannelIndex( uint64_t idx ) {
  if ( idx > kTgcChannelMask ) {
      clear();
      return false;
   }
   m_data &= ~( kTgcChannelMask <<  kTgcChannelShift );
   m_data |= (idx &  kTgcChannelMask) << kTgcChannelShift;
   return true;
}

inline bool MuonFixedLongId::setTgcChannel( int num ) {
  return setTgcChannelIndex( num -  kTgcChannelMin );
}

inline unsigned int MuonFixedLongId::tgcChannelIndex() const {
   return (m_data >> kTgcChannelShift) & kTgcChannelMask;
}

inline int MuonFixedLongId::tgcChannel() const {
   return tgcChannelIndex() + kTgcChannelMin;
}


// MMG specific methods
inline bool MuonFixedLongId::setMmgMultilayerIndex ( uint64_t idx )  {
  if ( idx > kMmgMultilayerMask ) {
      clear();
      return false;
   }
   m_data &= ~( kMmgMultilayerMask << kMmgMultilayerShift );
   m_data |= (idx & kMmgMultilayerMask) << kMmgMultilayerShift;
   return true;
}
inline bool MuonFixedLongId::setMmgMultilayer ( int num ) {
  return setMmgMultilayerIndex( num -  kMmgMultilayerMin );
}
inline unsigned int MuonFixedLongId::mmgMultilayerIndex () const {
   return (m_data >> kMmgMultilayerShift) & kMmgMultilayerMask;
}
inline int MuonFixedLongId::mmgMultilayer () const {
   return mmgMultilayerIndex() + kMmgMultilayerMin;
}

inline bool MuonFixedLongId::setMmgGasGapIndex ( uint64_t idx )  {
  if ( idx > kMmgGasGapMask ) {
      clear();
      return false;
   }
   m_data &= ~( kMmgGasGapMask << kMmgGasGapShift );
   m_data |= (idx & kMmgGasGapMask) << kMmgGasGapShift;
   return true;
}
inline bool MuonFixedLongId::setMmgGasGap ( int num ) {
  return setMmgGasGapIndex( num -  kMmgGasGapMin );
}
inline unsigned int MuonFixedLongId::mmgGasGapIndex () const {
   return (m_data >> kMmgGasGapShift) & kMmgGasGapMask;
}
inline int MuonFixedLongId::mmgGasGap () const {
   return mmgGasGapIndex() + kMmgGasGapMin;
}

inline bool MuonFixedLongId::setMmgStripIndex ( uint64_t idx )  {
  if ( idx > kMmgStripMask ) {
      clear();
      return false;
   }
   m_data &= ~( kMmgStripMask << kMmgStripShift );
   m_data |= (idx & kMmgStripMask) << kMmgStripShift;
   return true;
}
inline bool MuonFixedLongId::setMmgStrip ( int num ) {
  return setMmgStripIndex( num -  kMmgStripMin );
}
inline unsigned int MuonFixedLongId::mmgStripIndex () const {
   return (m_data >> kMmgStripShift) & kMmgStripMask;
}
inline int MuonFixedLongId::mmgStrip () const {
   return mmgStripIndex() + kMmgStripMin;
}
inline bool MuonFixedLongId::mmgIsStereo() const {
  if (mmgMultilayer() == 1) {
    return mmgGasGap() > 2;
  } else {
    return mmgGasGap() <= 2;
  }
}

// STGC specific code
inline bool MuonFixedLongId::setStgMultilayerIndex ( uint64_t idx )  {
  if ( idx > kStgMultilayerMask ) {
      clear();
      return false;
   }
   m_data &= ~( kStgMultilayerMask << kStgMultilayerShift );
   m_data |= (idx & kStgMultilayerMask) << kStgMultilayerShift;
   return true;
}
inline bool MuonFixedLongId::setStgMultilayer ( int num ) {
  return setStgMultilayerIndex( num -  kStgMultilayerMin );
}
inline unsigned int MuonFixedLongId::stgMultilayerIndex () const {
   return (m_data >> kStgMultilayerShift) & kStgMultilayerMask;
}
inline int MuonFixedLongId::stgMultilayer () const {
   return stgMultilayerIndex() + kStgMultilayerMin;
}

inline bool MuonFixedLongId::setStgGasGapIndex ( uint64_t idx )  {
  if ( idx > kStgGasGapMask ) {
      clear();
      return false;
   }
   m_data &= ~( kStgGasGapMask << kStgGasGapShift );
   m_data |= (idx & kStgGasGapMask) << kStgGasGapShift;
   return true;
}
inline bool MuonFixedLongId::setStgGasGap ( int num ) {
  return setStgGasGapIndex( num -  kStgGasGapMin );
}
inline unsigned int MuonFixedLongId::stgGasGapIndex () const {
   return (m_data >> kStgGasGapShift) & kStgGasGapMask;
}
inline int MuonFixedLongId::stgGasGap () const {
   return stgGasGapIndex() + kStgGasGapMin;
}

inline bool MuonFixedLongId::setStgChannelTypeIndex ( uint64_t idx )  {
  if ( idx > kStgChannelTypeMask ) {
      clear();
      return false;
   }
   m_data &= ~( kStgChannelTypeMask << kStgChannelTypeShift );
   m_data |= (idx & kStgChannelTypeMask) << kStgChannelTypeShift;
   return true;
}
inline bool MuonFixedLongId::setStgChannelType ( StgChannelType num ) {
  return setStgChannelTypeIndex( num -  kStgChannelTypeMin );
}
inline unsigned int MuonFixedLongId::stgChannelTypeIndex () const {
   return (m_data >> kStgChannelTypeShift) & kStgChannelTypeMask;
}
inline MuonFixedLongId::StgChannelType MuonFixedLongId::stgChannelType () const {
   return static_cast<MuonFixedLongId::StgChannelType>(stgChannelTypeIndex() + kStgChannelTypeMin);
}

inline bool MuonFixedLongId::setStgChannelIndex ( uint64_t idx )  {
  if ( idx > kStgChannelMask ) {
      clear();
      return false;
   }
   m_data &= ~( kStgChannelMask << kStgChannelShift );
   m_data |= (idx & kStgChannelMask) << kStgChannelShift;
   return true;
}
inline bool MuonFixedLongId::setStgChannel ( int num ) {
  return setStgChannelIndex( num -  kStgChannelMin );
}
inline unsigned int MuonFixedLongId::stgChannelIndex () const {
   return (m_data >> kStgChannelShift) & kStgChannelMask;
}
inline int MuonFixedLongId::stgChannel () const {
   return stgChannelIndex() + kStgChannelMin;
}

inline std::ostream& operator<<( std::ostream& os, const MuonCalib::MuonFixedLongId& id ) {
  return id.dump( os );
}
} //MuonCalib namespace 

/*
***** must solve conflicts between MuonCalib namespace and friendship
inline std::istream& operator>>( std::istream& is, MuonCalib::MuonFixedLongId& id){
  is >> id.m_data ;
  return is;
}
*/
#endif // MUONCALIBIDENTIFIER_MUONFIXEDLONGID_H
