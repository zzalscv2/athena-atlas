#!/usr/bin/env python

"""
Plot trigger and reconstruction efficiencies over entire data-periods.
"""

import numpy as np
import pandas as pd
import ROOT as R
import python_tools as pt
import math
from array import array
import time
import argparse
import os
import glob
    
parser = argparse.ArgumentParser()
parser.add_argument('--year', type=str, help='15-18, all for full Run-2')
parser.add_argument('--channel', type=str, help='Zee or Zmumu')
parser.add_argument('--indir', type=str, help='Input directory for CSV files')
parser.add_argument('--outdir', type=str, help='Output directory for plots')

args    = parser.parse_args()
year    = args.year
channel = args.channel
indir = args.indir
outdir = args.outdir

if year == "run3": 
    years = ["22", "23"]
    out_tag = "_run3"
    time_format = "%m/%y"
    if args.absolute:
        ymin, ymax = 0.5, 1.1
    else:
        ymin, ymax = 0.5, 1.1
    xtitle = 'Month / Year'
    date_tag = "Run 3, #sqrt{s} = 13.6 TeV"
    norm_type = "Run3"
    if channel != None: 
        xval = 0.30
        yval = 0.33
    else:
        xval = 0.43
        yval = 0.33
    set_size = 1
else: 
    years = [year]
    out_tag = year
    time_format = "%d/%m"
    ymin, ymax = 0.5, 1.1
    xtitle = 'Date in 20' + year
    date_tag = "Data 20" + year  + ", #sqrt{s} = 13.6 TeV"
    norm_type = "year"
    xval = 0.235
    yval = 0.86
    set_size = 0

def main():
    plot_efficiency(channel, years)

def plot_efficiency(channel, years):

    print("------------------------------------------")
    print("Begin Yearwise Efficiency Plots vs Time")
    print("------------------------------------------")

    arr_date  = []
    arr_trigeff = []
    arr_trigerr = []
    arr_recoeff  = []
    arr_recoerr = []
    run_num   = []

    for year in years:
        print("year = ", year)

        if year == "23":

            grl = []
            print("grl before = ")
            print(grl)
            maindir = args.indir

            grl = pt.get_grl(year)

            print("grl = ")
            print(grl)

        elif year == "22":

            grl = []
            print("grl before = ")
            print(grl)
            maindir = "/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/CSVOutputs/HighMu/data22_13p6TeV/temp/physics_Main_MC23a/"

            grl = pt.get_grl(year)

            print("grl = ")
            print(grl)

        for run in grl:

            print('Begin Run ', run, 'processing')

            dfz = pd.read_csv(maindir + "run_" + run + ".csv")
            dfz_small = dfz
            dfz_small['ZLumi']    = dfz_small[channel + 'Lumi']
            dfz_small['ZLumiErr'] = dfz_small[channel + 'LumiErr']
            dfz_small['TrigEff'] = dfz_small[channel + 'EffTrig']
            dfz_small['TrigErr'] = dfz_small[channel + 'ErrTrig']
            dfz_small['RecoEff'] = dfz_small[channel + 'EffReco']
            dfz_small['RecoErr'] = dfz_small[channel + 'ErrReco']
            dfz_small['LBLive'] = dfz_small['LBLive']
            dfz_small = dfz_small.drop(dfz_small[dfz_small.ZLumi == 0].index)

            # Cut out all runs shorter than 40 minutes
            if dfz_small['LBLive'].sum()/60 < 40:
                print("Skip Run", run, "because of live time", dfz_small['LBLive'].sum()/60, "min")
                continue
        
            # Grab start of the run for plotting later on
            run_start = dfz_small['LBStart'].iloc[0]
            timestamp = time.gmtime(run_start)
            timestamp = R.TDatime(timestamp[0], timestamp[1], timestamp[2], timestamp[3], timestamp[4], timestamp[5])
            timestamp = timestamp.Convert()

            # Calculate average trigger efficiency
            dfz_small['TrigEff'] *= dfz_small['LBLive']
            total_time = dfz_small['LBLive'].sum()
            trig_eff_avg = dfz_small['TrigEff'].sum()/total_time

            # Calculate average reconstruction efficiency
            dfz_small['RecoEff'] *= dfz_small['LBLive']
            total_time = dfz_small['LBLive'].sum()
            reco_eff_avg = dfz_small['RecoEff'].sum()/total_time

            # Calculate average trigger efficiency error
            dfz_small['TrigErr'] *= dfz_small['LBLive']
            dfz_small['TrigErr'] *= dfz_small['TrigErr']
            total_time = dfz_small['LBLive'].sum()
            trig_err_avg = math.sqrt(dfz_small['TrigErr'].sum())/total_time

            # Calculate average reconstruction efficiency error
            dfz_small['RecoErr'] *= dfz_small['LBLive']
            dfz_small['RecoErr'] *= dfz_small['RecoErr']
            total_time = dfz_small['LBLive'].sum()
            reco_err_avg = math.sqrt(dfz_small['RecoErr'].sum())/total_time
            
            arr_date.append(timestamp)
            arr_trigeff.append(trig_eff_avg)
            print('Average trigger efficiency = ', trig_eff_avg)
            arr_trigerr.append(trig_err_avg)
            arr_recoeff.append(reco_eff_avg)
            print('Average reconstruction efficiency = ', reco_eff_avg)
            arr_recoerr.append(reco_err_avg)
            run_num.append(run)

    arr_date = array('d', arr_date)
    print("date array = ", arr_date)

    arr_trigeff = np.array(arr_trigeff)
    print("trig array = ", arr_trigeff)
    arr_trigerr = np.array(arr_trigerr)
    arr_recoeff = np.array(arr_recoeff)
    print("reco array = ", arr_recoeff)
    arr_recoerr = np.array(arr_recoerr)

    if channel == "Zee": 
        lep = "e"
        channel_string = "Z #rightarrow ee"
        ymin, ymax = 0.79, 0.92
    elif channel == "Zmumu": 
        lep = "#mu"
        channel_string = "Z #rightarrow #mu#mu"
        ymin, ymax = 0.69, 0.96

    trig_graph = R.TGraphErrors(len(arr_date), arr_date, arr_trigeff, R.nullptr,arr_trigerr);
    trig_graph.GetHistogram().SetYTitle("Efficiency")
    trig_graph.GetHistogram().GetYaxis().SetRangeUser(ymin, ymax)
    trig_graph.GetXaxis().SetTimeDisplay(2)
    trig_graph.GetXaxis().SetNdivisions(9,R.kFALSE)
    trig_graph.GetXaxis().SetTimeFormat(time_format)
    trig_graph.GetXaxis().SetTimeOffset(0,"gmt")
    trig_graph.SetMarkerSize(1)
        
    reco_graph = R.TGraphErrors(len(arr_date), arr_date, arr_recoeff, R.nullptr,arr_recoerr);
    reco_graph.GetHistogram().GetYaxis().SetRangeUser(ymin, ymax)
    reco_graph.GetXaxis().SetTimeDisplay(2)
    reco_graph.GetXaxis().SetNdivisions(9,R.kFALSE)
    reco_graph.GetXaxis().SetTimeFormat(time_format)
    reco_graph.GetXaxis().SetTimeOffset(0,"gmt")
    reco_graph.SetMarkerSize(1)
    reco_graph.SetMarkerStyle(21)
    reco_graph.SetMarkerColor(R.kRed)
    reco_graph.SetLineColor(R.kRed)

    c1 = R.TCanvas()

    trig_graph.Draw("ap")
    reco_graph.Draw("p")

    if channel == "Zee":
        
        leg = R.TLegend(0.645, 0.74, 0.805, 0.94)
        pt.drawAtlasLabel(0.2, ymax-0.06, "Internal")
        if year in ['15', '16', '17', '18']:
            pt.drawText(0.2, ymax-0.12, "Data 20" + year + ", #sqrt{s} = 13 TeV")
        else:
            pt.drawText(0.2, ymax-0.12, "Data 20" + year + ", #sqrt{s} = 13.6 TeV")
        pt.drawText(0.2, ymax-0.18, channel_string + " counting")

    elif channel == "Zmumu":
        
        leg = R.TLegend(0.645, 0.4, 0.805, 0.6)
        pt.drawAtlasLabel(0.2, ymax-0.4, "Internal")
        if year in ['15', '16', '17', '18']:
            pt.drawText(0.2, ymax-0.46, "Data 20" + year + ", #sqrt{s} = 13 TeV")
        else:
            pt.drawText(0.2, ymax-0.46, "Data 20" + year + ", #sqrt{s} = 13.6 TeV")
        pt.drawText(0.2, ymax-0.52, channel_string + " counting")

    leg.SetBorderSize(0)
    leg.SetTextSize(0.07)
    leg.AddEntry(reco_graph, "#varepsilon_{reco}^{single-"+lep+"}", "ep")
    leg.AddEntry(trig_graph, "#varepsilon_{trig}^{single-"+lep+"}", "ep")
    
    leg.Draw()
    
    if channel == "Zee":
        new_trig_line = R.TLine(1683743066.0, 0.79, 1683743066.0, 0.92)

    elif channel == "Zmumu":
        new_trig_line = R.TLine(1683743066.0, 0.69, 1683743066.0, 0.96)
        
    new_trig_line.SetLineColor(R.kBlue)
    new_trig_line.SetLineWidth(3)
    new_trig_line.SetLineStyle(2)
    new_trig_line.Draw("same")
    R.gPad.Update()

    trig_graph.GetHistogram().SetXTitle("Date")
    c1.SaveAs(outdir + "eff_v_time_"+channel+"_data"+out_tag+"_"+".eps")
    c1.SaveAs(outdir + "eff_v_time_"+channel+"_data"+out_tag+"_"+".pdf")

if __name__ == "__main__":
    pt.setAtlasStyle()
    R.gROOT.SetBatch(R.kTRUE)
    main()
