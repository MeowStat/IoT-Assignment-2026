// ==================== WEBSOCKET ====================
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

window.addEventListener('load', onLoad);

function onLoad(event) {
    initWebSocket();
}

function onOpen(event) {
    console.log('Connection opened');
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function Send_Data(data) {
    if (websocket && websocket.readyState === WebSocket.OPEN) {
        websocket.send(data);
        console.log("Sent:", data);
    } else {
        console.warn("WebSocket is not ready!");
        alert("WebSocket is not connected!");
    }
}

function onMessage(event) {
    let d;
    try { d = JSON.parse(event.data); }
    catch (e) { console.warn("Not JSON:", event.data); return; }
    if (d.page !== "sensor") return;

    if (typeof d.temp !== 'undefined') updateSensorUI(parseFloat(d.temp), parseFloat(d.humi));

    if (d.state) {
        document.getElementById('val_state').textContent = d.state;
        const badge = document.getElementById('badge_state');
        badge.textContent = d.state;
        badge.className = 'status-badge ' +
            (d.state === 'NORMAL' ? 'badge-ok' : d.state === 'WARNING' ? 'badge-high' : 'badge-low');
    }
    if (typeof d.anomaly !== 'undefined') {
        document.getElementById('val_anomaly').textContent = d.anomaly;
        document.getElementById('val_anomaly_score').textContent =
            "Confidence: " + (typeof d.score === 'number' ? d.score.toFixed(2) : d.score);
    }
    if (typeof d.pump !== 'undefined') updateBtn('btnPump', d.pump);
    if (typeof d.fan  !== 'undefined') updateBtn('btnFan',  d.fan);
    if (typeof d.auto !== 'undefined') updateBtn('btnAuto', d.auto);
}


// ==================== SENSOR DISPLAY ====================
function updateSensorState(valueId, badgeId, value, low, high) {
    var valEl = document.getElementById(valueId);
    var badgeEl = document.getElementById(badgeId);
    
    // Update numerical value text
    valEl.textContent = value.toFixed(1);

    // Apply colors and update badge based on thresholds
    if (value < low) {
        badgeEl.textContent = 'Low';
        badgeEl.className = 'status-badge badge-low';
        valEl.style.color = '#3b82f6'; // Blue for Low
    } else if (value > high) {
        badgeEl.textContent = 'High';
        badgeEl.className = 'status-badge badge-high';
        valEl.style.color = '#ef4444'; // Red for High
    } else {
        badgeEl.textContent = 'OK';
        badgeEl.className = 'status-badge badge-ok';
        valEl.style.color = '#22c55e'; // Green for OK
    }
}

function updateSensorUI(temp, humi) {
    updateSensorState('val_temp', 'badge_temp', temp, 20, 35);   // FR1 thresholds
    updateSensorState('val_humi', 'badge_humi', humi, 40, 75);   // FR2 thresholds
}

// ==================== SPEC ACTUATOR + AUTO ====================
function toggleActuator(which) {
    Send_Data(JSON.stringify({ page: "actuator", value: { name: which, action: "toggle" } }));
}
function toggleAuto() {
    Send_Data(JSON.stringify({ page: "auto", value: { action: "toggle" } }));
}
function updateBtn(id, on) {
    const b = document.getElementById(id);
    if (!b) return;
    b.textContent = on ? "ON" : "OFF";
    b.classList.toggle('on', !!on);
}


// ==================== UI NAVIGATION ====================
let relayList = [];
let deleteTarget = null;

function showSection(id, event) {
    document.querySelectorAll('.section').forEach(sec => sec.style.display = 'none');
    document.getElementById(id).style.display = id === 'settings' ? 'flex' : 'block';
    document.querySelectorAll('.nav-item').forEach(i => i.classList.remove('active'));
    event.currentTarget.classList.add('active');
}


// ==================== DEVICE FUNCTIONS ====================
function openAddRelayDialog() {
    document.getElementById('addRelayDialog').style.display = 'flex';
}
function closeAddRelayDialog() {
    document.getElementById('addRelayDialog').style.display = 'none';
}
function saveRelay() {
    const name = document.getElementById('relayName').value.trim();
    const gpio = document.getElementById('relayGPIO').value.trim();
    if (!name || !gpio) return alert("Please fill in all fields!");
    relayList.push({ id: Date.now(), name, gpio, state: false });
    renderRelays();
    closeAddRelayDialog();
}
function renderRelays() {
    const container = document.getElementById('relayContainer');
    container.innerHTML = "";
    relayList.forEach(r => {
        const card = document.createElement('div');
        card.className = 'device-card';
        card.innerHTML = `
      <div class="device-icon">
        <svg viewBox="0 0 24 24"><polygon points="13 2 3 14 12 14 11 22 21 10 12 10 13 2"/></svg>
      </div>
      <h3>${r.name}</h3>
      <p>GPIO: ${r.gpio}</p>
      <button class="toggle-btn ${r.state ? 'on' : ''}" onclick="toggleRelay(${r.id})">
        ${r.state ? 'ON' : 'OFF'}
      </button>
      <button class="delete-icon" onclick="showDeleteDialog(${r.id})">
        <svg viewBox="0 0 24 24"><polyline points="3 6 5 6 21 6"/><path d="M19 6l-1 14H6L5 6"/><path d="M10 11v6M14 11v6M9 6V4h6v2"/></svg>
      </button>
    `;
        container.appendChild(card);
    });
}
function toggleRelay(id) {
    const relay = relayList.find(r => r.id === id);
    if (relay) {
        relay.state = !relay.state;
        const relayJSON = JSON.stringify({
            page: "device",
            value: {
                name: relay.name,
                status: relay.state ? "ON" : "OFF",
                gpio: relay.gpio
            }
        });
        Send_Data(relayJSON);
        renderRelays();
    }
}
function showDeleteDialog(id) {
    deleteTarget = id;
    document.getElementById('confirmDeleteDialog').style.display = 'flex';
}
function closeConfirmDelete() {
    document.getElementById('confirmDeleteDialog').style.display = 'none';
}
function confirmDelete() {
    relayList = relayList.filter(r => r.id !== deleteTarget);
    renderRelays();
    closeConfirmDelete();
}


// ==================== SETTINGS FORM ====================
document.getElementById("settingsForm").addEventListener("submit", function (e) {
    e.preventDefault();

    const ssid = document.getElementById("ssid").value.trim();
    const password = document.getElementById("password").value.trim();
    const token = document.getElementById("token").value.trim();
    const server = document.getElementById("server").value.trim();
    const port = document.getElementById("port").value.trim();

    const settingsJSON = JSON.stringify({
        page: "setting",
        value: {
            ssid: ssid,
            password: password,
            token: token,
            server: server,
            port: port
        }
    });

    Send_Data(settingsJSON);
    alert("✅ Configuration has been sent to the device!");
});
