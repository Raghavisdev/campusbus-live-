// Page logic for CampusBus Live.
// All data comes from mockApi.js (login, getStops, recommend), so connecting
// the real Flask backend later only changes that file.

const page = document.body.dataset.page;

if (page === "login") {
  setupLoginPage();
} else if (page === "home") {
  setupHomePage();
}

function setupLoginPage() {
  // Already logged in: skip the login page
  if (sessionStorage.getItem("studentName")) {
    window.location.href = "home.html";
    return;
  }

  const form = document.getElementById("loginForm");
  const input = document.getElementById("collegeId");
  const error = document.getElementById("loginError");
  const button = document.getElementById("loginButton");

  form.addEventListener("submit", async (event) => {
    event.preventDefault();

    error.textContent = "";
    button.disabled = true;
    button.textContent = "Logging in...";

    const result = await login(input.value.trim());

    if (result.ok) {
      sessionStorage.setItem("studentName", result.name);
      window.location.href = "home.html";
      return;
    }

    error.textContent = result.error;
    button.disabled = false;
    button.textContent = "Login";
  });
}

async function setupHomePage() {
  const name = sessionStorage.getItem("studentName");

  // Not logged in: go back to the login page
  if (!name) {
    window.location.href = "index.html";
    return;
  }

  document.getElementById("welcome").textContent = "Welcome, " + name;

  document.getElementById("logoutButton").addEventListener("click", () => {
    sessionStorage.clear();
    window.location.href = "index.html";
  });

  const fromSelect = document.getElementById("fromStop");
  const toSelect = document.getElementById("toStop");
  const form = document.getElementById("findForm");
  const button = document.getElementById("findButton");
  const resultBox = document.getElementById("result");

  const stops = await getStops();
  fillStopOptions(fromSelect, stops, "Select your stop");
  fillStopOptions(toSelect, stops, "Select destination");
  button.disabled = false;

  let searchId = 0;

  // Runs a search: result card first, then the live map under it
  async function search(showMissingError) {
    if (!fromSelect.value || !toSelect.value) {
      if (showMissingError) {
        showError(resultBox, "Please select both stops");
      }
      return;
    }

    const thisSearch = ++searchId;

    button.disabled = true;
    button.textContent = "Finding...";

    stopLive();

    const from = fromSelect.value;
    const to = toSelect.value;
    const result = await recommend(from, to);

    // The student changed a stop while this search was running: drop this answer
    if (thisSearch !== searchId) {
      return;
    }

    if (result.ok) {
      showBus(resultBox, result);
      startLive(from, to, resultBox);
    } else {
      showError(resultBox, result.error, result.plan);
    }

    button.disabled = false;
    button.textContent = "Find Bus";
  }

  // The button works, and so does simply choosing the stops
  form.addEventListener("submit", (event) => {
    event.preventDefault();
    search(true);
  });
  fromSelect.addEventListener("change", () => search(false));
  toSelect.addEventListener("change", () => search(false));
}

// ---------- Live map under the result card ----------
// Every 5 seconds the answer is fetched again, so the card and the map follow the bus.

let liveTimer = null;
let liveMap = null;

function stopLive() {
  clearInterval(liveTimer);
  liveTimer = null;

  if (liveMap) {
    liveMap.map.remove();
    liveMap = null;
  }
  document.getElementById("mapCard").hidden = true;
}

function startLive(fromId, toId, resultBox) {
  async function tick() {
    const data = await track(fromId, toId);

    if (!data.ok) {
      // The bus is gone (passed, silent) or has no road line: keep the message, hide the map
      if (data.error && data.error.startsWith("No bus")) {
        showError(resultBox, data.error, data.plan);
      }
      document.getElementById("mapCard").hidden = true;
      return;
    }

    showBus(resultBox, data.answer);
    drawMap(data);
  }

  tick();
  liveTimer = setInterval(tick, 5000);
}

function pin(className, text) {
  const element = document.createElement("div");
  element.className = "pin " + className;
  element.textContent = text;
  return element;
}

function drawMap(data) {
  const card = document.getElementById("mapCard");
  card.hidden = false;

  // Route, stops and the two markers are drawn once per search
  if (!liveMap) {
    const map = L.map("map");

    L.tileLayer("https://tile.openstreetmap.org/{z}/{x}/{y}.png", {
      maxZoom: 19,
      attribution: "&copy; OpenStreetMap contributors"
    }).addTo(map);

    const routeLine = L.polyline(data.route.line, { color: "#1a56db", weight: 5, opacity: 0.55 }).addTo(map);

    for (const stop of data.route.stops) {
      L.circleMarker([stop.lat, stop.lon], {
        radius: 6, color: "#143769", weight: 2, fillColor: "#ffffff", fillOpacity: 1
      }).addTo(map).bindTooltip(stop.name, {
        permanent: data.route.stops.length <= 8, direction: "top", offset: [0, -6]
      });
    }

    L.marker([data.student.lat, data.student.lon], {
      icon: L.divIcon({ className: "", html: pin("pin-me", "You"), iconSize: [60, 24], iconAnchor: [30, 12] })
    }).addTo(map).bindTooltip("You are here: " + data.student.name, { direction: "bottom", offset: [0, 12] });

    const bus = L.marker([data.bus.lat, data.bus.lon], {
      icon: L.divIcon({ className: "", html: pin("pin-bus", data.bus.id), iconSize: [44, 26], iconAnchor: [22, 13] }),
      zIndexOffset: 1000
    }).addTo(map);

    // The card was hidden until now, so let the map measure itself before fitting the route
    map.invalidateSize();
    map.fitBounds(routeLine.getBounds(), { padding: [30, 30] });
    setTimeout(() => {
      map.invalidateSize();
      map.fitBounds(routeLine.getBounds(), { padding: [30, 30] });
    }, 300);
    liveMap = { map: map, bus: bus, remaining: null };
  }

  // The bus and the road it still has to drive move every update
  liveMap.bus.setLatLng([data.bus.lat, data.bus.lon]);

  if (liveMap.remaining) {
    liveMap.map.removeLayer(liveMap.remaining);
  }
  liveMap.remaining = L.polyline(data.remaining, { color: "#ff8a00", weight: 7 }).addTo(liveMap.map);

  const seconds = Math.max(0, Math.round((Date.now() - new Date(data.bus.last_updated.replace(" ", "T") + "Z")) / 1000));
  document.getElementById("mapInfo").textContent =
    "Bus " + data.bus.id + " is " + data.distance_km + " km away. Last GPS update " + seconds + " s ago. Updates every 5 seconds.";
}

function fillStopOptions(select, stops, placeholder) {
  select.innerHTML = "";
  select.appendChild(new Option(placeholder, ""));

  for (const stop of stops) {
    // Show the stop name, send the stop id
    select.appendChild(new Option(stop.name, stop.id));
  }
}

function showBus(box, bus) {
  box.innerHTML = `
    <div class="bus-header">
      <span class="bus-number">${bus.bus}</span>
      <span class="route">Route ${bus.route}</span>
    </div>
    <ul class="bus-details">
      <li><span>Arrives in</span><strong>${bus.wait_min} min</strong></li>
      <li><span>Ride</span><strong>${bus.ride_min} min</strong></li>
      <li><span>Total</span><strong>${bus.eta_min} min</strong></li>
      <li><span>Seats left</span><strong>${bus.seats_left}</strong></li>
    </ul>
  `;
  box.classList.remove("is-error");
  box.hidden = false;
}

function showError(box, message, plan) {
  box.innerHTML = "";
  const text = document.createElement("p");
  text.textContent = message;
  box.appendChild(text);

  // No live bus, but the route graph (Dijkstra) can still tell how the stops connect
  if (plan) {
    const route = document.createElement("p");
    route.className = "plan";
    route.textContent = planText(plan);
    box.appendChild(route);
  }

  box.classList.add("is-error");
  box.hidden = false;
}

function planText(plan) {
  const legs = plan.legs.map((leg) => leg.route + " from " + leg.from + " to " + leg.to).join(", then ");
  const changes = plan.transfers === 0
    ? "no change of bus"
    : plan.transfers + (plan.transfers === 1 ? " change" : " changes") + " of bus";

  return "Route plan: " + legs + ". About " + plan.minutes + " min, " + changes + ".";
}