/*
 * ===================================================================
 * PROYECTO:      Termómetro Celular LTE
 * AUTOR:         [TU NOMBRE O NOMBRE DE EQUIPO]
 * FECHA:         Octubre 2025
 * VERSIÓN:       1.1 (Lectura de Sensor XN04)
 *
 * DESCRIPCIÓN:
 * Implementación para el Hackathon 2025 de Microside.
 * El sistema lee la temperatura del sensor XN04 (I2C) y la envía
 * a Blynk.Cloud usando el módulo celular XC03 (SIM7080G).
 * También recibe un umbral de temperatura desde Blynk para
 * realizar comparaciones locales.
 *
 * ===================================================================
 * HARDWARE UTILIZADO:
 * - Controlador:   Microside XC01 R5-I (ESP32-S3)
 * - Celular/GPS:   Microside XC03 (SIM7080G)
 * - Sensores:      Microside XN04 (Temp/Hum/Luz)
 * - Actuadores:    (Ninguno en esta versión)
 *
 * ===================================================================
 * MAPA DE PINES VIRTUALES (BLYNK Vpin):
 * -------------------------------------------------------------------
 * V0 (Entrada): Control Remoto de BOARD_LED (0=OFF, 1=ON)
 *
 * V2 (Salida):  Lectura de Temperatura (XN04)
 * V3 (Entrada): Umbral de Temperatura (para comparación local)
 * ===================================================================
 */

/*
 * ===================================================================
 * GLOSARIO DE FUNCIONES CLAVE (PARA EL HACKATHON)
 * ===================================================================
 * * --- LIBRERÍA BLYNK (DATOS) ---
 * * Blynk.virtualWrite(Vpin, valor)
 * (Dato: DISPOSITIVO -> NUBE)
 * Para QUÉ: Envía un 'valor' (ej. 25.3) desde tu XC01 HACIA la 
 * app de Blynk. Lo usas para actualizar un Gauge, Gráfico, etc.
 * * BLYNK_WRITE(Vpin)
 * (Dato: NUBE -> DISPOSITIVO)
 * Para QUÉ: Es una función "callback" (evento) que se ejecuta 
 * automáticamente cuando la app envía un valor (desde un slider, 
 * botón, etc.) HACIA tu XC01 en un 'Vpin' específico.
 * * Blynk.syncVirtual(Vpin, ...)
 * (Dato: NUBE -> DISPOSITIVO)
 * Para QUÉ: Le pide al servidor de Blynk que envíe el último
 * valor guardado para un pin de ENTRADA (como V0 o V3) al 
 * dispositivo. Esencial usarlo en BLYNK_CONNECTED() para 
 * restaurar el estado de tus controles si el micro se reinicia.
 *
 * --- LIBRERÍA BLYNK (CONEXIÓN Y TAREAS) ---
 * * Blynk.begin(auth, modem, apn, ...)
 * Para QUÉ: Es la función maestra de inicio. Le dice a 'modem'
 * (TinyGSM) que se conecte a la red celular (usando 'apn')
 * y luego se autentica con el servidor Blynk usando 'auth'.
 * * Blynk.run()
 * Para QUÉ: Es el "corazón" de Blynk. Debe estar en el loop()
 * y ejecutarse lo más rápido posible. Mantiene la conexión
 * viva ("heartbeat"), recibe datos (para BLYNK_WRITE) y 
 * envía datos (de virtualWrite).
 * * BLYNK_CONNECTED()
 * Para QUÉ: Función "callback" que se ejecuta automáticamente
 * justo cuando la conexión con Blynk se establece con éxito.
 * * BlynkTimer scheduler
 * Para QUÉ: Es un objeto para crear temporizadores no bloqueantes.
 * Es la alternativa correcta a usar delay() en un proyecto de IoT.
 * * scheduler.setInterval(milisegundos, funcion)
 * Para QUÉ: Le dice al 'scheduler' que ejecute una 'funcion'
 * específica (ej. updateTemperature) cada 'milisegundos'.
 * * --- LIBRERÍA TinyGsmClient (MÓDEM XC03) ---
 * * TinyGsm modem(SerialAT)
 * Para QUÉ: Crea el objeto "controlador" para nuestro módem XC03.
 * Le dice que debe usar 'SerialAT' (Serial2) para enviar comandos.
 * * modem.restart()
 * Para QUÉ: Envía los comandos AT de inicialización al SIM7080G
 * para configurarlo en un estado conocido y listo para conectar.
 * * --- LIBRERÍA Wire (I2C / SENSOR XN04) ---
 * * Wire.beginTransmission(direccion)
 * Para QUÉ: Inicia una "conversación" I2C con el dispositivo
 * esclavo en la 'direccion' (ej. 4 para el XN04).
 * * Wire.write(registro)
 * Para QUÉ: Envía un byte. En este caso, le dice al XN04
 * qué 'registro' interno queremos leer (ej. 0x01 para Temp).
 * * Wire.requestFrom(direccion, numBytes)
 * Para QUÉ: Le pide 'numBytes' de datos al esclavo I2C
 * (ej. 2 bytes para la temperatura).
 * * Wire.read()
 * Para QUÉ: Lee un solo byte de los datos que el sensor envió.
 * ===================================================================
 */


//##################################################################
// ### SECCIÓN 1: CONFIGURACIÓN DE BLYNK CLOUD ###
//##################################################################
#define BLYNK_TEMPLATE_ID "TMPL2hekXZT5P"
#define BLYNK_TEMPLATE_NAME "Termometro LTE"
#define BLYNK_AUTH_TOKEN "RfpUmVoCNWFIoimtYbpLQpyq-mTAhhSI"

#define BLYNK_PRINT Serial


//##################################################################
// ### SECCIÓN 2: LIBRERÍAS Y DEFINICIÓN DE HARDWARE (MÓDEM) ###
//##################################################################
#define TINY_GSM_MODEM_SIM7080      // Informa a la librería que usamos el XC03

#include <Arduino.h>
#include <Wire.h>                 // Librería para comunicación I2C (XN04)
#include <TinyGsmClient.h>        // Librería de control del módem (comandos AT)
#include <BlynkSimpleTinyGSM.h>   // Puente entre Blynk y TinyGSM


//##################################################################
// ### SECCIÓN 3: CONFIGURACIÓN DE PUERTOS Y MAPEO DE PINES ###
//##################################################################
#define SerialMon Serial
#define SerialAT Serial2
#define TINY_GSM_DEBUG SerialMon

// --- Mapeo de pines físicos del XC01 al estándar mikroBUS ---
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

// --- Mapeo de pines de la placa XC01 ---
#define BOARD_LED 16
#define PIN_MODEM_PK MIKROBUS_INT


//##################################################################
// ### SECCIÓN 4: CONFIGURACIÓN DE RED CELULAR Y SERVIDOR ###
//##################################################################
const char apn[] = "";  // <--- ¡RECUERDA PONER TU APN!
const char user[] = "";
const char pass[] = "";

const char domain[] = "ny3.blynk.cloud";
const char auth[] = BLYNK_AUTH_TOKEN;


//##################################################################
// ### SECCIÓN 5: OBJETOS GLOBALES Y VARIABLES ###
//##################################################################
static BlynkTimer scheduler;
static TinyGsm modem(SerialAT);

// --- Variable de estado global (Optimización) ---
static float currentTemperature = 0.0f;


//##################################################################
// ### SECCIÓN 6: DECLARACIÓN DE FUNCIONES ###
//##################################################################
float readXN04Temperature();
void updateTemperature();


//##################################################################
// ### SECCIÓN 7: FUNCIÓN DE ARRANQUE (SETUP) ###
//##################################################################
void setup()
{
    // --- 1. Inicializar Comunicaciones ---
    SerialMon.begin(115200);
    SerialAT.begin(115200, SERIAL_8N1, MIKROBUS_RX, MIKROBUS_TX);
    Wire.setPins(MIKROBUS_SDA, MIKROBUS_SCL); 
    Wire.begin();                            

    // --- 2. Inicializar Pines de E/S (I/O) ---
    pinMode(BOARD_LED, OUTPUT);
    digitalWrite(BOARD_LED, LOW);
    pinMode(PIN_MODEM_PK, OUTPUT);

    // --- 3. Reinicio por Hardware del Módem XC03 ---
    SerialMon.println("Reiniciando módem XC03 (pulso de 3s)...");
    digitalWrite(PIN_MODEM_PK, HIGH);
    delay(3000);
    digitalWrite(PIN_MODEM_PK, LOW);

    // --- 4. Conexión a la Red Celular y Blynk ---
    SerialMon.println("Iniciando modem LTE...");
    modem.restart();
    String modemInfo = modem.getModemInfo();
    SerialMon.print("Modem: ");
    SerialMon.println(modemInfo);

    SerialMon.println("Conectando a GPRS y Blynk...");
    Blynk.begin(auth, modem, apn, user, pass, domain);
    SerialMon.println("Conexión iniciada.");

    // --- 5. Programar Tareas ---
    scheduler.setInterval(30000UL, updateTemperature);
}


//##################################################################
// ### SECCIÓN 8: BUCLE PRINCIPAL (LOOP) ###
//##################################################################
void loop()
{
    Blynk.run();      
    scheduler.run();  
}


//##################################################################
// ### SECCIÓN 9: TAREAS PROGRAMADAS Y LECTURA DE SENSORES ###
//##################################################################

void updateTemperature()
{
    currentTemperature = readXN04Temperature();

    Serial.print("Temperatura actual: ");
    Serial.println(currentTemperature);

    Blynk.virtualWrite(V2, currentTemperature);
}

float readXN04Temperature()
{
    float temperature;
    uint16_t temperature_int;

    Wire.beginTransmission(4);
    Wire.write(0x01); // Registro 0x01 = Temperatura
    Wire.endTransmission();

    Wire.requestFrom(4, 2);

    temperature_int = ((Wire.read() << 8) | Wire.read());
    temperature = temperature_int / 100.0f;

    return temperature;
}


//##################################################################
// ### SECCIÓN 10: FUNCIONES DE EVENTOS BLYNK (CALLBACKS) ###
//##################################################################

BLYNK_CONNECTED()
{
    Serial.println("¡Conectado a Blynk.Cloud!");
    Blynk.syncVirtual(V0, V3);
}

BLYNK_DISCONNECTED()
{
    Serial.println("Desconectado de Blynk.Cloud");
}

BLYNK_WRITE(V0)
{
    int led = param.asInt();
    if ( led ){
        digitalWrite( BOARD_LED, HIGH );
    } else {
        digitalWrite( BOARD_LED, LOW );
    }
}

BLYNK_WRITE(V3)
{
    double treshold = param.asDouble();
    
    Serial.print("Nuevo umbral recibido: ");
    Serial.println(treshold);

    if ( currentTemperature < treshold ){
        Serial.println( "-> Temperatura actual MENOR al umbral" );
    } else {
        Serial.println( "-> Temperatura actual MAYOR o IGUAL al umbral" );
    }
}