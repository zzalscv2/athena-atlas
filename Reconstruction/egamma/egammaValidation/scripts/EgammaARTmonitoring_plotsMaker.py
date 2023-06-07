#!/usr/bin/env python
#
# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration.
#

import sys
from ROOT import (gDirectory, gROOT, gStyle, kTRUE,
                  TCanvas, TFile, TLegend, TPad, kBlack, kBlue,
                  kRed, kGreen, kOrange, kCyan, kPink, kGray)

# gROOT.SetBatch(kTRUE)
gStyle.SetOptStat(0)

cluster_list = [
    {'name': 'clusterAll', 'title': 'Clusters - Inclusive'},
    {'name': 'cluster10GeV', 'title': 'Clusters - 10 GeV'},
    {'name': 'clusterPromptAll', 'title': 'Clusters from Prompt - Inclusive'},
    {'name': 'clusterPrompt10GeV', 'title': 'Clusters from Prompt  - 10 GeV'},
]

cluster_list_photon = [
    {'name': 'clusterUnconvPhoton',
     'title': 'Clusters Unconverted Photons'},
    {'name': 'clusterConvPhoton',
     'title': 'Clusters Converted Photons'},
    {'name': 'clusterConvPhotonSi',
     'title': 'Clusters Converted Photons - Si'},
    {'name': 'clusterConvPhotonSiSi',
     'title': 'Clusters Converted Photons - SiSi'},
    {'name': 'clusterConvPhotonTRT',
     'title': 'Clusters Converted Photons - TRT'},
    {'name': 'clusterConvPhotonTRTTRT',
     'title': 'Clusters Converted Photons - TRTTRT'},
    {'name': 'clusterConvPhotonSiTRT',
     'title': 'Clusters Converted Photons - SiTRT'},
]

photon_cluster_list = [
    {'name': 'clusterUnconvPhoton',
     'title': 'Clusters Unconverted Photons'},
    {'name': 'clusterConvPhoton',
     'title': 'Clusters Converted Photons'},
    {'name': 'clusterConvPhotonSi',
     'title': 'Clusters Converted Photons - Si'},
    {'name': 'clusterConvPhotonSiSi',
     'title': 'Clusters Converted Photons - SiSi'},
    {'name': 'clusterConvPhotonTRT',
     'title': 'Clusters Converted Photons - TRT'},
    {'name': 'clusterConvPhotonTRTTRT',
     'title': 'Clusters Converted Photons - TRTTRT'},
    {'name': 'clusterConvPhotonSiTRT',
     'title': 'Clusters Converted Photons - SiTRT'},

]


electron_comparison_list = [
    {'name': 'showerShapesAll',
     'title': 'Shower Shape - Inclusive'},
    {'name': 'showerShapes10GeV',
     'title': 'Shower Shape - 10 GeV'},
    {'name': 'isolationAll',
     'title': 'Isolation'},
    {'name': 'recoElectronAll',
     'title': 'Reconstructed Electron'},
    {'name': 'truthRecoElectronLooseLH',
     'title': 'Reconstructed Electron LooseLH'},
    {'name': 'truthRecoElectronMediumLH',
     'title': 'Reconstructed Electron MediumLH'},
    {'name': 'truthRecoElectronTightLH',
     'title': 'Reconstructed Electron TightLH'},
    {'name': 'truthElectronAll',
     'title': 'True Electron'},
    {'name': 'truthPromptElectronAll',
     'title': 'True Prompt Electron'},
    {'name': 'truthElectronRecoElectronAll',
     'title': 'True Electron Reconstructed as Electron'},
    {'name': 'truthPromptElectronWithTrack',
     'title': 'True Prompt Electron with Track'},
    {'name': 'truthPromptElectronWithGSFTrack',
     'title': 'True Prompt Electron with GSFTrack'},
    {'name': 'truthPromptElectronWithReco',
     'title': 'True Prompt Electron with Reco Electron'},
    {'name': 'trackingEfficiency',
     'title': 'Tracking Efficiency'},
    {'name': 'GSFEfficiency',
     'title': 'GSF Efficiency'},
    {'name': 'matchingEfficiency',
     'title': 'Matching  Efficiency'},
    {'name': 'reconstructionEfficiency',
     'title': 'Reconstruction Efficiency'},
    {'name': 'recoElectronLooseLHEfficiency',
     'title': 'Reconstructed Electron LooseLH Efficiency'},
    {'name': 'recoElectronMediumLHEfficiency',
     'title': 'Reconstructed Electron MediumLH Efficiency'},
    {'name': 'recoElectronTightLHEfficiency',
     'title': 'Reconstructed Electron TightLH Efficiency'},
]

photon_comparison_list = [
    {'name': 'recoPhotonAll',
     'title': 'Reconstructed Photon'},
    {'name': 'truthPhotonRecoPhoton',
     'title': 'True photon reconstructed as photon'},
    {'name': 'truthConvPhoton',
     'title': 'True converted photon'},
    {'name': 'truthConvRecoConv',
     'title': 'True conversion reconstructed as converted photon'},
    {'name': 'truthConvRecoConv1Si',
     'title': 'True conversion reconstructed as 1 Si conv'},
    {'name': 'truthConvRecoConv1TRT',
     'title': 'True conversion reconstructed as 1 TRT conv'},
    {'name': 'truthConvRecoConv2Si',
     'title': 'True conversion reconstructed as Si-Si conv'},
    {'name': 'truthConvRecoConv2TRT',
     'title': 'True conversion reconstructed as TRT-TRT conv'},
    {'name': 'truthConvRecoConv2SiTRT',
     'title': 'True conversion reconstructed as Si-TRT conv'},
    {'name': 'truthConvRecoUnconv',
     'title': 'True conversion reconstructed as unconverted photon'},
    {'name': 'truthUnconvPhoton', 'title': 'True unconverted photon'},
    {'name': 'truthUnconvRecoConv',
     'title': 'True unconverted reconstructed as conv photon'},
    {'name': 'truthUnconvRecoUnconv',
     'title': 'True unconverted reconstructed as unconverted photon'},
    {'name': 'showerShapesAll',
     'title': 'Shower Shape - Inclusive'},
    {'name': 'showerShapes10GeV',
     'title': 'Shower Shape - 10 GeV'},
    {'name': 'isolationAll',
     'title': 'Isolation'},
    {'name': 'recoPhotonUnconvLooseLH',
     'title': 'Unconverted Photon LooseLH'},
    {'name': 'recoPhotonUnconvTightLH',
     'title': 'Unconverted Photon TightLH'},
    {'name': 'recoPhotonConvLooseLH',
     'title': 'Converted Photon LooseLH'},
    {'name': 'recoPhotonConvTightLH',
     'title': 'Converted Photon TightLH'},
    {'name': 'recoPhotonUnconvIsoFixedCutTight',
     'title': 'FixedCutTight Unconverted Photon'},
    {'name': 'recoPhotonUnconvIsoFixedCutTightCaloOnly',
     'title': 'FixedCutTightCaloOnly Unconverted Photon'},
    {'name': 'recoPhotonUnconvIsoFixedCutLoose',
     'title': 'FixedCutLoose Unconverted Photon'},
    {'name': 'recoPhotonConvIsoFixedCutTight',
     'title': 'FixedCutTight Converted Photon'},
    {'name': 'recoPhotonConvIsoFixedCutTightCaloOnly',
     'title': 'FixedCutTightCaloOnly Converted Photon'},
    {'name': 'recoPhotonConvIsoFixedCutLoose',
     'title': 'FixedCutLoose Converted Photon'},
    {'name': 'truthPhotonUnconvRecoUnconvEfficiency',
     'title': 'True Conv #rightarrow Conv'},
    {'name': 'truthPhotonRecoConvEfficiency',
     'title': 'True Conv #rightarrow Conv'},
    {'name': 'recoPhotonUnconvIsoFixedCutTightEfficiency',
     'title': 'True Conv #rightarrow Conv'},
    {'name': 'recoPhotonUnconvIsoFixedCutTightCaloOnlyEfficiency',
     'title': 'True Conv #rightarrow Conv'},
    {'name': 'recoPhotonUnconvIsoFixedCutLooseEfficiency',
     'title': 'True Conv #rightarrow Conv'},
    {'name': 'recoPhotonConvIsoFixedCutTightEfficiency',
     'title': 'True Conv #rightarrow Conv'},
    {'name': 'recoPhotonConvIsoFixedCutTightCaloOnlyEfficiency',
     'title': 'True Conv #rightarrow Conv'},
    {'name': 'recoPhotonConvIsoFixedCutLooseEfficiency',
     'title': 'True Conv #rightarrow Conv'},
    {'name': 'recoPhotonUnconvLooseLHEfficiency',
     'title': 'Unconverted Photon LooseLH Efficiency'},
    {'name': 'recoPhotonUnconvTightLHEfficiency',
     'title': 'Unconverted Photon TightLH Efficiency'},
    {'name': 'recoPhotonConvLooseLHEfficiency',
     'title': 'Converted Photon LooseLH Efficiency'},
    {'name': 'recoPhotonConvTightLHEfficiency',
     'title': 'Converted Photon TightLH Efficiency'},
]

photon_fraction_list = [
    {'name': 'truthPhotonConvRecoConvEfficiency',
        'color': kBlack, 'title': 'True Conv #rightarrow Conv'},
    {'name': 'truthPhotonConvRecoConv1SiEfficiency', 'color': kBlue +
        2, 'title': 'True Conv #rightarrow 1 Si Conv'},
    {'name': 'truthPhotonConvRecoConv1TRTEfficiency', 'color': kRed +
        2, 'title': 'True Conv #rightarrow 1 TRT Conv'},
    {'name': 'truthPhotonConvRecoConv2SiEfficiency', 'color': kGreen +
        2, 'title': 'True Conv #rightarrow Si-Si Conv'},
    {'name': 'truthPhotonConvRecoConv2TRTEfficiency', 'color': kOrange + 2,
     'title': 'True Conv #rightarrow TRT-TRT Conv'},
    {'name': 'truthPhotonConvRecoConv2SiTRTEfficiency', 'color': kCyan + 2,
     'title': 'True Conv #rightarrow Si-TRT Conv'},
    {'name': 'truthPhotonConvRecoUnconvEfficiency',
        'color': kPink + 2, 'title': 'True Conv #rightarrow Unconv'}
]

photonfake_fraction_list = [
    {'name': 'truthPhotonUnconvRecoConvEfficiency',
        'color': kBlack, 'title': 'True Unconv #rightarrow Conv'},
    {'name': 'truthPhotonUnconvRecoConv1SiEfficiency', 'color': kBlue +
        2, 'title': 'True Unconv #rightarrow 1 Si Conv'},
    {'name': 'truthPhotonUnconvRecoConv1TRTEfficiency', 'color': kRed +
        2, 'title': 'True Unconv #rightarrow 1 TRT Conv'},
    {'name': 'truthPhotonUnconvRecoConv2SiEfficiency', 'color': kGreen +
        2, 'title': 'True Unconv #rightarrow Si-Si Conv'},
    {'name': 'truthPhotonUnconvRecoConv2TRTEfficiency', 'color': kOrange + 2,
     'title': 'True Unconv #rightarrow TRT-TRT Conv'},
    {'name': 'truthPhotonUnconvRecoConv2SiTRTEfficiency', 'color': kCyan + 2,
     'title': 'True Unconv #rightarrow Si-TRT Conv'},
]

photon_efficiency_list = [
    {'name': 'truthPhotonRecoPhotonEfficiency',
        'color': kBlack, 'title': 'All photons'},
    {'name': 'truthPhotonRecoPhotonOrElectronEfficiency', 'color': kGreen + 2,
     'title': 'All photons + electrons'},
    {'name': 'truthPhotonConvRecoEfficiency', 'color': kRed,
     'title': 'True converted'},
    {'name': 'truthPhotonUnconvRecoEfficiency', 'color': kBlue,
        'title': 'True unconverted'}
]

photon_conversion_list = [
    {'name': 'truthConvRecoConv2Si', 'color': kGreen +
        2, 'title': 'True Conv #rightarrow Si-Si Conv'},
    {'name': 'truthConvRecoConv1Si', 'color': kBlue +
        2, 'title': 'True Conv #rightarrow 1 Si Conv'},
    {'name': 'truthConvRecoConv1TRT', 'color': kRed +
        2, 'title': 'True Conv #rightarrow 1 TRT Conv'},
    {'name': 'truthConvRecoConv2TRT', 'color': kOrange +
        2, 'title': 'True Conv #rightarrow TRT-TRT Conv'},
    {'name': 'truthConvRecoConv2SiTRT', 'color': kCyan +
        2, 'title': 'True Conv #rightarrow Si-TRT Conv'},
]

photon_track_list = [
    {'name': 'InDetTracks', 'color': kBlack,
        'title': 'All tracks'},
    {'name': 'InDetTracksMatchElectron', 'color': kOrange,
        'title': 'Matched to true electrons'},
    {'name': 'InDetTracksNotElectron', 'color': kBlue,
        'title': 'Not matched to true electrons'},
    {'name': 'InDetTracksMatchPion', 'color': kGreen +
        2, 'title': 'Matched to true Pion'},
    {'name': 'InDetTracksNotMatched', 'color': kCyan +
        2, 'title': 'Not matched to truth'}
]

photon_trackTRT_list = [
    {'name': 'InDetTracksTRT', 'color': kBlack,
        'title': 'All tracks'},
    {'name': 'InDetTracksTRTMatchElectron', 'color': kOrange,
        'title': 'Matched to true electrons'},
    {'name': 'InDetTracksTRTNotElectron', 'color': kBlue,
        'title': 'Not matched to true electrons'},
    {'name': 'InDetTracksTRTMatchPion', 'color': kGreen +
        2, 'title': 'Matched to true Pion'},
    {'name': 'InDetTracksTRTNotMatched', 'color': kCyan +
        2, 'title': 'Not matched to truth'}
]

photon_trackhighpT_list = [
    {'name': 'InDetTrackshighpT', 'color': kBlack,
        'title': 'All tracks'},
    {'name': 'InDetTracksMatchElectronhighpT', 'color': kOrange,
        'title': 'Matched to true electrons'},
    {'name': 'InDetTracksNotElectronhighpT', 'color': kBlue,
        'title': 'Not matched to true electrons'},
    {'name': 'InDetTracksMatchPionhighpT', 'color': kGreen +
        2, 'title': 'Matched to true Pion'},
    {'name': 'InDetTracksNotMatchedhighpT', 'color': kCyan +
        2, 'title': 'Not matched to truth'}
]

photon_trackTRThighpT_list = [
    {'name': 'InDetTracksTRThighpT', 'color': kBlack,
        'title': 'All tracks'},
    {'name': 'InDetTracksTRTMatchElectronhighpT', 'color': kOrange,
        'title': 'Matched to true electrons'},
    {'name': 'InDetTracksTRTNotElectronhighpT', 'color': kBlue,
        'title': 'Not matched to true electrons'},
    {'name': 'InDetTracksTRTMatchPionhighpT', 'color': kGreen +
        2, 'title': 'Matched to true Pion'},
    {'name': 'InDetTracksTRTNotMatchedhighpT', 'color': kCyan +
        2, 'title': 'Not matched to truth'}
]


def get_key_names(file, directory=""):
    """
    Function to get the key elements name from a given directory
    :param file: TFile
    :param directory: Directory
    :return:
    """
    file.cd(directory)
    return [key.GetName() for key in gDirectory.GetListOfKeys()]


def make_comparison_plots(type, f_base, f_nightly, result_file):
    """

    :param type: electron or gamma
    :param f_base: TFile with the baseline plots
    :param f_nightly: TFile with the nightly plots
    :param result_file: TFile with the resulting comparison
    """
    comparison_list = (
        photon_comparison_list if type == 'gamma'
        else electron_comparison_list)
    for folder in comparison_list:
        for histo in get_key_names(f_nightly, folder['name']):
            h_base = f_base.Get(folder['name'] + '/' + histo)
            h_nightly = f_nightly.Get(folder['name'] + '/' + histo)
            if not h_base or not h_nightly:
                print(histo,' is missing in one of the files ',h_base,h_nightly)
                continue
            if h_base.GetEntries() == 0 or h_nightly.GetEntries() == 0:
                continue
            make_ratio_plot(h_base, h_nightly, folder['title'], result_file)


def makeIQEPlots(inHist, name):
    outHist = inHist.QuantilesX(0.75, "EResolution_IQE_mu")
    outHist.GetXaxis().SetTitle("<#mu>")
    outHist.GetYaxis().SetTitle("IQE")
    outHist25 = inHist.QuantilesX(0.25, "EResolutio_IQE_mu_25")
    outHist.Add(outHist25, -1)
    outHist.Scale(1/1.349)

    return outHist.Clone(inHist.GetName() + "_" + name)


def make_profile_plots(f_base, f_nightly, result_file, particle_type):

    cluster_list_to_loop = cluster_list

    if particle_type == "gamma":
        cluster_list_to_loop = cluster_list + cluster_list_photon

    for i, folder in enumerate(cluster_list_to_loop):
        for histo in get_key_names(f_nightly, folder['name']):
            if '2D' not in histo and not 'profile' in histo:
                continue
            h_base = f_base.Get(folder['name'] + '/' + histo)
            h_nightly = f_nightly.Get(folder['name'] + '/' + histo)
            if not h_base or not h_nightly:
                print(histo,' is missing in one of the files ',h_base,h_nightly)
                continue
            if h_base.GetEntries() == 0 or h_nightly.GetEntries() == 0:
                continue
            if 'mu' in histo:
                h_base = makeIQEPlots(h_base, 'IQE')
                h_nightly = makeIQEPlots(h_nightly, 'IQE')
                y_axis_label = 'IQE'
            else:
                h_base.SetDirectory(0)
                h_nightly.SetDirectory(0)
                y_axis_label = "Mean %s" % (h_base.GetTitle())
                h_base.SetTitle("")
            make_ratio_plot(h_base, h_nightly,
                            folder['title'], result_file, y_axis_label)


def make_conversion_plot(f_base, f_nightly, result_file):
    """
    This function creates conversion plots to study reco vs true
    converion radius for the various conversion categoried
    """
    for histo in get_key_names(f_nightly, 'truthConvRecoConv2Si'):
        variable_name = histo.split("_", 1)[1]

        if variable_name != "convRadiusTrueVsReco":
            continue

        c1 = TCanvas()

        leg = TLegend(0.1, 0.75, 0.9, 0.9)
        leg.SetNColumns(2)

        leg2 = TLegend(0.5, 0.7, 0.9, 0.75)
        leg2.SetNColumns(2)

        for i, folder in enumerate(photon_conversion_list):

            baseline = f_base.Get(
                folder['name'] + '/' + folder['name'] + "_" + variable_name)
            baseline.SetDirectory(0)
            nightly = f_nightly.Get(
                folder['name'] + '/' + folder['name'] + "_" + variable_name)
            nightly.SetDirectory(0)

            if baseline.Integral() != 0:
                baseline.Scale(1/baseline.Integral())
            if nightly.Integral() != 0:
                nightly.Scale(1/nightly.Integral())

            baseline.SetMinimum(
                min(baseline.GetMinimum(), baseline.GetMinimum()) * 0.7)
            baseline.SetMaximum(
                max(baseline.GetMaximum(), baseline.GetMaximum()) * 1.4)

            baseline.GetXaxis().SetTitle(
                "R^{reco}_{conv. vtx} - R^{true}_{conv. vtx} [mm]")
            baseline.GetYaxis().SetTitle("normalized to unity")

            baseline.SetLineColor(folder['color'])
            nightly.SetLineColor(folder['color'])
            baseline.SetMarkerColor(folder['color'])
            nightly.SetMarkerColor(folder['color'])

            baseline.SetMarkerStyle(1)
            nightly.SetMarkerStyle(20)

            leg.AddEntry(nightly, folder['title'], "p")

            if i == 0:
                baseline.Draw("hist ")

                baselineDummy = baseline.Clone()
                baselineDummy.SetLineColor(kGray+3)
                baselineDummy.SetMarkerColor(kGray+3)
                nightlyDummy = nightly.Clone()
                nightlyDummy.SetLineColor(kGray+3)
                nightlyDummy.SetMarkerColor(kGray+3)
                leg2.AddEntry(baselineDummy, "Baseline", "l")
                leg2.AddEntry(nightlyDummy, "Nightly", "p")
            else:
                baseline.Draw("same hist")

            nightly.Draw("p same")

        leg.Draw()
        leg2.Draw()

        c1.Update()

        result_file.cd()

        c1.SaveAs("ConversionRadiusTrueVsReco.png")

        c1.Write("ConversionRadiusTrueVsReco")


def make_photon_fraction_plot(
        f_base, f_nightly, result_file,
        example_folder, folder_list, plot_name,
        axis_title, ymin, ymax, normalize=False):
    """
    This functions created a photon validation plot with efficiencies
    and fractions

    :param f_base TFile with the baseline histograms:
    :param f_nightly TFile with the nightly histograms:
    """
    for histo in get_key_names(f_nightly, example_folder):

        variable_name = histo.split("_", 1)[1]

        c1 = TCanvas()

        leg = TLegend(0.1, 0.75, 0.9, 0.9)
        leg.SetNColumns(2)

        leg2 = TLegend(0.5, 0.7, 0.9, 0.75)
        leg2.SetNColumns(2)

        for i, folder in enumerate(folder_list):

            baseline = f_base.Get(
                folder['name'] + '/' + folder['name'] + "_" + variable_name)
            baseline.SetDirectory(0)
            nightly = f_nightly.Get(
                folder['name'] + '/' + folder['name'] + "_" + variable_name)
            nightly.SetDirectory(0)

            if normalize and 'vs' not in variable_name:
                if baseline.Integral() != 0:
                    baseline.Scale(1/baseline.Integral())
                if nightly.Integral() != 0:
                    nightly.Scale(1/nightly.Integral())

            baseline.SetMinimum(ymin)
            baseline.SetMaximum(ymax)

            baseline.GetYaxis().SetTitle(axis_title)

            baseline.SetLineColor(folder['color'])
            nightly.SetLineColor(folder['color'])
            baseline.SetMarkerColor(folder['color'])
            nightly.SetMarkerColor(folder['color'])

            baseline.SetMarkerStyle(1)
            nightly.SetMarkerStyle(20)

            leg.AddEntry(nightly, folder['title'], "p")

            if i == 0:
                baseline.Draw("hist ")

                baselineDummy = baseline.Clone()
                baselineDummy.SetLineColor(kGray+3)
                baselineDummy.SetMarkerColor(kGray+3)
                nightlyDummy = nightly.Clone()
                nightlyDummy.SetLineColor(kGray+3)
                nightlyDummy.SetMarkerColor(kGray+3)
                leg2.AddEntry(baselineDummy, "Baseline", "l")
                leg2.AddEntry(nightlyDummy, "Nightly", "p")
            else:
                baseline.Draw("same hist")

            nightly.Draw("p same")

        leg.Draw()
        leg2.Draw()

        c1.Update()

        result_file.cd()

        c1.SaveAs(plot_name + "_" + variable_name + ".png")

        c1.Write(plot_name + "_" + variable_name)


def make_ratio_plot(h_base, h_nightly, name, result_file, y_axis_label=None):
    """

    :param h_base: Baseline histogram
    :param h_nightly: Nightly histogram
    :param name: Human-readable name of the histogram
    :param result_file: TFile where the output is saved
    :param y_axis_label: Y axis label is case is needed
    (fraction vs efficiency)
    """
    histogram_name = h_nightly.GetName()

    type_name = histogram_name.split("_", 1)[0]
    variable_name = histogram_name.split("_", 1)[1]

    c1 = TCanvas()

    main_pad = TPad("main_pad", "top", 0.00, 0.25, 1.00, 1.00)
    main_pad.SetLeftMargin(0.12)
    main_pad.SetRightMargin(0.04)
    main_pad.SetTopMargin(0.02)
    main_pad.SetBottomMargin(0.02)
    main_pad.SetTicky(0)
    main_pad.SetTickx(0)
    main_pad.Draw()

    ratio_pad = TPad("ratio_pad", "bottom", 0.00, 0.00, 1.00, 0.25)
    ratio_pad.SetLeftMargin(0.12)
    ratio_pad.SetRightMargin(0.04)
    ratio_pad.SetTopMargin(0.03)
    ratio_pad.SetTickx(0)
    ratio_pad.SetBottomMargin(0.36)
    ratio_pad.Draw()

    h_base.SetLineColor(4)
    h_base.SetLineWidth(2)

    h_nightly.SetMarkerStyle(8)
    h_nightly.SetMarkerSize(0.5)

    main_pad.cd()

    if y_axis_label is not None:
        h_base.GetYaxis().SetTitle(y_axis_label)
        h_base.GetYaxis().SetTitle(y_axis_label)

    if '2D' not in variable_name or 'profile' in variable_name:
        h_base.Draw()

    h_nightly.Draw(
        "same p" if '2D' not in variable_name or 'profile' in variable_name
        else 'colz')

    c1.Update()

    h_base.GetXaxis().SetLabelSize(0)
    h_base.GetXaxis().SetLabelOffset(999)

    h_base.SetMinimum(min(h_base.GetMinimum(), h_nightly.GetMinimum()) * 0.7)
    h_base.SetMaximum(max(h_base.GetMaximum(), h_nightly.GetMaximum()) * 1.3)

    leg = TLegend(0.4, 0.88, 0.9, 0.95)
    leg.SetHeader(name, "C")
    leg.SetNColumns(2)
    leg.SetFillStyle(0)
    leg.SetBorderSize(0)
    leg.AddEntry(h_base, "Baseline", "l")
    leg.AddEntry(h_nightly, "Nightly", "p")
    leg.Draw()

    c1.Update()

    ratio_pad.cd()

    h1clone = h_nightly.Clone()
    h1clone.Sumw2()
    h1clone.SetStats(0)
    h1clone.Divide(h_base)
    h1clone.SetMarkerColor(1)
    h1clone.SetMarkerStyle(20)
    h1clone.GetYaxis().SetRangeUser(0.95, 1.05)
    gStyle.SetOptStat(0)
    h1clone.GetXaxis().SetLabelSize(0.10)
    h1clone.GetXaxis().SetTitleSize(0.17)
    h1clone.GetYaxis().SetLabelSize(0.10)
    h1clone.GetYaxis().SetTitle("Ratio")
    h1clone.GetYaxis().CenterTitle(1)
    h1clone.GetYaxis().SetTitleSize(0.15)
    h1clone.GetYaxis().SetTitleOffset(0.3)
    h1clone.GetYaxis().SetNdivisions(505)

    h1clone.Draw("hist")

    c1.Update()

    result_file.cd()

    c1.SaveAs(type_name + '_' + variable_name + ".png")

    c1.Write(type_name + '_' + variable_name)


if __name__ == '__main__':

    gROOT.SetBatch(kTRUE)
    gStyle.SetOptStat(0)

    baseline_file = TFile(sys.argv[1])
    nightly_file = TFile(sys.argv[2])
    particle_type = sys.argv[3]  # it can be 'electron' or 'gamma'

    output_file = TFile("BN_ComparisonPlots_" +
                        particle_type + ".hist.root", "RECREATE")

    if particle_type == 'gamma':

        make_photon_fraction_plot(
            baseline_file,
            nightly_file,
            output_file, 'truthPhotonConvRecoConvEfficiency',
            photon_fraction_list, 'ConvertionEff_TrueConv',
            "Efficiency and fraction", 0., 1.3)
        make_photon_fraction_plot(
            baseline_file, nightly_file,
            output_file, 'truthPhotonUnconvRecoConvEfficiency',
            photonfake_fraction_list, 'ConvertionEff_TrueUnconv',
            "Efficiency and fraction", 0., 0.2)
        make_photon_fraction_plot(
            baseline_file, nightly_file,
            output_file, 'truthPhotonRecoPhotonEfficiency',
            photon_efficiency_list, 'PhotonEff',
            "Efficiency", 0.8, 1.15)
        make_photon_fraction_plot(
            baseline_file, nightly_file, output_file,
            'InDetTracks', photon_track_list, 'Track',
            "Tracks", 0., 1.3, True)
        make_photon_fraction_plot(
            baseline_file, nightly_file, output_file,
            'InDetTracksTRT',
            photon_trackTRT_list, 'TrackTRT',
            "Tracks", 0., 1.3, True)
        make_photon_fraction_plot(
            baseline_file, nightly_file,
            output_file, 'InDetTrackshighpT',
            photon_trackhighpT_list, 'TrackhighpT',
            "Tracks", 0., 1.3, True)
        make_photon_fraction_plot(
            baseline_file, nightly_file,
            output_file, 'InDetTracksTRThighpT',
            photon_trackTRThighpT_list, 'TrackTRThighpT',
            "Tracks", 0., 1.3, True)
        make_conversion_plot(baseline_file, nightly_file, output_file)

    make_comparison_plots(particle_type, baseline_file,
                          nightly_file, output_file)

    make_profile_plots(baseline_file, nightly_file, output_file, particle_type)
