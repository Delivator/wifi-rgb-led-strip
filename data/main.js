const colorPicker = new iro.ColorPicker("#color-picker-container", {
  color: "#00ff00"
});

const connection = new WebSocket('ws://' + location.hostname + ':81/', ['arduino']);

const rSlider = document.getElementById("rSlider");
const gSlider = document.getElementById("gSlider");
const bSlider = document.getElementById("bSlider");

let enableFade = false;

function sliderInput() {
  enableFade = false;
  let r = rSlider.value,
    g = gSlider.value,
    b = bSlider.value;
  colorPicker.color.rgb = { r, g, b };
  sendColors(r, g, b);
}

colorPicker.on("color:change", (color) => {
  enableFade = false;
  let r = color.rgb.r,
    g = color.rgb.g,
    b = color.rgb.b;
  rSlider.value = r;
  gSlider.value = g;
  bSlider.value = b;
  sendColors(r, g, b);
});

function sendColors(r, g, b) {
  r = Math.floor((1023 / 255) * r);
  g = Math.floor((1023 / 255) * g);
  b = Math.floor((1023 / 255) * b);
  if (connection.readyState === connection.OPEN) {
    connection.send("r" + r);
    connection.send("g" + g);
    connection.send("b" + b);
  }
}

connection.onopen = function () {
  connection.send('Connect ' + new Date());
};
connection.onerror = function (error) {
  console.log('WebSocket Error ', error);
};
connection.onmessage = function (e) {
  console.log('Server: ', e.data);
};
connection.onclose = function () {
  console.log('WebSocket connection closed');
};

function toggleFade() {
  enableFade = !enableFade;
  if (enableFade) {
    connection.send("R");
  } else {
    connection.send("N");
  }
}

// function rainbowEffect() {
//   rainbowEnable = !rainbowEnable;
//   if (rainbowEnable) {
//     connection.send("R");
//     document.getElementById('rainbow').style.backgroundColor = '#00878F';
//     rSlider.className = 'disabled';
//     gSlider.className = 'disabled';
//     bSlider.className = 'disabled';
//     rSlider.disabled = true;
//     gSlider.disabled = true;
//     bSlider.disabled = true;
//   } else {
//     connection.send("N");
//     document.getElementById("rainbow").style.backgroundColor = "#999";
//     rSlider.className = 'enabled';
//     gSlider.className = 'enabled';
//     bSlider.className = 'enabled';
//     rSlider.disabled = false;
//     gSlider.disabled = false;
//     bSlider.disabled = false;
//     sendColors(0, 0, 0);
//   }
// }

function intToHex(rgb) {
  let hex = Number(rgb).toString(16);
  if (hex.length < 2) hex = "0" + hex;
  return hex;
}

function rgbToHex(r, g, b) {
  return intToHex(r) + intToHex(g) + intToHex(b);
}