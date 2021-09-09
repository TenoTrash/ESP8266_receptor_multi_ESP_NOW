/*
  Basado en el ejemplo de Rui Santos https://RandomNerdTutorials.com/esp-now-many-to-one-esp8266-nodemcu/

  Módulo receptor de dos "emisores" que envían dos variables int random usando ESP-NOW
  Se determina si hay falla de comunicación cada 25 segundos mostrando una serie de "calaveras" en la columna de la derecha
  Se muestra todo en un LCD I2C de 20x4
  Marcelo Ferrarotti
  2021
*/

#include <ESP8266WiFi.h>
#include <espnow.h>

#include <ESP8266WiFi.h>
#include <espnow.h>

#include <LiquidCrystal_I2C.h>

// Lineas y columnas del LCD
int lcdColumns = 20;
int lcdRows = 4;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

//Variables globales para organizar los datos recibidos
int board1X;
int board1Y;
int board2X;
int board2Y;

//Para aviso_falla
unsigned long previousMillis = 0;
const long interval = 30000;

//Caracteres de corazón y calavera para mostrar en la pantalla
byte Heart[] = {
  B00000,
  B01010,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000,
  B00000
};

byte Skull[] = {
  B00000,
  B01110,
  B10101,
  B11011,
  B01110,
  B01110,
  B00000,
  B00000
};

// struct para recibir los datos
typedef struct struct_message {
  int id;
  int x;
  int y;
} struct_message;

struct_message myData;

// struct con el mismo formato para los dos emisores. Se necesita uno por cada emisor
struct_message board1;
struct_message board2;

// array para todos los datos
struct_message boardsStruct[2] = {board1, board2};

// Callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac_addr, uint8_t *incomingData, uint8_t len) {
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.printf("Board ID %u: %u bytes\n", myData.id, len);
  // Update the structures with the new incoming data
  boardsStruct[myData.id - 1].x = myData.x;
  boardsStruct[myData.id - 1].y = myData.y;
  Serial.printf("x value: %d \n", boardsStruct[myData.id - 1].x);
  Serial.printf("y value: %d \n", boardsStruct[myData.id - 1].y);
  Serial.println();
  board1X = boardsStruct[0].x;
  board1Y = boardsStruct[0].y;
  board2X = boardsStruct[1].x;
  board2Y = boardsStruct[1].y;

  titulos_display();

}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Inicializar el LCD
  lcd.init();
  lcd.backlight();

  //Caracteres customizados
  lcd.createChar(0, Heart);
  lcd.createChar(1, Skull);

  lcd.setCursor(0, 0);
  lcd.print("Inicio");
  lcd.setCursor(0, 1);
  lcd.write(0);

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  aviso_falla();
  
}

void aviso_falla() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    lcd.setCursor(19, 0);
    lcd.write(1);
    lcd.setCursor(19, 1);
    lcd.write(1);
    lcd.setCursor(19, 2);
    lcd.write(1);
    lcd.setCursor(19, 3);
    lcd.write(1);
  } 
}

void titulos_display() {
  // cursor columna 0, línea 0
  lcd.setCursor(0, 0);
  lcd.print("Receptor ESP-NOW");

  lcd.setCursor(0, 1);
  lcd.print("ID 1:");
  lcd.setCursor(5, 1);
  lcd.write(0);
  lcd.setCursor(0, 2);
  lcd.print("x:   ");
  lcd.setCursor(0, 3);
  lcd.print("y:   ");
  lcd.setCursor(3, 2);
  lcd.print(board1X);
  lcd.setCursor(3, 3);
  lcd.print(board1Y);

  lcd.setCursor(7, 1);
  lcd.print("ID 2:");
  lcd.setCursor(12, 1);
  lcd.write(0);
  lcd.setCursor(7, 2);
  lcd.print("x:   ");
  lcd.setCursor(7, 3);
  lcd.print("y:   ");
  lcd.setCursor(10, 2);
  lcd.print(board2X);
  lcd.setCursor(10, 3);
  lcd.print(board2Y);

  lcd.setCursor(19, 0);
  lcd.print(" ");
  lcd.setCursor(19, 1);
  lcd.print(" ");
  lcd.setCursor(19, 2);
  lcd.print(" ");
  lcd.setCursor(19, 3);
  lcd.print(" ");
}
