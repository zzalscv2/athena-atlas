/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONNSWASBUILT_STGCSTRIPCALCULATOR_H
#define MUONNSWASBUILT_STGCSTRIPCALCULATOR_H

#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>

#include "MuonNSWAsBuilt/Identifier.h"
#include "MuonNSWAsBuilt/Element.h"
#include "MuonNSWAsBuilt/CathodeBoardElement.h"
#include "GeoPrimitives/GeoPrimitives.h"

namespace NswAsBuilt {

  /**
   * The main interface of the NswAsBuilt package: determines strip positions
   * in the frame of the quadruplet.
   *
   * Once instanciated, the as-built geometry must be read in from a stream of
   * JSON formatted data, using the method parseJSON.
   *
   * Strip positions may then be queried, using the method getStgcStrip. (The method
   * getStgcStrip is thread-safe.)
   *
   */
  class StgcStripCalculator {
    public:
      using ParameterClass = Element::ParameterClass;

      /**
       * VALID: strip is found, INVALID: strip is not found
       */
      enum class IsValid {
        INVALID=0,
        VALID=1
      };

      /**
       * The return object for querying strip positions: three points along the
       * strip are provided, in the coordinate system of the quadruplet.
       */
      struct stgcStrip_t {
        IsValid isvalid{IsValid::INVALID};
        Amg::Vector3D center{Amg::Vector3D::Zero()};
        Amg::Vector3D left{Amg::Vector3D::Zero()};
        Amg::Vector3D right{Amg::Vector3D::Zero()};
      };

      /**
       * Computes 3 reference points along a strip identified by strip_id, in
       * coordinate system of quadruplet.
       * Use iclass=ParameterClass::NOMINAL or ParameterClass::CORRECTION to
       * get the nominal or as-built values, respectively.
       */
      stgcStrip_t getStgcStrip(ParameterClass iclass, stripIdentifier_t strip_id) const;

      /**
       * The return object for querying strip positions with the method
       * getPositionAlongStrip: a single point on strip is returned, in the
       * coordinate system of the quadruplet
       */
      struct position_t {
        IsValid isvalid{IsValid::INVALID};
        Amg::Vector3D pos{Amg::Vector3D::Zero()};
      };

      /**
       * Returns a the position of a point along the strip, parameterized by s,
       * in the coordinate system of the quadruplet.
       * s is in [-1,1]: -1 returns the left-most point along the strip, 0
       * returns the center point and +1 return the right-most point
       */
      position_t getPositionAlongStgcStrip(ParameterClass iclass, stripIdentifier_t strip_id, double sx, double sy) const;

      /**
       * Parses a std::istream with JSON-formatted configuration of the
       * as-built parameters
       */
      void parseJSON(const std::string& in);

    private:
      std::vector<std::unique_ptr<Element>> m_rootElements;
      std::unordered_map<pcbIdentifier_t, CathodeBoardElement> m_pcbMap;
    
      // Follow JSON parsing helper methods
      struct json_t; // wraps a nlhohman::json in a pimpl struct to hide dependency
      // Parse the JSON ROOT element
      void parseRootElement(json_t j);
      // Build a NswAsBuilt::Element, following description in JSON
      std::unique_ptr<Element> buildElement(json_t j) const;
      // Parse a quadruplet identifier in JSON
      quadrupletIdentifier_t getQuadrupletIdentifier(json_t j) const;
      // Parse a PCB identidier in JSON
      pcbIdentifier_t getPcbIdentifier(quadrupletIdentifier_t quad_id, json_t j) const;
      // Parse the strip information attached to an element
      void collectStrip(quadrupletIdentifier_t quad_id, Element& element, json_t j);
      // Parse the strip configuration parameters in JSON
      CathodeBoardElement::stgcStripConfiguration_t getStgcStripConfiguration(json_t j) const;

  };
}

#endif

