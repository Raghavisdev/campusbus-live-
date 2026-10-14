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

  form.addEventListener("submit", async (event) => {
    event.preventDefault();

    if (!fromSelect.value || !toSelect.value) {
      showError(resultBox, "Please select both stops");
      return;
    }

    button.disabled = true;
    button.textContent = "Finding...";

    const result = await recommend(fromSelect.value, toSelect.value);

    if (result.ok) {
      showBus(resultBox, result);
    } else {
      showError(resultBox, result.error);
    }

    button.disabled = false;
    button.textContent = "Find Bus";
  });
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

function showError(box, message) {
  box.innerHTML = "";
  const text = document.createElement("p");
  text.textContent = message;
  box.appendChild(text);
  box.classList.add("is-error");
  box.hidden = false;
}