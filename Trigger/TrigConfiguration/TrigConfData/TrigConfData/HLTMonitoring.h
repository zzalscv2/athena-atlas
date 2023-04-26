/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGCONFDATA_HLTMONITORING_H
#define TRIGCONFDATA_HLTMONITORING_H

#include "TrigConfData/ConstIter.h"
#include "TrigConfData/DataStructure.h"

#include <map>
#include <set>

namespace TrigConf {

   /** 
    * @brief HLT monitoring configuration
    *
    * Provides access to monitoring classifications corresponding to HLT chains.
    *
    * This file is primarily designed for python based access. This C++ wrapper remains minimalist
    * for the time being. Data may still be queried via the wrapped ptree.
    * 
    * Further accessor helper functions may be added at a later date and would be backwards 
    * compatible with existing files. 
    */
   class HLTMonitoring final : public DataStructure {
   public:

      /** Constructor */
      HLTMonitoring();

      /** Constructor initialized with configuration data 
       * @param data The ptree containing the HLT monitoring structure 
       */
      HLTMonitoring(const ptree & data);
      HLTMonitoring(const HLTMonitoring&) = default;
      HLTMonitoring(HLTMonitoring&&) = default;

      /** Destructor */
      virtual ~HLTMonitoring() override = default;

      // class name
      virtual std::string className() const override {
         return "HLTMonitoring";
      }

      /** Accessor to the number of HLT monitoring chains */
      std::size_t size() const;

      /** setter and getter for the supermasterkey */
      unsigned int smk() const;
      void setSMK(unsigned int psk);

      /** names of the monitored signatures */
      std::vector<std::string> signatureNames() const;

      /** names of targets */
      const std::set<std::string> & targets() const;

      const std::map<std::string, std::map<std::string, std::vector<std::string>>> & signatures() const;

      /** monitored chains by signature for a given target
       * @param signature signature like egammaMon, idMon, caloMon (complete list via signatureNames())
       * @param target monitoring target like shifter, t0, online (complete set via targets())
       */
      std::vector<std::string> chainsBySignatureAndTarget(const std::string & signature, const std::string & target) const;

      /** print overview of L1 Menu */
      void printMonConfig(bool full = false) const;

      /** Clearing the configuration data */
      virtual void clear() override;

   private:
   
      /** Update the internal data after modification of the data object */
      virtual void update() override { load(); };
      void load();

      /** the supermasterkey */
      unsigned int m_smk {0};

      /** internal storage of the information */
      std::map<std::string, std::map<std::string, std::vector<std::string>>> m_signatures{};

      /** names of monitoring targets like shifter, t0, online */
      std::set<std::string> m_targets{};
   };
}

#ifndef TRIGCONF_STANDALONE
#ifndef XAOD_STANDALONE

#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( TrigConf::HLTMonitoring , 250972509, 1 )

#include "AthenaKernel/CondCont.h"
CONDCONT_DEF( TrigConf::HLTMonitoring , 50201204 );

#endif
#endif

#endif
