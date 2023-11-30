/*
+  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MULTIFOLD_GNN_TOOL_H
#define MULTIFOLD_GNN_TOOL_H

// Tool includes
#include "AsgTools/AsgTool.h"
#include "FlavorTagDiscriminants/IBTagDecorator.h"
#include "FlavorTagDiscriminants/IJetTagConditionalDecorator.h"

#include "FlavorTagDiscriminants/GNNToolifiers.h"

// EDM includes
#include "xAODBTagging/BTaggingFwd.h"
#include "xAODJet/JetFwd.h"

#include <memory>
#include <string>
#include <map>

namespace FlavorTagDiscriminants {

  class MultifoldGNN;

  //
  // Tool to to flavor tag jet/btagging object
  // using GNN based taggers
  class MultifoldGNNTool : public asg::AsgTool,
                  virtual public IBTagDecorator,
                  virtual public IJetTagConditionalDecorator
  {

    ASG_TOOL_CLASS2(MultifoldGNNTool, IBTagDecorator, IJetTagConditionalDecorator)
    public:
      MultifoldGNNTool(const std::string& name);
      ~MultifoldGNNTool();

      StatusCode initialize() override;

      virtual void decorate(const xAOD::BTagging& btag) const override;
      virtual void decorate(const xAOD::Jet& jet) const override;
      virtual void decorateWithDefaults(const xAOD::Jet& jet) const override;

      virtual std::set<std::string> getDecoratorKeys() const override;
      virtual std::set<std::string> getAuxInputKeys() const override;
      virtual std::set<std::string> getConstituentAuxInputKeys() const override;

    private:

    std::vector<std::string> m_nn_files;
    std::string m_fold_hash_name;
      GNNToolProperties m_props;
      std::unique_ptr<const MultifoldGNN> m_gnn;
  };
}
#endif
