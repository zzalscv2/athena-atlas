#!/usr/bin/env python3

import ROOT as R
import python_tools as pt
import numpy as np
import os
import argparse
import re

parser = argparse.ArgumentParser()
parser.add_argument('--infile', type=str, help='input file path')
parser.add_argument('--outdir', type=str, help='output directory')

args = parser.parse_args()
infilename = args.infile
'''
if "mc" in infilename:
	run_number = "410000"
	run_folder = "run_"+run_number
else:
	run_number = infilename.replace('/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/MergedOutputs/HighMu/data22_13p6TeV/mc16e_cf/tree_', '')
	run_number = run_number.replace('.root', '')
	run_folder = "run_"+run_number
'''
pattern = re.compile(r"tree_(\d+)\.root")
match = pattern.search(infilename)
if match:
	run_number = match.group(1)
else:
	run_number = "410000"
run_folder = "run_"+run_number

#os.system("mkdir -p " +outdir+"/"+run_number+"/kinematics")

def main():
	
	plotter_GeV("m_Z_mass_opsmu", "Z#rightarrow#mu#mu (op. sign) Mass", "m_{#mu#mu} [GeV]", 66, 116, "Entries/GeV")
	plotter_GeV("m_leadingmu_pt", "Leading #mu p_{T}", "p^{#mu}_{T} [GeV]", 27, 200, "Entries/GeV")
	plotter_GeV("m_subleadingmu_pt", "Subleading #mu p_{T}", "p^{#mu}_{T} [GeV]", 27, 200, "Entries/GeV")
	plotter("m_leadingmu_eta", "Leading #mu #eta", "#eta^{#mu}", -2.5, 2.5, "Entries")
	plotter("m_subleadingmu_eta", "Subleading #mu #eta", "#eta^{#mu}", -2.5, 2.5, "Entries")
	plotter("m_leadingmu_phi", "Leading #mu #phi", "#phi^{#mu}", -3.4, 3.4, "Entries")
	plotter("m_subleadingmu_phi", "Subleading #mu #phi", "#phi^{#mu}",-3.4, 3.4, "Entries")

	plotter_GeV("m_Z_mass_opsele", "Z#rightarrow ee (op. sign) Mass", "m_{ee} [GeV]", 66, 116, "Entries/GeV")
	plotter_GeV("m_leadingele_pt", "Leading e p_{T}", "p^{e}_{T} [GeV]", 27, 200, "Entries/GeV")
	plotter_GeV("m_subleadingele_pt", "Subleading e p_{T}", "p^{e}_{T} [GeV]", 27, 200, "Entries/GeV")
	plotter("m_leadingele_eta", "Leading e #eta", "#eta^{e}", -2.5, 2.5, "Entries")
	plotter("m_subleadingele_eta", "Subleading e #eta", "#eta^{e}", -2.5, 2.5, "Entries")
	plotter("m_leadingele_phi", "Leading e #phi", "#phi^{e}", -3.4, 3.4, "Entries")
	plotter("m_subleadingele_phi", "Subleading e #phi", "#phi^{e}",-3.4, 3.4, "Entries")
	
def plotter_GeV(histname, title, xlabel, xmin, xmax, ylabel):

	c1 = R.TCanvas()
	pad1 = R.TPad("pad1", "pad1", 0, 0, 1, 1)
	pad1.SetRightMargin
	rfile = R.TFile(infilename)

	bins = get_bins(histname)

	hist1 = rfile.Get(run_folder+"/GLOBAL/DQTGlobalWZFinder/" + histname)
	hist2 = R.TH1F(histname, title, len(bins)-1, bins)
	for i in range(0, hist1.GetNbinsX()):
		hist2.SetBinContent(i, hist1.GetBinContent(i))
	hist2.Draw("E0")

	hist2.GetXaxis().SetTitle(xlabel)
	hist2.GetXaxis().SetRangeUser(xmin, xmax)
	hist2.GetYaxis().SetTitle(ylabel)

	if "pt" in histname:
		R.gPad.SetLogx()
		hist2.GetXaxis().SetMoreLogLabels()
		R.gPad.SetLogy()
		pt.drawAtlasLabel(0.2, 0.56, "Internal")
		pt.drawText(0.2, 0.50, "Data 2022")
		pt.drawText(0.2, 0.44, "#sqrt{s} = 13.6 TeV")
		pt.drawText(0.2, 0.38, "run "+run_number)
		if "mu" in histname:
			pt.drawText(0.2, 0.32, "p^{#mu}_{T} > 27 GeV")
			pt.drawText(0.2, 0.26, "|#eta^{#mu}| < 2.4")
			pt.drawText(0.2, 0.20, "66 < m_{#mu#mu} < 116 GeV")
		else:
			pt.drawText(0.2, 0.32, "p^{e}_{T} > 27 GeV")
			pt.drawText(0.2, 0.26, "|#eta^{e}| < 1.37 or 1.52 < |#eta^{e}| < 2.4")
			pt.drawText(0.2, 0.20, "66 < m_{ee} < 116 GeV")
		
	else:
		hist2.GetYaxis().SetRangeUser(0, hist2.GetMaximum()*1.2)
		pt.drawAtlasLabel(0.2, 0.86, "Internal")
		pt.drawText(0.2, 0.80, "Data 2022")
		pt.drawText(0.2, 0.74, "#sqrt{s} = 13.6 TeV")
		pt.drawText(0.2, 0.68, "run "+run_number)
		if "mu" in histname:
			pt.drawText(0.6, 0.86, "p^{#mu}_{T} > 27 GeV")
			pt.drawText(0.6, 0.80, "|#eta^{#mu}| < 2.4")
			pt.drawText(0.6, 0.74, "66 < m_{#mu#mu} < 116 GeV")
		else:
			pt.drawText(0.6, 0.86, "p^{e}_{T} > 27 GeV")
			pt.drawText(0.6, 0.80, "|#eta^{e}| < 1.37 OR")
			pt.drawText(0.6, 0.74, "1.52 < |#eta^{e}| < 2.4")
			pt.drawText(0.6, 0.68, "66 < m_{ee} < 116 GeV")

	c1.SaveAs(outdir + histname + ".pdf")

def plotter(histname, title, xlabel, xmin, xmax, ylabel):

	c1 = R.TCanvas()
	pad1 = R.TPad("pad1", "pad1", 0, 0, 1, 1)
	pad1.SetRightMargin
	rfile = R.TFile(infilename)

	bins = get_bins(histname)

	hist = rfile.Get(run_folder+"/GLOBAL/DQTGlobalWZFinder/" + histname)

	hist.Draw("HIST")

	hist.GetXaxis().SetTitle(xlabel)
	hist.GetXaxis().SetRangeUser(xmin, xmax)
	hist.GetYaxis().SetRangeUser(0, hist.GetMaximum()*1.7)
	hist.GetYaxis().SetTitle(ylabel)
		
	pt.drawAtlasLabel(0.2, 0.86, "Internal")
	pt.drawText(0.2, 0.80, "Data 2022")
	pt.drawText(0.2, 0.74, "#sqrt{s} = 13.6 TeV")
	pt.drawText(0.2, 0.68, "run "+run_number)
	if "mu" in histname:
		pt.drawText(0.6, 0.86, "p^{#mu}_{T} > 27 GeV")
		pt.drawText(0.6, 0.80, "|#eta^{#mu}| < 2.4")
		pt.drawText(0.6, 0.74, "66 < m_{#mu#mu} < 116 GeV")
	else:
		pt.drawText(0.6, 0.86, "p^{e}_{T} > 27 GeV")
		pt.drawText(0.6, 0.80, "|#eta^{e}| < 1.37 OR")
		pt.drawText(0.6, 0.74, "1.52 < |#eta^{e}| < 2.4")
		pt.drawText(0.6, 0.68, "66 < m_{ee} < 116 GeV")

	c1.SaveAs(outdir + histname + ".pdf")

def get_bins(histname):

	zCutLow = 66
	zCutHigh = 116
	nzbins = int(zCutHigh - zCutLow)

	if "mass" in histname:
		bins = np.linspace(zCutLow, zCutHigh, nzbins+1)
	elif "pt" in histname:
		bins = np.linspace(0, 200, 201)
	elif "eta" in histname:
		bins = np.linspace(-2.5, 2.5, 50)
	elif "phi" in histname:
		bins = np.linspace(-3.4, 3.4, 34)
	else:
		bins = np.linspace(0, 100, 101)

	return bins	

if __name__ == "__main__":
	pt.setAtlasStyle()
	R.gROOT.SetBatch(R.kTRUE)
	main()
