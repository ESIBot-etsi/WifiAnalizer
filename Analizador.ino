#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "ESP8266WiFi.h"


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_MOSI   13
#define OLED_CLK   14
#define OLED_DC    5
#define OLED_CS    15
#define OLED_RESET 4

#define GRAPH_BASELINE (SCREEN_HEIGHT - 10)
#define GRAPH_HEIGHT (SCREEN_HEIGHT - 30)
#define CHANNEL_WIDTH (SCREEN_WIDTH / 16)
#define BANNER_HEIGHT 8

// RSSI RANGE
#define RSSI_CEILING -40
#define RSSI_FLOOR -100
#define NEAR_CHANNEL_RSSI_ALLOW -70
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

  

void setup()
{
  Serial.begin(115200);
  if(!display.begin(SSD1306_SWITCHCAPVCC))
  {
    Serial.println(F("Asignación de SSD1306 fallida"));
    for(;;);
  }
  
  display.clearDisplay();
  display.display();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.println("     .------------.    ");
  display.println("     | Analizador |    ");
  display.println("     |    Wifi    |    ");
  display.println("     |  ESP8266   |    ");
  display.println("     .------------.    ");
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
  display.println("[^]  ESIBot - 2023  [^]");
  display.display();

  // Poner WiFi en modo "station" y desconectarlo de cualquier AP al que estuviese conectado
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  delay(100);
}

void loop()
{
  uint8_t ap_count[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int32_t max_rssi[] = {-100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100};

  // WiFi.scanNetworks devolverá el número de redes encontradas
  int n = WiFi.scanNetworks();

  //Limpiamos lo de antes
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  if (n==0){
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    display.setTextSize(2);
    display.println("No se han encontrado redes Wifi");
    
  }
  else{
    //Hacer plot de la info encontrada
    for(int i = 0; i < n; i++){
      int32_t channel = WiFi.channel(i);
      int32_t rssi = WiFi.RSSI(i);
      int height = constrain(map(rssi, RSSI_FLOOR, RSSI_CEILING, 1, GRAPH_HEIGHT), 1, GRAPH_HEIGHT);

      //Estadísticas del canal
      ap_count[channel - 1]++;
      if (rssi > max_rssi[channel - 1]) {
        max_rssi[channel - 1] = rssi;
      }
      display.drawLine(channel * CHANNEL_WIDTH, GRAPH_BASELINE - height, (channel - 1) * CHANNEL_WIDTH, GRAPH_BASELINE + 1, SSD1306_WHITE);
      display.drawLine(channel * CHANNEL_WIDTH, GRAPH_BASELINE - height, (channel + 1) * CHANNEL_WIDTH, GRAPH_BASELINE + 1, SSD1306_WHITE);

      display.setTextColor(SSD1306_WHITE);
      display.setCursor((channel - 1) * CHANNEL_WIDTH, GRAPH_BASELINE - 10 - height);
      display.print(WiFi.SSID(i));
      display.print('(');
      display.print(rssi);
      display.print(')');
      if (WiFi.encryptionType(i) == ENC_TYPE_NONE) {
        display.print('*');
      }

      delay(10);
    }
  }
  //Dibujar ejes de la gráfica
  display.drawFastHLine(0, GRAPH_BASELINE, 320, SSD1306_WHITE);
  for (int i = 1; i <= 14; i++) {
    display.setTextColor(SSD1306_WHITE);
    display.setCursor((i * CHANNEL_WIDTH) - ((i < 10)?3:6), GRAPH_BASELINE + 2);
    display.print(i);
    if (ap_count[i - 1] > 0) {
      display.setCursor((i * CHANNEL_WIDTH) - ((ap_count[i - 1] < 10)?9:12), GRAPH_BASELINE + 11);
      display.print('(');
      display.print(ap_count[i - 1]);
      display.print(')');
    }
  }
  display.display();
  //Espera a la siguiente medición
  delay(5000);
}
