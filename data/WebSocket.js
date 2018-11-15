const rSlider = document.getElementById('r');
const gSlider = document.getElementById('g');
const bSlider = document.getElementById('b');

var rainbowEnable = false;
var connection = new WebSocket('ws://' + location.hostname + ':81/', ['arduino']);
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

function sendR() {
  connection.send("r" + r.value);
}

function sendG() {
  connection.send("g" + g.value);
}

function sendB() {
  connection.send("b" + b.value);
}

function sendRGB() {
  console.log('RGB: ' + JSON.stringify(jsonObj));
  connection.send(JSON.stringify(jsonObj));
}

function rainbowEffect() {
  rainbowEnable = !rainbowEnable;
  if (rainbowEnable) {
    connection.send("R");
    document.getElementById('rainbow').style.backgroundColor = '#00878F';
    rSlider.className = 'disabled';
    gSlider.className = 'disabled';
    bSlider.className = 'disabled';
    rSlider.disabled = true;
    gSlider.disabled = true;
    bSlider.disabled = true;
  } else {
    connection.send("N");
    document.getElementById('rainbow').style.backgroundColor = '#999';
    rSlider.className = 'enabled';
    gSlider.className = 'enabled';
    bSlider.className = 'enabled';
    rSlider.disabled = false;
    gSlider.disabled = false;
    bSlider.disabled = false;
    sendRGB();
  }
}
