#!/usr/bin/env python3
"""
Plot RPC Trigger Primitives analysis histograms.
Compares input RPC RecHits vs filtered Trigger Primitives.

Usage: python3 plotRPCTP.py [input_file]
  Default input: rpcTP_histograms.root
"""

import sys
import ROOT

ROOT.gROOT.SetBatch(True)
ROOT.gStyle.SetOptStat(111111)
ROOT.gStyle.SetOptTitle(1)

fname = sys.argv[1] if len(sys.argv) > 1 else "rpcTP_histograms.root"
f = ROOT.TFile.Open(fname)
if not f or f.IsZombie():
    print(f"Error: cannot open {fname}")
    sys.exit(1)

prefix_input = "rpcTriggerPrimitivesAnalyzer/input/"
prefix_tp = "rpcTriggerPrimitivesAnalyzer/tp/"

# Colors
COL_INPUT = ROOT.kBlue
COL_TP = ROOT.kRed

def get_hist(name, prefix):
    h = f.Get(prefix + name)
    if not h:
        print(f"Warning: histogram {prefix + name} not found")
    return h

def overlay_plot(canvas, name, title, logy=False):
    """Draw input and TP histograms overlaid."""
    h_in = get_hist(name, prefix_input)
    h_tp = get_hist(name, prefix_tp)
    if not h_in or not h_tp:
        return

    h_in = h_in.Clone(name + "_input")
    h_tp = h_tp.Clone(name + "_tp")

    h_in.SetLineColor(COL_INPUT)
    h_in.SetLineWidth(2)
    h_in.SetFillColorAlpha(COL_INPUT, 0.15)

    h_tp.SetLineColor(COL_TP)
    h_tp.SetLineWidth(2)
    h_tp.SetFillColorAlpha(COL_TP, 0.15)

    h_in.SetTitle(title)

    ymax = max(h_in.GetMaximum(), h_tp.GetMaximum()) * 1.4
    h_in.SetMaximum(ymax)

    canvas.SetLogy(logy)
    h_in.Draw("HIST")
    h_tp.Draw("HIST SAME")

    leg = ROOT.TLegend(0.65, 0.75, 0.88, 0.88)
    leg.SetBorderSize(0)
    leg.SetFillStyle(0)
    leg.AddEntry(h_in, f"All RecHits ({h_in.GetEntries():.0f})", "f")
    leg.AddEntry(h_tp, f"Trigger Prims ({h_tp.GetEntries():.0f})", "f")
    leg.Draw()
    return leg  # prevent garbage collection

# --- 1D overlay plots ---
c1 = ROOT.TCanvas("c1", "RPC TP Analysis", 1600, 1200)
c1.Divide(4, 3)

plots_1d = [
    ("nHits",       "Number of hits per event"),
    ("globalEta",   "Global #eta"),
    ("globalPhi",   "Global #phi"),
    ("station",     "Station"),
    ("ring",        "Ring"),
    ("region",      "Region"),
    ("clusterSize", "Cluster size"),
    ("strip",       "First strip"),
    ("bx",          "BX"),
    ("time",        "Time [ns]"),
    ("sector",      "Sector"),
    ("roll",        "Roll"),
]

legends = []
for i, (name, title) in enumerate(plots_1d):
    c1.cd(i + 1)
    leg = overlay_plot(c1, name, title)
    legends.append(leg)

c1.SaveAs("rpcTP_plots_1d.png")
c1.SaveAs("rpcTP_plots_1d.pdf")
print("Saved rpcTP_plots_1d.png / .pdf")

# --- 2D eta-phi plots ---
c2 = ROOT.TCanvas("c2", "eta-phi", 1400, 600)
c2.Divide(2, 1)

for i, (prefix, label) in enumerate([(prefix_input, "All RecHits"), (prefix_tp, "Trigger Primitives")]):
    c2.cd(i + 1)
    h = f.Get(prefix + "etaPhi")
    if h:
        h = h.Clone()
        h.SetTitle(f"{label}: #eta vs #phi")
        h.Draw("COLZ")

c2.SaveAs("rpcTP_plots_etaphi.png")
c2.SaveAs("rpcTP_plots_etaphi.pdf")
print("Saved rpcTP_plots_etaphi.png / .pdf")

# --- 2D station-ring plots ---
c3 = ROOT.TCanvas("c3", "station-ring", 1400, 600)
c3.Divide(2, 1)

for i, (prefix, label) in enumerate([(prefix_input, "All RecHits"), (prefix_tp, "Trigger Primitives")]):
    c3.cd(i + 1)
    h = f.Get(prefix + "stationRing")
    if h:
        h = h.Clone()
        h.SetTitle(f"{label}: Station vs Ring")
        h.Draw("COLZ TEXT")

c3.SaveAs("rpcTP_plots_stationring.png")
c3.SaveAs("rpcTP_plots_stationring.pdf")
print("Saved rpcTP_plots_stationring.png / .pdf")

f.Close()
print("Done!")
