import FWCore.ParameterSet.Config as cms

process = cms.Process("ANALYSIS")

# Message logger
process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 1

# Geometry and conditions (needed for RPCGeometry)
process.load("Configuration.Geometry.GeometryExtendedRun4D121Reco_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, "auto:phase2_realistic_T33", "")

# Input: read the output file from the L1 rerun
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        "file:/afs/cern.ch/work/e/ecasilar/L1TUpgrade/CMSSW_15_1_0_pre4/src/output_Phase2_L1T.root"
    )
)

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(-1))

# TFileService for histogram output
process.TFileService = cms.Service("TFileService",
    fileName = cms.string("rpcTP_histograms.root")
)

# Our analyzer
process.load("L1Trigger.RPCTriggerPrimitives.rpcTriggerPrimitivesAnalyzer_cfi")

# The TP collection was produced with process name "L1P2GT", input RecHits with "HLT"
process.rpcTriggerPrimitivesAnalyzer.inputRecHits = cms.InputTag("hltRpcRecHits", "", "HLT")
process.rpcTriggerPrimitivesAnalyzer.triggerPrimitives = cms.InputTag("rpcTriggerPrimitiveDigis", "", "L1P2GT")

process.p = cms.Path(process.rpcTriggerPrimitivesAnalyzer)
