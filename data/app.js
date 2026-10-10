// ESP32 Clock Web Control Panel & Screen Simulator

const PRESETS = {
  cyberpunk: {
    accent: '#00FFFF',
    bg: '#081018',
    cardBg: '#101C28',
    cardBorder: '#203850',
    text: '#FFFFFF',
    muted: '#8CA0B8'
  },
  nord: {
    accent: '#88C0D0',
    bg: '#242933',
    cardBg: '#2E3440',
    cardBorder: '#434C5E',
    text: '#ECEFF4',
    muted: '#D8DEE9'
  },
  solar: {
    accent: '#FFB300',
    bg: '#140D02',
    cardBg: '#211503',
    cardBorder: '#3D2808',
    text: '#FFE082',
    muted: '#FFA000'
  },
  emerald: {
    accent: '#00FF66',
    bg: '#021408',
    cardBg: '#052610',
    cardBorder: '#0D4720',
    text: '#E8F5E9',
    muted: '#81C784'
  },
  oled: {
    accent: '#FFFFFF',
    bg: '#000000',
    cardBg: '#111111',
    cardBorder: '#2A2A2A',
    text: '#FFFFFF',
    muted: '#888888'
  },
  violet: {
    accent: '#C084FC',
    bg: '#0F0B1E',
    cardBg: '#1A1435',
    cardBorder: '#2E225A',
    text: '#FAF5FF',
    muted: '#A855F7'
  }
};

let state = {
  config: null,
  status: null,
  previewNightMode: false
};

// DOM Elements
const canvas = document.getElementById('displayPreview');
const ctx = canvas.getContext('2d');
const statusDot = document.getElementById('statusDot');
const statusText = document.getElementById('statusText');
const deviceIp = document.getElementById('deviceIp');
const toastEl = document.getElementById('toast');

// Initialize
document.addEventListener('DOMContentLoaded', () => {
  setupTabs();
  setupEventListeners();
  loadConfig();
  pollStatus();
  setInterval(pollStatus, 3000);
  setInterval(renderPreview, 1000); // 1-second clock tick in preview
});

// Tab Navigation
function setupTabs() {
  const tabs = document.querySelectorAll('.tab-btn');
  tabs.forEach(tab => {
    tab.addEventListener('click', () => {
      tabs.forEach(t => t.classList.remove('active'));
      document.querySelectorAll('.tab-pane').forEach(p => p.classList.remove('active'));
      tab.classList.add('active');
      const targetPane = document.getElementById(`pane${tab.dataset.tab.charAt(0).toUpperCase() + tab.dataset.tab.slice(1)}`);
      if (targetPane) targetPane.classList.add('active');
    });
  });
}

function showToast(message, duration = 3000) {
  toastEl.textContent = message;
  toastEl.classList.add('show');
  setTimeout(() => toastEl.classList.remove('show'), duration);
}

// Fetch Configuration from ESP32
async function loadConfig() {
  try {
    const res = await fetch('/api/config');
    if (!res.ok) throw new Error('Network error');
    state.config = await res.json();
    populateUI();
    renderPreview();
  } catch (err) {
    console.warn('Using default fallback config (offline mode):', err);
    state.config = {
      theme: {
        preset: 'cyberpunk',
        accent_color: '#00FFFF',
        bg_color: '#081018',
        card_bg_color: '#101C28',
        card_border_color: '#203850',
        time_color: '#00FFFF',
        date_color: '#FFFFFF',
        muted_text_color: '#8CA0B8',
        day_duty: 255,
        night_duty: 38,
        night_start_hour: 22,
        night_start_min: 0,
        night_end_hour: 7,
        night_end_min: 0
      },
      cards: [
        {
          id: 'clock',
          title: 'Clock & Date',
          description: 'Anti-aliased digital time and calendar date',
          icon: 'clock',
          enabled: true,
          order: 0,
          config: { format_12h: true, show_seconds: true, date_format: 'MDY' }
        },
        {
          id: 'weather',
          title: 'Current Weather',
          description: 'Live temperature, sky conditions, and humidity',
          icon: 'cloud-sun',
          enabled: true,
          order: 1,
          config: { city_name: 'San Francisco', lat: 37.7749, lon: -122.4194, use_fahrenheit: true, update_interval_min: 15 }
        },
        {
          id: 'forecast',
          title: '2-Day Forecast',
          description: 'Upcoming forecast and high/low temperatures',
          icon: 'calendar-days',
          enabled: true,
          order: 2,
          config: { show_icons: true }
        },
        {
          id: 'calendar',
          title: 'Calendar & Events',
          description: 'Upcoming calendar events and agenda timeline',
          icon: 'calendar',
          enabled: false,
          order: 3,
          config: { max_events: 3, show_countdown: true, calendar_url: '' }
        },
        {
          id: 'notifications',
          title: 'Email & Notifications',
          description: 'Unread email badges and messaging alert banners',
          icon: 'bell',
          enabled: false,
          order: 4,
          config: { show_badges: true, timeout_sec: 30 }
        },
        {
          id: 'system',
          title: 'System & Network',
          description: 'WiFi RSSI, IP address, uptime, and memory diagnostics',
          icon: 'cpu',
          enabled: true,
          order: 5,
          config: { show_wifi_rssi: true, show_uptime: true, show_free_heap: true }
        }
      ],
      system: {
        mdns_hostname: 'esp32-clock',
        timezone: 'PST8PDT,M3.2.0,M11.1.0',
        ntp_server: 'pool.ntp.org'
      }
    };
    populateUI();
    renderPreview();
  }
}

// Fetch Hardware Telemetry
async function pollStatus() {
  try {
    const res = await fetch('/api/status');
    if (!res.ok) throw new Error();
    const data = await res.json();
    state.status = data;

    statusDot.className = 'status-dot online';
    statusText.textContent = data.wifi.connected ? 'Connected' : 'WiFi Disconnected';
    deviceIp.textContent = data.wifi.ip || '192.168.x.x';

    // Diagnostics pane
    document.getElementById('diagFreeHeap').textContent = `${(data.free_heap / 1024).toFixed(1)} KB`;
    document.getElementById('diagTotalHeap').textContent = `${(data.heap_size / 1024).toFixed(1)} KB`;
    document.getElementById('diagRssi').textContent = `${data.wifi.rssi} dBm`;
    
    const uptimeSec = Math.floor(data.uptime_ms / 1000);
    const hrs = Math.floor(uptimeSec / 3600);
    const mins = Math.floor((uptimeSec % 3600) / 60);
    const secs = uptimeSec % 60;
    document.getElementById('diagUptime').textContent = `${hrs}h ${mins}m ${secs}s`;
  } catch (e) {
    statusDot.className = 'status-dot';
    statusText.textContent = 'Device Offline';
  }
}

// Populate UI Form Fields
function populateUI() {
  if (!state.config) return;
  const cfg = state.config;

  // Theme Fields
  syncColorInput('colorAccent', cfg.theme.accent_color);
  syncColorInput('colorBg', cfg.theme.bg_color);
  syncColorInput('colorCardBg', cfg.theme.card_bg_color);
  syncColorInput('colorBorder', cfg.theme.card_border_color);
  syncColorInput('colorText', cfg.theme.date_color);
  syncColorInput('colorMuted', cfg.theme.muted_text_color);

  // Active preset pill
  document.querySelectorAll('.preset-pill').forEach(btn => {
    btn.classList.toggle('active', btn.dataset.preset === cfg.theme.preset);
  });

  // Schedule Fields
  const pad = n => String(n).padStart(2, '0');
  document.getElementById('nightStart').value = `${pad(cfg.theme.night_start_hour)}:${pad(cfg.theme.night_start_min)}`;
  document.getElementById('nightEnd').value = `${pad(cfg.theme.night_end_hour)}:${pad(cfg.theme.night_end_min)}`;

  document.getElementById('dayBrightness').value = cfg.theme.day_duty;
  document.getElementById('dayBrightVal').textContent = Math.round((cfg.theme.day_duty / 255) * 100);

  document.getElementById('nightBrightness').value = cfg.theme.night_duty;
  document.getElementById('nightBrightVal').textContent = Math.round((cfg.theme.night_duty / 255) * 100);

  // System Fields
  document.getElementById('mdnsHostname').value = cfg.system.mdns_hostname || 'esp32-clock';
  document.getElementById('ntpServer').value = cfg.system.ntp_server || 'pool.ntp.org';
  document.getElementById('timezone').value = cfg.system.timezone || 'PST8PDT,M3.2.0,M11.1.0';

  // Render Modular Cards List
  renderCardsList();
}

function syncColorInput(id, val) {
  const picker = document.getElementById(id);
  const hex = document.getElementById(`${id}Hex`);
  if (picker && hex && val) {
    picker.value = val;
    hex.value = val.toUpperCase();
  }
}

// Dynamic Modular Card Rendering
function renderCardsList() {
  const container = document.getElementById('cardsList');
  if (!container || !state.config.cards) return;

  container.innerHTML = '';
  // Sort cards by order
  state.config.cards.sort((a, b) => a.order - b.order);

  state.config.cards.forEach((card, index) => {
    const item = document.createElement('div');
    item.className = 'card-item';
    item.dataset.id = card.id;

    item.innerHTML = `
      <div class="card-header">
        <div class="card-header-left">
          <div class="card-reorder">
            <button class="btn-arrow btn-up" title="Move Up" ${index === 0 ? 'disabled' : ''}>▲</button>
            <button class="btn-arrow btn-down" title="Move Down" ${index === state.config.cards.length - 1 ? 'disabled' : ''}>▼</button>
          </div>
          <div class="card-icon">
            ${getCardIconSvg(card.id)}
          </div>
          <div class="card-title-text">
            <h4>${card.title}</h4>
            <p>${card.description}</p>
          </div>
        </div>
        <div class="card-header-right">
          <label class="switch" title="Enable or hide card">
            <input type="checkbox" class="card-toggle" ${card.enabled ? 'checked' : ''}>
            <span class="slider"></span>
          </label>
        </div>
      </div>
      <div class="card-body">
        ${renderCardConfigFields(card)}
      </div>
    `;

    // Toggle Expand on header click (except when clicking switch or reorder)
    item.querySelector('.card-header').addEventListener('click', (e) => {
      if (e.target.closest('.switch') || e.target.closest('.card-reorder')) return;
      item.classList.toggle('expanded');
    });

    // Reorder Buttons
    item.querySelector('.btn-up').addEventListener('click', (e) => {
      e.stopPropagation();
      moveCard(index, index - 1);
    });
    item.querySelector('.btn-down').addEventListener('click', (e) => {
      e.stopPropagation();
      moveCard(index, index + 1);
    });

    // Toggle switch
    item.querySelector('.card-toggle').addEventListener('change', (e) => {
      card.enabled = e.target.checked;
      renderPreview();
    });

    // Wire Card-Specific Config Inputs
    bindCardConfigInputs(item, card);

    container.appendChild(item);
  });
}

function moveCard(fromIdx, toIdx) {
  if (toIdx < 0 || toIdx >= state.config.cards.length) return;
  const temp = state.config.cards[fromIdx];
  state.config.cards[fromIdx] = state.config.cards[toIdx];
  state.config.cards[toIdx] = temp;

  state.config.cards.forEach((c, idx) => c.order = idx);
  renderCardsList();
  renderPreview();
}

function getCardIconSvg(id) {
  switch (id) {
    case 'clock':
      return '<svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="10"/><polyline points="12 6 12 12 16 14"/></svg>';
    case 'weather':
      return '<svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="5"/><path d="M12 1v2M12 21v2M4.22 4.22l1.42 1.42M18.36 18.36l1.42 1.42M1 12h2M21 12h2M4.22 19.78l1.42-1.42M18.36 5.64l1.42-1.42"/></svg>';
    case 'forecast':
      return '<svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="3" y="4" width="18" height="18" rx="2" ry="2"/><line x1="16" y1="2" x2="16" y2="6"/><line x1="8" y1="2" x2="8" y2="6"/><line x1="3" y1="10" x2="21" y2="10"/></svg>';
    case 'calendar':
      return '<svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="3" y="4" width="18" height="18" rx="2" ry="2"/><line x1="16" y1="2" x2="16" y2="6"/><line x1="8" y1="2" x2="8" y2="6"/></svg>';
    case 'notifications':
      return '<svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M18 8A6 6 0 0 0 6 8c0 7-3 9-3 9h18s-3-2-3-9"/><path d="M13.73 21a2 2 0 0 1-3.46 0"/></svg>';
    case 'system':
      return '<svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="4" y="4" width="16" height="16" rx="2"/><rect x="9" y="9" width="6" height="6"/><line x1="9" y1="1" x2="9" y2="4"/><line x1="15" y1="1" x2="15" y2="4"/></svg>';
    default:
      return '<svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="10"/></svg>';
  }
}

// Generate Card Sub-settings HTML
function renderCardConfigFields(card) {
  const cfg = card.config || {};
  switch (card.id) {
    case 'clock':
      return `
        <div class="form-grid">
          <div class="form-row">
            <label>Time Format</label>
            <select class="input-field" data-key="format_12h">
              <option value="true" ${cfg.format_12h ? 'selected' : ''}>12-Hour (1:30 PM)</option>
              <option value="false" ${!cfg.format_12h ? 'selected' : ''}>24-Hour (13:30)</option>
            </select>
          </div>
          <div class="form-row">
            <label>Seconds Display</label>
            <select class="input-field" data-key="show_seconds">
              <option value="true" ${cfg.show_seconds ? 'selected' : ''}>Show Seconds</option>
              <option value="false" ${!cfg.show_seconds ? 'selected' : ''}>Hide Seconds</option>
            </select>
          </div>
          <div class="form-row full-width">
            <label>Date Format</label>
            <select class="input-field" data-key="date_format">
              <option value="MDY" ${cfg.date_format === 'MDY' ? 'selected' : ''}>Month Day, Year (Oct 9, 2026)</option>
              <option value="DMY" ${cfg.date_format === 'DMY' ? 'selected' : ''}>Day Month, Year (9 Oct 2026)</option>
              <option value="YMD" ${cfg.date_format === 'YMD' ? 'selected' : ''}>ISO 8601 (2026-10-09)</option>
            </select>
          </div>
        </div>
      `;
    case 'weather':
      return `
        <div class="form-grid">
          <div class="form-row">
            <label>City / Location Label</label>
            <input type="text" class="input-field" data-key="city_name" value="${cfg.city_name || ''}">
          </div>
          <div class="form-row">
            <label>Temperature Units</label>
            <select class="input-field" data-key="use_fahrenheit">
              <option value="true" ${cfg.use_fahrenheit ? 'selected' : ''}>Fahrenheit (°F)</option>
              <option value="false" ${!cfg.use_fahrenheit ? 'selected' : ''}>Celsius (°C)</option>
            </select>
          </div>
          <div class="form-row">
            <label>Latitude</label>
            <input type="number" step="0.0001" class="input-field" data-key="lat" value="${cfg.lat || 37.7749}">
          </div>
          <div class="form-row">
            <label>Longitude</label>
            <input type="number" step="0.0001" class="input-field" data-key="lon" value="${cfg.lon || -122.4194}">
          </div>
          <div class="form-row full-width">
            <button type="button" class="btn btn-sm btn-subtle btn-detect-gps">📍 Detect My Location (GPS)</button>
          </div>
        </div>
      `;
    case 'calendar': {
      const sources = (card.config && card.config.sources && card.config.sources.length)
        ? card.config.sources
        : [{ name: 'Primary', url: cfg.calendar_url || '', color: '#3B82F6' }];

      const sourcesHtml = sources.map((src, idx) => `
        <div class="calendar-source-card" data-index="${idx}">
          <div class="source-header">
            <span class="source-color-dot" style="background-color: ${src.color || '#3B82F6'}"></span>
            <input type="text" class="input-field source-name" placeholder="Calendar Name (e.g. Family)" value="${src.name || 'Calendar'}">
            <input type="color" class="source-color-picker" value="${src.color || '#3B82F6'}" title="Pick source color">
            <button type="button" class="btn-icon btn-remove-source" title="Remove feed" ${sources.length === 1 ? 'disabled style="opacity:0.4"' : ''}>✕</button>
          </div>
          <div class="source-url-row">
            <input type="text" class="input-field font-mono source-url" placeholder="https://calendar.google.com/calendar/ical/.../basic.ics" value="${src.url || ''}">
          </div>
        </div>
      `).join('');

      return `
        <div class="form-grid">
          <div class="form-row">
            <label>Max Events Displayed</label>
            <input type="number" min="1" max="5" class="input-field" data-key="max_events" value="${cfg.max_events || 3}">
          </div>
          <div class="form-row">
            <label>Show Next Event Countdown</label>
            <select class="input-field" data-key="show_countdown">
              <option value="true" ${cfg.show_countdown ? 'selected' : ''}>Enabled</option>
              <option value="false" ${!cfg.show_countdown ? 'selected' : ''}>Disabled</option>
            </select>
          </div>
          <div class="form-row full-width">
            <div class="section-label-row">
              <label>Calendar Feeds & Category Colors</label>
              <button type="button" class="btn-subtle btn-add-source">+ Add Calendar Feed</button>
            </div>
            <div class="calendar-sources-container">
              ${sourcesHtml}
            </div>
          </div>
        </div>
      `;
    }
    case 'notifications':
      return `
        <div class="form-grid">
          <div class="form-row">
            <label>Show Notification Badges</label>
            <select class="input-field" data-key="show_badges">
              <option value="true" ${cfg.show_badges ? 'selected' : ''}>Enabled</option>
              <option value="false" ${!cfg.show_badges ? 'selected' : ''}>Disabled</option>
            </select>
          </div>
          <div class="form-row">
            <label>Alert Dismiss Timeout (Seconds)</label>
            <input type="number" min="5" max="120" class="input-field" data-key="timeout_sec" value="${cfg.timeout_sec || 30}">
          </div>
        </div>
      `;
    case 'system':
      return `
        <div class="form-grid">
          <div class="form-row">
            <label>Show WiFi Signal RSSI</label>
            <select class="input-field" data-key="show_wifi_rssi">
              <option value="true" ${cfg.show_wifi_rssi ? 'selected' : ''}>Visible</option>
              <option value="false" ${!cfg.show_wifi_rssi ? 'selected' : ''}>Hidden</option>
            </select>
          </div>
          <div class="form-row">
            <label>Show Device Uptime</label>
            <select class="input-field" data-key="show_uptime">
              <option value="true" ${cfg.show_uptime ? 'selected' : ''}>Visible</option>
              <option value="false" ${!cfg.show_uptime ? 'selected' : ''}>Hidden</option>
            </select>
          </div>
        </div>
      `;
    default:
      return '<p class="hint">No additional configuration required for this card.</p>';
  }
}

function bindCardConfigInputs(item, card) {
  item.querySelectorAll('[data-key]').forEach(input => {
    input.addEventListener('change', () => {
      const key = input.dataset.key;
      let val = input.value;
      if (input.tagName === 'SELECT') {
        if (val === 'true') val = true;
        else if (val === 'false') val = false;
      } else if (input.type === 'number') {
        val = parseFloat(val);
      }
      if (!card.config) card.config = {};
      card.config[key] = val;
      renderPreview();
    });
  });

  if (card.id === 'calendar') {
    const updateSources = () => {
      const rows = item.querySelectorAll('.calendar-source-card');
      const sources = [];
      rows.forEach(row => {
        const name = row.querySelector('.source-name').value.trim() || 'Calendar';
        const url = row.querySelector('.source-url').value.trim();
        const color = row.querySelector('.source-color-picker').value || '#3B82F6';
        if (url) {
          sources.push({ name, url, color });
        }
      });
      if (!card.config) card.config = {};
      card.config.sources = sources;
      card.config.calendar_url = sources[0]?.url || '';
      renderPreview();
    };

    item.querySelectorAll('.source-name, .source-url').forEach(inp => {
      inp.addEventListener('input', updateSources);
    });

    item.querySelectorAll('.source-color-picker').forEach(cp => {
      cp.addEventListener('input', (e) => {
        const dot = cp.closest('.calendar-source-card').querySelector('.source-color-dot');
        if (dot) dot.style.backgroundColor = e.target.value;
        updateSources();
      });
    });

    item.querySelectorAll('.btn-remove-source').forEach(btn => {
      btn.addEventListener('click', (e) => {
        const cardEl = e.target.closest('.calendar-source-card');
        cardEl.remove();
        updateSources();
      });
    });

    const addBtn = item.querySelector('.btn-add-source');
    if (addBtn) {
      addBtn.addEventListener('click', () => {
        const presetColors = ['#10B981', '#F59E0B', '#8B5CF6', '#EC4899', '#06B6D4'];
        const container = item.querySelector('.calendar-sources-container');
        const count = container.querySelectorAll('.calendar-source-card').length;
        const nextColor = presetColors[count % presetColors.length];
        const newCard = document.createElement('div');
        newCard.className = 'calendar-source-card';
        newCard.innerHTML = `
          <div class="source-header">
            <span class="source-color-dot" style="background-color: ${nextColor}"></span>
            <input type="text" class="input-field source-name" placeholder="Calendar Name (e.g. Family)" value="Calendar ${count + 1}">
            <input type="color" class="source-color-picker" value="${nextColor}" title="Pick source color">
            <button type="button" class="btn-icon btn-remove-source" title="Remove feed">✕</button>
          </div>
          <div class="source-url-row">
            <input type="text" class="input-field font-mono source-url" placeholder="https://calendar.google.com/calendar/ical/.../basic.ics" value="">
          </div>
        `;
        container.appendChild(newCard);
        bindCardConfigInputs(item, card);
        updateSources();
      });
    }
  }

  // GPS detect button for weather card
  const gpsBtn = item.querySelector('.btn-detect-gps');
  if (gpsBtn) {
    gpsBtn.addEventListener('click', () => {
      if (navigator.geolocation) {
        gpsBtn.textContent = 'Detecting...';
        navigator.geolocation.getCurrentPosition(
          pos => {
            const lat = Number(pos.coords.latitude.toFixed(4));
            const lon = Number(pos.coords.longitude.toFixed(4));
            card.config.lat = lat;
            card.config.lon = lon;
            item.querySelector('[data-key="lat"]').value = lat;
            item.querySelector('[data-key="lon"]').value = lon;
            gpsBtn.textContent = '📍 Location Detected!';
            showToast(`Location set to: ${lat}, ${lon}`);
            renderPreview();
          },
          err => {
            gpsBtn.textContent = '📍 Detect Failed';
            showToast('Geolocation permission denied or unavailable');
          }
        );
      }
    });
  }
}

// Setup Event Listeners
function setupEventListeners() {
  // Preset Pills
  document.querySelectorAll('.preset-pill').forEach(btn => {
    btn.addEventListener('click', () => {
      const presetKey = btn.dataset.preset;
      const palette = PRESETS[presetKey];
      if (!palette) return;

      document.querySelectorAll('.preset-pill').forEach(b => b.classList.remove('active'));
      btn.classList.add('active');

      state.config.theme.preset = presetKey;
      state.config.theme.accent_color = palette.accent;
      state.config.theme.bg_color = palette.bg;
      state.config.theme.card_bg_color = palette.cardBg;
      state.config.theme.card_border_color = palette.cardBorder;
      state.config.theme.date_color = palette.text;
      state.config.theme.muted_text_color = palette.muted;
      state.config.theme.time_color = palette.accent;

      syncColorInput('colorAccent', palette.accent);
      syncColorInput('colorBg', palette.bg);
      syncColorInput('colorCardBg', palette.cardBg);
      syncColorInput('colorBorder', palette.cardBorder);
      syncColorInput('colorText', palette.text);
      syncColorInput('colorMuted', palette.muted);

      renderPreview();
    });
  });

  // Custom Color Pickers
  bindColorInput('colorAccent', 'accent_color');
  bindColorInput('colorBg', 'bg_color');
  bindColorInput('colorCardBg', 'card_bg_color');
  bindColorInput('colorBorder', 'card_border_color');
  bindColorInput('colorText', 'date_color');
  bindColorInput('colorMuted', 'muted_text_color');

  // Sliders
  document.getElementById('dayBrightness').addEventListener('input', e => {
    state.config.theme.day_duty = parseInt(e.target.value);
    document.getElementById('dayBrightVal').textContent = Math.round((e.target.value / 255) * 100);
  });
  document.getElementById('nightBrightness').addEventListener('input', e => {
    state.config.theme.night_duty = parseInt(e.target.value);
    document.getElementById('nightBrightVal').textContent = Math.round((e.target.value / 255) * 100);
  });

  // Schedule
  document.getElementById('nightStart').addEventListener('change', e => {
    const [h, m] = e.target.value.split(':').map(Number);
    state.config.theme.night_start_hour = h;
    state.config.theme.night_start_min = m;
  });
  document.getElementById('nightEnd').addEventListener('change', e => {
    const [h, m] = e.target.value.split(':').map(Number);
    state.config.theme.night_end_hour = h;
    state.config.theme.night_end_min = m;
  });

  // System
  document.getElementById('mdnsHostname').addEventListener('input', e => {
    state.config.system.mdns_hostname = e.target.value.trim();
  });
  document.getElementById('ntpServer').addEventListener('input', e => {
    state.config.system.ntp_server = e.target.value.trim();
  });
  document.getElementById('timezone').addEventListener('input', e => {
    state.config.system.timezone = e.target.value.trim();
  });

  // Toggle Day/Night Preview Simulator
  document.getElementById('btnToggleSimMode').addEventListener('click', () => {
    state.previewNightMode = !state.previewNightMode;
    const btn = document.getElementById('btnToggleSimMode');
    btn.textContent = state.previewNightMode ? '🌙 Night' : '☀️ Day';
    renderPreview();
  });

  document.getElementById('btnSimulateTime').addEventListener('click', renderPreview);

  // Save & Apply
  document.getElementById('btnSave').addEventListener('click', saveConfig);

  // Reboot Device
  document.getElementById('btnRestart').addEventListener('click', restartDevice);
}

function bindColorInput(elemId, configKey) {
  const picker = document.getElementById(elemId);
  const hex = document.getElementById(`${elemId}Hex`);

  picker.addEventListener('input', e => {
    hex.value = e.target.value.toUpperCase();
    state.config.theme[configKey] = e.target.value;
    if (configKey === 'accent_color') state.config.theme.time_color = e.target.value;
    renderPreview();
  });

  hex.addEventListener('input', e => {
    let val = e.target.value.trim();
    if (!val.startsWith('#')) val = '#' + val;
    if (/^#[0-9A-Fa-f]{6}$/.test(val)) {
      picker.value = val;
      state.config.theme[configKey] = val;
      if (configKey === 'accent_color') state.config.theme.time_color = val;
      renderPreview();
    }
  });
}

// Save Configuration to Flash
async function saveConfig() {
  const btn = document.getElementById('btnSave');
  btn.disabled = true;
  btn.textContent = 'Saving...';

  try {
    const res = await fetch('/api/config', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(state.config)
    });
    if (!res.ok) throw new Error('Save error');
    showToast('✓ Settings Saved & Applied to Clock!');
  } catch (err) {
    console.error('Failed to save to ESP32:', err);
    showToast('Saved locally (Offline Mode)');
  } finally {
    btn.disabled = false;
    btn.innerHTML = `<svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M19 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h11l5 5v11a2 2 0 0 1-2 2z"/><polyline points="17 21 17 13 7 13 7 21"/><polyline points="7 3 7 8 15 8"/></svg> Save & Apply`;
  }
}

// Reboot Device
async function restartDevice() {
  if (!confirm('Reboot the ESP32 Clock now?')) return;
  showToast('Rebooting ESP32 Clock...');
  try {
    await fetch('/api/restart', { method: 'POST' });
  } catch (e) {}
  setTimeout(() => location.reload(), 4000);
}

// ==========================================
// Live 320x240 Screen Canvas Simulator
// ==========================================
function renderPreview() {
  if (!state.config) return;

  const t = state.config.theme;
  const isNight = state.previewNightMode;

  // Active palette
  const colBg = isNight ? '#000000' : t.bg_color;
  const colCard = isNight ? '#120C0A' : t.card_bg_color;
  const colBorder = isNight ? '#2D1410' : t.card_border_color;
  const colAccent = isNight ? '#FF9800' : t.accent_color;
  const colText = isNight ? '#E2A064' : t.date_color;
  const colMuted = isNight ? '#8C5638' : t.muted_text_color;

  // 1. Background
  ctx.fillStyle = colBg;
  ctx.fillRect(0, 0, 320, 240);

  // Enabled cards
  const enabledCards = state.config.cards
    .filter(c => c.enabled)
    .sort((a, b) => a.order - b.order);

  // 2. Render Hero Top Card (Clock or first card)
  const clockCard = enabledCards.find(c => c.id === 'clock');
  if (clockCard) {
    drawRoundedRect(ctx, 8, 8, 304, 104, 16, colCard, colBorder);

    // Current Time
    const now = new Date();
    let hours = now.getHours();
    const is12 = clockCard.config?.format_12h !== false;
    let ampm = '';
    if (is12) {
      ampm = hours >= 12 ? 'PM' : 'AM';
      hours = hours % 12 || 12;
    }
    const mins = String(now.getMinutes()).padStart(2, '0');
    const secs = String(now.getSeconds()).padStart(2, '0');
    const showSecs = clockCard.config?.show_seconds !== false;
    const timeStr = showSecs ? `${hours}:${mins}:${secs}` : `${hours}:${mins}`;

    // Digits
    ctx.fillStyle = colAccent;
    ctx.font = '700 44px "Outfit", sans-serif';
    ctx.textAlign = 'center';
    ctx.textBaseline = 'middle';
    ctx.fillText(timeStr, 142, 54);

    // AM/PM Pill
    if (is12) {
      const badgeX = 236;
      drawRoundedRect(ctx, badgeX, 42, 36, 20, 6, isNight ? '#2A1408' : colBorder, colBorder);
      ctx.fillStyle = colAccent;
      ctx.font = '600 11px "Outfit", sans-serif';
      ctx.textAlign = 'center';
      ctx.textBaseline = 'middle';
      ctx.fillText(ampm, badgeX + 18, 52);
    }

    // Date String
    const dateOptions = { weekday: 'short', month: 'short', day: 'numeric', year: 'numeric' };
    const dateStr = now.toLocaleDateString('en-US', dateOptions);
    ctx.fillStyle = colText;
    ctx.font = '500 13px "Outfit", sans-serif';
    ctx.textAlign = 'center';
    ctx.fillText(dateStr, 160, 92);
  }

  // 3. Render Secondary Bottom Cards (Slots: (8, 120, 148, 112) and (164, 120, 148, 112))
  const bottomCards = enabledCards.filter(c => c.id !== 'clock');
  const leftCard = bottomCards[0];
  const rightCard = bottomCards[1];

  if (leftCard) {
    drawSubCard(ctx, 8, 120, 148, 112, leftCard, colCard, colBorder, colAccent, colText, colMuted, isNight);
  }
  if (rightCard) {
    drawSubCard(ctx, 164, 120, 148, 112, rightCard, colCard, colBorder, colAccent, colText, colMuted, isNight);
  }
}

function drawSubCard(ctx, x, y, w, h, card, colCard, colBorder, colAccent, colText, colMuted, isNight) {
  drawRoundedRect(ctx, x, y, w, h, 16, colCard, colBorder);

  switch (card.id) {
    case 'weather': {
      // Weather Icon & Temp
      ctx.fillStyle = isNight ? '#F59E0B' : '#38BDF8';
      ctx.beginPath();
      ctx.arc(x + 24, y + 26, 10, 0, Math.PI * 2);
      ctx.fill();

      ctx.fillStyle = colText;
      ctx.font = '700 24px "Outfit", sans-serif';
      ctx.textAlign = 'left';
      ctx.fillText('72°F', x + 44, y + 32);

      ctx.fillStyle = colAccent;
      ctx.font = '500 12px "Outfit", sans-serif';
      ctx.fillText('Clear Sky', x + 14, y + 60);

      ctx.fillStyle = colText;
      ctx.font = '500 11px "Outfit", sans-serif';
      ctx.fillText('H: 76° / L: 58°', x + 14, y + 80);

      ctx.fillStyle = colMuted;
      ctx.font = '400 10px "Outfit", sans-serif';
      ctx.fillText('Feels 70° | 48% Hum', x + 14, y + 98);
      break;
    }
    case 'forecast': {
      ctx.fillStyle = colAccent;
      ctx.font = '600 12px "Outfit", sans-serif';
      ctx.textAlign = 'left';
      ctx.fillText('2-Day Forecast', x + 14, y + 24);

      // Day 1
      ctx.fillStyle = colText;
      ctx.font = '600 11px "Outfit", sans-serif';
      ctx.fillText('Fri', x + 14, y + 54);
      ctx.fillStyle = colMuted;
      ctx.font = '400 11px "Outfit", sans-serif';
      ctx.fillText('74° / 55°', x + 58, y + 54);

      // Day 2
      ctx.fillStyle = colText;
      ctx.font = '600 11px "Outfit", sans-serif';
      ctx.fillText('Sat', x + 14, y + 86);
      ctx.fillStyle = colMuted;
      ctx.font = '400 11px "Outfit", sans-serif';
      ctx.fillText('70° / 52°', x + 58, y + 86);
      break;
    }
    case 'calendar': {
      ctx.fillStyle = colAccent;
      ctx.font = '600 12px "Outfit", sans-serif';
      ctx.textAlign = 'left';
      ctx.fillText('Upcoming Agenda', x + 14, y + 24);

      // Event 1 with Blue Dot
      ctx.fillStyle = '#3B82F6';
      ctx.beginPath();
      ctx.arc(x + 18, y + 48, 3, 0, Math.PI * 2);
      ctx.fill();

      ctx.fillStyle = colText;
      ctx.font = '600 12px "Outfit", sans-serif';
      ctx.fillText('Team Standup', x + 26, y + 52);

      ctx.fillStyle = colMuted;
      ctx.font = '400 10px "Outfit", sans-serif';
      ctx.fillText('in 25 mins • Zoom', x + 26, y + 70);

      // Event 2 with Emerald Dot
      ctx.fillStyle = '#10B981';
      ctx.beginPath();
      ctx.arc(x + 18, y + 91, 3, 0, Math.PI * 2);
      ctx.fill();

      ctx.fillStyle = colText;
      ctx.font = '500 11px "Outfit", sans-serif';
      ctx.fillText('Family Dinner', x + 26, y + 95);
      break;
    }
    case 'notifications': {
      ctx.fillStyle = colAccent;
      ctx.font = '600 12px "Outfit", sans-serif';
      ctx.textAlign = 'left';
      ctx.fillText('Notifications', x + 14, y + 24);

      ctx.fillStyle = colText;
      ctx.font = '600 12px "Outfit", sans-serif';
      ctx.fillText('✉ 3 Unread Emails', x + 14, y + 54);

      ctx.fillStyle = colMuted;
      ctx.font = '400 10px "Outfit", sans-serif';
      ctx.fillText('💬 Slack: 2 Mentions', x + 14, y + 82);
      break;
    }
    case 'system': {
      ctx.fillStyle = colAccent;
      ctx.font = '600 12px "Outfit", sans-serif';
      ctx.textAlign = 'left';
      ctx.fillText('System Telemetry', x + 14, y + 24);

      ctx.fillStyle = colMuted;
      ctx.font = '400 10px "Outfit", sans-serif';
      ctx.fillText('WiFi RSSI', x + 14, y + 48);
      ctx.fillStyle = colText;
      ctx.font = '600 11px "Outfit", sans-serif';
      ctx.fillText('-58 dBm', x + 75, y + 48);

      ctx.fillStyle = colMuted;
      ctx.font = '400 10px "Outfit", sans-serif';
      ctx.fillText('Free RAM', x + 14, y + 72);
      ctx.fillStyle = colText;
      ctx.font = '600 11px "Outfit", sans-serif';
      ctx.fillText('218 KB', x + 75, y + 72);

      ctx.fillStyle = colMuted;
      ctx.font = '400 10px "Outfit", sans-serif';
      ctx.fillText('Uptime', x + 14, y + 96);
      ctx.fillStyle = colText;
      ctx.font = '600 11px "Outfit", sans-serif';
      ctx.fillText('4h 12m', x + 75, y + 96);
      break;
    }
    default: {
      ctx.fillStyle = colAccent;
      ctx.font = '600 12px "Outfit", sans-serif';
      ctx.textAlign = 'left';
      ctx.fillText(card.title, x + 14, y + 24);
      break;
    }
  }
}

function drawRoundedRect(ctx, x, y, width, height, radius, fill, stroke) {
  ctx.beginPath();
  ctx.moveTo(x + radius, y);
  ctx.lineTo(x + width - radius, y);
  ctx.quadraticCurveTo(x + width, y, x + width, y + radius);
  ctx.lineTo(x + width, y + height - radius);
  ctx.quadraticCurveTo(x + width, y + height, x + width - radius, y + height);
  ctx.lineTo(x + radius, y + height);
  ctx.quadraticCurveTo(x, y + height, x, y + height - radius);
  ctx.lineTo(x, y + radius);
  ctx.quadraticCurveTo(x, y, x + radius, y);
  ctx.closePath();

  if (fill) {
    ctx.fillStyle = fill;
    ctx.fill();
  }
  if (stroke) {
    ctx.strokeStyle = stroke;
    ctx.lineWidth = 1;
    ctx.stroke();
  }
}
