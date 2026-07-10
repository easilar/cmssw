/** \class RPCTriggerPrimitivesProducer
 *
 * Implementation of the RPC Level-1 Trigger Primitive producer.
 *
 * Selects and filters RPC RecHits to produce trigger primitives
 * suitable for consumption by the Level-1 muon trigger (e.g. EMTF).
 *
 * Input: RPCRecHitCollection (from RPC local reconstruction)
 *
 * Output: filtered RPCRecHitCollection containing only endcap RPC hits
 * passing cluster width and BX cuts.
 *
 * Filtering criteria (matching what EMTF Phase-2 RPCTPCollector does):
 *   - Skip barrel RPC (region == 0)
 *   - Skip overlap region (RE1/3, RE2/3)
 *   - Cluster width <= 4 strips for standard RPC
 *   - Cluster width <= 6 strips for iRPC (endcap station >= 3, ring == 1)
 *   - BX within configurable window
 */

#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/RPCRecHit/interface/RPCRecHit.h"
#include "DataFormats/RPCRecHit/interface/RPCRecHitCollection.h"

#include "L1Trigger/RPCTriggerPrimitives/interface/RPCTriggerPrimitivesBuilder.h"

class RPCTriggerPrimitivesProducer : public edm::stream::EDProducer<> {
public:
  explicit RPCTriggerPrimitivesProducer(const edm::ParameterSet&);
  ~RPCTriggerPrimitivesProducer() override;

  void produce(edm::Event&, const edm::EventSetup&) override;

private:
  std::unique_ptr<RPCTriggerPrimitivesBuilder> builder_;

  edm::InputTag rpcRecHitProducer_;
  edm::EDGetTokenT<RPCRecHitCollection> rpcRecHitToken_;
};

RPCTriggerPrimitivesProducer::RPCTriggerPrimitivesProducer(const edm::ParameterSet& conf) {
  rpcRecHitProducer_ = conf.getParameter<edm::InputTag>("RPCRecHitProducer");
  rpcRecHitToken_ = consumes<RPCRecHitCollection>(rpcRecHitProducer_);

  produces<RPCRecHitCollection>();

  builder_ = std::make_unique<RPCTriggerPrimitivesBuilder>(conf);
}

RPCTriggerPrimitivesProducer::~RPCTriggerPrimitivesProducer() {}

void RPCTriggerPrimitivesProducer::produce(edm::Event& ev, const edm::EventSetup& setup) {
  // Get RPC RecHits from event
  edm::Handle<RPCRecHitCollection> rpcRecHits;
  ev.getByToken(rpcRecHitToken_, rpcRecHits);

  // Create empty output collection
  auto output = std::make_unique<RPCRecHitCollection>();

  if (!rpcRecHits.isValid()) {
    edm::LogWarning("RPCTriggerPrimitivesProducer|NoInputCollection")
        << "+++ Warning: Collection of RPC RecHits with label " << rpcRecHitProducer_.label()
        << " requested in configuration, but not found in the event..."
        << " Skipping production of RPC trigger primitives +++\n";
  } else {
    builder_->build(rpcRecHits.product(), *output);
  }

  // Put output in event
  ev.put(std::move(output));
}

DEFINE_FWK_MODULE(RPCTriggerPrimitivesProducer);
