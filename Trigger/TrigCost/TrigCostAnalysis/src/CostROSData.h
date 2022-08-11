/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGCOSTANALYSIS_COSTROSDATA_H
#define TRIGCOSTANALYSIS_COSTROSDATA_H 1

#include "AsgMessaging/MsgStream.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"

#include <boost/thread/tss.hpp>

#include <map>
#include <vector>

/**
 * @class CostROSData
 * @brief Caches and propagates event data to be used by monitoring algorithms.
 *
 * The cache is created on passed rosToRob map 
 *
*/

class CostROSData{
  public:

    /**
     * @brief Create object based on ROS to ROB mapping
     */
    void initialize(const std::map<std::string, std::vector<uint32_t>>& rosToRobMap);

    /**
     * @brief Return cached bin for given ROS name
     * @param[in] rosName ROS name
     * @return Bin number or -1 if not found
     */
    int getBinForROS(const std::string& rosName) const;

    /**
     * @brief Return ROS name for given ROB
     * @param[in] robId ROB id 
     * @return ROS name
     */
    std::string getROSForROB(uint32_t robId) const;

    /**
     * @brief Return list of ROBs name for given ROS
     * @param[in] rosName ROS name
     * @return List of ROBs
     */
    std::vector<uint32_t> getROBForROS(const std::string& rosName) const;

    /**
     * @brief Create ROB name in hex string format
     * @param[in] robId ROB id
     * @return ROB id saved in hexadecimal representation
     */
    std::string getROBName(uint32_t robId) const;

    /**
     * @brief Return number of saved unique ROSes
     * @return Number of ROSes
     */
    unsigned getNROS() const {return m_rosIdToBin.size();}

    /**
     * @brief Return ROS name to ROB ids map
     * @return ROS name to ROB ids map
     */
    const std::map<std::string, std::vector<uint32_t>>& getROStoROBMap() const {return m_rosToRob;}

    /**
     * @brief Return ROB id to ROS name map
     * @return ROB id to ROS name map
     */
    const std::map<uint32_t, std::string>& getROBtoROSMap() const {return m_robToRos;}

    /**
     * @brief Logging
     * @return Message stream reference.
     */  
    MsgStream& msg() const;

    /**
     * @brief Logging on a given level
     * @param[in] lvl Verbosity level
     * @return Message stream reference.
     */ 
    MsgStream& msg(const MSG::Level lvl) const;

    /**
     * @brief Returns if requested level is same or higher than logging level
     * @param[in] lvl Verbosity level
     * @return If requested level is same or higher than logging level
     */ 
    bool msgLvl(const MSG::Level lvl) const;

  private:
    std::map<std::string, std::vector<uint32_t>> m_rosToRob ; //!< Mapping of ROS corresponding to ROB requests
    std::map<uint32_t, std::string> m_robToRos; //!< Mapping of ROB corresponding to ROS
    std::map<std::string, int> m_rosIdToBin;  //!< Cached mapping of ros id to bin in ROS histograms
    
    mutable boost::thread_specific_ptr<MsgStream> m_msgStream;
};

#endif // TRIGCOSTANALYSIS_COSTROSDATA_H