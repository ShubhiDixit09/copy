/**
 * DHARTI - Data Store Module
 * Encapsulates authentic Indian administrative records and supports custom user uploads.
 */

const DEFAULT_PROJECT_METADATA = {
  project_id: "NHAI-NE7-PKG-04",
  project_name: "Bengaluru-Chennai Expressway (NE-7) - Package IV",
  requiring_body: "National Highways Authority of India (NHAI)",
  competent_authority: "Special Land Acquisition Officer (CALA), Devanahalli",
  jurisdiction_state: "Karnataka",
  district: "Bengaluru Rural",
  taluk: "Devanahalli",
  alignment_length_km: 12.0,
  chainage_start_km: 0.0,
  chainage_end_km: 12.0,
  gazette_notification_ref: "S.O. 2418(E) dated 14-06-2023",
  statutory_act: "National Highways Act, 1956 read with RFCTLARR Act, 2013",
  currency: "INR",
  coordinate_reference_system: "EPSG:4326 (WGS84)"
};

const DEFAULT_PARCELS = [
  {
    parcel_id: 101,
    ulpin: "29010041800101",
    district: "Bengaluru Rural",
    taluk: "Devanahalli",
    village: "Kundana",
    survey_number: "116",
    sub_division: "1",
    chainage_start_km: 0.0,
    chainage_end_km: 3.0,
    polygon_area_sqm: 30050.0,
    ror_area_sqm: 30000.0,
    owner_name: "Krishnappa Gowda",
    tenure_type: "Patta / Ryotwari",
    khata_number: "KH-881",
    state: "ConstructionReady",
    boundary_wkt: "POLYGON((77.6812 13.2410, 77.7089 13.2435, 77.7092 13.2415, 77.6815 13.2390, 77.6812 13.2410))",
    sanctioned_amount_inr: 42000000.0,
    disbursed_amount_inr: 42000000.0,
    bank_utr: "SBIN426245890101",
    payment_status: "SUCCESS",
    court_stay: false,
    dispute_details: "None - Title clear and possession taken.",
    rnr_allotments: "Allotted residential site 1,200 sqft in R&R Layout Phase 1"
  },
  {
    parcel_id: 102,
    ulpin: "29010041800102",
    district: "Bengaluru Rural",
    taluk: "Devanahalli",
    village: "Kundana",
    survey_number: "117",
    sub_division: "3B",
    chainage_start_km: 3.0,
    chainage_end_km: 5.0,
    polygon_area_sqm: 20020.0,
    ror_area_sqm: 20000.0,
    owner_name: "Anjanappa M",
    tenure_type: "Patta / Ryotwari",
    khata_number: "KH-882",
    state: "ConstructionReady",
    boundary_wkt: "POLYGON((77.7089 13.2435, 77.7274 13.2452, 77.7276 13.2432, 77.7092 13.2415, 77.7089 13.2435))",
    sanctioned_amount_inr: 31500000.0,
    disbursed_amount_inr: 31500000.0,
    bank_utr: "SBIN426245890102",
    payment_status: "SUCCESS",
    court_stay: false,
    dispute_details: "None - Solatium and interest fully credited.",
    rnr_allotments: "Livelihood annuity grant sanctioned."
  },
  {
    parcel_id: 118,
    ulpin: "29010041800118",
    district: "Bengaluru Rural",
    taluk: "Devanahalli",
    village: "Kundana",
    survey_number: "118",
    sub_division: "2A",
    chainage_start_km: 5.0,
    chainage_end_km: 6.5,
    polygon_area_sqm: 17250.0,
    ror_area_sqm: 15000.0,
    owner_name: "Ramesh Kumar (Disputed with Suresh Kumar)",
    tenure_type: "Patta / Ryotwari",
    khata_number: "KH-883",
    state: "InjunctionImposed",
    boundary_wkt: "POLYGON((77.7274 13.2452, 77.7412 13.2465, 77.7415 13.2445, 77.7276 13.2432, 77.7274 13.2452))",
    sanctioned_amount_inr: 45000000.0,
    disbursed_amount_inr: 0.0,
    bank_utr: "",
    payment_status: "REJECTED_FROZEN",
    court_stay: true,
    stay_details: "High Court WP 4021/2023 Injunction Order",
    dispute_details: "Succession dispute between co-heirs & area mismatch (Cadastral 17,250 sqm vs RoR 15,000 sqm)",
    rnr_allotments: "Pending dispute resolution"
  },
  {
    parcel_id: 104,
    ulpin: "29010041800104",
    district: "Bengaluru Rural",
    taluk: "Devanahalli",
    village: "Kundana",
    survey_number: "119",
    sub_division: "1",
    chainage_start_km: 6.5,
    chainage_end_km: 10.0,
    polygon_area_sqm: 35080.0,
    ror_area_sqm: 35000.0,
    owner_name: "Munivenkatamma",
    tenure_type: "Patta / Ryotwari",
    khata_number: "KH-884",
    state: "ConstructionReady",
    boundary_wkt: "POLYGON((77.7412 13.2465, 77.7735 13.2495, 77.7738 13.2475, 77.7415 13.2445, 77.7412 13.2465))",
    sanctioned_amount_inr: 52500000.0,
    disbursed_amount_inr: 52500000.0,
    bank_utr: "SBIN426245890104",
    payment_status: "SUCCESS",
    court_stay: false,
    dispute_details: "None - Vested under Section 3D.",
    rnr_allotments: "Vulnerable women-headed allowance credited."
  },
  {
    parcel_id: 105,
    ulpin: "29010041800105",
    district: "Bengaluru Rural",
    taluk: "Devanahalli",
    village: "Kundana",
    survey_number: "120",
    sub_division: "4",
    chainage_start_km: 10.0,
    chainage_end_km: 12.0,
    polygon_area_sqm: 20040.0,
    ror_area_sqm: 20000.0,
    owner_name: "Doddagangappa",
    tenure_type: "Patta / Ryotwari",
    khata_number: "KH-885",
    state: "PossessionConfirmed",
    boundary_wkt: "POLYGON((77.7735 13.2495, 77.7920 13.2512, 77.7923 13.2492, 77.7738 13.2475, 77.7735 13.2495))",
    sanctioned_amount_inr: 31500000.0,
    disbursed_amount_inr: 0.0,
    bank_utr: "",
    payment_status: "AWAITING_CLEARANCE",
    court_stay: false,
    dispute_details: "Compensation sanction pending PFMS treasury batch release.",
    rnr_allotments: "Agricultural relocation support scheduled."
  }
];

const DEFAULT_SIA_HOUSEHOLDS = [
  { household_id: "HH-SIA-2901-01", head: "Krishnappa Gowda", category: "TitleHolder", count: 4, vulnerable: false, award: true, rnr: true, parcel: "116/1" },
  { household_id: "HH-SIA-2901-02", head: "Anjanappa M", category: "TitleHolder", count: 5, vulnerable: false, award: true, rnr: true, parcel: "117/3B" },
  { household_id: "HH-SIA-2901-03", head: "Munivenkatamma", category: "TitleHolder", count: 3, vulnerable: true, award: true, rnr: true, parcel: "119/1" },
  { household_id: "HH-SIA-2901-04", head: "Doddagangappa", category: "TitleHolder", count: 6, vulnerable: false, award: true, rnr: true, parcel: "120/4" },
  { household_id: "HH-SIA-2901-05", head: "Ramesh Kumar", category: "TitleHolder", count: 4, vulnerable: false, award: true, rnr: true, parcel: "118/2A" },
  { household_id: "HH-SIA-2901-06", head: "Suresh Kumar", category: "CoTitleHolder", count: 4, vulnerable: false, award: true, rnr: true, parcel: "118/2A" },
  { household_id: "HH-SIA-2901-07", head: "Basavaraju & Family", category: "AgriculturalLaborer", count: 5, vulnerable: true, award: false, rnr: false, parcel: "118/2A" },
  { household_id: "HH-SIA-2901-08", head: "Gangadharan K", category: "TenantCultivator", count: 4, vulnerable: true, award: false, rnr: false, parcel: "118/2A" },
  { household_id: "HH-SIA-2901-09", head: "Narasamma", category: "VulnerableWomenHeaded", count: 2, vulnerable: true, award: false, rnr: false, parcel: "118/2A" },
  { household_id: "HH-SIA-2901-10", head: "Raju V", category: "VillageArtisan", count: 6, vulnerable: true, award: false, rnr: false, parcel: "118/2A" }
];

class DataStore {
  constructor() {
    this.project = JSON.parse(JSON.stringify(DEFAULT_PROJECT_METADATA));
    this.parcels = JSON.parse(JSON.stringify(DEFAULT_PARCELS));
    this.households = JSON.parse(JSON.stringify(DEFAULT_SIA_HOUSEHOLDS));
    this.auditLedger = [];
    this.initLedger();
  }

  initLedger() {
    this.auditLedger = [
      { id: "EVT-001", type: "GazetteNotificationPublished", timestamp: "2023-06-14T08:00:00Z", hash: "9a2f6b89c7d4e5f1", prevHash: "GENESIS_ROOT", details: "Section 3A Gazette Notification S.O. 2418(E) recorded" },
      { id: "EVT-002", type: "CadastralGISVectorIngested", timestamp: "2023-09-10T11:20:00Z", hash: "b14d87e2f5a93c60", prevHash: "9a2f6b89c7d4e5f1", details: "Bhoomi Cadastral boundaries for 5 parcels verified" },
      { id: "EVT-003", type: "HighCourtInjunctionRecorded", timestamp: "2023-11-12T14:15:00Z", hash: "c38e91024b7a1f55", prevHash: "b14d87e2f5a93c60", details: "eCourts WP 4021/2023 stay linked to Parcel P-118" },
      { id: "EVT-004", type: "SIAHouseholdCensusIngested", timestamp: "2024-02-18T10:00:00Z", hash: "d47a02c918ef3b44", prevHash: "c38e91024b7a1f55", details: "10 households recorded; 4 vulnerable families flagged" },
      { id: "EVT-005", type: "PFMSTreasuryAdvicesSynced", timestamp: "2026-08-20T16:30:00Z", hash: "e58b13d029fa4c77", prevHash: "d47a02c918ef3b44", details: "PFMS mandate batch processed (3 credited, 1 rejected, 1 pending)" }
    ];
  }

  appendAudit(eventType, details) {
    const prev = this.auditLedger[this.auditLedger.length - 1];
    const prevHash = prev ? prev.hash : "GENESIS_ROOT";
    const ts = new Date().toISOString();
    // Deterministic lightweight hash
    let str = eventType + ts + details + prevHash;
    let hash = 0;
    for (let i = 0; i < str.length; i++) {
      hash = (hash * 31 + str.charCodeAt(i)) >>> 0;
    }
    const hashHex = hash.toString(16).padStart(16, "0");
    const entry = {
      id: `EVT-${(this.auditLedger.length + 1).toString().padStart(3, "0")}`,
      type: eventType,
      timestamp: ts,
      hash: hashHex,
      prevHash: prevHash,
      details: details
    };
    this.auditLedger.push(entry);
    return entry;
  }

  reset() {
    this.project = JSON.parse(JSON.stringify(DEFAULT_PROJECT_METADATA));
    this.parcels = JSON.parse(JSON.stringify(DEFAULT_PARCELS));
    this.households = JSON.parse(JSON.stringify(DEFAULT_SIA_HOUSEHOLDS));
    this.initLedger();
  }
}

window.dataStore = new DataStore();
