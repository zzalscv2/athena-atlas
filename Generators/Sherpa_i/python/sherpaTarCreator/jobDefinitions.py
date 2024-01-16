# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import os,glob,shutil,re

def mkGetOpenLoopsJob(options):
    if not ("getOpenLoops" in options.performOnly or "all" in options.performOnly):
        return None

    if len(options.Sherpa_i.OpenLoopsLibs) == 0:
        return None

    # check availability of OL libs in /cvmfs and warn user
    if not options.OLskipcvmfs:
        olpath = str(os.environ['OPENLOOPSPATH'])
        cvmfsInstalledOpenLoopsLibs = glob.glob(olpath + "/proclib/*.so")
        if not any(not any(x in fil for fil in cvmfsInstalledOpenLoopsLibs) for x in options.Sherpa_i.OpenLoopsLibs):
            print("You requested the inclusion of OpenLoops libs in the tarball (genSeq.Sherpa_i.OpenLoopsLibs), but all of them are available centrally in /cvmfs. Will continue without including them in the tarball, and you can remove the genSeq.Sherpa_i.OpenLoopsLibs line from your JO.")
            return None

    # alternatively check wether all needed openLoops libaries are installed
    installedOpenLoopsLibs = glob.glob("Process/OpenLoops/proclib/*.so")
    #each string in openLoopsLibs has to be part of one string in installedOpenLoopsLibs
    if len(options.Sherpa_i.OpenLoopsLibs) == len(installedOpenLoopsLibs) and not any(not any(x in fil for fil in installedOpenLoopsLibs) for x in options.Sherpa_i.OpenLoopsLibs):
        return None

    #delete openloops folder if it exists
    if os.path.exists("Process/OpenLoops"):
        shutil.rmtree("Process/OpenLoops")

    job = options.batchSystemModule.batchJob("0.getOpenLoops", hours=2, nCores=options.ncoresScons, memMB=1, basedir=options.jobOptionDir[0])

    job.cmds += ["source $AtlasSetup/scripts/asetup.sh "+options.athenaVersion]
    job.cmds += ["set -e"]

    olbranch = options.OLbranch if options.OLbranch is not None else ('OpenLoops-'+os.environ['OPENLOOPSVER'])
    job.cmds += ["git clone -b "+olbranch+" https://gitlab.com/openloops/OpenLoops.git"]

    job.cmds += ["cd OpenLoops"]
    job.cmds += ["./scons num_jobs="+str(options.ncoresScons)]
    job.cmds += ["./openloops libinstall process_repositories="+options.OLprocessrepos+" num_jobs="+str(options.ncoresScons)+" "+' '.join(options.Sherpa_i.OpenLoopsLibs)]
    job.cmds += ["cd .."]

    job.cmds += ["rm -rf Process/OpenLoops"]
    job.cmds += ["mkdir -p Process/OpenLoops"]
    job.cmds += ["mv OpenLoops/proclib OpenLoops/lib Process/OpenLoops/"]
    job.cmds += ["rm -rf OpenLoops"]

    job.write()
    job.submit(dryRun=options.dryRun)
    return job


def mkCreateLibsJob(options, prevJob):
    if not ("createLibs" in options.performOnly or "all" in options.performOnly):
        return None

    procName = "Process/*.db" if os.environ["SHERPAVER"].startswith('2.') else "Process/*.zip"
    if len(glob.glob(procName)) > 0:
        return None

    job = options.batchSystemModule.batchJob("1.createLibs", hours=48, nCores=1, memMB=options.createLibsRAM, basedir=options.jobOptionDir[0])

    if prevJob:
        job.dependsOnOk.append(prevJob.id)

    job.cmds += ["source $AtlasSetup/scripts/asetup.sh "+options.athenaVersion]
    job.cmds += ["set -e"]

    if os.environ["SHERPAVER"].startswith('2.'):
      job.cmds += ["rm -rf Process/Amegic.db Process/Comix.db Process/Sherpa.db Process/Amegic"]
      job.cmds += ["echo 'genSeq.Sherpa_i.Parameters += [ \"INIT_ONLY=1\", \"EVENTS=0\", \"FRAGMENTATION=Off\", \"MI_HANDLER=None\", \"LOG_FILE=\"]\n' > events.py"]
    else:
      job.cmds += ["rm -rf Process/Amegic.zip Process/Comix.zip Process/Sherpa.zip Process/Amegic"]
      job.cmds += ["echo 'genSeq.Sherpa_i.BaseFragment += \"\"\"\nINIT_ONLY: 1\nEVENTS: 0\nFRAGMENTATION: Off\nMI_HANDLER: None\n\"\"\"' > events.py"]
      
    job.cmds += ["outputEVNTFile=$(mktemp -u /tmp/XXXXXXXX.pool.root)"]
    job.cmds += ["returncode=0"]
    job.cmds += ["Gen_tf.py --ecmEnergy="+str(options.ecm[0]*1000.)+" --maxEvents=1 --firstEvent=1 --randomSeed=10 --jobConfig="+options.jobOptionDir[0]+" --postInclude=events.py --outputEVNTFile=${outputEVNTFile} || returncode=$?"]
    job.cmds += ["echo Pasting log.generate ==============="]
    job.cmds += ["cat log.generate"]
    job.cmds += ["echo Gen_tf exited with return code $returncode"]
    job.cmds += ["if [ $returncode -eq 251 ]; then"]
    job.cmds += ["  echo 'ERROR: OpenLoops-libary is missing: add missing OpenLoops-libary (see http://openloops.hepforge.org/process_library.php) to variable genSeq.Sherpa_i.OpenLoopsLibs in the jobOption-file, remove Process directory and start again'"]
    job.cmds += ["  exit 1"]
    job.cmds += ["fi"]
    job.cmds += ["if [ $returncode -eq 65 ]; then"]
    job.cmds += ["  if grep -q 'Sherpa: Sherpa::InitializeTheRun throws normal exit' log.generate || grep -q 'ERROR Have to compile Amegic libraries' log.generate; then"]
    job.cmds += ["    echo 'Libraries successfully written out. (Ignore errors above.)'"]
    job.cmds += ["  elif grep -q 'INFO Have to compile Amegic libraries' log.generate; then"]
    job.cmds += ["    echo 'Libraries successfully written out with <=2.2.1. (Ignore errors above.)'"]
    job.cmds += ["  else"]
    job.cmds += ["    echo 'ERROR: Gen_tf.py has failed in an unexpected way. This likely means your job options are incorrect. This log file will have more details.'"]
    job.cmds += ["    exit 1"]
    job.cmds += ["  fi"]
    job.cmds += ["fi"]
    job.cmds += ["if [ $returncode -ne 0 -a $returncode -ne 251 -a $returncode -ne 65 ]; then"]
    job.cmds += ["  echo 'ERROR: unexpexted error from Gen_tf'"]
    job.cmds += ["  exit 1"]
    job.cmds += ["fi"]

    job.cmds += ["rm -rf ${outputEVNTFile} _joproxy* AtRndmGenSvc.out AthenaSummary_Generate.txt Generate_messageSvc_jobOptions.py Generate_runathena PoolFileCatalog.xml PoolFileCatalog.xml.BAK TransformTimer_Generate.pickle athfile-cache.ascii.gz config.pickle dmesg_trf.txt hostnamelookup.tmp inputDictionary.pickle jobInfo.xml jobInfo_Generate.xml jobReport* last.Generate last.runargs.gpickle runargs.Generate.gpickle runargs.Generate.py metadata_Generate.xml metadata.xml Sherpa_References.tex ntuple.pmon.stream setupevprod.sh share ntuple.pmon.gz testHepMC.root events.py Bdecays0.dat Bs2Jpsiphi.DEC DECAY.DEC G4particle_acceptlist.txt PDGTABLE.MeV pdt.table runargs.generate.py runwrapper.generate.sh eventLoopHeartBeat.txt susyParticlePdgid.txt TestHepMC.root log.generate mem.full.generate mem.summary.generate.json env.txt Run.dat Sherpa.yaml"]

    job.write()
    job.submit(dryRun=options.dryRun)
    return job


def mkMakelibsJob(options, prevJob):
    if not ("makelibs" in options.performOnly or "all" in options.performOnly):
        return None

    if os.path.exists("Process/Amegic/lib"):
        return None

    job = options.batchSystemModule.batchJob("2.makelibs", hours=48, nCores=options.ncoresMakelibs, memMB=1, basedir=options.jobOptionDir[0])

    if prevJob:
        job.dependsOnOk.append(prevJob.id)

    job.cmds += ["if ! test -f makelibs; then echo INFO: No makelibs file found; exit 0; fi"]

    job.cmds += ["source $AtlasSetup/scripts/asetup.sh "+options.athenaVersion]

    ## install scons to make it available for makelibs
    job.cmds += ["wget http://prdownloads.sourceforge.net/scons/scons-local-3.1.2.tar.gz"]
    job.cmds += ["tar xzf scons-local-*.tar.gz"]
    job.cmds += ["ln -s scons.py scons"]
    job.cmds += ["export PATH=$PATH:$PWD"]

    job.cmds += ["set -e"]
    job.cmds += ["./makelibs -j"+str(options.ncoresMakelibs)]
    job.cmds += ["rm -rf Process/Amegic/P2_*"]

    job.cmds += ["rm -rf scons*"]

    job.write()
    job.submit(dryRun=options.dryRun)
    return job


def mkIntegrateJob(options, ecm, prevJob):
    ecmfolder = "ecm"+('{0:g}'.format(ecm)).replace(".","p")+"TeV"

    if not ("integrate" in options.performOnly or "all" in options.performOnly):
        return None

    resname = "Results.db" if os.environ["SHERPAVER"].startswith('2.') else "Results.zip"
    if os.path.isfile(ecmfolder+"/"+resname) and os.path.isfile(ecmfolder+"/3.integrate.log"):
        return None

    # calculate integration time
    targetHours = 24
    targetCores = options.Sherpa_i.NCores
    if options.maxCores < options.Sherpa_i.NCores:
        targetHours = int(round(24.*options.Sherpa_i.NCores/options.maxCores))
        targetCores = options.maxCores

    if options.RAM > 100:
        options.Sherpa_i.MemoryMB = options.RAM
    job = options.batchSystemModule.batchJob("3.integrate", hours=targetHours, nCores=targetCores, memMB=options.Sherpa_i.MemoryMB, basedir=options.jobOptionDir[0]+"/"+ecmfolder)

    if prevJob:
        job.dependsOnOk.append(prevJob.id)

    job.cmds += ["source $AtlasSetup/scripts/asetup.sh "+options.athenaVersion]
    job.cmds += ["set -e"]

    if os.environ["SHERPAVER"].startswith('3.'):
        # write base fragment into Base.yaml
        job.cmds += ["cat > Base.yaml <<EOL"]
        job.cmds += [options.Sherpa_i.BaseFragment]
        job.cmds += ["EOL"]
        # disable ATLAS RNG as integration job runs outside Athena
        job.cmds += [r"sed '/.*EXTERNAL_RNG.*/d' -i Base.yaml"]
    #write rundata into Run.dat/Sherpa.yaml file for integration
    configname = "Run.dat" if os.environ["SHERPAVER"].startswith('2.') else "Sherpa.yaml"
    job.cmds += ["cat > "+configname+" <<EOL"]
    job.cmds += [options.Sherpa_i.RunCard]
    job.cmds += ["EOL"]
    #append infos in options
    ecmopt = ["BEAM_ENERGY_1="+str(ecm/2.*1000), "BEAM_ENERGY_2="+str(ecm/2.*1000)] \
             if os.environ["SHERPAVER"].startswith('2.') else ["BEAM_ENERGIES: "+str(ecm/2.*1000)]
    ignoreopt = ["RUNDATA","LOG_FILE","NNPDF_GRID_PATH","RESULT_DIRECTORY"]
    totopt = options.Sherpa_i.Parameters + ecmopt
    if options.Sherpa_i.PluginCode != "":
        if os.environ["SHERPAVER"].startswith('2.'):
            totopt.append("SHERPA_LDADD=Sherpa_iPlugin")
        else:
            totopt.append("SHERPA_LDADD: Sherpa_iPlugin")
    for s in totopt:
        if not (re.split(r'\W',s)[0] in ignoreopt):
            if os.environ["SHERPAVER"].startswith('2.'):
                job.cmds += [r"sed '/.*\}(run).*/i\ \ "+s+"' -i Run.dat"]
            else:
                job.cmds += [r"sed '/.*PROCESSES:.*/i"+s+"\\n' -i Sherpa.yaml"]

    olpath = str(os.environ['OPENLOOPSPATH'])
    lcglayer = olpath[olpath.find("LCG_"):olpath.find("/MCGenerators")]
    sftlayer = "/cvmfs/sft.cern.ch/lcg/releases/" + lcglayer
    lcgbase = "_".join(lcglayer.split('_')[:2]) # without postfix
    sftbase = "/cvmfs/sft.cern.ch/lcg/releases/" + lcgbase
    gccver = "8.3.0"
    with open(sftlayer + "/LCG_externals_" + (os.environ['LCG_PLATFORM']) + '.txt') as f:
      for line in f:
        if line.startswith('COMPILER'):
          gccver = line.strip().split(';')[-1]
          break

    if not options.sherpaInstallPath:
        options.sherpaInstallPath = sftlayer+"/MCGenerators/sherpa/${SHERPAVER}.openmpi3/${LCG_PLATFORM}"


    LCG_PLATFORM = str(os.environ['LCG_PLATFORM'])

    openmpi_path    = ":".join(glob.glob(sftbase+"/openmpi/*/"+LCG_PLATFORM+"/bin"))
    opal_prefix     =  ":".join(glob.glob(sftbase+"/openmpi/*/"+LCG_PLATFORM))
    ld_library_path = ":".join(glob.glob(sftbase+"/openmpi/*/"+LCG_PLATFORM+"/lib"))

    job.cmds += ["source /cvmfs/sft.cern.ch/lcg/releases/gcc/"+gccver+"/x86_64-centos7/setup.sh" ]
    job.cmds += ["export PATH="+openmpi_path+":$PATH"]
    job.cmds += ["export LHAPATH=/cvmfs/sft.cern.ch/lcg/external/lhapdfsets/current:/cvmfs/atlas.cern.ch/repo/sw/Generators/lhapdfsets/current/"]
    job.cmds += ["export OPAL_PREFIX="+opal_prefix]
    job.cmds += ["export LD_LIBRARY_PATH="+ld_library_path+":" \
                                          +sftbase+"/zlib/*/${LCG_PLATFORM}/lib:" \
                                          +sftbase+"/sqlite/*/${LCG_PLATFORM}/lib:" \
                                          +sftbase+"/HepMC/*/${LCG_PLATFORM}/lib:" \
                                          +sftlayer+"/MCGenerators/lhapdf/*/${LCG_PLATFORM}/lib:" \
                                          +sftlayer+"/fastjet/*/${LCG_PLATFORM}/lib:" \
                                          +olpath+"/lib:"+olpath+"/proclib:" \
                                          +options.sherpaInstallPath+"/lib64/SHERPA-MC:" \
                                          +options.sherpaInstallPath+"/lib/SHERPA-MC:$LD_LIBRARY_PATH"]

    cmdLineOpts = "EVENTS=0 FRAGMENTATION=Off MI_HANDLER=None"
    if os.environ["SHERPAVER"].startswith('2.'):
        cmdLineOpts += " BEAM_ENERGY_1="+str(ecm/2.*1000)+" BEAM_ENERGY_2="+str(ecm/2.*1000)
    else:
        cmdLineOpts += " \"RUNDATA: [Base.yaml, Sherpa.yaml]\" BEAM_ENERGIES="+str(ecm/2.*1000)
    job.cmds += ["mpirun -n {0} ".format(str(targetCores))+options.sherpaInstallPath+"/bin/Sherpa "+cmdLineOpts]

    job.write(extraDirs=[options.jobOptionDir[0]])
    job.submit(dryRun=options.dryRun)
    return job



def mkTarballmakerJob(options, ecm, prevJob):
    if not ("makeTarball" in options.performOnly or "all" in options.performOnly):
        return None

    physicsShort = options.mainJOfile.split(".")[1]
    ecmstring = ('{0:g}'.format(ecm)).replace(".","p")+"TeV"
    ecmfolder = "ecm"+ecmstring
    tarballname = "mc_"+ecmstring+"."+physicsShort+".GRID.tar.gz"

    job = options.batchSystemModule.batchJob("4.makeTarball", hours=1, nCores=1, memMB=1, basedir=options.jobOptionDir[0]+"/"+ecmfolder)
    job.cmds += ["set -e"]

    if prevJob:
        job.dependsOnOk.append(prevJob.id)

    resname = "Results.db" if os.environ["SHERPAVER"].startswith('2.') else "Results.zip"
    job.cmds += ["tar czhf "+options.jobOptionDir[0]+"/"+tarballname+" $(ls -d "+resname+" Process 3.integrate.log 2>/dev/null) "+" ".join(options.Sherpa_i.ExtraFiles)]
    for jodir in options.jobOptionDir[1:]:
        job.cmds += ["ln -s "+options.jobOptionDir[0]+"/"+tarballname+" "+jodir]

    job.write(useSingularity=False, extraDirs=options.jobOptionDir)
    job.submit(dryRun=options.dryRun)
    return job




def mkEvntGenTestJob(options, ecm, jodir, prevJob):
    if not ("evgen" in options.performOnly or "all" in options.performOnly):
        return None

    ecmstring = ('{0:g}'.format(ecm)).replace(".","p")+"TeV"
    ecmfolder = "ecm"+ecmstring

    job = options.batchSystemModule.batchJob("5.EvntGenTest", hours=24, nCores=1, memMB=1, basedir=jodir+"/"+ecmfolder)

    if prevJob:
        job.dependsOnOk.append(prevJob.id)

    job.cmds += ["source $AtlasSetup/scripts/asetup.sh "+options.athenaVersion]
    job.cmds += ["set -e"]

    job.cmds += ["rm -rf 5.EvntGenTest"]
    job.cmds += ["mkdir 5.EvntGenTest"]
    job.cmds += ["cd 5.EvntGenTest"]
    if os.environ["SHERPAVER"].startswith('2.'):
      job.cmds += ["echo 'genSeq.Sherpa_i.Parameters += [ \"LOG_FILE=\" ]' > events.py"]
    job.cmds += ["outputEVNTFile=$(mktemp -u /tmp/XXXXXXXX.pool.root)"]
    if os.environ["SHERPAVER"].startswith('2.'):
      job.cmds += ["Gen_tf.py --ecmEnergy="+str(ecm*1000.)+" --maxEvents=1 --firstEvent=1 --randomSeed=10 --jobConfig="+jodir+" --postInclude=events.py --outputEVNTFile=${outputEVNTFile} --maxEvents="+str(options.nEvts)]
    else:
      job.cmds += ["Gen_tf.py --ecmEnergy="+str(ecm*1000.)+" --maxEvents=1 --firstEvent=1 --randomSeed=10 --jobConfig="+jodir+" --outputEVNTFile=${outputEVNTFile} --maxEvents="+str(options.nEvts)]
    job.cmds += ["cat log.generate"]

    ## set min events
    job.cmds += ["post_ini_time=$(grep snapshot_post_ini log.generate | awk '{ print $5}')"]
    job.cmds += ["post_lastevt_time=$(grep snapshot_post_lastevt log.generate | awk '{ print $5}')"]
    job.cmds += ["nPer12h=$(printf '%.0f' $(echo \""+str(options.nEvts)+"*12*60*60*1000/($post_lastevt_time-$post_ini_time)\" | bc -l))"]
    job.cmds += ["finalEventsPerJob=0"]
    job.cmds += ["for i in 1 2 5 10 20 50 100 200 500 1000 2000 5000 10000; do if test $nPer12h -gt $i; then finalEventsPerJob=$i; fi; done"]
    job.cmds += ["echo \"Possible number of events per 12h: ${nPer12h} -> ${finalEventsPerJob} \""]
    job.cmds += ["if grep -q evgenConfig.nEventsPerJob "+jodir+"/mc.*.py; then"]
    job.cmds += [r'  sed -e "s/evgenConfig.nEventsPerJob.*=.*\([0-9]*\)/evgenConfig.nEventsPerJob = ${finalEventsPerJob}/g" -i '+jodir+"/mc.*.py"]
    job.cmds += ["else"]
    job.cmds += ["  echo \"evgenConfig.nEventsPerJob = ${finalEventsPerJob}\" >> "+jodir+"/mc.*.py"]
    job.cmds += ["fi"]

    # cp log.generate to DSID dir such that commit script can run on it
    job.cmds += ["cp log.generate ../../"]

    job.cmds += ["cd .."]
    job.cmds += ["rm -rf 5.EvntGenTest"]

    job.write(extraDirs=[jodir])
    job.submit(dryRun=options.dryRun)
    return job
