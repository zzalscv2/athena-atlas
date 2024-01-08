/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/* **************************************************************************
                           BTagTool.h  -  Description
                             -------------------
    begin   : 10.03.04
    authors : Andreas Wildauer (CERN PH-ATC), Fredrik Akesson (CERN PH-ATC)
    email   : andreas.wildauer@cern.ch, fredrik.akesson@cern.ch
    comments: prototype implementation of a general jet tag strategy for the case of b-tagging
   ***************************************************************************/

#ifndef BTAGGING_BTAGTOOL_H
#define BTAGGING_BTAGTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "BTagging/IBTagTool.h"
#include "JetTagTools/ITagTool.h"
#include "StoreGate/ReadHandleKey.h"

#include <string>
#include <vector>
#include <map>
#include <atomic>


/** The namespace of all packages in PhysicsAnalysis/JetTagging */
namespace Analysis
{

  /**  \class BTagTool
  The BTagTool class is a prototype implementation of a general jet tag strategy
  for the case of b-tagging. It works entirely on the new Atlas EDM objects which
  have been developed alongside (JetTag, TrackParticle, Vertex) or by others (ParticleJet, Jet).
  It can be called by any athena algtool or algorithm.
  In the case of jet tagging a prototype algorithm (BJetBuilder) has been implemented.

  As input it takes a Jet and a pointer to a JetTag object. The behaviour of the BTagTool
  can be steered by jobOptions.
  Before tagging jets the tool can call a jet truth match algtool which labels the
  JetTag object with truth information. The type of truth particle(s) the jet should be matched
  to and the jet label (it is a string) the JetTag object should be given have to be
  specified via job options of the instance of the JetTruthMatchTool used by the BTagTool.
  For the btagging the truth matching is done with b-quarks and b-baryons and the jet label
  is "b".

  After that the Jet and JetTag are passed to job option selected tools for tagging.
  All available tools reside in PhysicsAnalysis/JetTagging/JetTagTools. Each tool adds
  its tag information to the JetTag via an "info object". All info objects have to inherit
  from the ITagInfo base class and the JetTag can hold an arbitrary number of these objects.
  (The truth info, i.e. jet label, given by the JetTruthMatching algorithm is also stored in
  such an object. In this case it is called TruthInfo.). For more info on the info objects see
  PhysicsAnalysis/JetTagging/JetTagInfo.

  At the end of the BTagTool the information of all taggers is combined into a single signal likelihood
  which is stored in the JetTag itself (along with all background likelihoods). After the tool is finished
  the caller has a pointer to a JetTag object which is filled up with tag information.
  For more details on the JetTag object see PhysicsAnalysis/JetTagging/JetTagEvent.

    @author Andreas.Wildauer@cern.ch
    */

  class BTagTool : public extends<AthAlgTool, IBTagTool>
  {
    public:

      /** Constructors and destructors */
      BTagTool(const std::string&,const std::string&,const IInterface*);
      virtual ~BTagTool() = default;

      /** Main routines specific to an ATHENA algorithm */
      virtual StatusCode initialize() override;
      virtual StatusCode finalize() override;
      virtual void finalizeHistos() override;

      virtual
      StatusCode tagJet(const xAOD::Jet*, xAOD::BTagging*, const std::string &jetName, const xAOD::Vertex* vtx = 0) const override;
      virtual
      StatusCode tagJet(const xAOD::JetContainer * jetContainer, xAOD::BTaggingContainer * btaggingContainer, const std::string &jetName) const override;

    private:

      mutable std::atomic<unsigned int> m_nBeamSpotPvx{};
      mutable std::atomic<unsigned int> m_nAllJets{};

      ToolHandleArray< ITagTool > m_bTagToolHandleArray;
      std::map<std::string, ITagTool*> m_bTagTool; //!< map to the btag tools

      SG::ReadHandleKey<xAOD::VertexContainer> m_VertexCollectionName {this, "vxPrimaryCollectionName", "", "Input primary vertex container"};

  }; // End class
} // End namespace
#endif

