/*
 Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

/// @author RD Schaffer 



#ifndef ASG_ANALYSIS_ALGORITHMS__ASG_MASK_SELECTION_TOOL_H
#define ASG_ANALYSIS_ALGORITHMS__ASG_MASK_SELECTION_TOOL_H

#include <AsgTools/AsgTool.h>
#include <PATCore/IAsgSelectionTool.h>
#include <SelectionHelpers/ISelectionReadAccessor.h>
#include <xAODBase/IParticle.h>
#include <memory>
#include <string>
#include <vector>

namespace CP
{
    /// \brief an \ref IAsgSelectionTool that cuts on int decorations with mask
    ///
    /// Can provide two lists of variables and masks, for each variable, 
    /// apply mask as selection

    class AsgMaskSelectionTool final
    : public asg::AsgTool, virtual public IAsgSelectionTool
  {
    //
    // public interface
    //

    // Create a proper constructor for Athena
    ASG_TOOL_CLASS( AsgMaskSelectionTool, IAsgSelectionTool )


    /// \brief standard constructor
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   out of memory II
  public:
    AsgMaskSelectionTool (const std::string& name);




    //
    // inherited interface
    //

    virtual StatusCode initialize () override;

    virtual const asg::AcceptInfo& getAcceptInfo( ) const override;

    virtual asg::AcceptData accept( const xAOD::IParticle* part ) const override;



    //
    // private interface
    //

    /// tool properties
    /// \{
  private:
    std::vector<std::string>  m_selVars;
    std::vector<unsigned int> m_selMasks;
    std::vector<std::unique_ptr<ISelectionReadAccessor> > m_acc_selVars;

    /// \}


    /// \brief the \ref asg::AcceptInfo we are using
  private:
    asg::AcceptInfo m_accept;
  };
}

#endif
