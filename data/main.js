var gateway = `ws://${window.location.hostname}/ws`;
var webSocket;
window.addEventListener('load' , onload);

function onload() {
    initWebSocket();
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    webSocket = new WebSocket(gateway);
    webSocket.onopen = onOpen;
    webSocket.onclose = onClose;
    webSocket.onmessage = onMessage;
}

function onOpen() {
    webSocket.send("getValues")
    console.log('Connection opened');
}

function onClose() {
    console.log('Connection Closed');
    setTimeout(initWebSocket , 2000);
}

function onMessage(event) {
    let sensorRes = JSON.parse(event.data);

    // Nếu gói tin là sensor values (temperature..dust)
    if (sensorRes.temperature !== undefined) {
        console.log("Nhiet do: " , sensorRes.temperature);
        console.log("Do Am: " , sensorRes.humidity);
        console.log("Nong Do Bui: " , sensorRes.dust)
        document.getElementById("temperature").innerHTML = sensorRes.temperature;
        document.getElementById("humidity").innerHTML = sensorRes.humidity;
        document.getElementById("dust").innerHTML = sensorRes.dust;
    }

    // Nếu gói tin là slider values (sensor1..3)
    if (sensorRes.sensor1 !== undefined) {
        document.getElementById("sensor1").innerHTML = sensorRes.sensor1;
        document.getElementById("slider1").value = sensorRes.sensor1;

        document.getElementById("sensor2").innerHTML = sensorRes.sensor2;
        document.getElementById("slider2").value = sensorRes.sensor2;

        document.getElementById("sensor3").innerHTML = sensorRes.sensor3;
        document.getElementById("slider3").value = sensorRes.sensor3;
    }
}

function updateSliderPWM(element) {
    var sliderNumber = element.id.charAt(element.id.length - 1);
    var sliderValue = document.getElementById(element.id).value;
    document.getElementById("sensor" + sliderNumber).innerHTML = sliderValue;
    console.log(sliderValue);
    webSocket.send(sliderNumber + "s" + sliderValue);
}