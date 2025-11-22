char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <title>Golf Cart Settings</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background: #f4f4f4;
      padding: 20px;
      max-width: 350px;
      margin: 40px auto;
    }
    .container {
      background: white;
      padding: 25px;
      border-radius: 10px;
      box-shadow: 0 2px 10px rgba(0,0,0,0.1);
    }
    h1 {
      text-align: center;
      color: #333;
    }
    .light-control {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin: 15px 0;
      padding: 10px;
      background: #f9f9f9;
      border-radius: 8px;
      border-left: 5px solid;
    }
    .light-control label {
      font-weight: bold;
      flex: 1;
    }
    .light-control input[type="color"] {
      width: 60px;
      height: 40px;
      border: none;
      border-radius: 6px;
      cursor: pointer;
    }
    .blink-control {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin: 15px 0;
      padding: 10px;
      background: #f9f9f9;
      border-radius: 8px;
      border-left: 5px solid;
    }
    .blink-control label {
      font-weight: bold;
      flex: 1;
    }
    .blink-control input[type="text"] {
      width: 40px;
      height: 30px;
      border: none;
      border-radius: 6px;
      cursor: pointer;
    }
    .btn {
      display: block;
      margin: 20px auto 0;
      padding: 10px 20px;
      background: #007bff;
      color: white;
      border: none;
      border-radius: 6px;
      cursor: pointer;
      font-size: 16px;
    }
    .btn:hover {
      background: #0056b3;
    }
  </style>
</head>
<body>

  <div class="container">
    <h1>Golf Cart Settings</h1>

    <div class="light-control" style="border-left-color: #ff0000;">
      <label for="brakes">Brake lights</label>
      <input type="color" id="brakes" value="#ff0000" />
    </div>

    <div class="light-control" style="border-left-color: #c0c0c0;">
      <label for="reverse">Reverse lights</label>
      <input type="color" id="reverse" value="#c0c0c0" />
    </div>

    <div class="light-control" style="border-left-color: #ff7f00;">
      <label for="turn">Turn signal</label>
      <input type="color" id="turn" value="#ff7f00" />
    </div>

    <div class="light-control" style="border-left-color: #000080;">
      <label for="running">Running lights</label>
      <input type="color" id="running" value="#000080" />
    </div>

    <div class="blink-control" style="border-left-color: #00ff00;">
      <label for="blinkrate">Blink rate</label>
      <input type="text" inputmode="number" font-size="16px" id="blinkrate" name="blinkrate" step="1" min="1" value="900" />
    </div>

    <button class="btn" onclick="save()">Save Settings</button>
    <button class="btn" onclick="resetColors()">Reset to Defaults</button>
  </div>

  <script>
    function save() {
      const data = {
        brakes:  document.getElementById('brakes').value,
        reverse: document.getElementById('reverse').value,
        turn:    document.getElementById('turn').value,
        running: document.getElementById('running').value,
        blinkrate: document.getElementById('blinkrate').value
      };
      fetch('/save', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify(data)
      })
      .then(r => r.text())
      .then(alert);
    }
    // Reset all to default colors
    function resetColors() {
      document.getElementById('brakes').value = '#ff0000';
      document.getElementById('reverse').value = '#c0c0c0';
      document.getElementById('turn').value = '#ff7f00';
      document.getElementById('running').value = '#000080';
      document.getElementById('blinkrate').value = '900';
    }
  </script>

</body>
</html>
)rawliteral";
