const MOCK_STUDENTS = {
  "2512510001": "Test Student One",
  "2512510002": "Test Student Two",
  "2512510003": "Test Student Three"
};

const MOCK_STOPS = [
  { id: "S1", name: "Graphic Era Main Gate" },
  { id: "S2", name: "ISBT" },
  { id: "S3", name: "Majra" },
  { id: "S4", name: "Niranjanpur" },
  { id: "S5", name: "Patel Nagar" },
  { id: "S6", name: "Saharanpur Chowk" },
  { id: "S7", name: "Doordarshan Kendra" },
  { id: "S8", name: "Ajabpur Chowk" },
  { id: "S9", name: "Mata Mandir Chowk" },
  { id: "S10", name: "Kargi Chowk" },
  { id: "S11", name: "Brahmanwala Chowk" }
];

const MOCK_ROUTES = {
  R1: { stops: ["S1", "S2", "S3", "S4", "S5", "S6"], times: [5, 3, 4, 4, 5] },
  R2: { stops: ["S1", "S7", "S8", "S9", "S10", "S11", "S2"], times: [3, 2, 2, 4, 3, 4] }
};

const MOCK_BUSES = [
  { bus: "B1", route: "R1", capacity: 50, occupancy: 30 },
  { bus: "B2", route: "R1", capacity: 50, occupancy: 50 },
  { bus: "B3", route: "R2", capacity: 40, occupancy: 20 }
];

const MOCK_WAIT_MIN = 4;

function delay(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}

async function login(collegeId) {
  await delay(300);
  if (!/^\d{10}$/.test(collegeId)) {
    return { ok: false, error: "College ID must be 10 digits" };
  }
  if (!MOCK_STUDENTS[collegeId]) {
    return { ok: false, error: "Unknown college ID" };
  }
  return { ok: true, name: MOCK_STUDENTS[collegeId] };
}

async function getStops() {
  await delay(300);
  return MOCK_STOPS;
}


async function recommend(fromId, toId) {
  await delay(500);
  if (fromId === toId) {
    return { ok: false, error: "Start and destination must be different" };
  }

  let best = null;

  for (const b of MOCK_BUSES) {
    const seatsLeft = b.capacity - b.occupancy;
    if (seatsLeft <= 0) continue;

    const r = MOCK_ROUTES[b.route];
    const i = r.stops.indexOf(fromId);
    const j = r.stops.indexOf(toId);
    if (i === -1 || j === -1 || j <= i) continue;

    const rideMin = r.times.slice(i, j).reduce((a, t) => a + t, 0);
    const etaMin = MOCK_WAIT_MIN + rideMin;

    if (!best || etaMin < best.eta_min) {
      best = {
        ok: true, bus: b.bus, route: b.route,
        wait_min: MOCK_WAIT_MIN, ride_min: rideMin,
        eta_min: etaMin, seats_left: seatsLeft
      };
    }
  }

  return best || { ok: false, error: "No bus available for this trip" };
}