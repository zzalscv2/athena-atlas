#!/usr/bin/env python
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from sys import exit

from WorkflowTestRunner.ScriptUtils import setup_logger, setup_parser, get_test_setup, get_standard_performance_checks, \
    run_tests, run_checks, run_summary
from WorkflowTestRunner.StandardTests import PileUpTest, QTest, SimulationTest
from WorkflowTestRunner.Test import WorkflowRun, WorkflowType


def main():
    name = "Run3Tests"
    run = WorkflowRun.Run3

    # Setup the environment
    log = setup_logger(name)
    parser = setup_parser()
    options = parser.parse_args()
    setup = get_test_setup(name, options, log)

    # Define which tests to run
    tests_to_run = []
    if options.simulation:
        if not options.workflow or options.workflow is WorkflowType.FullSim:
            ami_tag = "s3760" if not options.ami_tag else options.ami_tag
            tests_to_run.append(SimulationTest(ami_tag, run, WorkflowType.FullSim, ["EVNTtoHITS"], setup, options.extra_args + " --postInclude Campaigns/postInclude.MC21BirksConstantTune.py"))
        if not options.workflow or options.workflow is WorkflowType.AF3:
            log.error("AF3 not supported yet")
    elif options.overlay:
        log.error("Overlay not supported yet")
        exit(1)
    elif options.pileup:
        if not options.workflow or options.workflow is WorkflowType.PileUpPresampling:
            ami_tag = "d1744" if not options.ami_tag else options.ami_tag
            tests_to_run.append(PileUpTest(ami_tag, run, WorkflowType.PileUpPresampling, ["HITtoRDO"], setup, options.extra_args))
    else:
        if not options.workflow or options.workflow is WorkflowType.MCReco:
            ami_tag = "q445" if not options.ami_tag else options.ami_tag
            if "--CA" in options.extra_args:
                tests_to_run.append(QTest(ami_tag, run, WorkflowType.MCReco, ["HITtoRDO", "RAWtoALL"], setup, options.extra_args + " --steering doRAWtoALL"))
            else:
                tests_to_run.append(QTest(ami_tag, run, WorkflowType.MCReco, ["HITtoRDO", "RDOtoRDOTrigger", "RAWtoALL"], setup, options.extra_args))
        if not options.workflow or options.workflow is WorkflowType.DataReco:
            ami_tag = "q449" if not options.ami_tag else options.ami_tag
            tests_to_run.append(QTest(ami_tag, run, WorkflowType.DataReco, ["RAWtoALL"], setup, options.extra_args))

    # Define which perfomance checks to run
    performance_checks = get_standard_performance_checks(setup)

    # Define and run jobs
    run_tests(setup, tests_to_run)

    # Run post-processing checks
    all_passed = run_checks(setup, tests_to_run, performance_checks)

    # final report
    run_summary(setup, tests_to_run, all_passed)


if __name__ == "__main__":
    main()
