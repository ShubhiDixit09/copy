/**
 * DHARTI - MIS Executive Report Generator
 * Generates formatted, print-ready National Land Acquisition Status Briefings.
 */

class MISReports {
  static generateReport(dataStore, pciResult) {
    const p = dataStore.project;
    const parcels = dataStore.parcels;
    const households = dataStore.households;
    const ledger = dataStore.auditLedger;

    const totalSanctioned = parcels.reduce((acc, x) => acc + x.sanctioned_amount_inr, 0);
    const totalDisbursed = parcels.reduce((acc, x) => acc + x.disbursed_amount_inr, 0);
    const totalPolygonArea = parcels.reduce((acc, x) => acc + x.polygon_area_sqm, 0);
    const readyCount = parcels.filter(x => x.state === "ConstructionReady").length;
    const stayCount = parcels.filter(x => x.court_stay).length;

    const nowStr = new Date().toLocaleString('en-IN', { timeZone: 'Asia/Kolkata' });

    let html = `
      <div class="mis-report-container" id="mis-printable-sheet">
        <div class="mis-report-header">
          <div class="mis-national-emblem">
            <span class="emblem-text">सत्यमेव जयते</span>
            <div class="emblem-sub">GOVERNMENT OF INDIA</div>
          </div>
          <div class="mis-header-title">
            <h2>NATIONAL LAND ACQUISITION & MANAGEMENT SYSTEM (DHARTI)</h2>
            <h3>STATUTORY EXECUTIVE MIS REPORT & EVIDENCE DOSSIER</h3>
            <p class="report-meta"><b>Generated On:</b> ${nowStr} (IST) | <b>Security Classification:</b> OFFICIAL / AUDIT SENSITIVE</p>
          </div>
        </div>

        <hr class="mis-divider">

        <div class="mis-section">
          <h4>1. PROJECT IDENTIFICATION & STATUTORY BASIS</h4>
          <table class="mis-meta-table">
            <tr>
              <td><b>Project Code:</b></td><td><code>${p.project_id}</code></td>
              <td><b>Corridor Length:</b></td><td>${p.alignment_length_km} km (Chainage 0.0 &rarr; 12.0 km)</td>
            </tr>
            <tr>
              <td><b>Project Name:</b></td><td colspan="3">${p.project_name}</td>
            </tr>
            <tr>
              <td><b>Requiring Body:</b></td><td>${p.requiring_body}</td>
              <td><b>Competent Authority:</b></td><td>${p.competent_authority}</td>
            </tr>
            <tr>
              <td><b>Jurisdiction:</b></td><td>${p.district}, ${p.jurisdiction_state}</td>
              <td><b>Gazette Notification:</b></td><td>${p.gazette_notification_ref}</td>
            </tr>
            <tr>
              <td><b>Governing Acts:</b></td><td colspan="3">${p.statutory_act}</td>
            </tr>
          </table>
        </div>

        <div class="mis-section">
          <h4>2. EXECUTIVE SUMMARY & KEY PARAMETERS MONITOR</h4>
          <div class="mis-kpi-grid">
            <div class="mis-kpi-card">
              <span class="kpi-label">Total Corridor Length</span>
              <span class="kpi-val">${p.alignment_length_km} km</span>
            </div>
            <div class="mis-kpi-card">
              <span class="kpi-label">Evidence-Ready PCI</span>
              <span class="kpi-val text-primary">${pciResult.pci}%</span>
            </div>
            <div class="mis-kpi-card">
              <span class="kpi-label">Longest Continuous Frontage</span>
              <span class="kpi-val">${pciResult.maxContinuousFrontageKm} km</span>
            </div>
            <div class="mis-kpi-card">
              <span class="kpi-label">Total Area Notified</span>
              <span class="kpi-val">${(totalPolygonArea / 10000).toFixed(2)} Ha</span>
            </div>
            <div class="mis-kpi-card">
              <span class="kpi-label">Parcels Handed Over</span>
              <span class="kpi-val">${readyCount} / ${parcels.length}</span>
            </div>
            <div class="mis-kpi-card">
              <span class="kpi-label">Total Disbursed (PFMS)</span>
              <span class="kpi-val text-success">₹${(totalDisbursed / 1e7).toFixed(2)} Cr</span>
            </div>
            <div class="mis-kpi-card">
              <span class="kpi-label">Pending Compensation</span>
              <span class="kpi-val text-warning">₹${((totalSanctioned - totalDisbursed) / 1e7).toFixed(2)} Cr</span>
            </div>
            <div class="mis-kpi-card">
              <span class="kpi-label">Active Judicial Stays</span>
              <span class="kpi-val ${stayCount > 0 ? 'text-danger' : 'text-success'}">${stayCount}</span>
            </div>
          </div>
        </div>

        <div class="mis-section">
          <h4>3. PARCEL-BY-PARCEL CADASTRAL & DISBURSEMENT INVENTORY</h4>
          <table class="mis-data-table">
            <thead>
              <tr>
                <th>P-ID</th>
                <th>ULPIN (14-Digit)</th>
                <th>Survey / Sub</th>
                <th>Owner / Khata</th>
                <th>Chainage</th>
                <th>Cadastral Area</th>
                <th>Compensation</th>
                <th>Bank UTR</th>
                <th>State Machine Status</th>
              </tr>
            </thead>
            <tbody>
              ${parcels.map(item => `
                <tr>
                  <td><b>P-${item.parcel_id}</b></td>
                  <td><code>${item.ulpin}</code></td>
                  <td>Sy ${item.survey_number}/${item.sub_division}</td>
                  <td>${item.owner_name} (${item.khata_number})</td>
                  <td>KM ${item.chainage_start_km} - ${item.chainage_end_km}</td>
                  <td>${item.polygon_area_sqm.toLocaleString()} sqm</td>
                  <td>₹${(item.disbursed_amount_inr / 1e7).toFixed(2)} Cr / ₹${(item.sanctioned_amount_inr / 1e7).toFixed(2)} Cr</td>
                  <td><code>${item.bank_utr || 'UNSETTLED'}</code></td>
                  <td><span class="status-pill status-${item.state}">${item.state}</span></td>
                </tr>
              `).join('')}
            </tbody>
          </table>
        </div>

        <div class="mis-section">
          <h4>4. SOCIAL IMPACT ASSESSMENT (SIA) & R&R HOUSEHOLD CENSUS</h4>
          <p><b>Statutory Standard:</b> RFCTLARR 2013 Chapter II / World Bank ESS5 "No Family Invisible" Protocol</p>
          <table class="mis-data-table">
            <thead>
              <tr>
                <th>HH ID</th>
                <th>Head of Household</th>
                <th>Category</th>
                <th>Members</th>
                <th>Vulnerable?</th>
                <th>Sec 23 Award</th>
                <th>R&R Entitlement</th>
              </tr>
            </thead>
            <tbody>
              ${households.map(h => `
                <tr class="${h.vulnerable && !h.award ? 'row-alert-danger' : ''}">
                  <td><code>${h.household_id}</code></td>
                  <td>${h.head}</td>
                  <td>${h.category}</td>
                  <td>${h.count}</td>
                  <td>${h.vulnerable ? '<span class="badge-tag tag-vulnerable">YES</span>' : 'No'}</td>
                  <td>${h.award ? '<span class="text-success">MAPPED</span>' : '<span class="text-danger">OMITTED</span>'}</td>
                  <td>${h.rnr ? 'Allotted' : 'Pending Review'}</td>
                </tr>
              `).join('')}
            </tbody>
          </table>
        </div>

        <div class="mis-section">
          <h4>5. CRYPTOGRAPHIC BITEMPORAL AUDIT CHAIN PROOF (TOP 5 EVENTS)</h4>
          <table class="mis-audit-table">
            <thead>
              <tr>
                <th>Event ID</th>
                <th>Event Type</th>
                <th>System Timestamp (ISO 8601)</th>
                <th>SHA-256 Ledger Hash</th>
                <th>Parent Hash</th>
              </tr>
            </thead>
            <tbody>
              ${ledger.slice(-5).map(evt => `
                <tr>
                  <td><b>${evt.id}</b></td>
                  <td>${evt.type}</td>
                  <td>${evt.timestamp}</td>
                  <td><code>${evt.hash}</code></td>
                  <td><code>${evt.prevHash}</code></td>
                </tr>
              `).join('')}
            </tbody>
          </table>
        </div>

        <div class="mis-footer">
          <p>Certified that the above land acquisition statistics are derived from authoritative state adapters and cryptographically bound to the DHARTI bitemporal ledger.</p>
          <div class="mis-signatures">
            <div class="sig-block">
              <span class="sig-line"></span>
              <span class="sig-title">Special Land Acquisition Officer (CALA)</span>
            </div>
            <div class="sig-block">
              <span class="sig-line"></span>
              <span class="sig-title">Project Director (NHAI PIU)</span>
            </div>
          </div>
        </div>
      </div>
    `;

    return html;
  }
}

window.MISReports = MISReports;
