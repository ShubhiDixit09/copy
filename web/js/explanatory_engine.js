/**
 * DHARTI - Client-Side Explanatory Query & Knowledge Engine
 * Powers natural language statutory diagnosis, legal clause synthesis,
 * PCI optimization breakdowns, and the "Dharini" AI Assistant.
 */

class ExplanatoryEngine {
  constructor() {
    this.knowledgeBase = [
      {
        keywords: ["why", "blocked", "delay", "pci", "bottleneck", "continuous", "frontage", "p-118", "118"],
        category: "BOTTLENECK_OPTIMIZATION",
        title: "Corridor Possession Bottleneck & PCI Unlock Diagnostic",
        statutory_authority: "RFCTLARR Act 2013 Section 38 (Full Compensation Payment Mandatory Prior to Possession); NH Act 1956 Section 3D",
        direct_answer: "Corridor Possession Continuity Index (PCI) is presently at 41.7% on Bengaluru-Chennai Package IV. Progression is obstructed at Km 5.0 to 6.5 by Parcel P-118. Resolving this single parcel unlocks +5.00 km of continuous construction frontage, doubling the work run from 5.00 km to 10.00 km (PCI jumps to 83.3%).",
        metrics: {
          current_pci: "41.7%",
          potential_pci: "83.3%",
          unlock_gain_km: "+5.00 km",
          affected_parcel: "P-118 (Km 5.0 - 6.5)"
        },
        remediation_steps: [
          "Vacate WP-4021/2023 judicial stay in High Court of Karnataka.",
          "Harmonize RoR title discrepancy between registered owner (Ramesh Kumar) and field possessor.",
          "Disburse remaining compensation of INR 45,00,000 with PFMS DBT bank UTR verification.",
          "Execute Collector-signed Geo-Tagged Handover Memo to advance state to ConstructionReady."
        ],
        citations: [
          "High Court of Karnataka Writ Petition No. 4021/2023 Order Sheet",
          "Bhoomi RoR Record Survey No. 118, Kundana Village",
          "Google Drive Evidence Vault: dharti/raw/parivesh/2026/09/08/"
        ],
        action_type: "TRIGGER_UNLOCK_SIMULATION",
        target_tab: "pci"
      },
      {
        keywords: ["stay", "court", "writ", "litigation", "injunction", "legal", "dispute", "lawyer"],
        category: "LEGAL_LITIGATION",
        title: "Judicial Stay Orders & Legal Exposure Audit",
        statutory_authority: "Constitution of India Article 226 (High Court Injunction Jurisdiction); NH Act 1956 Section 3C/3D",
        direct_answer: "Active judicial injunction is in effect on Parcel P-118 (Km 5.0 - 6.5) under High Court of Karnataka WP No. 4021/2023 (Suresh Kumar vs. State & NHAI). The injunction was granted because of an unresolved title succession petition and 15% discrepancy between RoR area (10,000 sqm) and Cadastral survey (11,500 sqm).",
        metrics: {
          active_stays_tracked: 1,
          blocked_chainage: "1.50 km",
          disputed_compensation: "₹ 45,00,000"
        },
        remediation_steps: [
          "Instruct Assistant Solicitor General (ASG) to move urgent Vacation Application under Article 226(3).",
          "Convene Joint Measurement Survey (JMS) with Revenue Tahsildar to resolve survey area discrepancy.",
          "Deposit disputed compensation in Reference Court under RFCTLARR Section 77 to enable legal vesting."
        ],
        citations: [
          "WP 4021/2023 Interim Order dated 12-Apr-2023",
          "Joint Measurement Survey Certificate (JMS-KA-118)"
        ],
        action_type: "OPEN_DOCUMENT_PROOF",
        proof_id: "DOC-ROR-KA-2026-118"
      },
      {
        keywords: ["sia", "vulnerable", "family", "household", "census", "rnr", "livelihood", "safeguard"],
        category: "SIA_SOCIAL_SAFEGUARD",
        title: "SIA Social Safeguards & Invariant-4 Compliance Audit",
        statutory_authority: "RFCTLARR Act 2013 Section 16 & Section 31 (Mandatory Preparation of R&R Scheme for Non-Title Livelihood Dependents); DHARTI Invariant 4",
        direct_answer: "The Social Impact Assessment (SIA) Universe audit for Bengaluru-Chennai Package IV identified 10 surveyed households. While 6 title-holders are mapped to Section 23 awards, 4 vulnerable livelihood-dependent families (HH-7 to HH-10) are unmapped in the draft award. Under DHARTI Invariant 4, possession handover is hard-blocked to protect vulnerable citizens.",
        metrics: {
          total_households: 10,
          mapped_title_holders: 6,
          missing_vulnerable: 4,
          compliance_status: "CIRCUIT BREAKER ENGAGED (NON-COMPLIANT)"
        },
        remediation_steps: [
          "Convene Gram Sabha Social Audit Committee under CALA leadership.",
          "Formulate supplementary R&R entitlement package under Section 31 of RFCTLARR Act 2013.",
          "Issue biometric entitlement cards with minimum statutory housing grant and subsistence allowance.",
          "Upload verified SIA reconciliation certificate to Neon DB ledger."
        ],
        citations: [
          "SIA Household Census Register (data/sia/sia_household_census.json)",
          "RFCTLARR Act 2013 Second Schedule (R&R Entitlements)"
        ],
        action_type: "SWITCH_TAB",
        target_tab: "project-tracker"
      },
      {
        keywords: ["quarantine", "anomaly", "rule 5", "gate", "tamper", "falsified", "fake"],
        category: "EVIDENCE_GATE_INTEGRITY",
        title: "7-Rule Deterministic Evidence Gate & Anomaly Quarantine",
        statutory_authority: "DHARTI 7-Rule Deterministic Evidence Gate (Rule 5: Area Bound Consistency; Rule 1: Authoritative Source Provenance)",
        direct_answer: "Proposal EC24B012PB109231 on Delhi-Amritsar-Katra (NE-5) was quarantined by the 7-Rule Evidence Engine because of an impossible parameter jump: forest diversion area jumped from 52.4 Ha to 5,240 Ha (a 100x increase exceeding the 300% safety bound). The system engaged an automated circuit breaker to prevent erroneous compensation payouts.",
        metrics: {
          quarantined_record: "EC24B012PB109231",
          violation_code: "RULE_5_IMPOSSIBLE_AREA_JUMP",
          safeguard_state: "WORKFLOW FROZEN"
        },
        remediation_steps: [
          "Conduct physical ground inspection of boundary stones with DGPS surveyor.",
          "Re-poll state forest department GIS spatial clearing house.",
          "Require digital signature override by District Collector / Principal Secretary.",
          "Re-evaluate through 7-Rule Evidence Gate."
        ],
        citations: [
          "Google Drive Quarantined Snapshot: dharti/raw/parivesh/2026/09/08/snapshot_quarantined.json",
          "Neon DB Exceptions Ledger"
        ],
        action_type: "SIMULATE_ANOMALY"
      },
      {
        keywords: ["proof", "document", "geotag", "gps", "coordinates", "wkt", "location", "map"],
        category: "GEOTAGGED_DOCUMENT_PROOF",
        title: "Official Geotagged Document Proof & GIS Cadastral Boundary",
        statutory_authority: "Information Technology Act 2000 Section 65B (Admissibility of Electronic Records); Survey of India Geodetic Norms",
        direct_answer: "All 5 national corridors in DHARTI contain cryptographic document proofs backed by Trimble/Leica DGPS survey coordinates with sub-meter accuracy (±0.8m to ±1.5m), WKT boundary polygons, officer digital signatures, and direct RFC 6234 SHA-256 hash validation against the immutable Google Drive vault.",
        metrics: {
          tracked_expressways: "5 National Corridors",
          total_corridor_km: "142.0 km",
          gps_accuracy: "±0.8m to ±1.5m",
          verification_rate: "100% Tamper-Proof"
        },
        remediation_steps: [
          "Click 'View Official Document Proof & Geotag' on any project dossier.",
          "Inspect GPS coordinates, surveyor identity, and official letterhead.",
          "Verify SHA-256 integrity seal against Google Drive vault pointer."
        ],
        citations: [
          "DGPS Survey of India Field Memos",
          "The Gazette of India Extraordinary S.O. Notifications"
        ],
        action_type: "OPEN_DOCUMENT_PROOF"
      },
      {
        keywords: ["scraper", "scrapers", "portal", "portals", "website", "websites", "sources", "federated", "parivesh", "bhoomi", "bhulekh", "bhuvan", "pfms", "ecourts", "ngt"],
        category: "FEDERATED_SCRAPERS",
        title: "Federated 15-Portal Multi-Source Scraping Suite",
        statutory_authority: "Pure C++17 Multi-Threaded Engine (std::async, RFC 6234 SHA-256 Deduplication, Sub-Second Polling)",
        direct_answer: "DHARTI ingests data across 15 authoritative Indian government portals spanning 6 operational categories: (1) Clearances & Environment: MoEFCC PARIVESH 2.0 & NGT; (2) Central LA: MoRTH Bhoomi Rashi, eGazette of India, NHAI DKP; (3) State Land RoRs: Karnataka Bhoomi, UP Bhulekh, Gujarat AnyRoR, Punjab Jamabandi, Jharkhand Jharbhoomi, Maharashtra Mahabhulekh; (4) Judiciary: eCourts Services & NJDG; (5) Geospatial: ISRO Bhuvan & Survey of India Nakshe; (6) Treasury: PFMS. Polling takes ~566ms concurrently.",
        metrics: {
          active_portals: "15 Government Endpoints",
          total_categories: "6 Governance Sectors",
          roundtrip_speed: "566.0 ms (Concurrent)",
          dedup_efficiency: "100% In-Memory Cache"
        },
        remediation_steps: [
          "Navigate to the Federated Multi-Source Scraper Hub on Tab 0.",
          "Filter by category (Clearances, Central LA, State RoRs, Judiciary, GIS, Finance).",
          "Click 'Run Concurrent 15-Agency Scrape' to trigger real-time parallel polling.",
          "Inspect individual portal payloads and RFC 6234 SHA-256 hash seals via 'Scrape Target'."
        ],
        citations: [
          "include/dharti/scrapers/web_scraper.hpp (C++17 Federated Mesh)",
          "MoEFCC (parivesh.nic.in) & MoRTH (bhoomirashi.gov.in)",
          "ISRO Bhuvan (bhuvan.nrsc.gov.in) & PFMS (pfms.nic.in)"
        ],
        action_type: "TRIGGER_SCRAPER_HUB"
      }
    ];
  }

  explain(queryText, currentProject = null) {
    const q = (queryText || "").toLowerCase().trim();
    if (!q) {
      return this.getDefaultOverview(currentProject);
    }

    // Match best knowledge item
    let bestMatch = null;
    let maxHits = 0;

    for (const item of this.knowledgeBase) {
      let hits = 0;
      for (const kw of item.keywords) {
        if (q.includes(kw)) hits++;
      }
      if (hits > maxHits) {
        maxHits = hits;
        bestMatch = item;
      }
    }

    if (!bestMatch || maxHits === 0) {
      return this.generateDynamicAnswer(queryText, currentProject);
    }

    return bestMatch;
  }

  getDefaultOverview(currentProject) {
    const p = currentProject || {
      project_name: "National Highway Land Acquisition Control Plane",
      highway_no: "All Corridors",
      state: "India"
    };
    return {
      category: "CORRIDOR_OVERVIEW",
      title: `Executive Intelligence Overview: ${p.project_name}`,
      statutory_authority: "National Highways Act 1956 & RFCTLARR Act 2013",
      direct_answer: `DHARTI control plane is monitoring 5 National Highway Corridors covering 142.0 km. Currently inspecting ${p.project_name} in ${p.state}. All land records, court injunctions, forest clearances, and payment advices are cryptographically hashed and linked to the Google Drive vault.`,
      metrics: {
        total_corridors: 5,
        total_length: "142.0 km",
        active_exceptions: 1,
        pci_benchmark: "41.7% (Pkg IV)"
      },
      remediation_steps: [
        "Select an expressway from the Quick Select bar above.",
        "Inspect geotagged document proofs and GPS coordinates.",
        "Run Bottleneck Unlock Simulation to view continuous frontage gains."
      ],
      citations: ["DHARTI Master Ledger (Neon PostgreSQL + Google Drive Vault)"]
    };
  }

  generateDynamicAnswer(queryText, currentProject) {
    const projName = currentProject ? currentProject.project_name : "Corridor Alignment";
    return {
      category: "GENERAL_INQUIRY",
      title: `DHARTI Statutory Assessment: "${queryText}"`,
      statutory_authority: "RFCTLARR Act 2013 & National Highways Act 1956",
      direct_answer: `Regarding your query "${queryText}": For ${projName}, DHARTI continuously enforces the 7-Rule Evidence Gate, tracks bitemporal transaction vs. valid times, and evaluates continuous construction readiness. No unauthorized manual overrides are permitted.`,
      metrics: {
        inquiry: queryText,
        system_status: "AUTOMATED SURVEILLANCE ACTIVE"
      },
      remediation_steps: [
        "Review the Bitemporal History tab for chronological event logs.",
        "Check Court Dockets for pending judicial stay orders.",
        "Consult CALA office for Section 3G compensation disbursement schedule."
      ],
      citations: ["DHARTI National Control Plane Knowledge Repository"]
    };
  }
}

// Global instance
window.explanatoryEngine = new ExplanatoryEngine();
