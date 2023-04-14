/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DQM_Algorithms_AFP_LBsOutOfRange_H
#define DQM_Algorithms_AFP_LBsOutOfRange_H

#include <dqm_core/Algorithm.h>
#include <dqm_core/AlgorithmConfig.h>
#include <dqm_core/Result.h>

#include <TObject.h>

#include <map>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

ERS_DECLARE_ISSUE_BASE( afp,
                        CantReadCool,
                        dqm_core::Exception,
                        "Cannot read folder '" << folder << "' from COOL database '" << database << "'",
                        ERS_EMPTY,
                        ( (std::string) database )( (std::string) folder ) );

namespace dqm_algorithms {
    class AFP_LBsOutOfRange : public dqm_core::Algorithm {
      public:
        using IOVSet = std::vector<std::pair<int, int>>;

        AFP_LBsOutOfRange();
        ~AFP_LBsOutOfRange();

        AFP_LBsOutOfRange* clone() override;
        dqm_core::Result* execute( const std::string& name, const TObject& object, const dqm_core::AlgorithmConfig& config ) override;
        void printDescriptionTo( std::ostream& out ) override;

        const IOVSet& fetchIOVs( uint32_t run, uint32_t channel );

      private:
        std::map<std::pair<uint32_t, uint32_t>, IOVSet> m_iov_cache;
    };
} // namespace dqm_algorithms

#endif // DQM_Algorithms_AFP_LBsOutOfRange_H
