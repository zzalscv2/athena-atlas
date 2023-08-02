#!/bin/bash

#script to make all plots

indir="/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/CSVOutputs/HighMu/data23_13p6TeV/temp/physics_Main_customgrl3/"
outdir="/eos/home-j/jnewell/data23_13p6TeV_physics_plot_customgrl3/"

filelist=$(ls $indir)
for file in $filelist;
do
	
	filename=${file/run_/}
	run_number=${filename/.csv/}

	mkdir -p $outdir$run_number

	# Kinematic plots
	#python plotting/plot_kinematics.py --infile $infile
	
	# Time dependent efficiency and luminosity plots
	 python plotting/efficiency_new.py --infile $indir$file --outdir $outdir
	 python plotting/luminosity_new.py --infile $indir$file --outdir $outdir
	 python plotting/luminosity_new.py --absolute --infile $indir$file --outdir $outdir

	# Pileup dependent efficiency and luminosity plots
	 python plotting/efficiency_new.py --usemu --infile $indir$file --outdir $outdir
	 python plotting/luminosity_new.py --usemu --infile $indir$file --outdir $outdir

done

