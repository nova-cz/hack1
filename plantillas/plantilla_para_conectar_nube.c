/*
 * ===================================================================
 * PROYECTO:      PLANTILLA PARA CONECTAR A LA NUBE (LTE)
 * AUTOR:         [TU NOMBRE O NOMBRE DE EQUIPO]
 * FECHA:         Octubre 2025
 * VERSIÓN:       1.0 (Plantilla Base)
 *
 * DESCRIPCIÓN:
 * Esta es la plantilla fundamental para el Hackathon 2025.
 * Conecta el controlador XC01 a Blynk.Cloud usando el
 * módulo celular XC03 (SIM7080G) para control y monitoreo básico.
 *
 * ===================================================================
 * HARDWARE UTILIZADO:
 * - Controlador:   Microside XC01 R5-I (ESP32-S3)
 * - Celular/GPS:   Microside XC03 (SIM7080G)
 *
 * ===================================================================
 * MAPA DE PINES VIRTUALES (BLYNK Vpin):
 * -------------------------------------------------------------------
 * V0 (Entrada): Control Remoto de BOARD_LED (0=OFF, 1=ON)
 * V1 (Salida):  Estado del BOARD_BUTTON (0=Presionado, 1=Liberado)
 * ===================================================================
 */

/*
 * ===================================================================
 * GLOSARIO DE FUNCIONES CLAVE (PARA BLYNK + LTE)
 * ===================================================================
 * * --- LIBRERÍA BLYNK (DATOS) ---
 * * Blynk.virtualWrite(Vpin, valor)
 * (Dato: DISPOSITIVO -> NUBE)
 * Para QUÉ: Envía un 'valor' (ej. 1, 0, 25.3) desde tu XC01 HACIA 
 * la app de Blynk (actualiza un widget).
 *
 * * BLYNK_WRITE(Vpin)
 * (Dato: NUBE -> DISPOSITIVO)
 * Para QUÉ: Es una función "callback" (evento) que se ejecuta 
 * automáticamente cuando la app envía un valor (desde un slider, 
 * botón, etc.) HACIA tu XC01 en un 'Vpin' específico.
 *
 * * Blynk.syncVirtual(Vpin, ...)
 * (Dato: NUBE -> DISPOSITIVO)
 * Para QUÉ: Le pide al servidor de Blynk que envíe el último
 * valor guardado para un pin de ENTRADA (como V0). Se usa en
 * BLYNK_CONNECTED() para restaurar el estado si el micro se reinicia.
 *
 * * --- LIBRERÍA BLYNK (CONEXIÓN Y TAREAS) ---
 * * Blynk.begin(auth, modem, apn, ...)
 * Para QUÉ: Es la función maestra de inicio. Le dice a 'modem'
 * (TinyGSM) que se conecte a la red celular (usando 'apn')
 * y luego se autentica con el servidor Blynk (usando 'auth').
 *
 * * Blynk.run()
 * Para QUÉ: Es el "corazón" de Blynk. Debe estar en el loop()
 * y ejecutarse miles de veces por segundo. Mantiene la conexión
 * viva ("heartbeat") y procesa toda la comunicación.
 *
 * * BLYNK_CONNECTED()
 * Para QUÉ: Función "callback" que se ejecuta automáticamente
 * justo cuando la conexión con Blynk se establece con éxito.
 *
 * * BlynkTimer scheduler
 * Para QUÉ: Objeto para crear temporizadores no bloqueantes.
 * Es la forma correcta de ejecutar tareas (ej. leer un sensor)
 * cada X segundos sin usar delay().
 *
 * * scheduler.setInterval(milisegundos, funcion)
 * Para QUÉ: Le dice al 'scheduler' que ejecute 'funcion'
 * cada 'milisegundos'.
 *
 * * --- LIBRERÍA TinyGsmClient (MÓDEM XC03) ---
 * * TinyGsm modem(SerialAT)
 * Para QUÉ: Crea el objeto "controlador" para nuestro módem XC03.
 * Le dice que debe usar 'SerialAT' (Serial2) para enviar comandos.
 *
 * * modem.restart()
 * Para QUÉ: Envía los comandos AT de inicialización al SIM7080G
 * para configurarlo en un estado conocido y listo para conectar.
 * ===================================================================
 */


//##################################################################
// ### SECCIÓN 1: CONFIGURACIÓN DE BLYNK CLOUD ###
//##################################################################
#define BLYNK_TEMPLATE_ID "TMPxxxxxx"
#define BLYNK_TEMPLATE_NAME "Device"
#define BLYNK_AUTH_TOKEN "YourAuthToken"

#define BLYNK_PRINT Serial


//##################################################################
// ### SECCIÓN 2: LIBRERÍAS Y DEFINICIÓN DE HARDWARE (MÓDEM) ###
//##################################################################

// Crítico: Informa a la librería TinyGSM que estamos usando el XC03
#define TINY_GSM_MODEM_SIM7080

#include <Arduino.h>
#include <Wire.h>                 // Librería I2C (preparada para el XN04)
#include <TinyGsmClient.h>        // Librería de control del módem
#include <BlynkSimpleTinyGSM.h>   // Puente entre Blynk y TinyGSM


//##################################################################
// ### SECCIÓN 3: CONFIGURACIÓN DE PUERTOS Y MAPEO DE PINES ###
//##################################################################

// --- Asignación de Puertos Serie ---
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
#define BOARD_BUTTON 0

// --- Pin de control del Módem XC03 ---
#define PIN_MODEM_PK MIKROBUS_INT


//##################################################################
// ### SECCIÓN 4: CONFIGURACIÓN DE RED CELULAR Y SERVIDOR ###
//##################################################################

// ¡DEBES RELLENAR ESTO CON EL APN DE TU SIM CARD!
const char apn[] = "";
const char user[] = "";
const char pass[] = "";

// --- Configuración del servidor Blynk ---
const char domain[] = "ny3.blynk.cloud";
const char auth[] = BLYNK_AUTH_TOKEN;


//##################################################################
// ### SECCIÓN 5: OBJETOS GLOBALES Y VARIABLES ###
//##################################################################
static BlynkTimer scheduler;
static TinyGsm modem(SerialAT);


//##################################################################
// ### SECCIÓN 6: TAREAS PROGRAMADAS (TIMER) ###
//##################################################################

/**
 * @brief Revisa el estado del botón BOOT y lo envía a Blynk (V1).
 * * Optimizado para enviar datos solo cuando hay un cambio.
 */
void updateButton( )
{
    static int prev_status = HIGH;

    int current_status = digitalRead( BOARD_BUTTON );

    if ( current_status == prev_status ){
        return;
    }

    prev_status = current_status;
    Blynk.virtualWrite(V1, current_status);
}


//##################################################################
// ### SECCIÓN 7: FUNCIÓN DE ARRANQUE (SETUP) ###
//##################################################################
void setup()
{
    // --- 1. Inicializar Comunicaciones ---
    
    // Inicia el monitor serie (USB) a 115200 baudios (bits por segundo).
    // Esta es la velocidad de la consola de depuración.
    SerialMon.begin(115200);

    // Inicia el Serial2 (hardware) a 115200 baudios, en los pines 9 (RX) y 10 (TX).
    // Esta es la velocidad a la que el XC01 hablará con el XC03.
    SerialAT.begin(115200, SERIAL_8N1, MIKROBUS_RX, MIKROBUS_TX);
    
    Wire.setPins(MIKROBUS_SDA, MIKROBUS_SCL);
    Wire.begin();

    // --- 2. Inicializar Pines de E/S (I/O) ---
    pinMode( BOARD_BUTTON, INPUT_PULLUP );
    pinMode(BOARD_LED, OUTPUT);
    digitalWrite(BOARD_LED, LOW);
    pinMode(PIN_MODEM_PK, OUTPUT);

    // --- 3. Reinicio por Hardware del Módem XC03 ---
    SerialMon.println("Reiniciando módem XC03 (pulso de 3s)...");
    
    // Esta secuencia es crítica:
    // El módem SIM7080G requiere un pulso de 3000ms (3 segundos)
    // en el pin Power Key (PK) para forzar un reinicio limpio.
    digitalWrite(PIN_MODEM_PK, HIGH);
    delay(3000); // Pausa BLOQUEANTE de 3 segundos (aceptable en setup())
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
    // '1000UL' = 1000 milisegundos (1 segundo). 'UL' es por 'Unsigned Long'.
    // Ejecuta updateButton cada 1 segundo.
    scheduler.setInterval(1000UL, updateButton);
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
// ### SECCIÓN 9: FUNCIONES DE EVENTOS BLYNK (CALLBACKS) ###
//##################################################################

BLYNK_CONNECTED()
{
    Serial.println("¡Conectado a Blynk.Cloud!");
    Blynk.syncVirtual(V0);
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