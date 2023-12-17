/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILEBYTESTREAM_TILEHID2RESRCID_H
#define TILEBYTESTREAM_TILEHID2RESRCID_H

#include "eformat/FullEventFragment.h"

class TileHWID;
class HWIdentifier;
class MsgStream;

#include <map>
#include <vector>
#include <string>
#include <stdint.h> 

/**
 * @class TileHid2RESrcID
 * @brief This class provides conversion between fragment ID and RESrcID.
 * @author Alexander Solodkov
 *
 * Fragment ID is the unique ID for every super-drawer.
 * RESrcID is used for identifying each ROD. 
 */ 

class TileHid2RESrcID {

public:

  typedef int COLLECTION_ID;
  
  /** constrcutor 
  */ 
  TileHid2RESrcID(const TileHWID* tileHWID=0, uint32_t runnum=0); 
  void setTileHWID (const TileHWID* tileHWID, uint32_t runnum=0);

  void initialize(uint32_t runnum);
  void initializeMuRcv(uint32_t runnum);

  void initialize(const std::vector<std::vector<uint32_t> > & fullmap);

  void setROD2ROBmap (const std::vector<std::string> & ROD2ROB,
                      MsgStream & log);

  void setROD2ROBmap (const eformat::FullEventFragment<const uint32_t*> * event,
                      bool& of2Default,
                      MsgStream & log);

  void setBSfrag (int frag_id, uint32_t bs_frag);
  void setDrawerType (int frag_id, uint32_t type);

  void printSpecial (MsgStream & log);

  /** make a ROB SrcID for a fragment ID
  */ 
  uint32_t getRobFromFragID  (int frag_id) const;
  uint32_t getRobFromTileMuRcvFragID (int frag_id) const;

  /** make a ROD SrcID for a fragment ID
  */ 
  uint32_t getRodID  (int frag_id) const;
  uint32_t getRodTileMuRcvID (int frag_id) const;

  /** Make a ROB Source ID from a ROD source ID
  */ 
  uint32_t getRobID  ( uint32_t rod_id) const; 
  uint32_t getRobID_8  ( uint32_t rod_id) const;

  /** Make a ROS Source ID from a ROB source ID
  */ 
  uint32_t getRosID  ( uint32_t rob_id) const; 
  uint32_t getRosID_8  ( uint32_t rob_id) const;

  /** Make a SubDetector ID from ROS source ID 
  */
  uint32_t getDetID  ( uint32_t ros_id) const;

  /** Retrieve run number for which hash was initialized
  */
  uint32_t getRunNum  () { return m_runnum; };

  /** Retrieve extra info - ByteStream frag ID and drawer typefor a given transient fragment ID
  */
  int getOfflineFragID(uint32_t bs_frag_id) const;
  uint32_t getBSfragID(int frag_id) const;
  uint32_t getDrawerType(int frag_id) const;
  const std::vector<uint32_t> & getDrawerInfo(int frag_id) const;

private: 

  const TileHWID* m_tileHWID;
  typedef std::map<int, uint32_t> FRAGRODMAP;
  typedef std::map<uint32_t, int> BS2OFFLINEMAP;
  typedef std::map<int, std::vector<uint32_t> > FRAGFULLMAP;
  FRAGRODMAP m_TileMuRcvFrag2ROD;
  BS2OFFLINEMAP m_bs2offline;
  FRAGFULLMAP m_frag2ROD;
  uint32_t m_runnum, m_TileMuRcvRunnum = 0U;
  std::vector<uint32_t> m_defaultDrawer;

  void updateBSmap();
};


#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"

CLASS_DEF(TileHid2RESrcID, 22911658, 0)
CONDCONT_DEF(TileHid2RESrcID, 23187372);

#endif 
