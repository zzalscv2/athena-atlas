/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/*
 *   */

#include "LWTNNCondAlg.h"
#include "LwtnnUtils/InputOrder.h"

#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "AthenaPoolUtilities/AthenaAttributeList.h"
#include "CoolKernel/IObject.h"
#include "FileCatalog/IFileCatalog.h"

// NN includes
#include "lwtnn/parse_json.hh"
#include "lwtnn/Exceptions.hh"
#include "lwtnn/lightweight_nn_streamers.hh"
#include "lwtnn/NanReplacer.hh"

// JSON parsers
#define BOOST_BIND_GLOBAL_PLACEHOLDERS // Needed to silence Boost pragma message
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "boost/property_tree/exceptions.hpp"


// for error messages
#include <memory>

#include <typeinfo>

namespace InDet {

  LWTNNCondAlg::LWTNNCondAlg (const std::string& name, ISvcLocator* pSvcLocator)
    : ::AthReentrantAlgorithm( name, pSvcLocator )
  {}

  StatusCode LWTNNCondAlg::initialize() {

    // Condition Handles
    ATH_CHECK( m_readKey.initialize() );
    ATH_CHECK( m_writeKey.initialize() );

    return StatusCode::SUCCESS;
  }

  StatusCode LWTNNCondAlg::finalize()
  {
    return StatusCode::SUCCESS;
  }

  StatusCode LWTNNCondAlg::configureLwtnn(std::unique_ptr<lwt::atlas::FastGraph> & thisNN,
                                        const std::string& thisJson) const {

    // Read DNN weights from input json config
    lwt::GraphConfig config;
    try {
      std::istringstream input_cfg( thisJson );
      config = lwt::parse_json_graph(input_cfg);
    } catch (boost::property_tree::ptree_error& err) {
      ATH_MSG_ERROR("NN file unreadable!");
      return StatusCode::FAILURE;
    }

    // pass the input order for the FastGraph
    lwt::atlas::InputOrder order;
    order.scalar.emplace_back("NNinputs", m_variableOrder );
    // sequence not needed for NN (more for RNN, but set anyway)
    order.sequence.emplace_back("NNinputs", m_variableOrder );

    // Build the network
    try {
      thisNN = std::make_unique<lwt::atlas::FastGraph>(config, order, "merge_1");
    } catch (lwt::NNConfigurationException& exc) {
      ATH_MSG_ERROR("NN configuration problem: " << exc.what());
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;

  }

  StatusCode LWTNNCondAlg::execute(const EventContext& ctx) const {

    SG::WriteCondHandle<LWTNNCollection> NnWriteHandle{m_writeKey, ctx};
    if (NnWriteHandle.isValid()) {
      ATH_MSG_DEBUG("Write CondHandle "<< NnWriteHandle.fullKey() << " is already valid");
      return StatusCode::SUCCESS;
    }

    SG::ReadCondHandle<CondAttrListCollection> readHandle{m_readKey, ctx};
    if(!readHandle.isValid()) {
      ATH_MSG_ERROR("Invalid read handle " << m_readKey.key());
      return StatusCode::FAILURE;
    }
    const CondAttrListCollection* atrcol{*readHandle};
    assert( atrcol != nullptr);

    // So now we have the string containing the json. Access it.
    // Retrieve channel 0 (only channel there is)
    const coral::AttributeList& attrList=atrcol->attributeList(0);

    // Check that it is filled as expected
    if ((attrList["NNConfigurations"]).isNull()) {
      ATH_MSG_ERROR( "NNConfigurations is NULL !" );
      return StatusCode::FAILURE;
    }

    // Retrieve the string
    // This is for a single LOB when it is all a giant block
    const std::string megajson = attrList["NNConfigurations"].data<cool::String16M>();

    // Parse the large json to extract the individual configurations for the NNs
    std::istringstream initializerStream(megajson);
    namespace pt = boost::property_tree;
    pt::ptree parentTree;
    pt::read_json(initializerStream, parentTree);
    std::ostringstream configStream;

    // This is for handling IOVs
    EventIDRange cdo_iov;
    if(!readHandle.range(cdo_iov)) {
      ATH_MSG_ERROR("Failed to get valid validity range from  " << readHandle.key());
      return StatusCode::FAILURE;
    }

    // Here I create a pointer to the object I want to write
    // And what I want to write is the map with my lwtnn networks.
    std::unique_ptr<LWTNNCollection> writeCdo{std::make_unique<LWTNNCollection>()};

    // First, extract configuration for the number network.
    pt::ptree subtreeNumberNetwork = parentTree.get_child("NumberNetwork");
    writeCdo->insert(std::make_pair(0,std::unique_ptr<lwt::atlas::FastGraph>(nullptr)));
    // If this json is empty, just fill a null pointer.
    if(subtreeNumberNetwork.empty()) {
      ATH_MSG_ERROR("You are trying to use lwtnn for the number network but have an empty configuration file; this should never happen!");
      return StatusCode::FAILURE;
    }
    // Otherwise, set up lwtnn.
    else {
      ATH_MSG_INFO("Setting up lwtnn for number network...");
      pt::write_json(configStream, subtreeNumberNetwork);
      std::string numberNetworkConfig = configStream.str();
      if ((configureLwtnn(writeCdo->at(0), numberNetworkConfig)).isFailure())
        return StatusCode::FAILURE;
    }

    // Now extract configuration for each position network.
    // For simplicity, we'll require all three configurations
    // in order to use lwtnn for positions.
    for (int i=1; i<4; i++) {
      const std::string key = "PositionNetwork_N"+std::to_string(i);
      configStream.str("");
      pt::ptree subtreePosNetwork = parentTree.get_child(key);
      pt::write_json(configStream, subtreePosNetwork);
      std::string posNetworkConfig = configStream.str();

      // Put a lwt network into the map
      writeCdo->insert(std::make_pair(i,std::unique_ptr<lwt::atlas::FastGraph>(nullptr)));

      // Now do empty check: if any one of these is empty we won't use lwtnn
      if(subtreePosNetwork.empty()) {
        ATH_MSG_ERROR("You are trying to use lwtnn for the position networks but have an empty configuration file; this should never happen!");
        return StatusCode::FAILURE;
      } else {
        // Otherwise, set up lwtnn
        ATH_MSG_INFO("Setting up lwtnn for n = " << i << " position network...");
        if ((configureLwtnn(writeCdo->at(i), posNetworkConfig)).isFailure())
          return StatusCode::FAILURE;
      }

    }

    // Write the networks to the store

    if(NnWriteHandle.record(cdo_iov,std::move(writeCdo)).isFailure()) {
      ATH_MSG_ERROR("Failed to record Trained network collection to "
                    << NnWriteHandle.key()
                    << " with IOV " << cdo_iov );
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }

}
