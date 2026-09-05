/**
 * DHARTI - Client-Side PCI Mathematical Engine
 * Exact algorithmic mirror of src/native/pci_engine.cpp
 */

class PCIEngine {
  static computePCI(parcels, corridorLengthKm) {
    // 1. Filter ready intervals
    const readyIntervals = parcels
      .filter(p => p.state === "ConstructionReady")
      .map(p => ({ start: p.chainage_start_km, end: p.chainage_end_km, id: p.parcel_id }));

    if (readyIntervals.length === 0) {
      return {
        pci: 0.0,
        maxContinuousFrontageKm: 0.0,
        totalReadyKm: 0.0,
        mergedSegments: []
      };
    }

    // 2. Sort intervals primarily by start ascending, secondarily by end descending
    readyIntervals.sort((a, b) => {
      if (Math.abs(a.start - b.start) > 1e-4) {
        return a.start - b.start;
      }
      return b.end - a.end;
    });

    // 3. Merge contiguous intervals
    const merged = [];
    let curStart = readyIntervals[0].start;
    let curEnd = readyIntervals[0].end;

    for (let i = 1; i < readyIntervals.length; i++) {
      const next = readyIntervals[i];
      if (next.start <= curEnd + 0.0001) {
        curEnd = Math.max(curEnd, next.end);
      } else {
        merged.push({ start: curStart, end: curEnd, length: curEnd - curStart });
        curStart = next.start;
        curEnd = next.end;
      }
    }
    merged.push({ start: curStart, end: curEnd, length: curEnd - curStart });

    // 4. Max continuous stretch & total ready stretch
    let maxRun = 0.0;
    let totalReady = 0.0;
    for (const seg of merged) {
      if (seg.length > maxRun) maxRun = seg.length;
      totalReady += seg.length;
    }

    const pci = corridorLengthKm > 0 ? Math.min(100.0, (maxRun / corridorLengthKm) * 100.0) : 0.0;

    return {
      pci: parseFloat(pci.toFixed(2)),
      maxContinuousFrontageKm: parseFloat(maxRun.toFixed(2)),
      totalReadyKm: parseFloat(totalReady.toFixed(2)),
      mergedSegments: merged
    };
  }

  static simulateBottleneckUnlocks(parcels, corridorLengthKm) {
    const baseline = this.computePCI(parcels, corridorLengthKm);
    const blocked = parcels.filter(p => p.state !== "ConstructionReady");

    const rankings = [];
    for (const cand of blocked) {
      // Simulate candidate as ConstructionReady
      const simParcels = parcels.map(p => {
        if (p.parcel_id === cand.parcel_id) {
          return { ...p, state: "ConstructionReady" };
        }
        return p;
      });

      const simResult = this.computePCI(simParcels, corridorLengthKm);
      const frontageGain = simResult.maxContinuousFrontageKm - baseline.maxContinuousFrontageKm;
      const pciGain = simResult.pci - baseline.pci;

      rankings.push({
        parcel_id: cand.parcel_id,
        survey_number: cand.survey_number,
        owner_name: cand.owner_name,
        current_state: cand.state,
        simulated_pci: simResult.pci,
        simulated_max_km: simResult.maxContinuousFrontageKm,
        frontage_gain_km: parseFloat(frontageGain.toFixed(2)),
        pci_gain_pct: parseFloat(pciGain.toFixed(2)),
        dispute_summary: cand.dispute_details || cand.stay_details || "Awaiting statutory notice"
      });
    }

    // Rank descending by frontage gain, then by simulated PCI
    rankings.sort((a, b) => {
      if (Math.abs(b.frontage_gain_km - a.frontage_gain_km) > 1e-4) {
        return b.frontage_gain_km - a.frontage_gain_km;
      }
      return b.simulated_pci - a.simulated_pci;
    });

    return {
      baseline: baseline,
      rankings: rankings
    };
  }
}

window.PCIEngine = PCIEngine;
