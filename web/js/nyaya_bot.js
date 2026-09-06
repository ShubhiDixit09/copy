/**
 * NyayaBot for DHARTI
 *
 * A deterministic, evidence-grounded prototype of the production NyayaBot
 * tool layer. Domain calculations remain inside DHARTI; this module only
 * resolves intent, applies role policy, explains results, cites evidence and
 * creates human-reviewable action drafts.
 */

const NYAYA_ROLE_POLICIES = Object.freeze({
  cala: {
    label: "CALA / Competent Authority",
    scopes: ["capabilities", "blockers", "pci", "families", "payment", "audit", "policy", "grievance"],
    canDraftOperationalAction: true
  },
  ministry: {
    label: "Central Ministry / MoRTH",
    scopes: ["capabilities", "blockers", "pci", "families", "payment", "audit", "policy"],
    canDraftOperationalAction: true
  },
  state: {
    label: "State Revenue Department",
    scopes: ["capabilities", "blockers", "pci", "families", "payment", "audit", "policy", "grievance"],
    canDraftOperationalAction: true
  },
  piu: {
    label: "NHAI Project Director",
    scopes: ["capabilities", "blockers", "pci", "families", "payment", "policy"],
    canDraftOperationalAction: true
  },
  auditor: {
    label: "CAG / Statutory Auditor",
    scopes: ["capabilities", "blockers", "pci", "families", "payment", "audit", "policy"],
    canDraftOperationalAction: false
  },
  claimant: {
    label: "Verified Affected Family",
    scopes: ["capabilities", "pci", "payment", "policy", "grievance"],
    canDraftOperationalAction: false
  }
});

const NYAYA_INTENT_PATTERNS = Object.freeze([
  ["security", /ignore\s+(all\s+)?(previous|system)|bypass|system\s+prompt|dump\s+(the\s+)?database|show\s+all\s+(owners|families)|reveal\s+(private|pii)/i],
  ["pci", /\bpci\b|constructi|continuous|corridor|frontage|unlock|buildable|nirman|ban\s+sakt/i],
  ["families", /famil|household|sia|r\s*&\s*r|rehabil|entitlement|parivaar|punarvas|invisible/i],
  ["payment", /payment|compensation|pfms|bank|utr|credit|paid|muawza|bhugtan|paisa/i],
  ["audit", /audit|history|timeline|who\s+changed|what\s+did.*know|event|receipt|kab\s+badla/i],
  ["policy", /law|act|section|rule|policy|deadline|statutory|legal|kanoon|dhara|niyam/i],
  ["grievance", /grievance|complaint|appeal|draft|notice|application|shikayat/i],
  ["blockers", /block|stuck|why|contradict|dispute|court|stay|issue|problem|kyun|ruka|atka/i],
  ["capabilities", /feature|what can|help|capabilit|everything|all\s+features|kya\s+kar/i]
]);

const NYAYA_COPY = Object.freeze({
  en: {
    welcome: "Ask about a parcel, payment, affected families, statutory rules, audit history or construction readiness.",
    noAccess: "This request is outside your current role scope.",
    noEvidence: "I cannot verify this from the evidence currently available.",
    decisionBoundary: "NyayaBot explains and drafts. An authorised human must approve every official action."
  },
  hi: {
    welcome: "भूमि खंड, भुगतान, प्रभावित परिवार, कानूनी नियम, ऑडिट इतिहास या निर्माण-तत्परता के बारे में पूछें।",
    noAccess: "यह जानकारी आपकी वर्तमान भूमिका की अनुमति से बाहर है।",
    noEvidence: "उपलब्ध साक्ष्यों से इसकी पुष्टि नहीं हो सकती।",
    decisionBoundary: "NyayaBot केवल समझाता और मसौदा बनाता है। आधिकारिक निर्णय अधिकृत अधिकारी ही लेते हैं।"
  },
  hinglish: {
    welcome: "Parcel, payment, affected families, legal rules, audit history ya construction readiness ke baare mein poochhiye.",
    noAccess: "Yeh request aapke current role scope ke bahar hai.",
    noEvidence: "Available evidence se is baat ko verify nahi kiya ja sakta.",
    decisionBoundary: "NyayaBot sirf explain aur draft karta hai; final action authorised officer approve karega."
  }
});

class NyayaBotEngine {
  constructor(dataStore, pciEngine) {
    this.dataStore = dataStore;
    this.pciEngine = pciEngine;
    this.receiptCounter = 0;
  }

  ask(question, context = {}) {
    const query = String(question || "").trim();
    const role = NYAYA_ROLE_POLICIES[context.role] ? context.role : "cala";
    const language = NYAYA_COPY[context.language] ? context.language : "en";
    const parcelId = this.resolveParcelId(query, context.parcelId);
    const intent = this.classifyIntent(query);
    const receiptId = this.nextReceiptId();
    const base = { intent, role, language, parcelId, receiptId };

    if (intent === "security") {
      return this.securityRefusal(base);
    }

    if (!NYAYA_ROLE_POLICIES[role].scopes.includes(intent)) {
      return this.accessRefusal(base);
    }

    let response;
    switch (intent) {
      case "blockers": response = this.explainBlockers(base); break;
      case "pci": response = this.explainPCI(base); break;
      case "families": response = this.explainFamilyInclusion(base); break;
      case "payment": response = this.explainPayment(base); break;
      case "audit": response = this.explainAudit(base); break;
      case "policy": response = this.explainPolicy(base); break;
      case "grievance": response = this.explainGrievance(base); break;
      default: response = this.explainCapabilities(base);
    }

    response.asOf = this.evidenceAsOf();
    response.boundary = NYAYA_COPY[language].decisionBoundary;
    response.confidence = response.citations && response.citations.length ? "High" : "Insufficient evidence";
    return response;
  }

  classifyIntent(query) {
    for (const [intent, pattern] of NYAYA_INTENT_PATTERNS) {
      if (pattern.test(query)) return intent;
    }
    return "capabilities";
  }

  resolveParcelId(query, fallback) {
    const match = query.match(/\bp\s*[-#]?\s*(\d{2,})\b/i);
    if (match) return Number(match[1]);
    return Number(fallback || 118);
  }

  nextReceiptId() {
    this.receiptCounter += 1;
    return `NYB-${Date.now().toString(36).toUpperCase()}-${String(this.receiptCounter).padStart(3, "0")}`;
  }

  evidenceAsOf() {
    const ledger = (this.dataStore && this.dataStore.auditLedger) || [];
    const evidenceEvents = ledger.filter(event => !String(event.type || "").startsWith("NyayaBot"));
    const latest = evidenceEvents[evidenceEvents.length - 1];
    return latest ? latest.timestamp : "No timestamped evidence";
  }

  citation(id, label, source) {
    return { id, label, source };
  }

  securityRefusal(base) {
    return {
      ...base,
      title: "ShieldAI safety gate engaged",
      summary: "I cannot bypass role controls, reveal protected records or expose internal instructions.",
      facts: ["The request was blocked before any DHARTI data tool was called.", "No query text or personal data was written to the audit ledger."],
      citations: [],
      boundary: "Use a permitted case-specific question or request access through the authorised workflow.",
      confidence: "Blocked by policy",
      asOf: this.evidenceAsOf()
    };
  }

  accessRefusal(base) {
    const copy = NYAYA_COPY[base.language];
    return {
      ...base,
      title: "Role-scoped access protected",
      summary: copy.noAccess,
      facts: ["Anyone may ask a question, but answers are limited by identity, purpose and case relationship.", "Switch to an authorised role or verify a Digital Claim ID to continue."],
      citations: [],
      boundary: copy.decisionBoundary,
      confidence: "Access restricted",
      asOf: this.evidenceAsOf()
    };
  }

  explainBlockers(base) {
    const parcel = this.findParcel(base.parcelId);
    if (!parcel) return this.notFound(base);

    const mismatch = parcel.ror_area_sqm
      ? Math.abs(parcel.polygon_area_sqm - parcel.ror_area_sqm) / parcel.ror_area_sqm * 100
      : 0;
    const facts = [`Current independent parcel state: ${parcel.state}.`];
    const citations = [];

    if (parcel.court_stay) {
      facts.push(`Judicial gate: active stay — ${parcel.stay_details}.`);
      citations.push(this.citation("EVT-003", "High Court injunction linked to parcel", "eCourts snapshot"));
    }
    if (mismatch > 1) {
      facts.push(`Spatial-title contradiction: cadastral area ${parcel.polygon_area_sqm.toLocaleString()} m² vs RoR ${parcel.ror_area_sqm.toLocaleString()} m² (${mismatch.toFixed(1)}%).`);
      citations.push(this.citation("EVT-002", "Cadastral and RoR area comparison", "Bhoomi / RoR evidence"));
    }
    if (parcel.disbursed_amount_inr < parcel.sanctioned_amount_inr || !parcel.bank_utr) {
      facts.push(`Payment gate: ₹${(parcel.disbursed_amount_inr / 1e7).toFixed(2)} Cr confirmed against ₹${(parcel.sanctioned_amount_inr / 1e7).toFixed(2)} Cr sanctioned; status ${parcel.payment_status}.`);
      citations.push(this.citation("EVT-005", "PFMS advice and bank acknowledgement", "PFMS / bank evidence"));
    }
    if (facts.length === 1 && parcel.state === "ConstructionReady") {
      facts.push("No active judicial, payment or evidence gate is blocking construction readiness.");
      citations.push(this.citation("LIVE-PARCEL", "Current verified parcel state", "DHARTI Evidence Twin"));
    }

    const canDraft = NYAYA_ROLE_POLICIES[base.role].canDraftOperationalAction;
    return {
      ...base,
      title: `Why parcel P-${parcel.parcel_id} is ${parcel.state === "ConstructionReady" ? "ready" : "blocked"}`,
      summary: parcel.state === "ConstructionReady"
        ? "The evidence gates currently support construction readiness."
        : `${Math.max(0, facts.length - 1)} evidence-backed gate(s) prevent premature handover.`,
      facts,
      citations,
      action: canDraft && parcel.state !== "ConstructionReady" ? {
        label: "Draft coordinated resolution task",
        eventType: "NyayaBotResolutionTaskDrafted",
        details: `Drafted multi-agency review for P-${parcel.parcel_id}; no parcel state changed.`
      } : null
    };
  }

  explainPCI(base) {
    const parcels = this.dataStore.parcels || [];
    const total = this.dataStore.project.alignment_length_km;
    const pci = this.pciEngine.computePCI(parcels, total);
    const simulation = this.pciEngine.simulateBottleneckUnlocks(parcels, total);
    const top = simulation.rankings[0];
    const facts = [
      `${pci.maxContinuousFrontageKm.toFixed(1)} km is continuously construction-ready out of ${total.toFixed(1)} km.`,
      `Total ready length is ${pci.totalReadyKm.toFixed(1)} km; PCI uses the longest continuous run, not aggregate acreage.`
    ];
    if (top) {
      facts.push(`Highest-unlock candidate: P-${top.parcel_id}; resolving all verified gates would simulate ${top.simulated_max_km.toFixed(1)} km continuity (${top.simulated_pci.toFixed(2)}% PCI).`);
    }

    return {
      ...base,
      title: `Possession Continuity Index: ${pci.pci.toFixed(2)}%`,
      summary: "Reported acquisition is not treated as proof that engineers can build continuously.",
      facts,
      citations: [
        this.citation("PCI-LIVE", "Live interval-merging calculation", "DHARTI PCI Engine"),
        this.citation("PARCEL-STATES", "Verified parcel readiness states", "DHARTI Evidence Twin")
      ],
      action: top && NYAYA_ROLE_POLICIES[base.role].canDraftOperationalAction ? {
        label: `Draft P-${top.parcel_id} unlock review`,
        eventType: "NyayaBotUnlockReviewDrafted",
        details: `Drafted unlock review for P-${top.parcel_id}; simulation only, no readiness state changed.`
      } : null
    };
  }

  explainFamilyInclusion(base) {
    const households = this.dataStore.households || [];
    const missing = households.filter(household => !household.award || !household.rnr);
    const vulnerable = missing.filter(household => household.vulnerable);
    const categories = [...new Set(missing.map(household => household.category))];

    return {
      ...base,
      title: `${missing.length} affected families require inclusion review`,
      summary: "DHARTI compares the SIA census with award and R&R mappings so livelihood-dependent families do not disappear between systems.",
      facts: [
        `${households.length} households surveyed; ${households.length - missing.length} currently mapped to both award and R&R.`,
        `${vulnerable.length} vulnerable households remain unmapped.`,
        `Unresolved categories: ${categories.join(", ") || "None"}. Identities are withheld from this summary.`
      ],
      citations: [
        this.citation("EVT-004", "SIA household census snapshot", "SIA evidence"),
        this.citation("AWARD-RNR-MAP", "Award and R&R mapping comparison", "DHARTI inclusion engine")
      ],
      action: NYAYA_ROLE_POLICIES[base.role].canDraftOperationalAction ? {
        label: "Draft supplementary inclusion review",
        eventType: "NyayaBotInclusionReviewDrafted",
        details: `Drafted human review for ${missing.length} unmapped households; no entitlement decision made.`
      } : null
    };
  }

  explainPayment(base) {
    if (base.role === "claimant") {
      return {
        ...base,
        title: "Digital Claim ID verification required",
        summary: "I can show only your own compensation and R&R record after case-linked authentication.",
        facts: ["Project-wide beneficiary names, bank details and UTRs remain hidden.", "After verification, NyayaBot can distinguish sanctioned, instructed, bank-acknowledged and credited states."],
        citations: [this.citation("ACCESS-POLICY", "Claimant self-service access rule", "DHARTI policy gate")],
        action: {
          label: "Start verified claim lookup",
          eventType: "NyayaBotClaimLookupDrafted",
          details: "Started claimant verification draft; no personal record accessed."
        }
      };
    }

    const parcel = this.findParcel(base.parcelId);
    if (!parcel) return this.notFound(base);
    const settled = parcel.payment_status === "SUCCESS" && Boolean(parcel.bank_utr) && parcel.disbursed_amount_inr >= parcel.sanctioned_amount_inr;
    return {
      ...base,
      title: `Payment trace for P-${parcel.parcel_id}`,
      summary: settled ? "Payment is evidence-confirmed at beneficiary-bank level." : "Payment is not yet evidence-confirmed as received.",
      facts: [
        `Sanctioned: ₹${(parcel.sanctioned_amount_inr / 1e7).toFixed(2)} Cr.`,
        `Bank-confirmed disbursement: ₹${(parcel.disbursed_amount_inr / 1e7).toFixed(2)} Cr.`,
        `PFMS status: ${parcel.payment_status}; bank acknowledgement: ${parcel.bank_utr || "missing"}.`
      ],
      citations: [this.citation("EVT-005", "PFMS advice reconciled with bank acknowledgement", "PFMS / bank evidence")],
      action: !settled && NYAYA_ROLE_POLICIES[base.role].canDraftOperationalAction ? {
        label: "Draft payment reconciliation task",
        eventType: "NyayaBotPaymentReviewDrafted",
        details: `Drafted payment reconciliation review for P-${parcel.parcel_id}; no payment instruction issued.`
      } : null
    };
  }

  explainAudit(base) {
    const events = (this.dataStore.auditLedger || [])
      .filter(event => !String(event.type || "").startsWith("NyayaBot"))
      .slice(-4)
      .reverse();
    return {
      ...base,
      title: "Evidence Twin audit reconstruction",
      summary: "The answer is reconstructed from timestamped events; newer records do not overwrite what was previously known.",
      facts: events.map(event => `${event.id} · ${event.type} · ${event.timestamp}`),
      citations: events.map(event => this.citation(event.id, event.details, "DHARTI append-only event ledger")),
      action: null
    };
  }

  explainPolicy(base) {
    return {
      ...base,
      title: "Applicable policy context",
      summary: "The demo project uses versioned policy references; rules remain separate from parcel facts and recommendations.",
      facts: [
        this.dataStore.project.statutory_act,
        "Active judicial restrictions override readiness recommendations.",
        "A generated explanation is not legal advice or an adjudication."
      ].filter(Boolean),
      citations: [
        this.citation("PROJECT-POLICY", this.dataStore.project.statutory_act || "Project statutory policy", "DHARTI project metadata"),
        this.citation("EVT-001", "Gazette notification evidence", "Gazette source snapshot")
      ],
      action: null
    };
  }

  explainGrievance(base) {
    const claimant = base.role === "claimant";
    return {
      ...base,
      title: claimant ? "Prepare a privacy-safe grievance" : "Prepare an evidence-linked case note",
      summary: claimant
        ? "NyayaBot can draft a grievance after verifying the claimant and the linked acquisition case."
        : "NyayaBot can assemble a draft from cited evidence without deciding the case.",
      facts: [
        "Draft includes the relevant parcel or claim reference, missing evidence and requested remedy.",
        "The user reviews the draft before submission.",
        "Submission and final decision remain inside the authorised maker-checker workflow."
      ],
      citations: [this.citation("DRAFTING-POLICY", "Human-reviewed document workflow", "NyayaBot / ShieldAI policy")],
      action: {
        label: claimant ? "Start verified grievance draft" : "Draft evidence-linked case note",
        eventType: claimant ? "NyayaBotGrievanceDraftStarted" : "NyayaBotCaseNoteDrafted",
        details: "Created a human-reviewable draft shell; no grievance submitted and no official state changed."
      }
    };
  }

  explainCapabilities(base) {
    return {
      ...base,
      title: "Ask, understand, simulate and act safely",
      summary: NYAYA_COPY[base.language].welcome,
      facts: [
        "Explain parcel blockers and cross-source contradictions.",
        "Calculate PCI and simulate highest-unlock parcels.",
        "Detect missing affected families without exposing identities.",
        "Trace compensation from sanction to bank acknowledgement.",
        "Explain versioned policy and statutory clocks.",
        "Reconstruct audit history and answer receipts.",
        "Draft tasks, case notes and grievances for human approval."
      ],
      citations: [this.citation("TOOL-CATALOG", "Authorised DHARTI tool catalogue", "NyayaBot orchestrator")],
      action: null
    };
  }

  findParcel(parcelId) {
    return (this.dataStore.parcels || []).find(parcel => Number(parcel.parcel_id) === Number(parcelId));
  }

  notFound(base) {
    return {
      ...base,
      title: `Parcel P-${base.parcelId} not found`,
      summary: NYAYA_COPY[base.language].noEvidence,
      facts: ["Check the parcel ID or select a parcel from the DHARTI map."],
      citations: [],
      boundary: NYAYA_COPY[base.language].decisionBoundary,
      confidence: "Insufficient evidence",
      asOf: this.evidenceAsOf()
    };
  }
}

class NyayaBotController {
  constructor(engine) {
    this.engine = engine;
    this.panel = document.getElementById("nyayabot-panel");
    this.backdrop = document.getElementById("nyayabot-backdrop");
    this.messages = document.getElementById("nyayabot-messages");
    this.input = document.getElementById("nyayabot-input");
    this.form = document.getElementById("nyayabot-form");
    this.roleSelect = document.getElementById("user-role-select");
    this.languageSelect = document.getElementById("nyayabot-language-select");
    this.draftedKeys = new Set();
    this.bindEvents();
    this.renderWelcome();
    this.refreshContext();
  }

  bindEvents() {
    document.getElementById("nyayabot-toggle").addEventListener("click", () => this.toggle());
    document.getElementById("nyayabot-close").addEventListener("click", () => this.close());
    this.backdrop.addEventListener("click", () => this.close());
    this.form.addEventListener("submit", event => {
      event.preventDefault();
      this.ask(this.input.value);
    });
    document.querySelectorAll("[data-nyaya-query]").forEach(button => {
      button.addEventListener("click", () => this.ask(button.dataset.nyayaQuery));
    });
    this.roleSelect.addEventListener("change", () => {
      this.refreshContext();
      this.renderSystemMessage(`Access scope changed to ${this.currentPolicy().label}.`);
    });
    window.addEventListener("dharti:parcel-selected", () => this.refreshContext());
    document.addEventListener("keydown", event => {
      if (event.key === "Escape" && this.panel.classList.contains("open")) this.close();
    });
  }

  currentPolicy() {
    return NYAYA_ROLE_POLICIES[this.roleSelect.value] || NYAYA_ROLE_POLICIES.cala;
  }

  context() {
    return {
      role: this.roleSelect.value,
      language: this.languageSelect.value,
      parcelId: window.app && window.app.selectedParcelId ? window.app.selectedParcelId : 118
    };
  }

  refreshContext() {
    const ctx = this.context();
    document.getElementById("nyayabot-role-context").textContent = this.currentPolicy().label;
    document.getElementById("nyayabot-parcel-context").textContent = `Parcel P-${ctx.parcelId}`;
  }

  open() {
    this.panel.classList.add("open");
    this.backdrop.classList.add("open");
    this.panel.setAttribute("aria-hidden", "false");
    this.backdrop.setAttribute("aria-hidden", "false");
    document.getElementById("nyayabot-toggle").setAttribute("aria-expanded", "true");
    this.refreshContext();
    setTimeout(() => this.input.focus(), 120);
  }

  close() {
    this.panel.classList.remove("open");
    this.backdrop.classList.remove("open");
    this.panel.setAttribute("aria-hidden", "true");
    this.backdrop.setAttribute("aria-hidden", "true");
    document.getElementById("nyayabot-toggle").setAttribute("aria-expanded", "false");
  }

  toggle() {
    if (this.panel.classList.contains("open")) this.close();
    else this.open();
  }

  askPreset(question) {
    this.open();
    this.ask(question);
  }

  ask(question) {
    const query = String(question || "").trim();
    if (!query) return;
    this.open();
    this.renderUserMessage(query);
    this.input.value = "";

    const response = this.engine.ask(query, this.context());
    this.recordAnswerReceipt(response);
    this.renderResponse(response);
  }

  recordAnswerReceipt(response) {
    if (!window.dataStore || typeof window.dataStore.appendAudit !== "function") return;
    const sourceIds = (response.citations || []).map(source => source.id).join(",") || "NONE";
    window.dataStore.appendAudit(
      "NyayaBotAnswerReceiptCreated",
      `${response.receiptId}; role=${response.role}; intent=${response.intent}; sources=${sourceIds}; raw query not stored`
    );
  }

  draftAction(response) {
    if (!response.action) return;
    const key = `${response.receiptId}:${response.action.eventType}`;
    if (this.draftedKeys.has(key)) {
      window.app.showNotification("This NyayaBot draft already exists; duplicate write prevented.", "warning");
      return;
    }
    this.draftedKeys.add(key);
    window.dataStore.appendAudit(response.action.eventType, `${response.action.details} Receipt ${response.receiptId}`);
    this.renderSystemMessage("Draft created for maker-checker review. No payment, entitlement or parcel state was changed.");
    window.app.showNotification("NyayaBot created a review draft—human approval is required.", "success");
  }

  renderWelcome() {
    this.messages.innerHTML = "";
    const card = document.createElement("article");
    card.className = "nyayabot-message nyayabot-answer";
    card.innerHTML = `
      <span class="nyayabot-message-label">NYAYABOT</span>
      <h3>Ask DHARTI, not a generic chatbot</h3>
      <p>I use authorised DHARTI tools to answer from live parcel, payment, family, policy and audit evidence.</p>
      <div class="nyayabot-boundary">Anyone can ask. Every answer is restricted by role, purpose and case relationship.</div>
    `;
    this.messages.appendChild(card);
  }

  renderUserMessage(text) {
    const card = document.createElement("article");
    card.className = "nyayabot-message nyayabot-user";
    const label = document.createElement("span");
    label.className = "nyayabot-message-label";
    label.textContent = "YOU";
    const paragraph = document.createElement("p");
    paragraph.textContent = text;
    card.append(label, paragraph);
    this.messages.appendChild(card);
    this.scrollToBottom();
  }

  renderSystemMessage(text) {
    const card = document.createElement("div");
    card.className = "nyayabot-system-message";
    card.textContent = text;
    this.messages.appendChild(card);
    this.scrollToBottom();
  }

  renderResponse(response) {
    const card = document.createElement("article");
    card.className = "nyayabot-message nyayabot-answer";

    const citations = (response.citations || []).map(source => `
      <li><strong>${this.escape(source.id)}</strong> · ${this.escape(source.label)} <span>${this.escape(source.source)}</span></li>
    `).join("");
    const facts = (response.facts || []).map(fact => `<li>${this.escape(fact)}</li>`).join("");
    const action = response.action ? `<button type="button" class="nyayabot-draft-action">${this.escape(response.action.label)}</button>` : "";

    card.innerHTML = `
      <div class="nyayabot-answer-meta">
        <span class="nyayabot-message-label">NYAYABOT</span>
        <span class="nyayabot-confidence">${this.escape(response.confidence)}</span>
      </div>
      <h3>${this.escape(response.title)}</h3>
      <p>${this.escape(response.summary)}</p>
      ${facts ? `<ul class="nyayabot-facts">${facts}</ul>` : ""}
      ${citations ? `<details class="nyayabot-citations"><summary>Verified sources (${response.citations.length})</summary><ul>${citations}</ul></details>` : ""}
      <div class="nyayabot-boundary">${this.escape(response.boundary || "")}</div>
      <div class="nyayabot-receipt"><span>${this.escape(response.receiptId)}</span><span>Evidence as of ${this.escape(response.asOf)}</span></div>
      ${action}
    `;

    const actionButton = card.querySelector(".nyayabot-draft-action");
    if (actionButton) actionButton.addEventListener("click", () => this.draftAction(response));
    this.messages.appendChild(card);
    this.scrollToBottom();
  }

  escape(value) {
    return String(value == null ? "" : value)
      .replaceAll("&", "&amp;")
      .replaceAll("<", "&lt;")
      .replaceAll(">", "&gt;")
      .replaceAll('"', "&quot;")
      .replaceAll("'", "&#039;");
  }

  scrollToBottom() {
    this.messages.scrollTop = this.messages.scrollHeight;
  }
}

if (typeof module !== "undefined" && module.exports) {
  module.exports = { NyayaBotEngine, NYAYA_ROLE_POLICIES };
}

if (typeof window !== "undefined") {
  window.NyayaBotEngine = NyayaBotEngine;
  window.NYAYA_ROLE_POLICIES = NYAYA_ROLE_POLICIES;
  window.addEventListener("DOMContentLoaded", () => {
    const engine = new NyayaBotEngine(window.dataStore, window.PCIEngine);
    window.nyayaBot = new NyayaBotController(engine);
  });
}
