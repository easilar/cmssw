// RPCEfficiencyDumper
//
// One-shot dumper that prints every RPC roll the geometry contains together
// with the per-strip efficiency that the conditions DB (`RPCStripNoisesRcd`)
// assigns to it.
//
// Output: one TSV row per roll. Columns:
//   region  ring(=wheel for barrel)  station  sector  layer  subsector  roll
//   nstrips_geom   nstrips_noise
//   eff_mean       eff_min      eff_max
//   n_strips_dead  (= strips with eff <= effThreshold)
//   eta            phi           rawId
//
// `nstrips_geom` is the geometry-reported strip count; `nstrips_noise` is the
// number of NoiseItem entries the conditions DB has for the roll. Rolls that
// exist in geometry but have no entry in RPCStripNoises will have
// nstrips_noise == 0 and eff_* fields == NaN.
//
// No input file required — driver uses EmptySource + GeometryRecoDB + GT.

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/ESGetToken.h"

#include "Geometry/RPCGeometry/interface/RPCGeometry.h"
#include "Geometry/RPCGeometry/interface/RPCRoll.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"
#include "CondFormats/RPCObjects/interface/RPCStripNoises.h"
#include "CondFormats/DataRecord/interface/RPCStripNoisesRcd.h"
#include "DataFormats/MuonDetId/interface/RPCDetId.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <vector>

class RPCEfficiencyDumper : public edm::one::EDAnalyzer<> {
public:
  explicit RPCEfficiencyDumper(const edm::ParameterSet&);
  void analyze(const edm::Event&, const edm::EventSetup&) override;

private:
  edm::ESGetToken<RPCGeometry,    MuonGeometryRecord> geomToken_;
  edm::ESGetToken<RPCStripNoises, RPCStripNoisesRcd>  noiseToken_;
  std::string outFile_;
  double effThreshold_;
  bool dumped_ = false;
};

RPCEfficiencyDumper::RPCEfficiencyDumper(const edm::ParameterSet& ps)
    : geomToken_(esConsumes<RPCGeometry,    MuonGeometryRecord>()),
      noiseToken_(esConsumes<RPCStripNoises, RPCStripNoisesRcd>()),
      outFile_(ps.getUntrackedParameter<std::string>("outFile", "rpc_geometry_efficiency.tsv")),
      effThreshold_(ps.getUntrackedParameter<double>("effThreshold", 0.0)) {}

void RPCEfficiencyDumper::analyze(const edm::Event&, const edm::EventSetup& setup) {
  if (dumped_) return;
  dumped_ = true;

  const auto& rpcGeom = setup.getData(geomToken_);
  const auto& noise   = setup.getData(noiseToken_);
  const auto& vNoise  = noise.getVNoise();

  // Build a per-rawId vector of efficiencies, replicating RPCSimSetUp's logic.
  std::map<uint32_t, std::vector<float>> effByRawId;
  int prev_dpid = -1;
  for (const auto& it : vNoise) {
    if (it.dpid != prev_dpid) prev_dpid = it.dpid;
    RPCDetId id(it.dpid);
    effByRawId[id.rawId()].push_back(it.eff);
  }

  std::ofstream out(outFile_);
  out << "region\tring\tstation\tsector\tlayer\tsubsector\troll\t"
         "nstrips_geom\tnstrips_noise\teff_mean\teff_min\teff_max\tn_dead\t"
         "eta\tphi\trawId\n";

  const auto& rolls = rpcGeom.rolls();
  int n_total = 0, n_with_noise = 0, n_chamber_dead = 0;
  for (const RPCRoll* r : rolls) {
    if (!r) continue;
    ++n_total;
    const RPCDetId id = r->id();
    const int nstrips_geom = r->nstrips();
    const auto center = r->surface().toGlobal(LocalPoint(0.f, 0.f, 0.f));

    int nstrips_noise = 0;
    int n_dead = 0;
    float eff_min = std::numeric_limits<float>::quiet_NaN();
    float eff_max = std::numeric_limits<float>::quiet_NaN();
    float eff_mean = std::numeric_limits<float>::quiet_NaN();
    auto effIt = effByRawId.find(id.rawId());
    if (effIt != effByRawId.end()) {
      ++n_with_noise;
      const auto& v = effIt->second;
      nstrips_noise = static_cast<int>(v.size());
      double sum = 0.;
      eff_min = std::numeric_limits<float>::infinity();
      eff_max = -std::numeric_limits<float>::infinity();
      for (float e : v) {
        sum += e;
        if (e < eff_min) eff_min = e;
        if (e > eff_max) eff_max = e;
        if (e <= effThreshold_) ++n_dead;
      }
      eff_mean = static_cast<float>(sum / v.size());
      if (n_dead == nstrips_noise) ++n_chamber_dead;
    }

    out << id.region() << '\t' << id.ring() << '\t' << id.station() << '\t'
        << id.sector() << '\t' << id.layer() << '\t' << id.subsector() << '\t'
        << id.roll() << '\t' << nstrips_geom << '\t' << nstrips_noise << '\t'
        << eff_mean << '\t' << eff_min << '\t' << eff_max << '\t' << n_dead << '\t'
        << center.eta() << '\t' << center.phi() << '\t' << id.rawId() << '\n';
  }
  out.close();

  std::cout << "RPCEfficiencyDumper: " << n_total << " rolls in geometry\n";
  std::cout << "  rolls with at least one NoiseItem in conditions: " << n_with_noise << "\n";
  std::cout << "  rolls with EVERY strip eff <= " << effThreshold_
            << " (i.e. fully dead): " << n_chamber_dead << "\n";
  std::cout << "  wrote " << outFile_ << "\n";
}

DEFINE_FWK_MODULE(RPCEfficiencyDumper);
