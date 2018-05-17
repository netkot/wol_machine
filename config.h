const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_pass";

const char* www_username = "your_login";
const char* www_password = "your_pass";



// GPIOs
// ============================
char* pinNames[] = {
  "GPIO 2 (internal LED)",
  "GPIO 5"
};
char* pinGpios[] = {
  "2",
  "5"
};
int total_gpios = 2;


// Computers
// ============================
char* compIps[] = {
  "192.168.1.10",
  "192.168.1.12"
};
char* compMacs[] = {
  "FF-FF-FF-FF-FF-FF",
  "FF-FF-FF-FF-FF-FF"
};
char* compNames[] = {
  "My-PC",
  "Printer-PC",
};
int total_comps = 2;
