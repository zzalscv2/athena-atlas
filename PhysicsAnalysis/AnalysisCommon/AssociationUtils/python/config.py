# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

""" AssociationUtils/config.py
    This file contains the configuration helper code for the overlap removal
    tools in Athena. It is a work in progress. It may in fact be possible to
    utilize some of this code also in RootCore with some PyROOT magic, but for
    now you should just use the C++ helper code in OverlapRemovalInit if you're
    working in RootCore.
"""

from AthenaCommon.Constants import INFO

from AssociationUtils.AssociationUtilsConf import (
    ORUtils__OverlapRemovalTool as OverlapRemovalTool,
    ORUtils__DeltaROverlapTool as DeltaROverlapTool,
    ORUtils__EleEleOverlapTool as EleEleOverlapTool,
    ORUtils__EleJetOverlapTool as EleJetOverlapTool,
    ORUtils__EleMuSharedTrkOverlapTool as EleMuSharedTrkOverlapTool,
    ORUtils__MuJetOverlapTool as MuJetOverlapTool,
    ORUtils__TauLooseEleOverlapTool as TauLooseEleOverlapTool,
    ORUtils__TauLooseMuOverlapTool as TauLooseMuOverlapTool
)

#-------------------------------------------------------------------------------
def recommended_tools(masterName='OverlapRemovalTool',
                      inputLabel='selected', outputLabel='DFCommonJets_isOverlap',
                      bJetLabel='', boostedLeptons=False,
                      outputPassValue=False,
                      linkOverlapObjects=False,
                      doEleEleOR=False,
                      doElectrons=True, doMuons=True, doJets=True,
                      doTaus=True, doPhotons=True, doFatJets=False,
                      **kwargs):
    """
    Provides the pre-configured overlap removal recommendations.
    All overlap tools will be (private) added to the master tool
    which is then returned by this function.

    Arguments:
      masterName         - set the name of the master tool.
      inputLabel         - set the InputLabel property for all tools.
      outputLabel        - set the OutputLabel property for all tools.
      bJetLabel          - set user bjet decoration name. Leave blank to
                           disable btag-aware overlap removal.
      boostedLeptons     - enable sliding dR cones for boosted lepton
                           analyses.
      outputPassValue    - set the OutputPassValue property for all tools
                           which determines whether passing objects are
                           marked with true or false.
      linkOverlapObjects - enable ElementLinks to overlap objects.
      doEleEleOR         - enable electron-electron overlap removal.
      doXXXX             - these flags enable/disable object types to
                           configure tools for: doElectrons, doMuons,
                           doJets, doTaus, doPhotons, doFatJets.
      kwargs             - additional properties to be applied to all tools.
                           For example: OutputLevel.
    """

    # These properties can be applied to all tools
    common_args = {
        'InputLabel' : inputLabel,
        'OutputLabel' : outputLabel,
        'OutputPassValue' : outputPassValue
    }
    # Extend with additional user-defined global properties
    common_args.update(kwargs)

    # Configure the master tool
    orTool = OverlapRemovalTool(masterName, **common_args)

    # Overlap tools share an additional common property for object linking
    common_args['LinkOverlapObjects'] = linkOverlapObjects

    # Electron-electron
    if doElectrons and doEleEleOR:
        orTool.EleEleORT = EleEleOverlapTool('EleEleORT', **common_args)

    # Electron-muon
    if doElectrons and doMuons:
        orTool.EleMuORT = EleMuSharedTrkOverlapTool('EleMuORT', **common_args)
    # Electron-jet
    if doElectrons and doJets:
        orTool.EleJetORT = EleJetOverlapTool('EleJetORT',
                                             BJetLabel=bJetLabel,
                                             UseSlidingDR=boostedLeptons,
                                             **common_args)
    # Muon-jet
    if doMuons and doJets:
        orTool.MuJetORT = MuJetOverlapTool('MuJetORT',
                                            BJetLabel=bJetLabel,
                                            UseSlidingDR=boostedLeptons,
                                            **common_args)

    # Tau-electron
    if doTaus and doElectrons:
        orTool.TauEleORT = DeltaROverlapTool('TauEleORT', DR=0.2, **common_args)
    # Tau-muon
    if doTaus and doMuons:
        orTool.TauMuORT = DeltaROverlapTool('TauMuORT', DR=0.2, **common_args)
    # Tau-jet
    if doTaus and doJets:
        orTool.TauJetORT = DeltaROverlapTool('TauJetORT', DR=0.2, **common_args)

    # Photon-electron
    if doPhotons and doElectrons:
        orTool.PhoEleORT = DeltaROverlapTool('PhoEleORT', **common_args)
    # Photon-muon
    if doPhotons and doMuons:
        orTool.PhoMuORT = DeltaROverlapTool('PhoMuORT', **common_args)
    # Photon-jet
    if doPhotons and doJets:
        orTool.PhoJetORT = DeltaROverlapTool('PhoJetORT', **common_args)

    # Electron-fatjet
    if doElectrons and doFatJets:
        orTool.EleFatJetORT = DeltaROverlapTool('EleFatJetORT', DR=1.0, **common_args)
    # Jet-fatjet
    if doJets and doFatJets:
        orTool.JetFatJetORT = DeltaROverlapTool('JetFatJetORT', DR=1.0, **common_args)

    return orTool


#-------------------------------------------------------------------------------
def harmonized_tools(name='OverlapRemovalTool', OutputLevel=INFO,
                     input_label='selected', output_label='overlaps',
                     output_pass_value=False, do_taus=True, do_photons=True):
    """
    Provides the pre-configured overlap removal tools according to the
    recommendations of the harmonization document. The recommended_tools
    function gives the updated recommendations.

    DEPRECATED - please update to latest recommendations.
    """
    from warnings import warn
    warn('The harmoinzation OR recommendations are deprecated')

    # Call the above function for standard pre-configured tools
    orTool = recommended_tools(name, OutputLevel, input_label, output_label,
                               output_pass_value=output_pass_value,
                               do_taus=do_taus, do_photons=do_photons)

    # TODO: RESTORE TAU-LOOSE-LEP
    # Use tau loose-lep OR implementations
    if do_taus:
        tau_args = {
            'InputLabel' : inputLabel,
            'OutputLabel' : outputLabel,
            'OutputPassValue' : outputPassValue
        }
        orTool.TauEleORT = TauLooseEleOverlapTool('TauEleORT', **tau_args)
        orTool.TauMuORT = TauLooseMuOverlapTool('TauMuORT', **tau_args)

    # Override properties
    orTool.EleMuORT.RemoveCaloMuons = False
    orTool.MuJetORT.ApplyRelPt = False
    orTool.MuJetORT.UseGhostAssociation = False
    orTool.MuJetORT.InnerDR = 0.4

    return orTool
