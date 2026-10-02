'use strict';

const get = (id) => document.getElementById(id);
const SESSION_KEY = 'campusbus-user';
const PREVIEW_PASSWORD = 'CampusBus@123';
const UNIVERSITY_EMAIL = /^[a-z0-9._%+-]+@geu\.ac\.in$/i;
let stops = [];
let refreshHandler = null;

function isValidEmail(email) {
    return UNIVERSITY_EMAIL.test(email);
}

function showDashboard(email) {
    get('login-view').hidden = true;
    get('dashboard-view').hidden = false;
    get('account').hidden = false;

    get('account-email').textContent = email;
    get('password').value = '';
}

function showLogin() {
    get('dashboard-view').hidden = true;
    get('account').hidden = true;
    get('login-view').hidden = false;
    get('account-email').textContent = '';
    get('login-form').reset();
    clearLoginError();
    get('userid').focus();
}

function showLoginError(message, fieldId) {
    const error = get('login-error');
    error.textContent = message;
    error.hidden = false;
    get(fieldId).focus();
}

function clearLoginError() {
    get('login-error').hidden = true;
    get('login-error').textContent = '';
}

// Login
get('login-form').addEventListener('submit', function(event) {
    event.preventDefault();
    clearLoginError();
    const  email = get('userid').value.trim().toLowerCase();
    const password = get('password').value;
    if (!isValidEmail(email)) {
        showLoginError(
            'Enter a valid university email ending in @geu.ac.in.',
            'userid'
        );
        return;
    }
    if (password !== PREVIEW_PASSWORD) {
        showLoginError('Incorrect password. Please try again.', 'password');
        return;
    }
    sessionStorage.setItem(SESSION_KEY, email);
    showDashboard(email);
});

get('userid').addEventListener('input', clearLoginError);
get('password').addEventListener('input', clearLoginError);

// Show / hide password
get('toggle-password').addEventListener('click', function() {
    const passwordInput = get('password');
    if (passwordInput.type === 'password') {
        passwordInput.type = 'text';
        this.textContent = 'Hide';
    } else {
         passwordInput.type = 'password';
        this.textContent = 'Show';
    }
});

// Logout
get('logout').addEventListener('click', function() {
    sessionStorage.removeItem(SESSION_KEY);
    showLogin();
});

// Stop selection
function getSelectedStop() {
    return stops.find(function(stop) {
        return stop.id === get('stop-select').value;
    });
}

function clearBoard(title, message) {
    get('arrivals-body').innerHTML = '';
    get('empty-title').textContent = title;
    get('empty-description').textContent = message;
     get('empty-state').hidden = false;

    get('updated-at').textContent = 'Last updated: —';
    get('eta-note').textContent = '';
}
 
  
function stopChanged() {
     const stop = getSelectedStop();

    if (stop) {
        get('selected-stop-label').textContent = stop.name;
        clearBoard(
            'Waiting for arrival information',
            'Arrival information for this stop will appear here.'
        );
    } else {
        get('selected-stop-label').textContent = 'No stop selected';
        clearBoard(
            'Choose your stop',
            'Select a stop to see its approaching buses.'
        );
    }
    get('refresh').disabled = !stop || !refreshHandler;
    get('board-error').hidden = true;

    if (stop && refreshHandler) {
        refreshHandler(stop.id);
    }
}

get('stop-select').addEventListener('change', stopChanged);

get('refresh').addEventListener('click', function() {
    const stop = getSelectedStop();
    if (stop && refreshHandler) {
        refreshHandler(stop.id);
    }
});

// Functions used later by the backend
window.CampusBusUI = {

    setNetwork: function(network) {
        stops = network.stops || [];

        const select = get('stop-select');

        select.innerHTML = '';

        const firstOption = document.createElement('option');
        firstOption.value = '';
        firstOption.textContent = stops.length
            ? 'Select your stop'
            : 'Stops are not available yet';

        select.appendChild(firstOption);

        stops.forEach(function(stop) {
            const option = document.createElement('option');

            option.value = stop.id;
            option.textContent = stop.name;

            select.appendChild(option);
        });

        select.disabled = stops.length === 0;

        if (stops.length) {
            get('stop-help').textContent =
                'Select the stop where you want to board.';
        } else {
            get('stop-help').textContent =
                'Stops will appear when the route network is connected.';
        }
    },

    onRefresh: function(handler) {
        refreshHandler = handler;

        if (getSelectedStop()) {
            get('refresh').disabled = false;
        }
    },

    getSelectedStopId: function() {
        return get('stop-select').value;
    },
    setConnection: function(connected) {
        const status = get('connection-status');

        status.classList.toggle('connected', connected);

        status.innerHTML =
            '<span class="status-dot"></span>' +
            (connected ? 'Service connected' : 'Service disconnected');
    },

    showError: function(message) {
        get('board-error').textContent = message;
        get('board-error').hidden = false;
    }
};

// Restore login during the same browser session
const savedEmail = sessionStorage.getItem(SESSION_KEY);

if (savedEmail && isValidEmail(savedEmail)) {
    showDashboard(savedEmail);
}