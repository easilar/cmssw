import FWCore.ParameterSet.Config as cms

rpcTriggerPrimitiveDigis = cms.EDProducer(
    "RPCTriggerPrimitivesProducer",
    # Input collection: RPC RecHits from local reconstruction
    RPCRecHitProducer = cms.InputTag("rpcRecHits"),
    # Maximum cluster width for standard RPC (endcap, ring 2-3)
    maxClusterWidthRPC = cms.int32(4),
    # Maximum cluster width for iRPC (endcap, station >= 3, ring 1)
    maxClusterWidthIRPC = cms.int32(6),
    # Skip barrel RPC hits (only produce endcap trigger primitives)
    skipBarrel = cms.bool(True),
    # Skip overlap region hits (RE1/3, RE2/3)
    skipOverlap = cms.bool(True),
    # BX window for trigger primitive selection
    minBX = cms.int32(-1),
    maxBX = cms.int32(1),
    # Verbosity: 0 = quiet, 1 = accepted TPs, 2 = all (including rejected)
    verbose = cms.int32(0),
)
