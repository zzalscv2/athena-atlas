/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILECALIBBLOBOBJS_TILECALIBUTILS_H
#define TILECALIBBLOBOBJS_TILECALIBUTILS_H

/** 
    @brief Static class providing several utility functions and constants. 
    @author Nils Gollub <nils.gollub@cern.ch>
*/

#include <string>

class TileCalibUtils{
 public:
  
  /** @brief Returns the full tag string, composed of camelized folder name and tag part.
      @param folder The full COOL folder path capitalized, eg. "/TILE/FILTER/OF1"
      @param tag The tag part, eg. "XX-YYY" */
  static std::string getFullTag(const std::string& folder, const std::string& tag);

  /** @brief Returns a drawer hash. 
      This function assumes drawer context (i.e. only ROS and drawer are present)
      @param ros The ROS index
      @param drawer The drawer index*/
  static unsigned int getDrawerIdx(unsigned int ros, unsigned int drawer);

  /** @brief Returns a drawer hash from fragId
      This function assumes drawer context (i.e. only ROS and drawer are present)
      @param fragId The fragment id, e.g. 0x100 for LBA01 */
  static unsigned int getDrawerIdxFromFragId(unsigned int fragId);

  /** @brief Returns a channel hash. 
      This function assumes channel context 
      @param ros The ROS index
      @param drawer The drawer index
      @param channel The channel number */
  static unsigned int getChanIdx(unsigned int ros, unsigned int drawer, unsigned int channel);
  
  /** @brief Returns a channel hash. 
      This function assumes channel context 
      @param drawerIdx The drawer index
      @param channel The channel number */
  static unsigned int getChanIdx(unsigned int drawerIdx, unsigned int channel);

  /** @brief Returns a channel hash. 
      This function assumes channel context 
      @param fragId The fragment id
      @param channel The channel number */
  static unsigned int getChanIdxFromFragId(unsigned int fragId, unsigned int channel);

  /** @brief Returns an ADC hash. 
      This function assumes ADC context)
      @param ros The ROS index
      @param drawer The drawer index
      @param channel The channel number 
      @param adc The gain index */
  static unsigned int getAdcIdx(unsigned int ros, unsigned int drawer, unsigned int channel, unsigned int adc);

  /** @brief Returns an ADC hash. 
      This function assumes drawer context
      @param drawerIdx The drawer index
      @param channel The channel number 
      @param adc The gain index */
  static unsigned int getAdcIdx(unsigned int drawerIdx, unsigned int channel, unsigned int adc);

  /** @brief Returns an ADC hash. 
      This function assumes drawer context
      @param fragId The fragment id
      @param channel The channel number 
      @param adc The gain index */
  static unsigned int getAdcIdxFromFragId(unsigned int fragId, unsigned int channel, unsigned int adc);

  /** @brief Returns the maximal channel number for a given drawer.
      Returns 20 for ros==0, returns 64 for 1 <= ros <= 4. 
      @param ros The ROS index */
  static unsigned int getMaxDrawer(unsigned int ros);
  
  /** @brief Returns the COOL channel number for the comment channel. 
      WARNING: The comment channel number should never be changed! */
  static unsigned int getCommentChannel(){return 1000;}
  
  /** @brief Return the drawer name, e.g. LBA17 */
  static std::string getDrawerString(unsigned int ros, unsigned int drawer);

  /** @brief Returns the default drawer for a given input drawer. 
      @param drawerIdx The input drawer index*/
  static unsigned int getDefaultDrawerIdx(unsigned int drawerIdx);

  /** @brief Returns the default drawer for a given input drawer. 
      @param ros    The input ros
      @param drawer The input drawer */
  static unsigned int getDefaultDrawerIdx(unsigned int ros, unsigned int drawer);

  /** @brief Returns the first drawer Idx in a partition of a given input drawer. 
      @param drawerIdx The input drawer index*/
  static unsigned int getFirstDrawerInPartitionIdx(unsigned int drawerIdx);
  
  /** @brief Returns the input in fixed point precision.
      @param val   The input value
      @param nBits Number of bits used for encoding */
  static float fixedPointPrecision(float val, unsigned int nBits=16);

  /** @brief Python compatibility function */
  static unsigned int max_ros() {return MAX_ROS;}
  /** @brief Python compatibility function */
  static unsigned int max_drawer() {return MAX_DRAWER;}
  /** @brief Python compatibility function */
  static unsigned int max_drawr0() {return MAX_DRAWR0;}
  /** @brief Python compatibility function */
  static unsigned int max_chan() {return MAX_CHAN;}
  /** @brief Python compatibility function */
  static unsigned int max_gain() {return MAX_GAIN;}
  /** @brief Python compatibility function */
  static unsigned int max_draweridx() {return MAX_DRAWERIDX;}
  /** @brief Python compatibility function */
  static unsigned int trips_draweridx() {return TRIPS_DRAWERIDX;}
  /** @brief Python compatibility function */
  static unsigned int trips_ros() {return TRIPS_ROS;}
  /** @brief Python compatibility function */
  static unsigned int trips_drawer() {return TRIPS_DRAWER;}
  /** @brief Python compatibility function */
  static unsigned int definitions_draweridx() {return DEFINITIONS_DRAWERIDX;}
  /** @brief Python compatibility function */
  static unsigned int bad_definition_chan() {return BAD_DEFINITION_CHAN;}
  /** @brief Python compatibility function */
  static unsigned int noisy_definition_chan() {return NOISY_DEFINITION_CHAN;}
  /** @brief Python compatibility function */
  static unsigned int nogainl1_definition_chan() {return NOGAINL1_DEFINITION_CHAN;}
  /** @brief Python compatibility function */
  static unsigned int badtiming_definition_chan() {return BADTIMING_DEFINITION_CHAN;}
  /** @brief Python compatibility function */
  static unsigned int wrongbcid_definition_chan() {return WRONGBCID_DEFINITION_CHAN;}
  /** @brief Python compatibility function */
  static unsigned int timingdmubcoffset_definition_chan() {return TIMINGDMUBCOFFSET_DEFINITION_CHAN;}

  static const unsigned int MAX_ROS      =  5;  /**< @brief Number of ROSs               */
  static const unsigned int MAX_DRAWER   = 64;  /**< @brief Number of drawers in ROS 1-4 */
  static const unsigned int MAX_DRAWR0   = 20;  /**< @brief Number of drawers in ROS 0   */
  static const unsigned int MAX_CHAN     = 48;  /**< @brief Number of channels in drawer */
  static const unsigned int MAX_GAIN     =  2;  /**< @brief Number of gains per channel  */
  static const unsigned int MAX_DRAWERIDX = 276;  /**< @brief Maximal drawer index         */
  static const unsigned int LAS_PART_CHAN= 43;  /**< @brief Empty channel number to store laser partition variation */
  static const unsigned int TRIPS_DRAWERIDX= 2; /**< @brief DrawerIdx used for storing trips probabilities */
  static const unsigned int TRIPS_ROS    =  0;      /**< @brief Ros used for storing trips probabilities */
  static const unsigned int TRIPS_DRAWER =  2;   /**< @brief Drawer used for storing trips probabilities */
  static const unsigned int DEFINITIONS_DRAWERIDX = 1;   /**< @brief Drawer used for storing of bad and noisy channel definitions */
  static const unsigned int BAD_DEFINITION_CHAN = 0;   /**< @brief Channel used for storing of bad channel definitions */
  static const unsigned int NOISY_DEFINITION_CHAN = 1;   /**< @brief Channel used for storing of noisy channel definitions */
  static const unsigned int NOGAINL1_DEFINITION_CHAN = 2;   /**< @brief Channel used for storing of NoGainLevel1 channel definitions */
  static const unsigned int BADTIMING_DEFINITION_CHAN = 3;   /**< @brief Channel used for storing of bad timing channel definitions */
  static const unsigned int WRONGBCID_DEFINITION_CHAN = 4;   /**< @brief Channel used for storing of wrong BCID channel definitions */
  static const unsigned int TIMINGDMUBCOFFSET_DEFINITION_CHAN = 5;   /**< @brief Channel used for storing of affected timing channel definitions */
  static const unsigned int MAX_MINIDRAWER = 4;  /**< @brief Number of minidrawers */
  static const unsigned int MAX_MINIDRAWER_CHAN = 12;  /**< @brief Number of channels in minidrawer*/
  static const unsigned int FELIX_FRAGID_OFFSET = 0x1000;  /**< @brief Offset for frag ID used for FELIX in frag ID to ROB ID map*/
};

#endif
