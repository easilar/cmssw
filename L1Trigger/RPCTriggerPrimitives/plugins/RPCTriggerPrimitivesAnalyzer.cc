/** \class RPCTriggerPrimitivesAnalyzer
 *
 * EDAnalyzer to compare input RPC RecHits vs filtered RPC Trigger Primitives.
 *
 * Produces histograms of kinematic and detector variables for both collections:
 *   - Global eta, phi (from RPC geometry)
 *   - Station, ring, region, sector, roll
 *   - Cluster size, strip, BX, time
 *   - Hit multiplicity per event
 *   - 2D eta-phi occupancy
 */

#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/RPCRecHit/interface/RPCRecHit.h"
#include "DataFormats/RPCRecHit/interface/RPCRecHitCollection.h"
#include "DataFormats/MuonDetId/interface/RPCDetId.h"

#include "Geometry/RPCGeometry/interface/RPCGeometry.h"
#include "Geometry/RPCGeometry/interface/RPCRoll.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"

#include "TH1F.h"
#include "TH2F.h"

#include <string>

class RPCTriggerPrimitivesAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit RPCTriggerPrimitivesAnalyzer(const edm::ParameterSet&);
  ~RPCTriggerPrimitivesAnalyzer() override = default;

  void analyze(const edm::Event&, const edm::EventSetup&) override;

private:
  // Struct to hold a set of histograms for one collection
  struct HistSet {
    TH1F* h_nHits;
    TH1F* h_globalEta;
    TH1F* h_globalPhi;
    TH1F* h_station;
    TH1F* h_ring;
    TH1F* h_region;
    TH1F* h_sector;
    TH1F* h_roll;
    TH1F* h_clusterSize;
    TH1F* h_strip;
    TH1F* h_bx;
    TH1F* h_time;
    TH2F* h_etaPhi;
    TH2F* h_stationRing;
  };

  HistSet bookHistograms(const std::string& prefix, const std::string& title);
  void fillHistograms(HistSet& hists, const RPCRecHitCollection* hits, const RPCGeometry* rpcGeom);

  edm::EDGetTokenT<RPCRecHitCollection> inputToken_;
  edm::EDGetTokenT<RPCRecHitCollection> tpToken_;
  edm::ESGetToken<RPCGeometry, MuonGeometryRecord> rpcGeomToken_;

  HistSet inputHists_;
  HistSet tpHists_;
};

RPCTriggerPrimitivesAnalyzer::RPCTriggerPrimitivesAnalyzer(const edm::ParameterSet& iConfig) {
  usesResource("TFileService");

  inputToken_ = consumes<RPCRecHitCollection>(iConfig.getParameter<edm::InputTag>("inputRecHits"));
  tpToken_ = consumes<RPCRecHitCollection>(iConfig.getParameter<edm::InputTag>("triggerPrimitives"));
  rpcGeomToken_ = esConsumes<RPCGeometry, MuonGeometryRecord>();

  inputHists_ = bookHistograms("input", "All RPC RecHits");
  tpHists_ = bookHistograms("tp", "RPC Trigger Primitives");
}

RPCTriggerPrimitivesAnalyzer::HistSet RPCTriggerPrimitivesAnalyzer::bookHistograms(
    const std::string& prefix, const std::string& title) {
  edm::Service<TFileService> fs;
  HistSet h;

  auto sub = fs->mkdir(prefix);

  h.h_nHits = sub.make<TH1F>("nHits", (title + ";Number of hits;Events").c_str(), 200, 0, 2000);
  h.h_globalEta = sub.make<TH1F>("globalEta", (title + ";#eta;Hits").c_str(), 100, -3.0, 3.0);
  h.h_globalPhi = sub.make<TH1F>("globalPhi", (title + ";#phi [rad];Hits").c_str(), 100, -3.15, 3.15);
  h.h_station = sub.make<TH1F>("station", (title + ";Station;Hits").c_str(), 5, 0.5, 5.5);
  h.h_ring = sub.make<TH1F>("ring", (title + ";Ring;Hits").c_str(), 5, 0.5, 5.5);
  h.h_region = sub.make<TH1F>("region", (title + ";Region;Hits").c_str(), 5, -2.5, 2.5);
  h.h_sector = sub.make<TH1F>("sector", (title + ";Sector;Hits").c_str(), 13, 0.5, 13.5);
  h.h_roll = sub.make<TH1F>("roll", (title + ";Roll;Hits").c_str(), 6, 0.5, 6.5);
  h.h_clusterSize = sub.make<TH1F>("clusterSize", (title + ";Cluster size;Hits").c_str(), 20, 0.5, 20.5);
  h.h_strip = sub.make<TH1F>("strip", (title + ";First strip;Hits").c_str(), 130, 0.5, 130.5);
  h.h_bx = sub.make<TH1F>("bx", (title + ";BX;Hits").c_str(), 11, -5.5, 5.5);
  h.h_time = sub.make<TH1F>("time", (title + ";Time [ns];Hits").c_str(), 100, -50.0, 50.0);
  h.h_etaPhi = sub.make<TH2F>("etaPhi", (title + ";#eta;#phi [rad]").c_str(), 100, -3.0, 3.0, 100, -3.15, 3.15);
  h.h_stationRing = sub.make<TH2F>("stationRing", (title + ";Station;Ring").c_str(), 5, 0.5, 5.5, 5, 0.5, 5.5);

  return h;
}

void RPCTriggerPrimitivesAnalyzer::fillHistograms(
    HistSet& hists, const RPCRecHitCollection* hits, const RPCGeometry* rpcGeom) {
  int nHits = 0;

  for (auto it = hits->begin(); it != hits->end(); ++it) {
    const RPCRecHit& rechit = *it;
    const RPCDetId& id = rechit.rpcId();

    // Get global position from geometry
    const RPCRoll* roll = rpcGeom->roll(id);
    if (!roll)
      continue;

    GlobalPoint gp = roll->toGlobal(rechit.localPosition());
    float eta = gp.eta();
    float phi = gp.phi();

    hists.h_globalEta->Fill(eta);
    hists.h_globalPhi->Fill(phi);
    hists.h_station->Fill(id.station());
    hists.h_ring->Fill(id.ring());
    hists.h_region->Fill(id.region());
    hists.h_sector->Fill(id.sector());
    hists.h_roll->Fill(id.roll());
    hists.h_clusterSize->Fill(rechit.clusterSize());
    hists.h_strip->Fill(rechit.firstClusterStrip());
    hists.h_bx->Fill(rechit.BunchX());
    hists.h_time->Fill(rechit.time());
    hists.h_etaPhi->Fill(eta, phi);
    hists.h_stationRing->Fill(id.station(), id.ring());

    ++nHits;
  }

  hists.h_nHits->Fill(nHits);
}

void RPCTriggerPrimitivesAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  const auto& rpcGeom = iSetup.getData(rpcGeomToken_);

  edm::Handle<RPCRecHitCollection> inputHits;
  iEvent.getByToken(inputToken_, inputHits);

  edm::Handle<RPCRecHitCollection> tpHits;
  iEvent.getByToken(tpToken_, tpHits);

  if (inputHits.isValid()) {
    fillHistograms(inputHists_, inputHits.product(), &rpcGeom);
  }

  if (tpHits.isValid()) {
    fillHistograms(tpHists_, tpHits.product(), &rpcGeom);
  }
}

DEFINE_FWK_MODULE(RPCTriggerPrimitivesAnalyzer);
