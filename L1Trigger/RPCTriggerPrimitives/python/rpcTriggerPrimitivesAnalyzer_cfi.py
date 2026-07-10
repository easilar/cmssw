import FWCore.ParameterSet.Config as cms

rpcTriggerPrimitivesAnalyzer = cms.EDAnalyzer("RPCTriggerPrimitivesAnalyzer",
    inputRecHits = cms.InputTag("hltRpcRecHits"),
    triggerPrimitives = cms.InputTag("rpcTriggerPrimitiveDigis"),
)
