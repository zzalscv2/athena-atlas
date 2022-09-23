/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JIVEXML__XMLFORMATTOOL_H
#define JIVEXML__XMLFORMATTOOL_H

#include "JiveXML/IFormatTool.h"
#include "AthenaBaseComps/AthAlgTool.h"

namespace JiveXML{

  /**
   * Writes data stored in a DataMap into an XML stream in the format required
   * by the Atlantis event display
   *
   * @author J. Couchman
   * @author S. Boeser
   */

  class XMLFormatTool : virtual public IFormatTool, public AthAlgTool{

    public:
      
      /// Constructor
      XMLFormatTool( const std::string&, const std::string&, const IInterface*);

      /// Initialize
      virtual StatusCode initialize() override;
      
      /// Finalize
      virtual StatusCode finalize() override;

      /** Set additional tags */
      virtual StatusCode SetTag ( const TagType tag ) override;

      /** Start a new event */
      virtual StatusCode StartEvent( const unsigned long EventNumber, const unsigned int RunNumber, 
                                     const std::string& DateTime,
                                     const unsigned int lumiBlock,
                                     const std::string& eventProperty,
                                     const std::string& geometryVersion ) override;

      /** Finialize this event */
      virtual StatusCode EndEvent() override;

      /** Append a formatted version of one event component */
      virtual StatusCode AddToEvent( const std::string& component,
                                     const std::string& key,
                                     const DataMap* aMap) override;

      /** Return the formated stringstream */
      virtual const std::ostringstream* getFormattedEvent() const override;


   private:

      /// the string buffer that will hold the complete event
      std::ostringstream* m_EventBuffer;

      /// the release tag 
      std::string m_release;

  };


} //namespace

#endif
