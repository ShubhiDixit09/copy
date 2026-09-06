/**
 * DHARTI - Main Application Controller
 * Coordinates GIS map, data store, PCI mathematical simulation, and interactive sandbox.
 */

class AppController {
  constructor() {
    this.mapView = null;
    this.sandbox = new TestSandbox();
    this.activeTab = "cockpit";
    this.selectedParcelId = 118;
  }

  init() {
    this.mapView = new MapView("map-canvas");
    this.mapView.init();
    this.updateUI();
    this.bindEvents();
    this.applyRoleView();
  }

  bindEvents() {
    // Navigation Tabs
    document.querySelectorAll(".nav-tab-btn").forEach(btn => {
      btn.addEventListener("click", (e) => {
        const target = e.currentTarget.getAttribute("data-tab");
        this.switchTab(target);
      });
    });

    const roleSelect = document.getElementById("user-role-select");
    if (roleSelect) {
      roleSelect.addEventListener("change", () => this.applyRoleView());
    }

    // Custom Data Uploader
    const uploadBtn = document.getElementById("btn-apply-custom-json");
    if (uploadBtn) {
      uploadBtn.addEventListener("click", () => this.handleCustomDataApply());
    }

    const resetBtn = document.getElementById("btn-reset-data");
    if (resetBtn) {
      resetBtn.addEventListener("click", () => {
        window.dataStore.reset();
        this.mapView.renderParcels();
        this.updateUI();
        this.showNotification("System reset to baseline 12.0 km corridor state.", "info");
      });
    }

    const printBtn = document.getElementById("btn-print-mis");
    if (printBtn) {
      printBtn.addEventListener("click", () => window.print());
    }
  }

  switchTab(tabId) {
    this.activeTab = tabId;
    document.querySelectorAll(".nav-tab-btn").forEach(btn => {
      btn.classList.toggle("active", btn.getAttribute("data-tab") === tabId);
    });

    document.querySelectorAll(".tab-content-panel").forEach(panel => {
      panel.classList.toggle("active", panel.id === `panel-${tabId}`);
    });

    if (tabId === "cockpit" && this.mapView && this.mapView.map) {
      setTimeout(() => this.mapView.map.invalidateSize(), 200);
    }

    if (tabId === "mis") {
      this.renderMISReport();
    } else if (tabId === "pci") {
      this.renderPCISimulator();
    }
  }

  updateUI() {
    const parcels = window.dataStore.parcels;
    const project = window.dataStore.project;
    const households = window.dataStore.households;

    const pciData = PCIEngine.computePCI(parcels, project.alignment_length_km);

    // 1. KPI Stats
    const totalAreaSqm = parcels.reduce((sum, p) => sum + p.polygon_area_sqm, 0);
    const totalSanctioned = parcels.reduce((sum, p) => sum + p.sanctioned_amount_inr, 0);
    const totalDisbursed = parcels.reduce((sum, p) => sum + p.disbursed_amount_inr, 0);
    const readyParcels = parcels.filter(p => p.state === "ConstructionReady");
    const activeStays = parcels.filter(p => p.court_stay);

    this.setText("kpi-corridor-km", `${project.alignment_length_km} km`);
    this.setText("kpi-pci-val", `${pciData.pci}%`);
    this.setText("kpi-frontage-km", `${pciData.maxContinuousFrontageKm} km`);
    this.setText("kpi-area-notified", `${(totalAreaSqm / 10000).toFixed(2)} Ha`);
    this.setText("kpi-parcels-ready", `${readyParcels.length} / ${parcels.length}`);
    this.setText("kpi-disbursed-inr", `₹${(totalDisbursed / 1e7).toFixed(2)} Cr`);
    this.setText("kpi-sanctioned-inr", `₹${(totalSanctioned / 1e7).toFixed(2)} Cr`);
    this.setText("kpi-stays-count", activeStays.length);
    this.setText("kpi-households-count", households.length);

    // Progress bar
    const pciBar = document.getElementById("pci-metric-progress-bar");
    if (pciBar) {
      pciBar.style.width = `${pciData.pci}%`;
    }

    // Corridor Linear Strip View
    this.renderCorridorStrip(parcels, project.alignment_length_km);

    // Parcel List Table
    this.renderParcelTable(parcels);

    // Selected Parcel Drawer
    this.renderSelectedParcelDrawer();

    // Map refresh
    if (this.mapView) {
      this.mapView.renderParcels();
    }
  }

  renderCorridorStrip(parcels, totalLengthKm) {
    const container = document.getElementById("corridor-linear-strip");
    if (!container) return;

    container.innerHTML = "";
    parcels.forEach(p => {
      const segLength = p.chainage_end_km - p.chainage_start_km;
      const widthPct = (segLength / totalLengthKm) * 100;

      const segDiv = document.createElement("div");
      segDiv.className = `strip-segment state-${p.state} ${p.court_stay ? 'has-stay' : ''} ${p.parcel_id === this.selectedParcelId ? 'selected-segment' : ''}`;
      segDiv.style.width = `${widthPct}%`;
      segDiv.title = `P-${p.parcel_id} (KM ${p.chainage_start_km} - ${p.chainage_end_km}): ${p.state}`;

      segDiv.innerHTML = `
        <span class="seg-label">P-${p.parcel_id}</span>
        <span class="seg-km">${segLength.toFixed(1)}k</span>
      `;

      segDiv.addEventListener("click", () => this.selectParcel(p.parcel_id));
      container.appendChild(segDiv);
    });
  }

  renderParcelTable(parcels) {
    const tbody = document.getElementById("parcel-inventory-tbody");
    if (!tbody) return;

    tbody.innerHTML = parcels.map(p => `
      <tr class="${p.parcel_id === this.selectedParcelId ? 'table-active-row' : ''}" onclick="window.app.selectParcel(${p.parcel_id})">
        <td><b>P-${p.parcel_id}</b></td>
        <td><code>${p.ulpin}</code></td>
        <td>Sy ${p.survey_number}/${p.sub_division}</td>
        <td>KM ${p.chainage_start_km} &rarr; ${p.chainage_end_km}</td>
        <td>₹${(p.disbursed_amount_inr / 1e7).toFixed(2)} Cr</td>
        <td><span class="status-badge badge-${p.state}">${p.state}</span></td>
        <td>${p.court_stay ? '<span class="text-danger font-semibold">STAY ACTIVE</span>' : '<span class="text-success">Clear</span>'}</td>
      </tr>
    `).join("");
  }

  selectParcel(parcelId) {
    this.selectedParcelId = parcelId;
    this.renderSelectedParcelDrawer();
    this.renderParcelTable(window.dataStore.parcels);
    this.renderCorridorStrip(window.dataStore.parcels, window.dataStore.project.alignment_length_km);
    if (this.mapView) {
      this.mapView.highlightParcel(parcelId);
    }
    window.dispatchEvent(new CustomEvent("dharti:parcel-selected", { detail: { parcelId } }));
  }

  applyRoleView() {
    const roleSelect = document.getElementById("user-role-select");
    const role = roleSelect ? roleSelect.value : "cala";
    const isClaimant = role === "claimant";
    const claimantView = document.getElementById("claimant-access-view");

    document.body.classList.toggle("claimant-view", isClaimant);
    if (claimantView) claimantView.hidden = !isClaimant;

    if (isClaimant && window.nyayaBot) {
      window.nyayaBot.open();
    }
  }

  renderSelectedParcelDrawer() {
    const p = window.dataStore.parcels.find(x => x.parcel_id === this.selectedParcelId);
    if (!p) return;

    this.setText("insp-parcel-id", `Parcel P-${p.parcel_id} (Survey ${p.survey_number}/${p.sub_division})`);
    this.setText("insp-ulpin", p.ulpin);
    this.setText("insp-owner", p.owner_name);
    this.setText("insp-chainage", `KM ${p.chainage_start_km} - KM ${p.chainage_end_km} (${(p.chainage_end_km - p.chainage_start_km).toFixed(1)} km)`);
    this.setText("insp-state", p.state);
    this.setText("insp-area", `${p.polygon_area_sqm.toLocaleString()} sqm (RoR: ${p.ror_area_sqm.toLocaleString()} sqm)`);
    this.setText("insp-comp", `Disbursed ₹${(p.disbursed_amount_inr / 1e7).toFixed(2)} Cr / Sanctioned ₹${(p.sanctioned_amount_inr / 1e7).toFixed(2)} Cr`);
    this.setText("insp-utr", p.bank_utr || "None / Unreconciled");
    this.setText("insp-disputes", p.dispute_details || "Clear");

    const stayEl = document.getElementById("insp-stay-box");
    if (stayEl) {
      if (p.court_stay) {
        stayEl.style.display = "block";
        stayEl.innerHTML = `<b>HIGH COURT STAY:</b> ${p.stay_details}`;
      } else {
        stayEl.style.display = "none";
      }
    }
  }

  renderPCISimulator() {
    const sim = PCIEngine.simulateBottleneckUnlocks(window.dataStore.parcels, window.dataStore.project.alignment_length_km);
    this.setText("sim-baseline-pci", `${sim.baseline.pci}%`);
    this.setText("sim-baseline-frontage", `${sim.baseline.maxContinuousFrontageKm} km`);

    const tbody = document.getElementById("pci-rankings-tbody");
    if (!tbody) return;

    if (sim.rankings.length === 0) {
      tbody.innerHTML = `<tr><td colspan="6" class="text-center text-success py-4">All corridor parcels are ConstructionReady! 100% PCI achieved.</td></tr>`;
      return;
    }

    tbody.innerHTML = sim.rankings.map((r, idx) => `
      <tr class="${idx === 0 ? 'top-priority-row' : ''}">
        <td><span class="rank-badge rank-${idx + 1}">#${idx + 1}</span></td>
        <td><b>P-${r.parcel_id}</b> (Sy ${r.survey_number})</td>
        <td><span class="status-badge badge-${r.current_state}">${r.current_state}</span></td>
        <td><b>+${r.frontage_gain_km} km</b></td>
        <td><b class="text-primary">${r.simulated_pci}%</b> (+${r.pci_gain_pct}%)</td>
        <td><small>${r.dispute_summary}</small></td>
      </tr>
    `).join("");
  }

  renderMISReport() {
    const container = document.getElementById("mis-report-view");
    if (!container) return;
    const pci = PCIEngine.computePCI(window.dataStore.parcels, window.dataStore.project.alignment_length_km);
    container.innerHTML = MISReports.generateReport(window.dataStore, pci);
  }

  handleCustomDataApply() {
    const textarea = document.getElementById("custom-json-input");
    if (!textarea || !textarea.value.trim()) {
      this.showNotification("Please provide valid JSON input.", "warning");
      return;
    }

    try {
      const parsed = JSON.parse(textarea.value.trim());
      if (parsed.parcels && Array.isArray(parsed.parcels)) {
        window.dataStore.parcels = parsed.parcels;
        if (parsed.alignment_length_km) {
          window.dataStore.project.alignment_length_km = parsed.alignment_length_km;
        }
        this.updateUI();
        this.switchTab("cockpit");
        this.showNotification(`Successfully ingested ${parsed.parcels.length} custom parcels!`, "success");
      } else {
        this.showNotification("JSON must contain a 'parcels' array.", "danger");
      }
    } catch (e) {
      this.showNotification(`JSON Parsing Error: ${e.message}`, "danger");
    }
  }

  showNotification(msg, type = "info") {
    const container = document.getElementById("notification-toast-container");
    if (!container) return;

    const toast = document.createElement("div");
    toast.className = `custom-toast toast-${type}`;
    toast.innerHTML = `
      <span class="toast-icon"></span>
      <span class="toast-msg">${msg}</span>
    `;

    container.appendChild(toast);
    setTimeout(() => {
      toast.classList.add("toast-fade-out");
      setTimeout(() => toast.remove(), 400);
    }, 4000);
  }

  setText(id, val) {
    const el = document.getElementById(id);
    if (el) el.textContent = val;
  }
}

window.app = new AppController();
window.addEventListener("DOMContentLoaded", () => window.app.init());
