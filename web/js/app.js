/**
 * DHARTI - Main Application Controller
 * Coordinates GIS map, data store, PCI mathematical simulation, and interactive sandbox.
 */

class AppController {
  constructor() {
    this.mapView = null;
    this.sandbox = new TestSandbox();
    this.activeTab = "project-tracker";
    this.selectedParcelId = 118;
  }

  init() {
    this.mapView = new MapView("map-canvas");
    this.mapView.init();
    this.updateUI();
    this.bindEvents();
    if (window.ProjectTracker) {
      window.projectTracker = new ProjectTracker();
    }
  }

  bindEvents() {
    // Navigation Tabs
    document.querySelectorAll(".nav-tab-btn").forEach(btn => {
      btn.addEventListener("click", (e) => {
        const target = e.currentTarget.getAttribute("data-tab");
        this.switchTab(target);
      });
    });

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

    // Explanatory Query Run button
    const explainBtn = document.getElementById("btn-run-explanatory-query");
    if (explainBtn) {
      explainBtn.addEventListener("click", () => {
        const inp = document.getElementById("explanatory-user-input");
        const val = inp ? inp.value.trim() : "";
        this.renderExplanatoryView(val);
      });
    }

    // Explanatory Prompts
    document.querySelectorAll(".quick-explain-btn").forEach(btn => {
      btn.addEventListener("click", (e) => {
        const q = e.currentTarget.getAttribute("data-query");
        const inp = document.getElementById("explanatory-user-input");
        if (inp) inp.value = q;
        this.renderExplanatoryView(q);
      });
    });

    // History selection buttons
    document.querySelectorAll(".history-select-btn").forEach(btn => {
      btn.addEventListener("click", (e) => {
        document.querySelectorAll(".history-select-btn").forEach(b => b.classList.remove("active-history-btn"));
        e.currentTarget.classList.add("active-history-btn");
        const pid = e.currentTarget.getAttribute("data-pid");
        this.renderHistoryTimelineView(pid);
      });
    });
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
    } else if (tabId === "explanatory-ai") {
      const inp = document.getElementById("explanatory-user-input");
      const q = inp ? inp.value.trim() : "";
      this.renderExplanatoryView(q || "Why is Bengaluru-Chennai corridor blocked?");
    } else if (tabId === "history-timeline") {
      const activeBtn = document.querySelector(".history-select-btn.active-history-btn");
      const pid = activeBtn ? activeBtn.getAttribute("data-pid") : "NHAI-NE7-PKG-04";
      this.renderHistoryTimelineView(pid);
    }
  }

  renderExplanatoryView(queryText = "") {
    const container = document.getElementById("explanatory-result-container");
    if (!container) return;
    const currentProj = window.projectTracker ? window.projectTracker.currentProject : null;
    const resp = window.explanatoryEngine ? window.explanatoryEngine.explain(queryText, currentProj) : null;
    if (!resp) return;

    let metricsHtml = "";
    if (resp.metrics) {
      metricsHtml = `<div class="dharini-metrics-chips" style="margin: 14px 0; display: flex; flex-wrap: wrap; gap: 8px;">`;
      for (const [k, v] of Object.entries(resp.metrics)) {
        metricsHtml += `<span class="dharini-metric-tag" style="background:#e0e7ff; color:#3730a3; padding:6px 12px; border-radius:16px; font-size:0.8rem; font-weight:700;"><strong>${k.replace(/_/g, ' ').toUpperCase()}:</strong> ${v}</span>`;
      }
      metricsHtml += `</div>`;
    }

    const loc = (currentProj && currentProj.geolocation) ? currentProj.geolocation : { latitude: 13.1986, longitude: 77.7066, chainage_start_km: 0.0, chainage_end_km: 12.0 };

    let stepsHtml = "";
    (resp.remediation_steps || []).forEach((s, idx) => {
      stepsHtml += `<li style="margin-bottom: 8px;"><strong>[${idx + 1}]</strong> ${s}</li>`;
    });

    let citationsHtml = "";
    (resp.citations || []).forEach(c => {
      citationsHtml += `<li style="margin-bottom: 4px;">📜 ${c}</li>`;
    });

    container.innerHTML = `
      <div class="gov-card" style="border-left: 5px solid #002147; padding: 24px;">
        <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 14px; flex-wrap: wrap; gap: 10px;">
          <div>
            <span class="status-badge-gov bg-gov-info">${resp.category}</span>
            <h3 style="font-size: 1.2rem; color: #002147; font-weight: 800; margin-top: 6px;">${resp.title}</h3>
          </div>
          <button id="btn-ask-dharini-direct" type="button" class="btn-gov-warning" style="background: #0062cc; color: #ffffff; padding: 8px 16px; border-radius: 4px; font-weight: 700; cursor: pointer; border: none; box-shadow: 0 2px 6px rgba(0,98,204,0.3);">
            👩‍💼 Ask NICCI AI (Voice Speech)
          </button>
        </div>

        <div style="font-size: 0.95rem; line-height: 1.6; color: #1e293b; background: #f8fafc; padding: 16px; border-radius: 6px; border: 1px solid #e2e8f0; margin-bottom: 16px;">
          <strong>Executive Statutory Verdict:</strong> ${resp.direct_answer}
        </div>

        ${metricsHtml}

        <div style="background: #fffbeb; border: 1px solid #fde68a; padding: 12px 16px; border-radius: 6px; margin-bottom: 18px; display: flex; align-items: center; gap: 10px;">
          <span style="font-size: 1.2rem;">⚖️</span>
          <div style="font-size: 0.85rem; color: #92400e;">
            <strong>Statutory Authority & Act References:</strong> ${resp.statutory_authority}
          </div>
        </div>

        <!-- Geo-Location & Frontage Preview -->
        <div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(280px, 1fr)); gap: 16px; margin-bottom: 18px;">
          <div style="background: #ffffff; border: 1px solid #cbd5e1; border-radius: 6px; padding: 14px;">
            <div style="font-size: 0.85rem; font-weight: 700; color: #002147; margin-bottom: 8px;">📍 GPS Geotagged Field Verification</div>
            <div style="font-size: 0.82rem; color: #475569; line-height: 1.6;">
              <div>Latitude/Longitude: <strong class="text-mono text-purple">${loc.latitude}° N, ${loc.longitude}° E</strong></div>
              <div>Corridor Chainage: <strong>Km ${loc.chainage_start_km} to Km ${loc.chainage_end_km}</strong></div>
              <div>DGPS Precision: <strong>±1.2m (WGS84 High-Precision Fix)</strong></div>
            </div>
            <button id="btn-view-proof-from-explain" type="button" class="btn-gov-primary" style="margin-top: 12px; width: 100%; font-size: 0.78rem; padding: 6px 12px;">
              📜 Open Certified Document Proof & Geotag
            </button>
          </div>

          <div style="background: #ffffff; border: 1px solid #cbd5e1; border-radius: 6px; padding: 14px;">
            <div style="font-size: 0.85rem; font-weight: 700; color: #002147; margin-bottom: 8px;">📋 Actionable CALA Remediation Checklist</div>
            <ul style="font-size: 0.82rem; color: #334155; padding-left: 16px; line-height: 1.5;">
              ${stepsHtml}
            </ul>
          </div>
        </div>

        <div>
          <div style="font-size: 0.8rem; font-weight: 700; color: #64748b; text-transform: uppercase; margin-bottom: 6px;">Authoritative Evidence Vault References:</div>
          <ul style="font-size: 0.8rem; color: #475569; line-height: 1.5; padding-left: 18px;">
            ${citationsHtml}
          </ul>
        </div>
      </div>
    `;

    const dhariniBtn = document.getElementById("btn-ask-dharini-direct");
    if (dhariniBtn && window.dhariniAssistant) {
      dhariniBtn.addEventListener("click", () => {
        window.dhariniAssistant.toggleDrawer();
        if (queryText) {
          window.dhariniAssistant.inputEl.value = queryText;
          window.dhariniAssistant.handleSend();
        }
      });
    }

    const proofBtn = document.getElementById("btn-view-proof-from-explain");
    if (proofBtn && window.documentProofModal && currentProj) {
      proofBtn.addEventListener("click", () => {
        const proof = (currentProj.document_proofs && currentProj.document_proofs.forest_clearance) || {
          document_title: resp.title,
          document_type: "STATUTORY_CLEARANCE",
          issuing_authority: resp.statutory_authority,
          official_letter_no: currentProj.proposal_no || "NHAI/EVID/2026",
          location: currentProj.geolocation,
          conditions: resp.remediation_steps
        };
        window.documentProofModal.open(proof, currentProj);
      });
    }
  }

  renderHistoryTimelineView(selectedPid = "NHAI-NE7-PKG-04") {
    const container = document.getElementById("history-timeline-view-container");
    if (!container) return;

    const projList = (window.projectTracker && window.projectTracker.projectsData) ? window.projectTracker.projectsData : [];
    const proj = projList.find(p => p.project_id === selectedPid) || projList[0];
    if (!proj) return;

    const historyItems = proj.history || [];
    let rowsHtml = "";
    historyItems.forEach((h) => {
      rowsHtml += `
        <tr class="history-table-row">
          <td style="padding: 10px 12px; border-bottom: 1px solid #e2e8f0;"><span class="history-ver-badge" style="background:#e0e7ff; color:#3730a3; padding:4px 8px; border-radius:4px; font-weight:700;">${h.version_id}</span></td>
          <td style="padding: 10px 12px; border-bottom: 1px solid #e2e8f0; font-family:monospace; font-weight:700; color:#002147;">${h.event_type}</td>
          <td style="padding: 10px 12px; border-bottom: 1px solid #e2e8f0; font-size:0.85rem;">${h.summary}</td>
          <td style="padding: 10px 12px; border-bottom: 1px solid #e2e8f0; font-family:monospace; font-size:0.75rem;">
            <div><strong>Prior:</strong> ${h.prior_value}</div>
            <div style="color:#16a34a; font-weight:700;"><strong>New:</strong> ${h.new_value}</div>
          </td>
          <td style="padding: 10px 12px; border-bottom: 1px solid #e2e8f0; font-size:0.8rem;">${h.recorded_by}</td>
          <td style="padding: 10px 12px; border-bottom: 1px solid #e2e8f0; font-family:monospace; font-size:0.78rem;">${h.valid_time.replace("T", " ").replace("Z", "")}</td>
          <td style="padding: 10px 12px; border-bottom: 1px solid #e2e8f0; font-family:monospace; font-size:0.78rem;">${h.transaction_time.replace("T", " ").replace("Z", "")}</td>
          <td style="padding: 10px 12px; border-bottom: 1px solid #e2e8f0;">
            <a href="https://drive.google.com/file/d/${h.drive_file_id}/view?usp=drivesdk" target="_blank" style="color:#4338ca; font-weight:700; text-decoration:none; font-size:0.8rem;">
              📁 Vault ↗
            </a>
          </td>
        </tr>
      `;
    });

    container.innerHTML = `
      <div style="background: #ffffff; border: 1px solid #cbd5e1; border-radius: var(--radius-md); padding: 20px; box-shadow: var(--shadow-sm);">
        <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 16px; flex-wrap: wrap; gap: 10px;">
          <div>
            <h3 style="font-size: 1.15rem; color: #002147; font-weight: 800; margin-bottom: 4px;">
              ${proj.project_name} <span class="badge-gov-highway">${proj.highway_no}</span>
            </h3>
            <div style="font-size: 0.8rem; color: #475569;">
              State: <strong>${proj.state_name || proj.state}</strong> | District: <strong>${proj.district}</strong> | Alignment: <strong>${proj.corridor_length_km || 12.0} km</strong>
            </div>
          </div>
          <span class="history-ledger-badge" style="background: #ecfdf5; color: #065f46; border: 1px solid #a7f3d0; padding: 6px 12px; border-radius: 16px; font-weight: 700; font-size: 0.8rem;">
            ✓ Bitemporal Audit Integrity 100%
          </span>
        </div>

        <div class="table-responsive">
          <table class="history-diff-table" style="width: 100%; border-collapse: collapse; font-size: 0.85rem;">
            <thead>
              <tr style="background: #f1f5f9; text-align: left;">
                <th style="padding: 10px 12px; border-bottom: 2px solid #cbd5e1;">Version</th>
                <th style="padding: 10px 12px; border-bottom: 2px solid #cbd5e1;">Event Envelope</th>
                <th style="padding: 10px 12px; border-bottom: 2px solid #cbd5e1;">Summary of State Change</th>
                <th style="padding: 10px 12px; border-bottom: 2px solid #cbd5e1;">Side-by-Side Delta (Prior vs New)</th>
                <th style="padding: 10px 12px; border-bottom: 2px solid #cbd5e1;">Authorized By</th>
                <th style="padding: 10px 12px; border-bottom: 2px solid #cbd5e1;">Valid Time (Tv)</th>
                <th style="padding: 10px 12px; border-bottom: 2px solid #cbd5e1;">Transaction Time (Tt)</th>
                <th style="padding: 10px 12px; border-bottom: 2px solid #cbd5e1;">Google Drive</th>
              </tr>
            </thead>
            <tbody>
              ${rowsHtml || '<tr><td colspan="8" style="padding: 16px; text-align: center; color: #64748b;">No revisions recorded.</td></tr>'}
            </tbody>
          </table>
        </div>
      </div>
    `;
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
