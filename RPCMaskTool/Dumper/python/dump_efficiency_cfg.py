# dump_efficiency_cfg.py
#
# Walks every roll in the RPCGeometry and writes a TSV with that roll's
# assigned strip efficiencies (mean/min/max/n_dead) according to the
# conditions DB attached to the chosen Global Tag.
#
# No input file needed.
#
# Usage:
#   cmsRun python/dump_efficiency_cfg.py
#     [globalTag=auto:phase2_realistic_T33]
#     [era=Phase2C17I13M9]
#     [outFile=rpc_geometry_efficiency.tsv]
#     [effThreshold=0.0]

import importlib
import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing

opt = VarParsing("analysis")
opt.register("globalTag", "auto:phase2_realistic_T33",
             VarParsing.multiplicity.singleton, VarParsing.varType.string,
             "GlobalTag string passed to GlobalTag(...).")
opt.register("era", "Phase2C17I13M9",
             VarParsing.multiplicity.singleton, VarParsing.varType.string,
             "Era cff to use, e.g. 'Run3', 'Run2_2018'.")
opt.register("outFile", "rpc_geometry_efficiency.tsv",
             VarParsing.multiplicity.singleton, VarParsing.varType.string,
             "Output TSV path (one row per roll).")
opt.register("effThreshold", 0.0,
             VarParsing.multiplicity.singleton, VarParsing.varType.float,
             "Strips with eff <= threshold are counted as 'dead'.")
opt.register("firstRun", 1,
             VarParsing.multiplicity.singleton, VarParsing.varType.int,
             "Run number for the EmptySource.")
opt.parseArguments()

era_mod = importlib.import_module(f"Configuration.Eras.Era_{opt.era}_cff")
era = getattr(era_mod, opt.era)

process = cms.Process("RPCEFFDUMP", era)

process.load("Configuration.StandardSequences.GeometryRecoDB_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, opt.globalTag, "")

process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(1))
process.source = cms.Source(
    "EmptySource",
    firstRun=cms.untracked.uint32(opt.firstRun),
    numberEventsInRun=cms.untracked.uint32(1),
)

process.dumper = cms.EDAnalyzer(
    "RPCEfficiencyDumper",
    outFile      = cms.untracked.string(opt.outFile),
    effThreshold = cms.untracked.double(opt.effThreshold),
)
process.p = cms.Path(process.dumper)

process.MessageLogger = cms.Service(
    "MessageLogger",
    cerr=cms.untracked.PSet(threshold=cms.untracked.string("WARNING"),
                             FwkReport=cms.untracked.PSet(
                                 reportEvery=cms.untracked.int32(1))),
)
