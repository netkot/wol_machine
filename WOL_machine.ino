#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266Ping.h>
#include <WiFiUDP.h>
#include <ArduinoOTA.h>

// Here is your passwords and local network data
#include "config.h"

ESP8266WebServer server(80);
WiFiUDP udp;


// =========== SETUP =============================
void setup(void) {

  for (int i = 0; i < total_gpios; i++)    {
    int nx_pin = String(pinGpios[i]).toInt();
    pinMode(nx_pin, OUTPUT);
  }

  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  ArduinoOTA.begin();

  server.on("/", handleForm);
  server.on("/ajax.php", handleAjax);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
}



// ================================
// FUNCTIONS
// ================================


void handleForm() {
  if (!server.authenticate(www_username, www_password)) {
    return server.requestAuthentication();
  }  else  {
    String out = "";
    String html_page = create_html_page (out);
    server.send(200, "text/html", html_page);
  }
}


void handleAjax() {
  if (!server.authenticate(www_username, www_password)) {
    return server.requestAuthentication();
  }  else  {
    String out;
    int led_pin;
    int led_state = HIGH;
    boolean its_pin = false;

    String post_led_num = server.arg("led_num");
    String post_led_state = server.arg("state");

    if (post_led_state == "1")   led_state = LOW;

    for (int i = 0; i < total_gpios; i++)    {
      if (post_led_num == String(pinGpios[i])) its_pin = true;
    }

    // its pin number
    if (its_pin)   {
      out = "GPIO " + post_led_num + " set to " + led_state;
      digitalWrite(post_led_num.toInt(), led_state);
    }
    // its macs number
    else    {
      // wake_comp (comp_mac);

      String mac = post_led_num;
      int bcast = String("255").toInt();

      IPAddress target_ip;
      target_ip = WiFi.localIP();
      target_ip[3] = bcast;

      byte target_mac[6];
      macStringToBytes(mac, target_mac);

      sendWOL(target_ip, target_mac);
      out = "WOL sent to " + target_ip.toString() + " " + mac;
    }
    server.send(200, "text/plain", out);
  }
}


void handleNotFound() {
  if (!server.authenticate(www_username, www_password)) {
    return server.requestAuthentication();
  }  else  {
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    server.send(404, "text/plain", message);
  }
}

String show_post ()
{
  String message = "";
  message += "==============================<br />";
  message += "Show request data<br /><br />";
  message += "Method:<br />";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "<br />Arguments:<br />";
  message += server.args();
  message += "<br /><br />";
  for (uint8_t i = 0; i < server.args(); i++)  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "<br />";
  }
  message += "==============================<br />";
  return message;
}



String create_html_page (String content)  {
  String out = "<html><title>ESP 8266 Server</title><meta name=viewport content='width=device-width, initial-scale=1'><script src='https://code.jquery.com/jquery-3.1.0.min.js'  integrity='sha256-cCueBR6CsyA4/9szpPfrX3s49M9vUU5BgtiJj06wt/s='  crossorigin='anonymous'></script><style>body{font-family: Georgia;text-align: center;padding: 20px;}button{display: block;margin: 20px auto;padding: 20px 20px;border: 1px #ccc solid;border-radius: 6px;background: linear-gradient(to bottom, #fff, #ccc);cursor: pointer;font-size: 18px;width: 100%;max-width: 500px;}button:active{background: linear-gradient(to bottom, #ccc, #fff);}button.active{background: linear-gradient(to bottom, #fff, #f1d171);box-shadow: 0 0 10px #ffc107;}button:focus{outline: none;}</style><body>";

  int nx_pin;
  int nx_pin_value;
  String pin_class;

  String nx_ip;

  out += "<h2>GPIOs:</h2>";

  for (int i = 0; i < total_gpios; i++)    {
    nx_pin = String(pinGpios[i]).toInt();
    nx_pin_value = digitalRead (nx_pin);

    pin_class = (nx_pin_value == 0) ? ("active") : ("");
    out += "<button rel=" + String (pinGpios[i]) + " class=" + pin_class + ">" + String (pinNames[i]) + "</button>";
  }

  out += "<h2>Computers:</h2>";

  for (int i = 0; i < total_comps; i++)    {
    pin_class = (Ping.ping(compIps[i])) ? ("active") : ("");
    out += "<button rel='" + String (compMacs[i]) + "' class='" + pin_class + "'>" + String (compIps[i]) + "</button>";
  }

  out += content;
  out += "<div class=mess></div><script>$(document).ready(function (){$('button').click(function(){var led_num = $(this).attr('rel');var state  = ($(this).hasClass('active')) ? '0' : '1';$(this).toggleClass('active');$.post('/ajax.php',{led_num: led_num, state: state}, function(response){$('.mess').text(response).animate({opacity: 0}, 2000, function() {$(this).text('').css('opacity',1)});});});});</script></body></html>";
  return out;
}




// Send a Wake-On-LAN packet for the given MAC address, to the given IP  address. Often the IP address will be the local broadcast.
// ========================================================
void sendWOL(const IPAddress ip, const byte mac[]) {
  digitalWrite(LED_BUILTIN, HIGH);
  byte preamble[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  udp.beginPacket(ip, 9);
  udp.write(preamble, 6);
  for (uint8 i = 0; i < 16; i++) {
    udp.write(mac, 6);
  }
  udp.endPacket();
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
}

byte valFromChar(char c) {
  if (c >= 'a' && c <= 'f') return ((byte) (c - 'a') + 10) & 0x0F;
  if (c >= 'A' && c <= 'F') return ((byte) (c - 'A') + 10) & 0x0F;
  if (c >= '0' && c <= '9') return ((byte) (c - '0')) & 0x0F;
  return 0;
}

/*
  Very simple converter from a String representation of a MAC address to
  6 bytes. Does not handle errors or delimiters, but requires very little
  code space and no libraries.
*/
void macStringToBytes(String mac, byte *bytes) {
  mac.replace("-", "");
  if (mac.length() >= 12) {
    for (int i = 0; i < 6; i++) {
      bytes[i] = (valFromChar(mac.charAt(i * 2)) << 4) | valFromChar(mac.charAt(i * 2 + 1));
    }
  } else {
    Serial.println("Incorrect MAC format.");
  }
}