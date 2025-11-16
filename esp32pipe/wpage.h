const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head><title>ESP32 Test</title></head>

<body style="background-color:PaleTurquoise;">

<div id="led">
  <p style="border: 1px solid black">The ESP32 demo</p><BR>
  <button type="button" style="background-color:DodgerBlue;" onclick="sendLed(1)">Onboard LED on</button>
  <button type="button" style="background-color:SteelBlue;" onclick="sendLed(0)">Onboard LED off</button>
	&nbsp;&nbsp;&nbsp;
	Now : <span id="ledState">unavailable</span><BR><BR>
</div>
<hr><br>

<div id="dat">
  Variable from server : <span id="sVal">NA</span>&nbsp;&nbsp;&nbsp;&nbsp;
  <button type="button" id="sBtn" style="background-color:PaleTurquoise;">Onboard button</button><br><br>
  <input type='text' id='cVal' name='cVal' style='font-size:20px;width:150px' value='42'/>&nbsp;&nbsp;
  <button type="button" onclick="sendVal(cVal.value)">Send to server</button><br><br><hr>
  <span id="dbgStr">-</span>
</div>

<script>

function sendLed(led) {
  let xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("ledState").innerHTML =
      this.responseText;  //expect response for this action
    }
  };
  xhttp.open("GET", "setLED?LEDstate="+led, true);
  xhttp.send();
}

function sendVal(str) {
  let xhttp = new XMLHttpRequest();
  xhttp.open("GET", "setVal?valStr="+str, true);
  xhttp.send(); //no response expected
}

let ucnt = 0;
let buttonState = 1;
let xhr = new XMLHttpRequest();
    xhr.timeout=200;

function xhrDataRequest() {
  xhr.open("GET", "data", true);
  xhr.send();
}

// Call a function repetatively with defined interval
setInterval(xhrDataRequest,100); //update rate in ms

xhr.onload = function() { //http callback
  if (this.readyState == 4 && this.status == 200) {
    let msg = this.responseText;  
    let i_data = msg.indexOf("data=");

    if (i_data >= 0) //Expected "data="+String(to_webpage)+"|"+digitalRead(BUTTON);
    {
      const sep = msg.indexOf('|');
      const val = parseInt(msg.substring(i_data+5,sep)); //Str to int example
      const btn = parseInt(msg.substring(sep+1));
      document.getElementById("sVal").innerHTML = val.toString(); //Int to Str example
      if (btn != buttonState) {
        buttonState = btn;
        if (btn == 0) { document.getElementById("sBtn").style.backgroundColor = "OrangeRed"; }
        else document.getElementById("sBtn").style.backgroundColor = "PaleTurquoise";
      }
    }
    
    document.getElementById("dbgStr").innerHTML = "// "+ucnt.toString()+" by response : "+msg; 
    ucnt++; 
  }
};

</script>
</body>
</html>
)=====";
