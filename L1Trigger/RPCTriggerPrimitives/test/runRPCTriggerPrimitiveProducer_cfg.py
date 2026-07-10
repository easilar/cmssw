import FWCore.ParameterSet.Config as cms

process = cms.Process("RPCTriggerPrimitives")

# Message logger
process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 1

# Source: replace with your input file
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        # Add your input file here, e.g.:
        # 'file:/path/to/your/input.root'
    )
)

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(10))

# Load RPC Trigger Primitive producer
process.load("L1Trigger.RPCTriggerPrimitives.rpcTriggerPrimitiveDigis_cfi")

# Enable verbose output for testing
process.rpcTriggerPrimitiveDigis.verbose = cms.int32(1)

# Output
process.out = cms.OutputModule("PoolOutputModule",
    fileName = cms.untracked.string("rpcTriggerPrimitives.root"),
    outputCommands = cms.untracked.vstring(
        "drop *",
        "keep *_rpcTriggerPrimitiveDigis_*_*",
        "keep *_rpcRecHits_*_*",
    )
)

# Path and EndPath
process.p = cms.Path(process.rpcTriggerPrimitiveDigis)
process.ep = cms.EndPath(process.out)
