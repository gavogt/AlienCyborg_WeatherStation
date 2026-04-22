const metricNodes = {
  temperature: document.querySelector('[data-metric="temperature"]'),
  humidity: document.querySelector('[data-metric="humidity"]'),
  pressure: document.querySelector('[data-metric="pressure"]'),
  co2: document.querySelector('[data-metric="co2"]')
};

const rainState = document.getElementById("rainState");
const rainVisual = document.getElementById("rainVisual");
const lastSync = document.getElementById("lastSync");
const healthIndex = document.getElementById("healthIndex");

let tick = 0;

function setText(node, value) {
  if (node) {
    node.textContent = value;
  }
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

  setText(metricNodes.temperature, temperature.toFixed(1));
  setText(metricNodes.humidity, Math.round(humidity));
  setText(metricNodes.pressure, pressure.toFixed(1));
  setText(metricNodes.co2, co2);
  setText(rainState, isWet ? "Rain detected" : "Dry surface");
  setRainVisual(isWet);
  setText(healthIndex, `${97 + (tick % 3)}%`);
  setText(lastSync, new Date().toLocaleTimeString([], { hour: "2-digit", minute: "2-digit", second: "2-digit" }));
}

updateDemoTelemetry();
window.setInterval(updateDemoTelemetry, 2200);
