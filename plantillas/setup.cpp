#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>


#define MIKROBUS_AN 4
#define MIKROBUS_RST 15
#define MIKROBUS_CS 6
#define MIKROBUS_SCK 8
#define MIKROBUS_MISO 18
#define MIKROBUS_MOSI 17
#define MIKROBUS_PWM 5
#define MIKROBUS_INT 7
#define MIKROBUS_RX 9
#define MIKROBUS_TX 10
#define MIKROBUS_SCL 13
#define MIKROBUS_SDA 12

#define BOARD_LED 16


void setup(){

// Escanear I2C 
//     Wire.begin();
//   Serial.begin(9600);
//   while (!Serial); // Espera a que el puerto serie se conecte
//   Serial.println("\nEscaneando bus I2C...");

    // mikroBUS GPIO
    pinMode( MIKROBUS_AN, OUTPUT );
    pinMode( MIKROBUS_RST, OUTPUT );
    pinMode( MIKROBUS_CS, OUTPUT );
    pinMode( MIKROBUS_PWM, OUTPUT );
    pinMode( MIKROBUS_INT, OUTPUT );
    pinMode( BOARD_LED, OUTPUT );

    digitalWrite( MIKROBUS_AN, HIGH );
    digitalWrite( MIKROBUS_RST, HIGH );
    digitalWrite( MIKROBUS_CS, HIGH );
    digitalWrite( MIKROBUS_PWM, HIGH );
    digitalWrite( MIKROBUS_INT, HIGH );
    digitalWrite( BOARD_LED, HIGH );

    // UART config
    Serial.begin( 115200 ); // USB/UART0
    Serial2.begin( 115200, SERIAL_8N1, MIKROBUS_RX, MIKROBUS_TX ); // mikroBUS
    
    // I2C config
    Wire.setPins( MIKROBUS_SDA, MIKROBUS_SCL ); // mikroBUS
    Wire.begin();

    // SPI config
    SPI.begin( MIKROBUS_SCK, MIKROBUS_MISO, MIKROBUS_MOSI ); // mikroBUS

    //-----------------
    //----- Código ----
    //------------------

    // Para el output de X01 (importante ponerlo antes de usar los leds)
    // pinMode(BOARD_LED, OUTPUT);
    // Encender el led
    // digitalWrite(BOARD_LED, LOW);

}


void loop(){
    // Escaneo de I2C
    // byte error, address;
    // int nDevices = 0;

    // for(address = 1; address < 127; address++ ) {
    //     Wire.beginTransmission(address);
    //     error = Wire.endTransmission();

    //     if (error == 0) {
    //     Serial.print("Dispositivo I2C encontrado en la dirección 0x");
    //     if (address < 16) {
    //         Serial.print("0");
    //     }
    //     Serial.println(address, HEX);
    //     nDevices++;
    //     }
    //     else if (error == 4) {
    //     Serial.print("Error desconocido en la dirección 0x");
    //     if (address < 16) {
    //         Serial.print("0");
    //     }
    //     Serial.println(address, HEX);
    //     }    
    // }
    // if (nDevices == 0) {
    //     Serial.println("No se encontraron dispositivos I2C\n");
    // }
    // else {
    //     Serial.println("Escaneo finalizado\n");
    // }
    // delay(5000); // Espera 5 segundos para volver a escanear


}