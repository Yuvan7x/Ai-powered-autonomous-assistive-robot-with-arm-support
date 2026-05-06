#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

/* ================= WIFI ================= */
const char *ssid     = "Admin";
const char *password = "12344321";

WebServer server(80);

/* ================= WIFI RECONNECT ================= */
unsigned long lastReconnectAttempt = 0;
const long reconnectInterval = 10000;

void connectToWiFi() {
  Serial.println("Connecting WiFi...");
  WiFi.disconnect(true);
  delay(500);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(500);
    Serial.print(".");
    retry++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ Connected!");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ Failed");
  }
}

/* ================= SERVOS ================= */
Servo baseServo, clipServo, lowerServo, upperServo;

int BASE_MIN = 0,   BASE_MAX = 180;
int CLIP_MIN = 0,   CLIP_MAX = 80;
int LOWER_MIN = 10, LOWER_MAX = 100;
int UPPER_MIN = 40, UPPER_MAX = 170;

/* ================= MOTOR ================= */
#define ENA 25
#define IN1 26
#define IN2 27
#define ENB 14
#define IN3 12
#define IN4 13

int motorSpeed = 180;

const int freq = 1000;
const int resolution = 8;

void setupPWM() {
  ledcAttach(ENA, freq, resolution);
  ledcAttach(ENB, freq, resolution);
}

void setSpeed(int speed) {
  ledcWrite(ENA, speed);
  ledcWrite(ENB, speed);
}

void stopMotor() {
  setSpeed(0);
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

void forward() {
  setSpeed(motorSpeed);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void backward() {
  setSpeed(motorSpeed);
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}

void left() {
  setSpeed(motorSpeed);
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void right() {
  setSpeed(motorSpeed);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}

/* ================= WEB PAGE ================= */
String page() {
  return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta charset="UTF-8">

<style>
body{
  font-family: 'Segoe UI';
  background: radial-gradient(circle,#0f2027,#000);
  color:white;
  text-align:center;
}

.card{
  margin:15px;
  padding:20px;
  border-radius:20px;
  background: rgba(255,255,255,0.05);
}

/* D-PAD */
.dpad{
  display:flex;
  flex-direction:column;
  align-items:center;
  gap:10px;
}

.middle-row{
  display:flex;
  gap:10px;
}

.btn{
  width:70px;
  height:70px;
  border-radius:15px;
  border:none;
  font-size:24px;
  background: linear-gradient(145deg,#00c6ff,#0072ff);
  color:white;
}

.btn:active{
  transform: scale(0.9);
}

.stop{
  background:#ff4b5c;
}

input{width:90%;}
.speed{color:#00c6ff;}

</style>
</head>

<body>

<h2>🦾 ARM & ROBO</h2>

<div class="card">
<h3>Movement</h3>

<div class="dpad">

  <button class="btn"
    ontouchstart="cmd('f')" ontouchend="cmd('s')"
    onmousedown="cmd('f')" onmouseup="cmd('s')">▲</button>

  <div class="middle-row">
    <button class="btn"
      ontouchstart="cmd('l')" ontouchend="cmd('s')"
      onmousedown="cmd('l')" onmouseup="cmd('s')">◀</button>

    <button class="btn stop" onclick="cmd('s')">■</button>

    <button class="btn"
      ontouchstart="cmd('r')" ontouchend="cmd('s')"
      onmousedown="cmd('r')" onmouseup="cmd('s')">▶</button>
  </div>

  <button class="btn"
    ontouchstart="cmd('b')" ontouchend="cmd('s')"
    onmousedown="cmd('b')" onmouseup="cmd('s')">▼</button>

</div>

</div>

<div class="card">
<h3>Speed</h3>
<div class="speed" id="s">Speed: 180 km/h</div>
<input type="range" min="0" max="255" value="180" oninput="setSpeed(this.value)">
</div>

<div class="card">
<h3>Arm</h3>

Base
<input type="range" min="0" max="180" oninput="servo('b',this.value)">

Upper
<input type="range" min="40" max="170" oninput="servo('u',this.value)">

Lower
<input type="range" min="10" max="100" oninput="servo('l',this.value)">

Clipper
<input type="range" min="0" max="80" oninput="servo('c',this.value)">

</div>

<script>
function cmd(x){ fetch('/cmd?m='+x); }
function servo(j,v){ fetch('/servo?j='+j+'&v='+v); }

function setSpeed(v){
 document.getElementById("s").innerHTML="Speed: "+v+" km/h";
 fetch('/speed?v='+v);
}

document.addEventListener("touchend", ()=>cmd('s'));
</script>

</body>
</html>
)rawliteral";
}

/* ================= HANDLERS ================= */
void handleRoot(){ server.send(200,"text/html",page()); }

void handleCmd(){
  String m=server.arg("m");
  if(m=="f") forward();
  else if(m=="b") backward();
  else if(m=="l") left();
  else if(m=="r") right();
  else stopMotor();
  server.send(200,"text/plain","OK");
}

void handleServo(){
  String j=server.arg("j");
  int v=server.arg("v").toInt();

  if(j=="b") baseServo.write(constrain(v,BASE_MIN,BASE_MAX));
  if(j=="c") clipServo.write(constrain(v,CLIP_MIN,CLIP_MAX));
  if(j=="l") lowerServo.write(constrain(v,LOWER_MIN,LOWER_MAX));
  if(j=="u") upperServo.write(constrain(v,UPPER_MIN,UPPER_MAX));

  server.send(200,"text/plain","OK");
}

void handleSpeed(){
  motorSpeed = server.arg("v").toInt();
  server.send(200,"text/plain","OK");
}

/* ================= SETUP ================= */
void setup(){
  Serial.begin(115200);

  connectToWiFi();

  baseServo.attach(18);
  clipServo.attach(19);
  lowerServo.attach(21);
  upperServo.attach(22);

  pinMode(IN1,OUTPUT); pinMode(IN2,OUTPUT);
  pinMode(IN3,OUTPUT); pinMode(IN4,OUTPUT);

  setupPWM();

  server.on("/",handleRoot);
  server.on("/cmd",handleCmd);
  server.on("/servo",handleServo);
  server.on("/speed",handleSpeed);

  server.begin();
}

/* ================= LOOP ================= */
void loop(){
  server.handleClient();

  if(WiFi.status()!=WL_CONNECTED &&
     millis()-lastReconnectAttempt>reconnectInterval){
    connectToWiFi();
    lastReconnectAttempt=millis();
  }
}