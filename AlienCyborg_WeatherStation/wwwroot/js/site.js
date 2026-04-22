const metricNodes = {
  temperature: document.querySelector('[data-metric="temperature"]'),
  temperatureUnit: document.querySelector('[data-metric="temperatureUnit"]'),
  humidity: document.querySelector('[data-metric="humidity"]'),
  pressure: document.querySelector('[data-metric="pressure"]'),
  co2: document.querySelector('[data-metric="co2"]')
};

const temperatureUnitButtons = document.querySelectorAll("[data-temp-unit]");
const rainState = document.getElementById("rainState");
const rainVisual = document.getElementById("rainVisual");
const lastSync = document.getElementById("lastSync");
const healthIndex = document.getElementById("healthIndex");

let tick = 0;
let temperatureUnit = "F";
let latestTemperatureC = 22.8;

function setText(node, value) {
  if (node) {
    node.textContent = value;
  }
}

function formatTemperature(tempC) {
  if (temperatureUnit === "C") {
    return tempC.toFixed(1);
  }

  return ((tempC * 9) / 5 + 32).toFixed(1);
}

function renderTemperature() {
  setText(metricNodes.temperature, formatTemperature(latestTemperatureC));
  setText(metricNodes.temperatureUnit, temperatureUnit);
}

function setTemperatureUnit(unit) {
  temperatureUnit = unit;
  temperatureUnitButtons.forEach((button) => {
    const isActive = button.dataset.tempUnit === unit;
    button.classList.toggle("is-active", isActive);
    button.setAttribute("aria-pressed", isActive.toString());
  });
  renderTemperature();
}

function setRainVisual(isWet) {
  if (!rainVisual) {
    return;
  }

  rainVisual.src = isWet ? rainVisual.dataset.wetSrc : rainVisual.dataset.drySrc;
  rainVisual.alt = isWet ? "Rain detected alien weather station scene" : "Dry alien weather station scene";
  rainVisual.classList.toggle("is-wet", isWet);
}

function updateDemoTelemetry() {
  tick += 1;

  const temperature = 22.8 + Math.sin(tick / 5) * 1.2;
  const humidity = 46 + Math.cos(tick / 6) * 4;
  const pressure = 1012.4 + Math.sin(tick / 8) * 2.1;
  const co2 = 612 + Math.round(Math.cos(tick / 4) * 38);
  const isWet = tick % 23 > 17;

  latestTemperatureC = temperature;
  renderTemperature();
  setText(metricNodes.humidity, Math.round(humidity));
  setText(metricNodes.pressure, pressure.toFixed(1));
  setText(metricNodes.co2, co2);
  setText(rainState, isWet ? "Rain detected" : "Dry surface");
  setRainVisual(isWet);
  setText(healthIndex, `${97 + (tick % 3)}%`);
  setText(lastSync, new Date().toLocaleTimeString([], { hour: "2-digit", minute: "2-digit", second: "2-digit" }));
}

temperatureUnitButtons.forEach((button) => {
  button.addEventListener("click", () => setTemperatureUnit(button.dataset.tempUnit));
});

setTemperatureUnit("F");
updateDemoTelemetry();
window.setInterval(updateDemoTelemetry, 2200);
