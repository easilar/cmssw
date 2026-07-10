#ifndef L1Trigger_RPCTriggerPrimitives_RPCTriggerPrimitivesBuilder_h
#define L1Trigger_RPCTriggerPrimitives_RPCTriggerPrimitivesBuilder_h

/** \class RPCTriggerPrimitivesBuilder
 *
 * Algorithm to select and filter RPC trigger primitives from RPC RecHits.
 *
 * Filters out barrel hits, overlap region hits (RE1/3, RE2/3),
 * and applies cluster width cuts (different for standard RPC and iRPC).
 *
 * Configured via the Producer's ParameterSet.
 */

#include "DataFormats/RPCRecHit/interface/RPCRecHit.h"
#include "DataFormats/RPCRecHit/interface/RPCRecHitCollection.h"
#include "DataFormats/MuonDetId/interface/RPCDetId.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

class RPCTriggerPrimitivesBuilder {
public:
  explicit RPCTriggerPrimitivesBuilder(const edm::ParameterSet&);
  ~RPCTriggerPrimitivesBuilder();

  void build(const RPCRecHitCollection* rpcRecHits, RPCRecHitCollection& output);

private:
  // Determine if a chamber is iRPC (improved RPC): endcap, station >= 3, ring == 1
  bool isIRPC(const RPCDetId& id) const;

  // Check if a rechit passes all selection criteria
  bool passesSelection(const RPCDetId& id, const RPCRecHit& rechit) const;

  // Configurable parameters
  int maxClusterWidthRPC_;   // max cluster width for standard RPC (default: 4)
  int maxClusterWidthIRPC_;  // max cluster width for iRPC (default: 6)
  bool skipBarrel_;          // skip barrel RPC (default: true)
  bool skipOverlap_;         // skip overlap region RE1/3, RE2/3 (default: true)
  int minBX_;                // minimum BX to accept (default: -1)
  int maxBX_;                // maximum BX to accept (default: 1)
  int verbose_;
};

#endif
