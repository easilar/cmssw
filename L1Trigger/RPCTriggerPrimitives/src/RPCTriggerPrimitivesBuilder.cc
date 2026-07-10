#include "L1Trigger/RPCTriggerPrimitives/interface/RPCTriggerPrimitivesBuilder.h"

#include <map>
#include <vector>

RPCTriggerPrimitivesBuilder::RPCTriggerPrimitivesBuilder(const edm::ParameterSet& conf) {
  maxClusterWidthRPC_ = conf.getParameter<int>("maxClusterWidthRPC");
  maxClusterWidthIRPC_ = conf.getParameter<int>("maxClusterWidthIRPC");
  skipBarrel_ = conf.getParameter<bool>("skipBarrel");
  skipOverlap_ = conf.getParameter<bool>("skipOverlap");
  minBX_ = conf.getParameter<int>("minBX");
  maxBX_ = conf.getParameter<int>("maxBX");
  verbose_ = conf.getParameter<int>("verbose");
}

RPCTriggerPrimitivesBuilder::~RPCTriggerPrimitivesBuilder() {}

bool RPCTriggerPrimitivesBuilder::isIRPC(const RPCDetId& id) const {
  // iRPC: endcap chambers at stations 3 and 4 with ring 1
  return (id.region() != 0) && (id.station() >= 3) && (id.ring() == 1);
}

bool RPCTriggerPrimitivesBuilder::passesSelection(const RPCDetId& id, const RPCRecHit& rechit) const {
  // Skip barrel
  if (skipBarrel_ && id.region() == 0) {
    return false;
  }

  // Skip overlap region: RE1/3 and RE2/3
  if (skipOverlap_ && id.station() <= 2 && id.ring() == 3) {
    return false;
  }

  // BX window
  if (rechit.BunchX() < minBX_ || rechit.BunchX() > maxBX_) {
    return false;
  }

  // Cluster width cut
  int maxWidth = isIRPC(id) ? maxClusterWidthIRPC_ : maxClusterWidthRPC_;
  if (rechit.clusterSize() > maxWidth) {
    return false;
  }

  return true;
}

void RPCTriggerPrimitivesBuilder::build(const RPCRecHitCollection* rpcRecHits, RPCRecHitCollection& output) {
  // RPCRecHitCollection is a RangeMap: all rechits for a given DetId must
  // be inserted together. First collect passing rechits grouped by DetId,
  // then insert each group into the output.

  std::map<RPCDetId, std::vector<RPCRecHit>> selectedHits;

  for (auto it = rpcRecHits->begin(); it != rpcRecHits->end(); ++it) {
    const RPCRecHit& rechit = *it;
    const RPCDetId& detId = rechit.rpcId();

    if (!passesSelection(detId, rechit)) {
      if (verbose_ > 1) {
        edm::LogInfo("RPCTriggerPrimitives")
            << "Rejected RPC rechit: region=" << detId.region() << " station=" << detId.station()
            << " ring=" << detId.ring() << " sector=" << detId.sector() << " roll=" << detId.roll()
            << " BX=" << rechit.BunchX() << " clusterSize=" << rechit.clusterSize();
      }
      continue;
    }

    if (verbose_ > 0) {
      edm::LogInfo("RPCTriggerPrimitives")
          << "Accepted RPC TP: region=" << detId.region() << " station=" << detId.station()
          << " ring=" << detId.ring() << " sector=" << detId.sector() << " roll=" << detId.roll()
          << " strip=" << rechit.firstClusterStrip() << " clusterSize=" << rechit.clusterSize()
          << " BX=" << rechit.BunchX() << " time=" << rechit.time()
          << " iRPC=" << isIRPC(detId);
    }

    selectedHits[detId].push_back(rechit);
  }

  // Insert grouped rechits into the output RangeMap
  for (auto& [detId, rechits] : selectedHits) {
    output.put(detId, rechits.begin(), rechits.end());
  }
}
