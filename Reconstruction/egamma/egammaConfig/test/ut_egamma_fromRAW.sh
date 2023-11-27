#!/bin/sh

# From RAW Calo, Tracking, egamma
python -m  egammaConfig.runEgammaOnly -m 1 > log 2>&1
stat=$?
if [ $stat -eq 0 ] 
then
	echo "=== Egamma Alone from RAW SUCCESS === "
else
	echo "=== Egamma Alone from RAW  FAILURE ==="
	echo " printing full log ===> "
	cat log 
	exit $stat
fi

