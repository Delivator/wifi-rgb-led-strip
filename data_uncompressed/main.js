const colorPicker = new iro.ColorPicker("#color-picker-container", {
  color: {
    h: Math.floor(Math.random() * 360),
    s: 100,
    v: 100
  }
});

const connection = new WebSocket("ws://" + location.hostname + ":81/", ["arduino"]);

const rSlider = document.getElementById("rSlider");
const gSlider = document.getElementById("gSlider");
const bSlider = document.getElementById("bSlider");
const delaySlider = document.getElementById("delaySlider");

let preventEvent = false;

connection.onopen = function () {
  connection.send("Connect " + new Date());
};
connection.onerror = function (error) {
  console.log("WebSocket Error ", error);
};
connection.onmessage = function (e) {
  console.log("Server: ", e.data);
};
connection.onclose = function () {
  console.log("WebSocket connection closed");
};

rSlider.addEventListener("mousedown", () => { preventEvent = true; });
rSlider.addEventListener("mouseup", () => { preventEvent = false; });
gSlider.addEventListener("mousedown", () => { preventEvent = true; });
gSlider.addEventListener("mouseup", () => { preventEvent = false; });
bSlider.addEventListener("mousedown", () => { preventEvent = true; });
bSlider.addEventListener("mouseup", () => { preventEvent = false; });

colorPicker.on("color:change", (color) => {
  if (preventEvent) return;
  let r = color.rgb.r,
    g = color.rgb.g,
    b = color.rgb.b;
  rSlider.value = r;
  gSlider.value = g;
  bSlider.value = b;
  sendColors(r, g, b);
});

function rgbInput() {
  let r = rSlider.value,
    g = gSlider.value,
    b = bSlider.value;
  colorPicker.color.rgb = { r, g, b };
  sendColors(r, g, b);
}

function delayInput() {
  let delay = delaySlider.value;
  if (connection.readyState === connection.OPEN) connection.send("d" + delay);
}

function sendColors(r, g, b) {
  r = Math.floor((1023 / 255) * r);
  g = Math.floor((1023 / 255) * g);
  b = Math.floor((1023 / 255) * b);

  let rgb = r << 20 | g << 10 | b;

  if (connection.readyState === connection.OPEN) connection.send("#" + rgb.toString(16));
}

function animationOff() {
  if (connection.readyState === connection.OPEN) connection.send("a0");
}

function fade() {
  if (connection.readyState === connection.OPEN) connection.send("a1");
}

function blink() {
  if (connection.readyState === connection.OPEN) connection.send("a2");
}

function breathing() {
  if (connection.readyState === connection.OPEN) connection.send("a3");
}

function intToHex(rgb) {
  let hex = Number(rgb).toString(16);
  if (hex.length < 2) hex = "0" + hex;
  return hex;
}

function rgbToHex(r, g, b) {
  return intToHex(r) + intToHex(g) + intToHex(b);
}
