# RPCMaskTool/Dumper

Two one-shot CMSSW analyzers that read the **RPC chamber masking carried by a
conditions Global Tag** and write it out as plain TSV. Useful for answering
the question *"which RPC strips / rolls / chambers does this GT treat as dead
in simulation?"* without having to run a full digi+reco chain.

The mechanism the dumpers inspect is the `RPCStripNoisesRcd` ESRecord — the
same record that `SimMuon/RPCDigitizer` reads to decide, per strip and per
event, whether to produce a digi (see
`SimMuon/RPCDigitizer/src/RPCSimAverageNoiseEffCls.cc` ~line 191:
`if (fire < veff[centralStrip - 1]) { ... }`). Strips where `eff <= 0` never
fire — they show up as empty chambers in the output.

## Plugins

| plugin | what it writes |
|---|---|
| `RPCDeadStripDumper` | TSV with one row per strip with `eff <= threshold` |
| `RPCEfficiencyDumper` | TSV with one row per RPC roll (mean/min/max eff, n_dead, eta, phi, rawId) |

Both run on `cms.Source("EmptySource")` — **no RECO/RAW/SIM input file needed**.

## Quick start

```bash
# 1. fresh release area (any directory you can write to)
cmsrel CMSSW_15_0_10_patch2
cd CMSSW_15_0_10_patch2/src
cmsenv

# 2. pull this branch from easilar/cmssw into the release src/
git init -q                        # if not already a git repo
git remote add easilar https://github.com/easilar/cmssw.git
git fetch easilar add-rpc-mask-tool-dumper --depth=1
git checkout easilar/add-rpc-mask-tool-dumper -- RPCMaskTool

# 3. build
scram b -j8

# 4. run the dumpers
cd RPCMaskTool/Dumper
cmsRun python/dump_dead_strips_cfg.py     # -> rpc_dead_strips.tsv
cmsRun python/dump_efficiency_cfg.py      # -> rpc_geometry_efficiency.tsv

# 5. (optional) convert dead-strip list into the project's
#     "disconnected_chambers" 4-column format:
python3 scripts/convert_to_disconnected.py \
        rpc_dead_strips.tsv disconnected_chambers.tsv
```

That's it. Each `cmsRun` takes about a minute (mostly loading geometry +
conditions).

## CLI options

Both drivers share the same options:

```
cmsRun python/dump_dead_strips_cfg.py \
    globalTag=auto:phase2_realistic_T33 \   # any GT string
    era=Phase2C17I13M9 \                    # Configuration.Eras.Era_<era>_cff
    outFile=rpc_dead_strips.tsv \
    effThreshold=0.0 \                      # eff <= threshold counts as dead
    firstRun=1                              # IOV anchor for the EmptySource
```

Examples:

```bash
# Run-3 MC GT
cmsRun python/dump_dead_strips_cfg.py \
    era=Run3 globalTag=auto:run3_mc outFile=run3_mc_dead_strips.tsv

# Run-3 data GT, real Run-3 IOV
cmsRun python/dump_dead_strips_cfg.py \
    era=Run3 globalTag=auto:run3_data \
    firstRun=383000 outFile=run3_data_dead_strips.tsv

# treat anything below 50% efficiency as "dead"
cmsRun python/dump_dead_strips_cfg.py effThreshold=0.5
```

## Output schemas

### `rpc_dead_strips.tsv` (RPCDeadStripDumper)

```
region  ring  station  sector  layer  subsector  roll  strip  rawId  eff  noise
```

- `region`: 0 = barrel, ±1 = endcap
- `ring`: for barrel this is the wheel (-2..+2); for endcap it is the RPC ring
- `station`: RB1..RB4 (barrel) / RE1..RE4 (endcap)
- `strip`: 1-based strip index within the roll
- `eff` is the strip's assigned per-event firing probability

### `rpc_geometry_efficiency.tsv` (RPCEfficiencyDumper)

```
region  ring  station  sector  layer  subsector  roll  nstrips_geom
nstrips_noise  eff_mean  eff_min  eff_max  n_dead  eta  phi  rawId
```

One row per RPC roll the geometry contains. `n_dead` is the number of strips
in that roll with `eff <= effThreshold`. Rolls present in geometry but absent
from `RPCStripNoises` (rare) have `nstrips_noise = 0` and `eff_* = NaN`.

### `disconnected_chambers.tsv` (convert_to_disconnected.py)

```
wheel  sector  station  position
```

Same schema as the project-level RPC blacklists used elsewhere in the analysis
(`disconnected_<year>_chambers.tsv`). One row per fully-dead chamber-layer
(RB1/RB2, position = `in`/`out`) or chamber-half (RB3/RB4, position = `+`/`-`).

## Notes

- `RPCStripNoises::NoiseItem` has fields `{dpid, noise, eff, time}`. The `dpid`
  is the **rawId of the `RPCDetId`**. Consecutive entries with the same `dpid`
  represent strips 1, 2, 3, ... of that roll — the same convention used in
  `SimMuon/RPCDigitizer/src/RPCSimSetUp.cc::setRPCSetUp` when filling
  `_mapDetIdEff`.
- Endcap chambers are emitted in the strip / efficiency dumps but are dropped
  by `convert_to_disconnected.py` (the disconnected-chambers schema is barrel
  only).
