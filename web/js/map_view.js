/**
 * DHARTI - Leaflet GIS Map Visualization Service
 * Parses WKT geometries, renders parcel boundaries, and binds interactive inspectors.
 */

class MapView {
  constructor(mapContainerId) {
    this.mapContainerId = mapContainerId;
    this.map = null;
    this.parcelLayers = {};
    this.alignmentLine = null;
  }

  init() {
    if (this.map) return;

    // Centered around Devanahalli, Bengaluru Rural corridor (13.245° N, 77.735° E)
    this.map = L.map(this.mapContainerId, {
      center: [13.245, 77.737],
      zoom: 13,
      zoomControl: true
    });

    // CartoDB Positron - Pristine Light Theme Map Tiles
    L.tileLayer('https://{s}.basemaps.cartocdn.com/light_all/{z}/{x}/{y}{r}.png', {
      attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors &copy; <a href="https://carto.com/attributions">CARTO</a>',
      subdomains: 'abcd',
      maxZoom: 19
    }).addTo(this.map);

    this.renderParcels();
  }

  parseWKT(wktString) {
    // Basic parser for POLYGON((lon lat, ...))
    const coordsMatch = wktString.match(/\(\((.*?)\)\)/);
    if (!coordsMatch || !coordsMatch[1]) return [];

    const coordPairs = coordsMatch[1].split(",");
    const latLngs = [];

    for (const pair of coordPairs) {
      const parts = pair.trim().split(/\s+/);
      if (parts.length >= 2) {
        const lon = parseFloat(parts[0]);
        const lat = parseFloat(parts[1]);
        if (!isNaN(lat) && !isNaN(lon)) {
          latLngs.push([lat, lon]);
        }
      }
    }
    return latLngs;
  }

  getStyleForState(state, isStay) {
    if (isStay || state === "InjunctionImposed") {
      return {
        color: "#dc2626",
        fillColor: "#ef4444",
        weight: 3,
        fillOpacity: 0.55,
        dashArray: "6, 4"
      };
    }
    switch (state) {
      case "ConstructionReady":
        return {
          color: "#059669",
          fillColor: "#10b981",
          weight: 2,
          fillOpacity: 0.55
        };
      case "PossessionConfirmed":
        return {
          color: "#2563eb",
          fillColor: "#3b82f6",
          weight: 2,
          fillOpacity: 0.50
        };
      case "AwardDeclared":
        return {
          color: "#d97706",
          fillColor: "#f59e0b",
          weight: 2,
          fillOpacity: 0.45
        };
      default:
        return {
          color: "#475569",
          fillColor: "#94a3b8",
          weight: 2,
          fillOpacity: 0.40
        };
    }
  }

  renderParcels() {
    if (!this.map) return;

    // Clear existing
    Object.values(this.parcelLayers).forEach(layer => this.map.removeLayer(layer));
    this.parcelLayers = {};
    if (this.alignmentLine) this.map.removeLayer(this.alignmentLine);

    const parcels = window.dataStore.parcels;
    const bounds = L.latLngBounds([]);
    const alignmentPoints = [];

    parcels.forEach(p => {
      const latLngs = this.parseWKT(p.boundary_wkt);
      if (latLngs.length > 0) {
        latLngs.forEach(pt => bounds.extend(pt));
        alignmentPoints.push(latLngs[0]);

        const style = this.getStyleForState(p.state, p.court_stay);
        const polygon = L.polygon(latLngs, style).addTo(this.map);

        // Tooltip & Popup
        polygon.bindTooltip(`<b>Parcel ${p.parcel_id} (Sy ${p.survey_number})</b><br>${p.state}`, {
          sticky: true,
          className: "custom-map-tooltip"
        });

        polygon.bindPopup(`
          <div class="map-popup-card">
            <div class="popup-header">
              <span class="popup-title">Parcel ID: P-${p.parcel_id}</span>
              <span class="popup-badge badge-${p.state}">${p.state}</span>
            </div>
            <div class="popup-body">
              <p><b>ULPIN:</b> <code>${p.ulpin}</code></p>
              <p><b>Survey No:</b> ${p.survey_number}/${p.sub_division} (${p.village})</p>
              <p><b>Owner:</b> ${p.owner_name}</p>
              <p><b>Chainage:</b> KM ${p.chainage_start_km} &rarr; KM ${p.chainage_end_km} (${(p.chainage_end_km - p.chainage_start_km).toFixed(1)} km)</p>
              <p><b>Polygon Area:</b> ${p.polygon_area_sqm.toLocaleString()} sqm (RoR: ${p.ror_area_sqm.toLocaleString()} sqm)</p>
              <p><b>Disbursement:</b> ₹${(p.disbursed_amount_inr / 1e7).toFixed(2)} Cr / ₹${(p.sanctioned_amount_inr / 1e7).toFixed(2)} Cr</p>
              ${p.bank_utr ? `<p><b>Bank UTR:</b> <code>${p.bank_utr}</code></p>` : `<p><b>Bank UTR:</b> <span class="text-danger">None / Unreconciled</span></p>`}
              ${p.court_stay ? `<div class="popup-alert-danger"><b>High Court Stay Active:</b> ${p.stay_details}</div>` : ""}
            </div>
            <button class="popup-action-btn" onclick="window.app.selectParcel(${p.parcel_id})">Inspect in Sandbox</button>
          </div>
        `);

        polygon.on('click', () => {
          window.app.selectParcel(p.parcel_id);
        });

        this.parcelLayers[p.parcel_id] = polygon;
      }
    });

    if (alignmentPoints.length > 1) {
      this.alignmentLine = L.polyline(alignmentPoints, {
        color: "#4f46e5",
        weight: 4,
        dashArray: "8, 6",
        opacity: 0.8
      }).addTo(this.map);
    }

    if (bounds.isValid()) {
      this.map.fitBounds(bounds, { padding: [30, 30] });
    }
  }

  highlightParcel(parcelId) {
    Object.keys(this.parcelLayers).forEach(id => {
      const layer = this.parcelLayers[id];
      const p = window.dataStore.parcels.find(x => x.parcel_id === parseInt(id));
      if (parseInt(id) === parcelId) {
        layer.setStyle({
          weight: 4,
          color: "#4338ca",
          fillOpacity: 0.8
        });
        layer.openPopup();
      } else if (p) {
        layer.setStyle(this.getStyleForState(p.state, p.court_stay));
      }
    });
  }
}

window.MapView = MapView;
