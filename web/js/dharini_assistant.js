/**
 * DHARTI - NICCI (NIC Chat Interface) & Dharini AI Assistant
 * Official National Informatics Centre & MoRTH AI Government Assistant.
 * 
 * Features:
 * - Authentic NICCI UI with "NEED HELP? ASK NICCI" pill callout
 * - Splash Screen with State Emblem of India and name onboarding
 * - Full Voice Narration (Text-To-Speech) with speaking sound-wave animations
 * - Speech Recognition (Microphone Speech-To-Text)
 * - Millisecond timestamps (HH:mm:ss:SSS) & official circular avatar
 * - Sub-toolbar (Speaker, Clear, Minimize, Maximize, External)
 * - Deep integration with DHARTI 5-corridor control plane, DGPS proofs, court stays, and PCI simulator
 */

class NicciAssistant {
  constructor() {
    this.isOpen = false;
    this.isMaximized = false;
    this.ttsEnabled = true; // Voice enabled by default as requested: "she is speaking as well"
    this.isSpeaking = false;
    this.isListening = false;
    this.userName = sessionStorage.getItem("dharti_nicci_username") || "";
    this.recognition = null;
    this.speechSynth = window.speechSynthesis || null;
    this.selectedVoice = null;
    
    this.initSpeechSynth();
    this.initSpeechRecognition();
    this.init();
  }

  initSpeechSynth() {
    if (!this.speechSynth) return;
    const loadVoices = () => {
      const voices = this.speechSynth.getVoices();
      // Try finding female Indian or English voices
      this.selectedVoice = voices.find(v => (v.name.includes("India") || v.lang === "en-IN") && (v.name.includes("Female") || v.name.includes("Google") || v.name.includes("Heera") || v.name.includes("Veena")))
        || voices.find(v => v.lang.startsWith("en") && (v.name.includes("Female") || v.name.includes("Samantha") || v.name.includes("Karen") || v.name.includes("Victoria") || v.name.includes("Zira")))
        || voices.find(v => v.lang.startsWith("en"))
        || voices[0];
    };
    loadVoices();
    if (this.speechSynth.onvoiceschanged !== undefined) {
      this.speechSynth.onvoiceschanged = loadVoices;
    }
  }

  initSpeechRecognition() {
    const SpeechRecognition = window.SpeechRecognition || window.webkitSpeechRecognition;
    if (SpeechRecognition) {
      try {
        this.recognition = new SpeechRecognition();
        this.recognition.continuous = false;
        this.recognition.interimResults = false;
        this.recognition.lang = "en-IN";

        this.recognition.onstart = () => {
          this.isListening = true;
          this.updateMicUi(true);
        };

        this.recognition.onresult = (event) => {
          const transcript = event.results[0][0].transcript;
          if (this.inputEl) {
            this.inputEl.value = transcript;
            this.handleSend();
          }
        };

        this.recognition.onerror = (e) => {
          console.warn("Speech recognition error:", e);
          this.isListening = false;
          this.updateMicUi(false);
        };

        this.recognition.onend = () => {
          this.isListening = false;
          this.updateMicUi(false);
        };
      } catch (err) {
        console.warn("Speech recognition init failed:", err);
      }
    }
  }

  updateMicUi(listening) {
    const micBtn = document.getElementById("nicci-btn-mic");
    if (!micBtn) return;
    if (listening) {
      micBtn.classList.add("mic-recording");
      micBtn.title = "Listening... Speak your query";
      micBtn.innerHTML = "🔴";
    } else {
      micBtn.classList.remove("mic-recording");
      micBtn.title = "Speak query using microphone";
      micBtn.innerHTML = "🎙️";
    }
  }

  init() {
    if (document.getElementById("nicci-ai-widget")) return;

    const widget = document.createElement("div");
    widget.id = "nicci-ai-widget";
    widget.className = "nicci-widget-container";
    widget.innerHTML = `
      <!-- Floating Callout Pill & Circular Avatar -->
      <div id="nicci-fab-wrapper" class="nicci-fab-wrapper">
        <div id="nicci-speech-callout" class="nicci-speech-callout" title="Ask NICCI - National Digital Assistant">
          <span>NEED HELP? ASK NICCI</span>
          <div class="nicci-callout-arrow"></div>
        </div>
        <div id="nicci-fab" class="nicci-fab" title="NIC Chat Interface (NICCI)">
          <div class="nicci-avatar-outer-ring">
            <img src="assets/nicci_avatar.jpg" alt="NICCI Digital Assistant" class="nicci-fab-img">
            <span class="nicci-fab-online-dot"></span>
          </div>
        </div>
      </div>

      <!-- Main Drawer Window -->
      <div id="nicci-drawer" class="nicci-drawer hidden">
        
        <!-- STATE 1: Splash Screen (Matching Official NICCI Welcome Card) -->
        <div id="nicci-splash-view" class="nicci-splash-view ${this.userName ? 'hidden' : ''}">
          <div class="nicci-splash-top-bar">
            <button id="nicci-splash-minimize" class="nicci-splash-btn" title="Minimize">⌵</button>
          </div>
          
          <div class="nicci-splash-content">
            <div class="nicci-emblem-wrap">
              <img src="assets/emblem_of_india.svg" alt="Emblem of India" class="nicci-white-emblem">
            </div>
            
            <h2 class="nicci-splash-title">NIC Chat Interface (NICCI)</h2>
            <p class="nicci-splash-subtitle">National Land Acquisition & Infrastructure Control Plane AI</p>

            <div class="nicci-splash-avatar-box">
              <div class="nicci-splash-avatar-glow"></div>
              <img src="assets/nicci_avatar.jpg" alt="NICCI" class="nicci-splash-avatar-img">
            </div>

            <div class="nicci-splash-form">
              <input type="text" id="nicci-user-name-input" class="nicci-splash-input" placeholder="Your name" value="${this.userName || ''}" autocomplete="off">
              <button id="nicci-btn-start" class="nicci-btn-start">Start</button>
            </div>
          </div>
        </div>

        <!-- STATE 2: Active Chat View (Matching NICCI Interface) -->
        <div id="nicci-chat-view" class="nicci-chat-view ${this.userName ? '' : 'hidden'}">
          <!-- Top Header -->
          <div class="nicci-header">
            <div class="nicci-header-left">
              <div class="nicci-header-avatar-wrap">
                <img src="assets/nicci_avatar.jpg" alt="NICCI" class="nicci-header-avatar-img" id="nicci-header-avatar">
                <span class="nicci-avatar-sound-waves" id="nicci-sound-waves"></span>
              </div>
              <div class="nicci-header-info">
                <div class="nicci-header-name">
                  <span>NICCI</span>
                  <span class="nicci-status-dot"></span>
                </div>
                <div class="nicci-header-role">Your Digital Assistant</div>
              </div>
            </div>
            <div class="nicci-header-right">
              <button id="nicci-btn-menu" class="nicci-hdr-icon-btn" title="Menu">☰</button>
            </div>
          </div>

          <!-- Secondary Toolbar -->
          <div class="nicci-sub-toolbar">
            <div class="nicci-toolbar-left">
              <button id="nicci-btn-expand" class="nicci-tool-btn" title="Toggle Fullscreen">⤢</button>
              <button id="nicci-btn-move" class="nicci-tool-btn" title="Drag & Move">✥</button>
            </div>
            <div class="nicci-toolbar-right">
              <button id="nicci-btn-voice" class="nicci-tool-btn active-voice" title="Toggle Voice Narration">
                <span id="nicci-voice-icon">🔊</span>
              </button>
              <button id="nicci-btn-clear" class="nicci-tool-btn" title="Clear Chat History">🗑️</button>
              <button id="nicci-btn-minimize" class="nicci-tool-btn" title="Minimize Window">🗕</button>
              <button id="nicci-btn-popout" class="nicci-tool-btn" title="Open Explanatory AI Tab">↗</button>
            </div>
          </div>

          <!-- Messages Area -->
          <div id="nicci-messages" class="nicci-messages-area">
            <!-- Messages inserted dynamically -->
          </div>

          <!-- Bottom Input Bar -->
          <div class="nicci-input-bar">
            <input type="text" id="nicci-user-input" class="nicci-input" placeholder="Ask NICCI about land bottlenecks, stays, geotags..." autocomplete="off">
            <button id="nicci-btn-mic" class="nicci-btn-tool-mic" title="Speak query with microphone">🎙️</button>
            <button id="nicci-btn-send" class="nicci-btn-send" title="Send Query">
              <svg width="18" height="18" viewBox="0 0 24 24" fill="currentColor">
                <path d="M2.01 21L23 12 2.01 3 2 10l15 2-15 2z"/>
              </svg>
            </button>
          </div>
        </div>

      </div>
    `;

    document.body.appendChild(widget);
    this.containerEl = widget;
    this.messagesEl = document.getElementById("nicci-messages");
    this.inputEl = document.getElementById("nicci-user-input");

    this.bindEvents();

    // If user already logged in previously, populate welcome message
    if (this.userName) {
      this.renderInitialGreeting();
    }
  }

  bindEvents() {
    // Open/Close via FAB or Callout
    const fab = document.getElementById("nicci-fab");
    const callout = document.getElementById("nicci-speech-callout");
    if (fab) fab.addEventListener("click", () => this.toggleDrawer());
    if (callout) callout.addEventListener("click", () => this.toggleDrawer());

    // Splash Start Button
    const startBtn = document.getElementById("nicci-btn-start");
    const nameInput = document.getElementById("nicci-user-name-input");
    if (startBtn) {
      startBtn.addEventListener("click", () => this.handleStart());
    }
    if (nameInput) {
      nameInput.addEventListener("keydown", (e) => {
        if (e.key === "Enter") this.handleStart();
      });
    }

    // Splash Minimize
    const splashMin = document.getElementById("nicci-splash-minimize");
    if (splashMin) splashMin.addEventListener("click", () => this.closeDrawer());

    // Sub-toolbar buttons
    const minBtn = document.getElementById("nicci-btn-minimize");
    if (minBtn) minBtn.addEventListener("click", () => this.closeDrawer());

    const voiceBtn = document.getElementById("nicci-btn-voice");
    if (voiceBtn) {
      voiceBtn.addEventListener("click", () => this.toggleVoice());
    }

    const clearBtn = document.getElementById("nicci-btn-clear");
    if (clearBtn) {
      clearBtn.addEventListener("click", () => this.clearChat());
    }

    const expandBtn = document.getElementById("nicci-btn-expand");
    if (expandBtn) {
      expandBtn.addEventListener("click", () => this.toggleMaximize());
    }

    const popoutBtn = document.getElementById("nicci-btn-popout");
    if (popoutBtn) {
      popoutBtn.addEventListener("click", () => {
        const explainTab = document.querySelector('.nav-tab-btn[data-tab="explanatory-ai"]');
        if (explainTab) explainTab.click();
        this.closeDrawer();
      });
    }

    // Input & Send
    const sendBtn = document.getElementById("nicci-btn-send");
    if (sendBtn) sendBtn.addEventListener("click", () => this.handleSend());
    if (this.inputEl) {
      this.inputEl.addEventListener("keydown", (e) => {
        if (e.key === "Enter") this.handleSend();
      });
    }

    // Microphone STT
    const micBtn = document.getElementById("nicci-btn-mic");
    if (micBtn) {
      micBtn.addEventListener("click", () => this.toggleMicrophone());
    }

    // Interactive Delegated Clicks (chips & buttons inside bubbles)
    if (this.messagesEl) {
      this.messagesEl.addEventListener("click", (e) => {
        const chip = e.target.closest(".nicci-chip");
        if (chip) {
          const query = chip.getAttribute("data-query");
          if (query && this.inputEl) {
            this.inputEl.value = query;
            this.handleSend();
          }
        }

        const actionBtn = e.target.closest(".nicci-action-btn");
        if (actionBtn) {
          const action = actionBtn.getAttribute("data-action");
          this.executeAction(action);
        }
      });
    }
  }

  handleStart() {
    const input = document.getElementById("nicci-user-name-input");
    const val = (input ? input.value : "").trim() || "Guest Officer";
    this.userName = val;
    sessionStorage.setItem("dharti_nicci_username", val);

    document.getElementById("nicci-splash-view").classList.add("hidden");
    document.getElementById("nicci-chat-view").classList.remove("hidden");

    this.renderInitialGreeting();
  }

  renderInitialGreeting() {
    if (!this.messagesEl || this.messagesEl.children.length > 0) return;

    const time1 = this.getCurrentTimestamp();
    const time2 = this.getCurrentTimestamp(150);
    const time3 = this.getCurrentTimestamp(300);

    const greetingHtml = `
      <div class="nicci-msg-row nicci-bot-row">
        <div class="nicci-msg-bubble">
          <div class="nicci-msg-text">Namaskaar ${this.escapeHtml(this.userName)}</div>
          <div class="nicci-msg-time">${time1}</div>
        </div>
        <img src="assets/nicci_avatar.jpg" alt="NICCI" class="nicci-bubble-avatar">
      </div>

      <div class="nicci-msg-row nicci-bot-row">
        <div class="nicci-msg-bubble">
          <div class="nicci-msg-text">This is <strong>NICCI</strong>, your digital assistant for any query related to Open Government Data and the DHARTI National Land Acquisition Control Plane.</div>
          <div class="nicci-msg-time">${time2}</div>
        </div>
        <img src="assets/nicci_avatar.jpg" alt="NICCI" class="nicci-bubble-avatar">
      </div>

      <div class="nicci-msg-row nicci-bot-row">
        <div class="nicci-msg-bubble">
          <div class="nicci-msg-text">What can I do for you today?</div>
          <div class="nicci-chips-cluster">
            <span class="nicci-chip" data-query="Why is Bengaluru-Chennai corridor blocked?">🎯 Why is NE-7 blocked?</span>
            <span class="nicci-chip" data-query="Show me geotagged proof with GPS coordinates">📍 Geotagged Proof & GPS</span>
            <span class="nicci-chip" data-query="What is the legal court stay status?">⚖️ Court Stay Orders</span>
            <span class="nicci-chip" data-query="Audit SIA vulnerable families and census">👥 SIA Vulnerable Families</span>
            <span class="nicci-chip" data-query="Search corridors in Gujarat and Punjab">🔍 Search Corridors (NE-4, NE-5)</span>
          </div>
          <div class="nicci-msg-time">${time3}</div>
        </div>
        <img src="assets/nicci_avatar.jpg" alt="NICCI" class="nicci-bubble-avatar">
      </div>
    `;

    this.messagesEl.innerHTML = greetingHtml;
    this.scrollToBottom();

    // Speak initial greeting aloud if voice enabled!
    if (this.ttsEnabled) {
      this.speak(`Namaskaar ${this.userName}! This is NICCI, your digital assistant for the DHARTI Land Acquisition Control Plane. How can I help you?`);
    }
  }

  toggleDrawer() {
    const drawer = document.getElementById("nicci-drawer");
    if (!drawer) return;
    this.isOpen = !this.isOpen;
    if (this.isOpen) {
      drawer.classList.remove("hidden");
      if (this.inputEl && !this.userName) {
        const nameInp = document.getElementById("nicci-user-name-input");
        if (nameInp) nameInp.focus();
      } else if (this.inputEl) {
        this.inputEl.focus();
      }
    } else {
      drawer.classList.add("hidden");
      if (this.speechSynth) this.speechSynth.cancel();
      this.setSpeakingState(false);
    }
  }

  closeDrawer() {
    const drawer = document.getElementById("nicci-drawer");
    if (drawer) {
      drawer.classList.add("hidden");
      this.isOpen = false;
      if (this.speechSynth) this.speechSynth.cancel();
      this.setSpeakingState(false);
    }
  }

  toggleMaximize() {
    const drawer = document.getElementById("nicci-drawer");
    if (!drawer) return;
    this.isMaximized = !this.isMaximized;
    if (this.isMaximized) {
      drawer.classList.add("maximized");
    } else {
      drawer.classList.remove("maximized");
    }
  }

  toggleVoice() {
    this.ttsEnabled = !this.ttsEnabled;
    const voiceBtn = document.getElementById("nicci-btn-voice");
    const icon = document.getElementById("nicci-voice-icon");
    if (this.ttsEnabled) {
      if (voiceBtn) voiceBtn.classList.add("active-voice");
      if (icon) icon.textContent = "🔊";
      this.speak("Voice narration activated. I will speak my answers aloud.");
    } else {
      if (voiceBtn) voiceBtn.classList.remove("active-voice");
      if (icon) icon.textContent = "🔇";
      if (this.speechSynth) this.speechSynth.cancel();
      this.setSpeakingState(false);
    }
  }

  toggleMicrophone() {
    if (!this.recognition) {
      alert("Speech recognition is not supported in this browser environment. Please type your query.");
      return;
    }
    if (this.isListening) {
      this.recognition.stop();
      this.isListening = false;
      this.updateMicUi(false);
    } else {
      try {
        this.recognition.start();
      } catch (err) {
        console.warn(err);
      }
    }
  }

  clearChat() {
    if (this.messagesEl) {
      this.messagesEl.innerHTML = "";
      this.renderInitialGreeting();
    }
  }

  handleSend() {
    const text = (this.inputEl ? this.inputEl.value : "").trim();
    if (!text) return;

    this.appendUserMessage(text);
    this.inputEl.value = "";

    // Show typing dots animation matching Image 3
    const typingId = this.showTypingIndicator();

    setTimeout(() => {
      this.removeTypingIndicator(typingId);
      const currentProj = window.projectTracker ? window.projectTracker.currentProject : null;
      const explanation = window.explanatoryEngine ? window.explanatoryEngine.explain(text, currentProj) : null;
      this.appendBotResponse(explanation, text, currentProj);
    }, 600);
  }

  appendUserMessage(text) {
    const row = document.createElement("div");
    row.className = "nicci-msg-row nicci-user-row";
    const time = this.getCurrentTimestamp();
    row.innerHTML = `
      <div class="nicci-msg-bubble user-bubble">
        <div class="nicci-msg-text">${this.escapeHtml(text)}</div>
        <div class="nicci-msg-time">${time}</div>
      </div>
    `;
    this.messagesEl.appendChild(row);
    this.scrollToBottom();
  }

  showTypingIndicator() {
    const id = "typing-" + Date.now();
    const row = document.createElement("div");
    row.id = id;
    row.className = "nicci-msg-row nicci-bot-row nicci-typing-row";
    row.innerHTML = `
      <div class="nicci-msg-bubble typing-bubble">
        <div class="nicci-typing-dots">
          <span></span><span></span><span></span>
        </div>
      </div>
      <img src="assets/nicci_avatar.jpg" alt="NICCI" class="nicci-bubble-avatar">
    `;
    this.messagesEl.appendChild(row);
    this.scrollToBottom();
    return id;
  }

  removeTypingIndicator(id) {
    const el = document.getElementById(id);
    if (el) el.remove();
  }

  appendBotResponse(resp, queryText, currentProj) {
    if (!resp) return;

    const row = document.createElement("div");
    row.className = "nicci-msg-row nicci-bot-row";
    const time = this.getCurrentTimestamp();

    let actionsHtml = "";
    if (resp.category === "BOTTLENECK_OPTIMIZATION" || resp.action_type === "TRIGGER_UNLOCK_SIMULATION") {
      actionsHtml = `
        <div class="nicci-action-cluster">
          <button class="nicci-action-btn" data-action="SIMULATE_UNLOCK">
            ⚡ Run Continuous Frontage Simulator
          </button>
          <button class="nicci-action-btn btn-sec" data-action="OPEN_PROOF_P118">
            📍 View P-118 Geotagged Proof
          </button>
        </div>
      `;
    } else if (resp.category === "GEOTAGGED_DOCUMENT_PROOF" || resp.action_type === "OPEN_DOCUMENT_PROOF") {
      actionsHtml = `
        <div class="nicci-action-cluster">
          <button class="nicci-action-btn" data-action="OPEN_CURRENT_PROOF">
            📜 Open Official Geotagged Proof & Seal
          </button>
          <button class="nicci-action-btn btn-sec" data-action="SWITCH_GIS_MAP">
            🗺️ Open GIS Corridor Map
          </button>
        </div>
      `;
    } else if (resp.category === "LEGAL_LITIGATION") {
      actionsHtml = `
        <div class="nicci-action-cluster">
          <button class="nicci-action-btn" data-action="OPEN_CURRENT_PROOF">
            ⚖️ View High Court WP-4021/2023 Stay
          </button>
        </div>
      `;
    } else if (resp.category === "SIA_SOCIAL_SAFEGUARD") {
      actionsHtml = `
        <div class="nicci-action-cluster">
          <button class="nicci-action-btn" data-action="SWITCH_PROJECT_TRACKER">
            👥 View SIA Ingestion Dossier
          </button>
        </div>
      `;
    } else if (resp.category === "FEDERATED_SCRAPERS" || resp.action_type === "TRIGGER_SCRAPER_HUB") {
      actionsHtml = `
        <div class="nicci-action-cluster">
          <button class="nicci-action-btn" data-action="RUN_CONCURRENT_SCRAPE">
            ⚡ Run Concurrent 15-Portal Scrape
          </button>
          <button class="nicci-action-btn btn-sec" data-action="SCROLL_SCRAPER_HUB">
            🏛️ View Federated Scraper Hub
          </button>
        </div>
      `;
    }

    let metricsHtml = "";
    if (resp.metrics) {
      metricsHtml = `<div class="nicci-metrics-row">`;
      for (const [k, v] of Object.entries(resp.metrics)) {
        metricsHtml += `<span class="nicci-metric-badge"><strong>${this.formatKey(k)}:</strong> ${v}</span>`;
      }
      metricsHtml += `</div>`;
    }

    row.innerHTML = `
      <div class="nicci-msg-bubble">
        <div class="nicci-bubble-header">
          <span class="nicci-cat-tag">${resp.category || "GOVERNMENT_VERDICT"}</span>
          <span class="nicci-verified-tag">✓ Tamper-Proof</span>
        </div>
        <div class="nicci-msg-title">${resp.title || "Statutory Assessment"}</div>
        <div class="nicci-msg-text">${resp.direct_answer}</div>
        ${metricsHtml}
        <div class="nicci-statutory-clause">
          <span>⚖️ <strong>Statutory Authority:</strong> ${resp.statutory_authority || "RFCTLARR Act 2013 & NH Act 1956"}</span>
        </div>
        ${actionsHtml}
        <div class="nicci-msg-time">${time}</div>
      </div>
      <img src="assets/nicci_avatar.jpg" alt="NICCI" class="nicci-bubble-avatar">
    `;

    // Attach click listeners to action buttons
    row.querySelectorAll(".nicci-action-btn").forEach(btn => {
      btn.addEventListener("click", () => {
        const action = btn.getAttribute("data-action");
        this.executeAction(action);
      });
    });

    this.messagesEl.appendChild(row);
    this.scrollToBottom();

    // Voice narration if enabled
    if (this.ttsEnabled) {
      this.speak(resp.direct_answer);
    }
  }

  executeAction(action) {
    if (!action) return;

    if (action === "SIMULATE_UNLOCK") {
      const pciTab = document.querySelector('.nav-tab-btn[data-tab="pci"]');
      if (pciTab) pciTab.click();
    } else if (action === "SWITCH_GIS_MAP") {
      const mapTab = document.querySelector('.nav-tab-btn[data-tab="cockpit"]');
      if (mapTab) mapTab.click();
    } else if (action === "SWITCH_PROJECT_TRACKER") {
      const trackerTab = document.querySelector('.nav-tab-btn[data-tab="project-tracker"]');
      if (trackerTab) trackerTab.click();
    } else if (action === "RUN_CONCURRENT_SCRAPE") {
      const trackerTab = document.querySelector('.nav-tab-btn[data-tab="project-tracker"]');
      if (trackerTab) trackerTab.click();
      if (window.projectTracker) window.projectTracker.triggerConcurrentScrape();
    } else if (action === "SCROLL_SCRAPER_HUB") {
      const trackerTab = document.querySelector('.nav-tab-btn[data-tab="project-tracker"]');
      if (trackerTab) trackerTab.click();
      const hub = document.querySelector(".scraper-hub-card");
      if (hub) hub.scrollIntoView({ behavior: 'smooth' });
    } else if (action === "OPEN_CURRENT_PROOF" || action === "OPEN_PROOF_P118") {
      if (window.projectTracker && window.projectTracker.currentProject && window.documentProofModal) {
        const p = window.projectTracker.currentProject;
        const proof = (p.document_proofs && p.document_proofs.forest_clearance) || {
          document_title: "Statutory Land Acquisition Clearance & Sanction Order",
          document_type: "STAGE1_FOREST_CLEARANCE",
          issuing_authority: "Ministry of Road Transport and Highways (MoRTH)",
          official_letter_no: p.proposal_no || "F.No. 4-KAB819/2026-RO",
          issuance_date: "2026-08-14",
          location: p.geolocation,
          rfc6234_sha256: p.fc_sha256,
          conditions: [
            "Conforms strictly to Section 38 RFCTLARR Act 2013 prior to physical possession.",
            "Trimble R12 DGPS boundary stones embedded on ground."
          ]
        };
        window.documentProofModal.open(proof, p);
      }
    }
  }

  speak(text) {
    if (!this.speechSynth) return;
    this.speechSynth.cancel();

    // Clean markdown/symbols from speech text
    const cleanText = text
      .replace(/[*_#`[\]()]/g, "")
      .replace(/https?:\/\/\S+/g, "")
      .replace(/₹/g, "rupees")
      .replace(/Ha\b/g, "hectares")
      .replace(/km\b/g, "kilometers");

    const utterance = new SpeechSynthesisUtterance(cleanText);
    utterance.rate = 1.0;
    utterance.pitch = 1.05; // Articulate, warm tone
    if (this.selectedVoice) {
      utterance.voice = this.selectedVoice;
    }

    utterance.onstart = () => {
      this.setSpeakingState(true);
    };

    utterance.onend = () => {
      this.setSpeakingState(false);
    };

    utterance.onerror = () => {
      this.setSpeakingState(false);
    };

    this.speechSynth.speak(utterance);
  }

  setSpeakingState(speaking) {
    this.isSpeaking = speaking;
    const waves = document.getElementById("nicci-sound-waves");
    const avatar = document.getElementById("nicci-header-avatar");
    const fabWrapper = document.getElementById("nicci-fab-wrapper");

    if (speaking) {
      if (waves) waves.classList.add("speaking-active");
      if (avatar) avatar.classList.add("avatar-speaking-pulse");
      if (fabWrapper) fabWrapper.classList.add("fab-speaking-pulse");
    } else {
      if (waves) waves.classList.remove("speaking-active");
      if (avatar) avatar.classList.remove("avatar-speaking-pulse");
      if (fabWrapper) fabWrapper.classList.remove("fab-speaking-pulse");
    }
  }

  getCurrentTimestamp(offsetMs = 0) {
    const d = new Date(Date.now() + offsetMs);
    const h = String(d.getHours()).padStart(2, "0");
    const m = String(d.getMinutes()).padStart(2, "0");
    const s = String(d.getSeconds()).padStart(2, "0");
    const ms = String(d.getMilliseconds()).padStart(3, "0");
    return `${h}:${m}:${s}:${ms}`;
  }

  scrollToBottom() {
    if (this.messagesEl) {
      this.messagesEl.scrollTop = this.messagesEl.scrollHeight;
    }
  }

  formatKey(key) {
    return key.replace(/_/g, " ").replace(/\b\w/g, l => l.toUpperCase());
  }

  escapeHtml(str) {
    return (str || "").replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
  }
}

// Global instance (supports both dhariniAssistant and nicciAssistant aliases)
window.nicciAssistant = new NicciAssistant();
window.dhariniAssistant = window.nicciAssistant;
