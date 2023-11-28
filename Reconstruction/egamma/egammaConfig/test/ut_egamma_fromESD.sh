#!/bin/sh

# From ESD partial egamma reco
python -m  egammaConfig.runEgammaOnlyESD -m 1 > log 2>&1
stat=$?
if [ $stat -eq 0 ] 
then
	echo "=== Egamma Alone from ESD SUCCESS === "
else
	echo "=== Egamma Alone from ESD  FAILURE ==="
	echo " printing full log ===> "
	cat log 
	exit $stat
fi

