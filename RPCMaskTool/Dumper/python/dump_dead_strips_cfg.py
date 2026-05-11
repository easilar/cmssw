# dump_dead_strips_cfg.py
#
# Self-contained driver for RPCDeadStripDumper. Resolves the RPCStripNoises
# payload from a CMSSW conditions Global Tag and writes the dead-strip list.
#
# No RECO/RAW/SIM/DIGI input file needed — uses EmptySource, just loads
# geometry + GT.
#
# Run with:
#   cmsRun python/dump_dead_strips_cfg.py
#     [globalTag=auto:phase2_realistic_T33]
#     [era=Phase2C17I13M9]
#     [outFile=rpc_dead_strips.tsv]
#     [effThreshold=0.0]
#
# Examples:
#   # Phase II realistic (default)
#   cmsRun python/dump_dead_strips_cfg.py
#
#   # Run-3 data conditions
#   cmsRun python/dump_dead_strips_cfg.py \
#          era=Run3 globalTag=auto:run3_data outFile=run3_data_dead_strips.tsv
#
#   # Run-3 MC
#   cmsRun python/dump_dead_strips_cfg.py \
#          era=Run3 globalTag=auto:run3_mc outFile=run3_mc_dead_strips.tsv

import importlib
import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing

opt = VarParsing("analysis")
opt.register("globalTag", "auto:phase2_realistic_T33",
             VarParsing.multiplicity.singleton, VarParsing.varType.string,
             "GlobalTag string passed to GlobalTag(...) (e.g. 'auto:run3_mc').")
opt.register("era", "Phase2C17I13M9",
             VarParsing.multiplicity.singleton, VarParsing.varType.string,
             "Era cff to use, e.g. 'Phase2C17I13M9', 'Run3', 'Run2_2018'.")
opt.register("outFile", "rpc_dead_strips.tsv",
             VarParsing.multiplicity.singleton, VarParsing.varType.string,
             "Output TSV path.")
opt.register("effThreshold", 0.0,
             VarParsing.multiplicity.singleton, VarParsing.varType.float,
             "Strips with eff <= threshold are considered dead.")
opt.register("firstRun", 1,
             VarParsing.multiplicity.singleton, VarParsing.varType.int,
             "Run number for the EmptySource (sets the conditions IOV).")
opt.parseArguments()

# Dynamically load the requested era. Eras live in Configuration.Eras with
# module name Era_<era>_cff. The era object inside the module has the same name.
era_mod = importlib.import_module(f"Configuration.Eras.Era_{opt.era}_cff")
era = getattr(era_mod, opt.era)

process = cms.Process("RPCDEADDUMP", era)

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
    "RPCDeadStripDumper",
    outFile      = cms.untracked.string(opt.outFile),
    effThreshold = cms.untracked.double(opt.effThreshold),
)
process.p = cms.Path(process.dumper)

# Quiet most of the noise.
process.MessageLogger = cms.Service(
    "MessageLogger",
    cerr=cms.untracked.PSet(
        threshold=cms.untracked.string("WARNING"),
        FwkReport=cms.untracked.PSet(reportEvery=cms.untracked.int32(1)),
    ),
)
