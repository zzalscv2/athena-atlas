#!/bin/bash

#script to make all plots

outdir="/eos/user/s/salibocu/run3/Plots/data22_13p6TeV/"
indir="/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/CSVOutputs/HighMu/data22_13p6TeV/"

filelist=$(ls $indir)
for file in $filelist;
do
	
	filename=${file/run_/}
	run_number=${filename/.csv/}

	mkdir -p $outdir$run_number

	#infile="/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/MergedOutputs/HighMu/data22_13p6TeV/tree_"$run_number".root"
	#infile="/eos/user/s/salibocu/MergedOutputs/v3/tree_"$run_number".root"

	# Kinematic plots
	#python plotting/plot_kinematics.py --infile $infile
	
	# Time dependent efficiency and luminosity plots
	python plotting/efficiency.py --infile $indir$file --outdir $outdir
	python plotting/luminosity.py --infile $indir$file --outdir $outdir
	python plotting/luminosity.py --absolute --infile $indir$file --outdir $outdir

	# Pileup dependent efficiency and luminosity plots
	python plotting/efficiency.py --usemu --infile $indir$file --outdir $outdir
	python plotting/luminosity.py --usemu --infile $indir$file --outdir $outdir

done

