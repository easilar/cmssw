// RPCDeadStripDumper
//
// Self-contained one-shot dumper of the RPC dead-strip / dead-chamber list
// carried by a CMSSW conditions Global Tag, via the RPCStripNoisesRcd record.
//
// What it does
// ------------
// Reads `RPCStripNoises` from the EventSetup once, walks the per-strip vector
// of {dpid, noise, eff, time} entries, replicates the strip-index logic that
// `SimMuon/RPCDigitizer/src/RPCSimSetUp.cc::setRPCSetUp` uses (the dpid is the
// RPCDetId rawId; consecutive entries with the same dpid are strips 1, 2, 3,
// ... of the same roll), and prints / writes the list of strips with eff <= 0
// — which is exactly the list of strips the digitizer treats as dead.
//
// What it needs
// -------------
// No input file. The cfg uses an EmptySource and just loads geometry +
// conditions from the GT chosen in the python config.
//
// Output
// ------
// - stdout: human-readable summary + per-chamber per-roll dead-strip lists
// - rpc_dead_strips.tsv (path settable from the cfg): one row per (rawId, strip)
//   for every strip with eff <= 0, columns are
//     region  ring(=wheel for barrel)  station  sector  layer  subsector
//     roll  strip  rawId  eff  noise

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/ESGetToken.h"

#include "CondFormats/RPCObjects/interface/RPCStripNoises.h"
#include "CondFormats/DataRecord/interface/RPCStripNoisesRcd.h"
#include "DataFormats/MuonDetId/interface/RPCDetId.h"

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <vector>

class RPCDeadStripDumper : public edm::one::EDAnalyzer<> {
public:
  explicit RPCDeadStripDumper(const edm::ParameterSet&);
  void analyze(const edm::Event&, const edm::EventSetup&) override;

private:
  edm::ESGetToken<RPCStripNoises, RPCStripNoisesRcd> noiseToken_;
  std::string outFile_;
  double effThreshold_;
  bool dumped_ = false;
};

RPCDeadStripDumper::RPCDeadStripDumper(const edm::ParameterSet& ps)
    : noiseToken_(esConsumes<RPCStripNoises, RPCStripNoisesRcd>()),
      outFile_(ps.getUntrackedParameter<std::string>("outFile", "rpc_dead_strips.tsv")),
      effThreshold_(ps.getUntrackedParameter<double>("effThreshold", 0.0)) {}

void RPCDeadStripDumper::analyze(const edm::Event&, const edm::EventSetup& setup) {
  if (dumped_) return;
  dumped_ = true;

  const auto& noise = setup.getData(noiseToken_);
  const auto& v = noise.getVNoise();

  struct RollStats { int total = 0; int dead = 0; std::vector<int> dead_strips; };
  std::map<uint32_t, RollStats> per_roll;
  int prev_dpid = -1, strip = 0, n_zero = 0;
  float min_eff = 1e9, max_eff = -1e9;

  std::ofstream out(outFile_);
  out << "region\tring\tstation\tsector\tlayer\tsubsector\troll\tstrip\trawId\teff\tnoise\n";

  for (const auto& it : v) {
    if (it.dpid != prev_dpid) { strip = 1; prev_dpid = it.dpid; }
    else                       { ++strip; }
    RPCDetId id(it.dpid);
    auto& rs = per_roll[id.rawId()];
    ++rs.total;
    if (it.eff < min_eff) min_eff = it.eff;
    if (it.eff > max_eff) max_eff = it.eff;
    if (it.eff <= effThreshold_) {
      ++rs.dead;
      rs.dead_strips.push_back(strip);
      ++n_zero;
      out << id.region() << '\t' << id.ring() << '\t' << id.station() << '\t'
          << id.sector() << '\t' << id.layer() << '\t' << id.subsector() << '\t'
          << id.roll() << '\t' << strip << '\t' << id.rawId() << '\t'
          << it.eff << '\t' << it.noise << '\n';
    }
  }
  out.close();

  std::cout << "RPCDeadStripDumper: " << v.size() << " total NoiseItem entries\n";
  std::cout << "  eff range: [" << min_eff << ", " << max_eff << "]\n";
  std::cout << "  strips with eff <= " << effThreshold_ << ": " << n_zero << "\n";
  std::cout << "  wrote " << outFile_ << "\n\n";

  // Group rolls by (region, ring, station, sector) for the human summary
  std::map<std::tuple<int,int,int,int>, std::vector<std::pair<uint32_t, RollStats>>> by_chamber;
  for (const auto& [raw, rs] : per_roll) {
    if (rs.dead == 0) continue;
    RPCDetId id(raw);
    by_chamber[{id.region(), id.ring(), id.station(), id.sector()}].push_back({raw, rs});
  }

  std::cout << "=== Chambers carrying any dead strips: " << by_chamber.size() << " ===\n";
  std::cout << "  region=0 is the barrel; region=+/-1 is the endcap\n\n";
  for (const auto& [chk, rolls] : by_chamber) {
    int reg, wh, st, se; std::tie(reg, wh, st, se) = chk;
    int dead = 0, tot = 0;
    for (const auto& [_, rs] : rolls) { dead += rs.dead; tot += rs.total; }
    const char* what = (reg == 0 ? "W " : (reg > 0 ? "RE +" : "RE -"));
    std::cout << what << (reg == 0 ? (wh >= 0 ? "+" : "") : "") << wh
              << "  St" << st << "  S" << se
              << "  ->  " << dead << "/" << tot << " strips dead "
              << "(" << (100.0 * dead / tot) << " %)\n";
    for (const auto& [raw, rs] : rolls) {
      RPCDetId id(raw);
      std::cout << "    L" << id.layer()
                << " sub" << id.subsector()
                << " r"   << id.roll()
                << "  : " << rs.dead << "/" << rs.total << " dead";
      if (!rs.dead_strips.empty()) {
        const int N = rs.dead_strips.size();
        std::cout << "  strips=[";
        if (N <= 12) {
          for (int i = 0; i < N; ++i)
            std::cout << rs.dead_strips[i] << (i + 1 < N ? "," : "");
        } else {
          for (int i = 0; i < 6; ++i) std::cout << rs.dead_strips[i] << ",";
          std::cout << "...,";
          for (int i = N - 3; i < N; ++i)
            std::cout << rs.dead_strips[i] << (i + 1 < N ? "," : "");
        }
        std::cout << "]";
      }
      std::cout << "\n";
    }
  }
}

DEFINE_FWK_MODULE(RPCDeadStripDumper);
