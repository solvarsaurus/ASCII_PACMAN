#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>
#include <Preferences.h>

WebServer server(80);
WiFiManager wm;
Preferences preferences;

// --- 1. THE MONOCHROME TERMINAL CORE ---
const char* hostPage PROGMEM = R"=====(
<!DOCTYPE html><html><head><meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
<title>PACMAN TERMINAL</title>
<style>
  :root { --p-green: #33ff00; --p-bg: #050505; }
  body { background: var(--p-bg); color: var(--p-green); font-family: 'Courier New', monospace; text-align: center; margin: 0; overflow: hidden; }
  .crt::after { content: " "; display: block; position: fixed; top: 0; left: 0; bottom: 0; right: 0; background: linear-gradient(rgba(18, 16, 16, 0) 50%, rgba(0, 0, 0, 0.15) 50%), linear-gradient(90deg, rgba(255, 0, 0, 0.05), rgba(0, 255, 0, 0.02), rgba(0, 0, 255, 0.05)); z-index: 10; background-size: 100% 3px, 3px 100%; pointer-events: none; }
  .screen-border { border: 12px solid #1a1a1a; border-radius: 20px; display: inline-block; padding: 5px; background: #000; box-shadow: 0 0 40px #33ff0022; margin-top: 10px; }
  #gameCanvas { background: #000; image-rendering: pixelated; width: 300px; height: 350px; }
  .btn { width: 70px; height: 70px; background: #111; border: 2px solid var(--p-green); color: var(--p-green); font-size: 24px; border-radius: 50%; cursor: pointer; outline: none; }
  .btn:active { background: var(--p-green); color: #000; }
  .blink { animation: blinker 1s linear infinite; }
  @keyframes blinker { 50% { opacity: 0; } }
  .hidden { display: none; }
</style></head><body class="crt">

<div id="splash-view">
  <pre style="font-size: 10px; line-height: 9px; margin-top: 50px;">
   .-------------------------------------------.
   |  ____   _    ____ __  __   _    _   _     |
   | |  _ \ / \  / ___|  \/  | / \  | \ | |    |
   | | |_) / _ \| |   | |\/| |/ _ \ |  \| |    |
   | |  __/ ___ | |___| |  | / ___ \| |\  |    |
   | |_| /_/   \_\____|_|  |/_/   \_\_| \_|    |
   |                                           |
   '-------------------------------------------'
  </pre>
  <pre style="font-size: 12px; color: #33ff00;">
      .--.   .--.   .--.   .--.
     | o  | | o  | | o  | | o  |
     |    | |    | |    | |    |
      '--'   '--'   '--'   '--'
  </pre>
  <h2 class="blink" style="margin-top: 30px;">[ PRESS START ]</h2>
  <button class="btn" style="width: 150px; border-radius: 10px;" onclick="startGame()">START</button>
</div>

<div id="game-view" class="hidden">
  <div style="font-size: 8px; line-height: 7px; margin-top: 10px; white-space: pre;">
   [PACMAN-OS v1.5] [LEVEL: <span id="lvlNum">1</span>] [espcatgames.local]
  </div>
  <div class="screen-border"><canvas id="gameCanvas" width="200" height="210"></canvas></div>
  <div id="statusMsg" style="font-weight:bold; margin-top:5px;">SCORE: 0 | BEST: ---</div>
  <div style="display: grid; grid-template-columns: repeat(3, 75px); gap: 10px; justify-content: center; margin-top: 15px;">
    <div></div><button class="btn" onmousedown="it(38)">▲</button><div></div>
    <button class="btn" onmousedown="it(37)">◀</button>
    <button class="btn" onclick="location.reload()" style="font-size:12px">EXIT</button>
    <button class="btn" onmousedown="it(39)">▶</button>
    <div></div><button class="btn" onmousedown="it(40)">▼</button><div></div>
  </div>
</div>

<script>
  const canvas = document.getElementById('gameCanvas');
  const ctx = canvas.getContext('2d');
  const ts = 10;
  let score = 0, highScore = 0, level = 1, gameSpeed = 150;
  let pelletsRemaining = 0, gameOver = false, powerTimer = 0, isPaused = true;

  const mazeTemplate = [
    [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1],
    [1,3,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,3,1],
    [1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1],
    [1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1],
    [1,2,1,1,2,1,2,1,1,1,1,1,1,2,1,2,1,1,2,1],
    [1,2,2,2,2,1,2,2,2,1,1,2,2,2,1,2,2,2,2,1],
    [1,1,1,1,2,1,1,1,0,1,1,0,1,1,1,2,1,1,1,1],
    [1,1,1,1,2,1,0,0,0,0,0,0,0,0,1,2,1,1,1,1],
    [1,1,1,1,2,1,0,1,1,0,0,1,1,0,1,2,1,1,1,1],
    [0,0,0,0,2,0,0,1,0,0,0,0,1,0,0,2,0,0,0,0],
    [1,1,1,1,2,1,0,1,1,1,1,1,1,0,1,2,1,1,1,1],
    [1,1,1,1,2,1,0,0,0,0,0,0,0,0,1,2,1,1,1,1],
    [1,1,1,1,2,1,0,1,1,1,1,1,1,0,1,2,1,1,1,1],
    [1,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,1],
    [1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1],
    [1,2,2,1,2,2,2,2,2,0,0,2,2,2,2,2,1,2,2,1],
    [1,1,2,1,2,1,2,1,1,1,1,1,1,2,1,2,1,2,1,1],
    [1,2,2,2,2,1,2,2,2,1,1,2,2,2,1,2,2,2,2,1],
    [1,2,1,1,1,1,1,1,2,1,1,2,1,1,1,1,1,1,2,1],
    [1,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,1],
    [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]
  ];

  let maze = JSON.parse(JSON.stringify(mazeTemplate));
  let player = { x: 10, y: 15, curDir: null };
  let blinky = { x: 9, y: 8, isFrightened: false };

  function startGame() {
    document.getElementById('splash-view').classList.add('hidden');
    document.getElementById('game-view').classList.remove('hidden');
    isPaused = false;
    initGame();
    draw();
  }

  function initGame() {
    pelletsRemaining = maze.flat().filter(x => x === 2 || x === 3).length;
    fetch('/api/highscore').then(r => r.json()).then(d => { highScore = d.score; updateStatus(); });
  }

  function updateStatus() {
    document.getElementById('statusMsg').innerText = gameOver ? "KERNEL PANIC: CAUGHT" : `SCORE: ${score} | BEST: ${highScore}`;
  }

  function update() {
    if(gameOver || isPaused) return;
    if(powerTimer > 0) powerTimer--;

    let tx = player.x, ty = player.y;
    if(player.curDir == 38) ty--; else if(player.curDir == 40) ty++;
    if(player.curDir == 37) tx--; else if(player.curDir == 39) tx++;

    if(maze[ty] && maze[ty][tx] !== 1) {
      player.x = tx; player.y = ty;
      if(maze[ty][tx] === 2 || maze[ty][tx] === 3) {
        if(maze[ty][tx] === 3) powerTimer = 40;
        maze[ty][tx] = 0; score += 10; pelletsRemaining--;
        if(pelletsRemaining <= 0) {
           level++;
           document.getElementById('lvlNum').innerText = level;
           maze = JSON.parse(JSON.stringify(mazeTemplate));
           player = { x: 10, y: 15, curDir: null };
           gameSpeed = Math.max(60, 150 - (level * 15));
        }
        updateStatus();
      }
    }

    let bx = blinky.x, by = blinky.y;
    let ops = [{x:bx, y:by-1}, {x:bx, y:by+1}, {x:bx-1, y:by}, {x:bx+1, y:by}];
    let valid = ops.filter(o => maze[o.y] && maze[o.y][o.x] !== 1);
    let mult = (powerTimer > 0) ? -1 : 1;
    valid.sort((a,b) => mult * (Math.sqrt((a.x-player.x)**2+(a.y-player.y)**2) - Math.sqrt((b.x-player.x)**2+(b.y-player.y)**2)));
    if(valid[0]) { blinky.x = valid[0].x; blinky.y = valid[0].y; }

    if(blinky.x === player.x && blinky.y === player.y) {
      if(powerTimer > 0) { blinky.x = 9; blinky.y = 8; score += 200; }
      else { gameOver = true; if(score > highScore) fetch('/api/setscore?score='+score, {method:'POST'}); updateStatus(); }
    }
  }

  function draw() {
    ctx.fillStyle = "#000"; ctx.fillRect(0, 0, canvas.width, canvas.height);
    for(let r=0; r<maze.length; r++) {
      for(let c=0; c<maze[r].length; c++) {
        if(maze[r][c] === 1) { ctx.strokeStyle = "#33ff00"; ctx.strokeRect(c*ts, r*ts, ts, ts); }
        else if(maze[r][c] === 2) { ctx.fillStyle = "#33ff00"; ctx.fillRect(c*ts+4, r*ts+4, 2, 2); }
        else if(maze[r][c] === 3) { ctx.beginPath(); ctx.arc(c*ts+5, r*ts+5, 3, 0, 2*Math.PI); ctx.fill(); }
      }
    }
    ctx.fillStyle = "#33ff00"; ctx.beginPath(); ctx.arc(player.x*ts+5, player.y*ts+5, 4, 0, 2*Math.PI); ctx.fill();
    ctx.strokeStyle = "#33ff00"; ctx.lineWidth = (powerTimer > 0 ? 1 : 3);
    ctx.beginPath(); ctx.arc(blinky.x*ts+5, blinky.y*ts+5, 4, 0, 2*Math.PI); ctx.stroke();
    update();
    setTimeout(() => requestAnimationFrame(draw), gameSpeed);
  }
  function it(k) { player.curDir = k; }
</script></body></html>)=====";

void setup() {
  Serial.begin(115200);
  wm.autoConnect("ESP games");
  MDNS.begin("espcatgames");
  preferences.begin("pacman-data", false);
  server.on("/", []() { server.send_P(200, "text/html", hostPage); });
  server.on("/api/highscore", []() { server.send(200, "application/json", "{\"score\":" + String(preferences.getInt("highscore", 0)) + "}"); });
  server.on("/api/setscore", []() { 
    if(server.hasArg("score")) preferences.putInt("highscore", server.arg("score").toInt());
    server.send(200); 
  });
  server.begin();
}
void loop() { server.handleClient(); }