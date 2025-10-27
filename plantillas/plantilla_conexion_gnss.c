/*
 * ===================================================================
 * PROYECTO:      DEMO: ALTERNAR GNSS Y GPRS
 * VERSIÓN:       1.0 (Demo de Secuencia)
 *
 * DESCRIPCIÓN:
 * Este script es una demostración técnica (no una app funcional)
 * que muestra la secuencia de comandos para obedecer la Regla de Oro
 * del XC03: alternar entre el modo GNSS (GPS) y el modo GPRS (SIM).
 *
 * NOTA: El loop() fallará en obtener un fix o conexión real
 * debido a la falta de tiempos de espera (delays/timers).
 * ===================================================================
 * HARDWARE UTILIZADO:
 * - Controlador:   Microside XC01 R5-I (ESP32-S3)
 * - Celular/GPS:   Microside XC03 (SIM7080G)
 * ===================================================================
 */

/*
 * ===================================================================
 * GLOSARIO DE FUNCIONES CLAVE (PARA ALTERNAR MODOS)
 * ===================================================================
 * * --- REGLA DE ORO DEL XC03 (SIM7080G) ---
 * * NO SE PUEDE USAR GNSS (GPS) Y GPRS/LTE (RED CELULAR) AL MISMO TIEMPO.
 * * Debes apagar uno antes de encender el otro.
 *
 * * --- FUNCIONES GNSS (GPS) ---
 * * modem.enableGPS()
 * Para QUÉ: Enciende el receptor GNSS. (GPRS debe estar apagado).
 *
 * * modem.getGPS( &lat, &lon, ...)
 * Para QUÉ: Intenta leer la posición del módem. Devuelve 'true'
 * si tiene un "fix" válido.
 *
 * * modem.disableGPS()
 * Para QUÉ: Apaga el receptor GNSS. Necesario antes de
 * poder conectarse a la red celular.
 *
 * * --- FUNCIONES GPRS/LTE (SIM) ---
 * * modem.gprsConnect( apn, user, pass )
 * Para QUÉ: Enciende la red celular y se conecta al APN.
 * (GNSS debe estar apagado).
 *
 * * modem.isGprsConnected()
 * Para QUÉ: Devuelve 'true' si el módem está conectado a la
 * red celular.
 *
 * * modem.gprsDisconnect()
 * Para QUÉ: Apaga la conexión de red celular. Necesario
 * antes de poder encender el GNSS.
 * ===================================================================
 */


//##################################################################
// ### SECCIÓN 1: CONFIGURACIÓN DEL MÓDEM Y LIBRERÍAS ###
//##################################################################
#define TINY_GSM_MODEM_SIM7080

#include <Arduino.h>
#include <TinyGsmClient.h>


//##################################################################
// ### SECCIÓN 2: CONFIGURACIÓN DE PUERTOS Y PINES ###
//##################################################################
// (Nota: Faltan los defines de SerialMon y MIKROBUS, pero
//  asumimos los pines estándar para esta demo)
#define SerialMon Serial
#define SerialAT Serial2
#define PIN_MODEM_PK 7 // Pin 7 (MIKROBUS_INT) para el Power Key


//##################################################################
// ### SECCIÓN 3: CONFIGURACIÓN DE RED (SIM) ###
//##################################################################

// ¡DEBES RELLENAR ESTOS VALORES!
const char apn[] = "";     // APN de tu operador celular
const char user[] = "";    // Usuario del APN (normalmente vacío)
const char pass[] = "";    // Password del APN (normalmente vacío)
const char pin[] = "";     // PIN de 4 dígitos de tu SIM (dejar "" si no tiene)

// Crea el objeto 'modem'
static TinyGsm modem(SerialAT);


//##################################################################
// ### SECCIÓN 4: VARIABLES GLOBALES PARA DATOS GNSS ###
//##################################################################
// Variables "contenedoras" que modem.getGPS() llenará
float latitude;
float longitude;
float speed;
float alt;
int vsat;
int usat;
float accuracy;
int year;
int month;
int day;
int hour;
int minute;
int second;


//##################################################################
// ### SECCIÓN 5: FUNCIÓN DE ARRANQUE (SETUP) ###
//##################################################################
void setup() {
    // --- 1. Inicializar Comunicaciones ---
    // (AÑADIDO FALTANTE) Inicia el monitor serial (USB)
    SerialMon.begin(115200); 
    
    // Inicia la comunicación con el módem a 115200 baudios
    // (Asumimos que RX es 9 y TX es 10)
    SerialAT.begin(115200); 

    // --- 2. Inicializar Pines de E/S (I/O) ---
    pinMode(PIN_MODEM_PK, OUTPUT);

    // --- 3. Reinicio por Hardware del Módem XC03 ---
    SerialMon.println("Reiniciando módem XC03 (pulso de 3s)...");
    digitalWrite(PIN_MODEM_PK, HIGH);
    delay(3000); // 3000ms = 3 segundos
    digitalWrite(PIN_MODEM_PK, LOW);
    
    // Espera a que el módem esté listo
    delay(3000);

    // --- 4. Configuración Inicial del Módem ---
    SerialMon.println("Iniciando modem (restart)...");
    modem.restart();

    // Si la SIM tiene PIN, intenta desbloquearla
    if (strlen(pin) > 0) {
        SerialMon.println("Desbloqueando SIM...");
        // (BUG CORREGIDO: Usamos la variable 'pin', no 'GSM_PIN')
        if (!modem.simUnlock(pin)) {
            SerialMon.println("¡FALLO AL DESBLOQUEAR SIM! Verifica el PIN.");
        }
    }
    SerialMon.println("Setup completado. Iniciando loop de demostración...");
}


//##################################################################
// ### SECCIÓN 6: BUCLE PRINCIPAL (LOOP DE DEMOSTRACIÓN) ###
//##################################################################
void loop() {
    SerialMon.println("--- Inicio del Ciclo de Demo ---");

    // --- 1. DEMO MODO GNSS (GPS) ---
    SerialMon.println("Habilitando GNSS...");
    modem.enableGPS();

    // Intenta leer el GPS. (Fallará porque no tiene tiempo de 'fix')
    if (modem.getGPS(&latitude, &longitude, &speed, &alt, &vsat,
                     &usat, &accuracy, &year, &month, &day, &hour, &minute, &second))
    {
        // (Es casi imposible que el código llegue aquí)
        SerialMon.println("¡Posición GPS obtenida!"); 
        SerialMon.print("Latitud: ");
        SerialMon.println((double)latitude, 5);
        SerialMon.print("Longitud: ");
        SerialMon.println((double)longitude, 5);
    } else {
        SerialMon.println("GNSS habilitado, pero sin 'fix' (falta tiempo).");
    }

    // Apaga el GNSS (para cumplir la Regla de Oro)
    SerialMon.println("Deshabilitando GNSS...");
    modem.disableGPS();

    // --- 2. DEMO MODO GPRS (SIM) ---
    SerialMon.println("Habilitando GPRS (SIM)...");
    
    // Intenta conectar a GPRS. (Fallará porque no tiene tiempo de conectar)
    modem.gprsConnect(apn, user, pass);
    
    // (Pausa corta solo para dar tiempo a que el comando anterior se envíe)
    delay(100); 

    if (modem.isGprsConnected()) {
        // (Es casi imposible que el código llegue aquí)
        SerialMon.println("¡GPRS Conectado!");
    } else {
        SerialMon.println("GPRS conectando, pero sin red (falta tiempo).");
    }

    // Apaga el GPRS (para cumplir la Regla de Oro)
    SerialMon.println("Deshabilitando GPRS...");
    modem.gprsDisconnect();

    SerialMon.println("--- Fin del Ciclo de Demo ---");
    delay(5000); // Espera 5 segundos antes de repetir
}