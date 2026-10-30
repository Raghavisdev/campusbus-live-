// Real backend: same functions and JSON shapes as mockApi.js, but each one
// calls the Flask API (vision/app.py), which runs the C++ recommendation.

async function callApi(url, options) {
  try {
    const response = await fetch(url, options);
    return await response.json();
  } catch (error) {
    return { ok: false, error: "Cannot reach the server. Is Flask running?" };
  }
}

// POST /api/login  →  { ok: true, name } | { ok: false, error }
async function login(collegeId) {
  return callApi("/api/login", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ college_id: collegeId })
  });
}

// GET /api/stops  →  [ { id, name }, ... ]
async function getStops() {
  const stops = await callApi("/api/stops");
  return Array.isArray(stops) ? stops : [];
}

// GET /api/recommend?from=&to=
//   →  { ok: true, bus, route, wait_min, ride_min, eta_min, seats_left }
//   |  { ok: false, error }
async function recommend(fromId, toId) {
  const query = new URLSearchParams({ from: fromId, to: toId });
  return callApi("/api/recommend?" + query.toString());
}
