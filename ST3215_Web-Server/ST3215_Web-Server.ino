#include <WiFi.h>
#include <WebServer.h>
#include <SCServo.h>
#include <AccelStepper.h>

SMS_STS st;

// ======================
// WIFI ACCESS POINT
// ======================

const char* ssid = "Servo_Control";
const char* password = "12345678";

// ======================
// SERVO UART
// ======================

#define S_RXD 16
#define S_TXD 17

// ======================
// TMC2209 PINS
// ======================

#define STEP_PIN 23
#define DIR_PIN 22

// ======================
// ACCELSTEPPER
// ======================

AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

#define STEP_SIZE 400

long targetPosition = 0;

WebServer server(80);

// ======================
// HTML PAGE
// ======================

String webpage = R"=====(

<!DOCTYPE html>
<html>
<head>
<title>Servo Control</title>
<meta name="viewport" content="width=device-width, initial-scale=1">

<style>

body{
  font-family: Arial;
  text-align:center;
  background:#f0f0f0;
}

.card{
  background:white;
  width:300px;
  margin:20px auto;
  padding:20px;
  border-radius:10px;
  box-shadow:0px 0px 10px rgba(0,0,0,0.2);
}

input[type=range]{
  width:100%;
}

button{
  padding:15px;
  margin:10px;
  font-size:18px;
}

</style>

</head>

<body>

<h2>ESP32 Robot Control</h2>

<div class="card">
<h3>Servo 1</h3>
<input type="range" min="1350" max="3200" value="2350"
oninput="setServo(1,this.value)">
<p id="v1">1600</p>
</div>

<div class="card">
<h3>Servo 2</h3>
<input type="range" min="1200" max="2800" value="2000"
oninput="setServo(2,this.value)">
<p id="v2">2000</p>
</div>

<div class="card">
<h3>Servo 3</h3>
<input type="range" min="900" max="2600" value="1750"
oninput="setServo(3,this.value)">
<p id="v3">1000</p>
</div>

<div class="card">
<h3>Servo 4</h3>
<input type="range" min="0" max="1400" value="700"
oninput="setServo(4,this.value)">
<p id="v4">700</p>
</div>

<div class="card">

<h3>Stepper Motor</h3>

<label>Step Size</label><br><br>

<input type="number" id="stepSize" value="4000" min="1" max="10000">

<br><br>

<button onclick="moveStepper(-1)">
Forward
</button>

<button onclick="moveStepper(1)">
Reverse
</button>

</div>

<script>

function setServo(id,pos)
{
  document.getElementById("v"+id).innerHTML = pos;

  var xhttp = new XMLHttpRequest();

  xhttp.open("GET", "/set?id="+id+"&pos="+pos, true);

  xhttp.send();
}

// ONLY if any servo need to be rotated in reverse manner
function setServo_mirrored(id,pos)
{
  let mirrored = 5600 - pos;

  document.getElementById("v"+id).innerHTML = mirrored;

  var xhttp = new XMLHttpRequest();

  xhttp.open("GET", "/set?id=2&pos="+mirrored, true);

  xhttp.send();
}

// Stepper
const STEP_SIZE = 400;
function moveStepper(dir)
{
  let stepSize = document.getElementById("stepSize").value;

  let steps = dir * stepSize;

  var xhttp = new XMLHttpRequest();

  xhttp.open("GET", "/stepper?steps="+steps, true);

  xhttp.send();
}

</script>

</body>
</html>

)=====";

// ======================
// MOVE SERVO
// ======================

void moveServo(int id, int pos)
{
  // Safety limits

  int speed,acceleration;

  if(id == 1){
    pos = constrain(pos, 1350, 3200);
    speed = 3000;
    acceleration = 25;
  }

  else if(id == 2){
    pos = constrain(pos, 1200, 2800);
    speed = 1800;
    acceleration = 10;
  }

  else if(id == 3){
    pos = constrain(pos, 900, 2600);
    speed = 1800;
    acceleration = 10;
  }

  else if(id == 4){
    pos = constrain(pos, 0, 1400);
    speed = 1800;
    acceleration = 10;
  }

  // Smooth movement
  st.WritePosEx(id, pos, speed, acceleration);

  Serial.print("Servo ");
  Serial.print(id);
  Serial.print(" -> ");
  Serial.println(pos);
}

// ======================
// HANDLE ROOT PAGE
// ======================

void handleRoot()
{
  server.send(200, "text/html", webpage);
}

// ======================
// HANDLE SERVO REQUEST
// ======================

void handleSet()
{
  if(server.hasArg("id") && server.hasArg("pos"))
  {
    int id = server.arg("id").toInt();
    int pos = server.arg("pos").toInt();

    moveServo(id, pos);

    server.send(200, "text/plain", "OK");
  }
  else
  {
    server.send(400, "text/plain", "Bad Request");
  }
}

// ======================
// HANDLE STEPPER
// ======================

void handleStepper()
{
  if(server.hasArg("steps"))
  {
    long steps = server.arg("steps").toInt();

    // Relative movement
    stepper.move(steps);

    targetPosition = stepper.targetPosition();

    Serial.print("Move: ");
    Serial.print(steps);

    Serial.print(" | Target: ");
    Serial.println(targetPosition);

    server.send(200, "text/plain", "OK");
  }
  else
  {
    server.send(400, "text/plain", "Bad Request");
  }
}

// ======================
// SETUP
// ======================

void setup()
{
  Serial.begin(115200);

  // Servo UART
  Serial1.begin(1000000, SERIAL_8N1, S_RXD, S_TXD);

  st.pSerial = &Serial1;

  // Stepper setup
  stepper.setMaxSpeed(2500);

  stepper.setAcceleration(1200);

  stepper.setMinPulseWidth(2);

  stepper.setCurrentPosition(0);

  // Create WiFi AP
  WiFi.softAP(ssid, password);

  Serial.println();
  Serial.println("WiFi Access Point Started");

  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Web routes
  server.on("/", handleRoot);

  server.on("/set", handleSet);

  server.on("/stepper", handleStepper);

  server.begin();

  Serial.println("Web Server Started");
}

// ======================
// LOOP
// ======================

void loop()
{
  server.handleClient();

  stepper.run();
}