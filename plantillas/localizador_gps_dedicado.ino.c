/*
 * ===================================================================
 * PROYECTO:      Localizador GPS Dedicado (Solo GNSS)
 * VERSIÓN:       1.0
 *
 * DESCRIPCIÓN:
 * Este script demuestra el uso correcto del modo GNSS del XC03.
 * Inicia el módem y entra en un bucle de espera activa (con timeout)
 * para adquirir un "fix" de GPS. Una vez obtenido, imprime las
 * coordenadas en el monitor serie.
 *
 * NOTA: Este script NO utiliza la red celular (GPRS/LTE).
 * ===================================================================
 * HARDWARE UTILIZADO:
 * - Controlador:   Microside XC01 R5-I (ESP32-S3)
 * - Celular/GPS:   Microside XC03 (SIM7080G)
 * ===================================================================
 */

/*
 * ===================================================================
 * GLOSARIO DE FUNCIONES CLAVE (PARA GPS y TIMERS)
 * ===================================================================
 * * --- LIBRERÍA TinyGsmClient (MÓDEM XC03) ---
 * * modem.restart()
 * Para QUÉ: Configura el módem con comandos AT iniciales.
 * Esencial después de un reinicio por hardware.
 *
 * * modem.gprsDisconnect()
 * Para QUÉ: Se desconecta forzosamente de la red celular.
 * (REGLA DE ORO: Se usa en setup() para asegurar que GPRS esté
 * apagado ANTES de intentar encender el GPS).
 *
 * * modem.enableGPS()
 * Para QUÉ: Habilita la antena y el receptor GNSS del XC03.
 * (REGLA DE ORO: Falla si la red GPRS/LTE está activa).
 *
 * * modem.getGPS( &lat, &lon, ...)
 * Para QUÉ: Pide al módem que "llene" nuestras variables
 * globales (lat, lon, etc.) con los datos actuales del GPS.
 * Devuelve 'true' solo si ya tiene un "fix" (posición válida).
 * Devuelve 'false' si aún está buscando señal.
 *
 * * --- FUNCIONES DE TEMPORIZACIÓN (ARDUINO C++) ---
 * * millis()
 * Para QUÉ: Devuelve el número de milisegundos que han pasado
 * desde que el XC01 se encendió. Es la base de nuestro
 * temporizador no bloqueante.
 *
 * * while ( ( millis() - timer ) < timeout ) { ... }
 * Para QUÉ: Esta es la lógica clave del "timeout".
 * 'timer' = El momento en que empezamos a contar.
 * 'millis() - timer' = El tiempo que ha transcurrido.
 * El bucle 'while' se repetirá (intentando obtener GPS)
 * hasta que el tiempo transcurrido supere el 'timeout'.
 *
 * * --- FUNCIONES DE ARRANQUE (ARDUINO C++) ---
 * * Serial.begin(baudios)
 * Para QUÉ: Inicia el monitor serial USB (SerialMon) para
 * que puedas ver los mensajes de depuración en tu computadora.
 *
 * * SerialAT.begin(baudios, config, RX, TX)
 * Para QUÉ: Inicia el puerto serial de hardware (Serial2)
 * en los pines específicos del mikroBUS (9 y 10) para
 * hablar con el módem XC03.
 *
 * * pinMode(pin, MODO)
 * Para QUÉ: Configura un pin (ej. PIN_MODEM_PK) como
 * SALIDA (OUTPUT) o ENTRADA (INPUT).
 *
 * * digitalWrite(pin, ESTADO)
 * Para QUÉ: Escribe un valor digital (HIGH o LOW) en un
 * pin de salida. Lo usamos para la secuencia de reinicio
 * del módem (el pulso de 3 segundos).
 *
 * * delay(milisegundos)
 * Para QUÉ: Pausa la ejecución del programa.
 * IMPORTANTE: Generalmente se evita, pero es ACEPTABLE
 * usarlo dentro del 'setup()' (para el reinicio del módem)
 * o para pausas simples en loops de demostración.
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

// --- Asignación de Puertos Serie ---
#define SerialMon Serial
#define SerialAT Serial2
#define TINY_GSM_DEBUG SerialMon // Imprime comandos AT en el monitor

// --- Mapeo de pines mikroBUS ---
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
#define PIN_MODEM_PK MIKROBUS_INT // Pin 7 para el Power Key


//##################################################################
// ### SECCIÓN 3: CONFIGURACIÓN DE RED (SIM) ###
//##################################################################
// (Nota: El APN no se usa en este script, pero se deja como referencia)
const char apn[] = "";
const char user[] = "";
const char pass[] = "";


//##################################################################
// ### SECCIÓN 4: OBJETOS GLOBALES Y DECLARACIONES ###
//##################################################################
static TinyGsm modem(SerialAT); // Objeto módem
bool updateGNSS();              // Prototipo de la función


//##################################################################
// ### SECCIÓN 5: FUNCIÓN DE ARRANQUE (SETUP) ###
//##################################################################
void setup() {

    // --- 1. Inicializar Comunicaciones ---
    SerialMon.begin(115200);
    SerialAT.begin(115200, SERIAL_8N1, MIKROBUS_RX, MIKROBUS_TX);

    // --- 2. Inicializar Pines de E/S (I/O) ---
    pinMode(BOARD_LED, OUTPUT);
    digitalWrite(BOARD_LED, LOW);
    pinMode(PIN_MODEM_PK, OUTPUT);

    // --- 3. Reinicio por Hardware del Módem XC03 ---
    SerialMon.println("Reiniciando módem XC03 (pulso de 3s)...");
    digitalWrite(PIN_MODEM_PK, HIGH);
    delay(3000);
    digitalWrite(PIN_MODEM_PK, LOW);
    
    // Espera a que el módem esté listo
    delay(3000); 

    // --- 4. Configuración Inicial del Módem ---
    SerialMon.println("Iniciando modem LTE (restart)...");
    modem.restart();
    String modemInfo = modem.getModemInfo();
    SerialMon.print("Modem: ");
    SerialMon.println(modemInfo);

    // --- 5. HABILITACIÓN DEL MODO GNSS (GPS) ---
    // (REGLA DE ORO: Aseguramos que GPRS esté apagado)
    SerialMon.println("Desconectando GPRS (por si acaso)...");
    modem.gprsDisconnect();
    
    SerialMon.println("Habilitando GNSS...");
    if (!modem.enableGPS()) {
        SerialMon.println("¡Error fatal! No se pudo iniciar el GNSS.");
        // Bucle infinito: el programa no puede continuar.
        while (1) {
            delay(1000);
        }
    }
    SerialMon.println("GNSS Habilitado. Buscando satélites...");
}


//##################################################################
// ### SECCIÓN 6: BUCLE PRINCIPAL (LOOP) ###
//##################################################################
void loop() {
    // Intenta obtener la posición
    if (updateGNSS()) {
        // Si lo logra, parpadea el LED
        SerialMon.println("¡Posición obtenida!");
        digitalWrite(BOARD_LED, HIGH);
        delay(100);
        digitalWrite(BOARD_LED, LOW);
    } else {
        SerialMon.println("Aún buscando 'fix'...");
    }

    // Espera 1 segundo antes de volver a intentarlo
    delay(1000UL);
}


//##################################################################
// ### SECCIÓN 7: FUNCIÓN DE ADQUISICIÓN DE GPS (EL CEREBRO) ###
//##################################################################

/**
 * @brief Intenta obtener un "fix" de GPS dentro de un timeout.
 * @return true si se obtuvo la posición, false si se agotó el tiempo.
 */
bool updateGNSS() {
    // Bandera para el primer arranque (requiere más tiempo)
    static bool coldboot = true;
    
    // Tiempos de espera (en milisegundos)
    const unsigned long timeout_coldboot = 90000; // 90 segundos
    const unsigned long timeout_run = 30000;      // 30 segundos

    // Variables locales para almacenar los datos del GPS
    float latitude, longitude, speed, alt, accuracy;
    int vsat, usat, year, month, day, hour, minute, second;

    unsigned long timer = 0;
    unsigned long timeout = 0;
    bool success = false;

    // Asigna el timeout correcto (largo si es coldboot, corto si no)
    if (coldboot) {
        timeout = timeout_coldboot;
        SerialMon.println("Iniciando 'Cold Boot' (timeout: 90s)...");
    } else {
        timeout = timeout_run;
    }

    // Comienza el temporizador
    timer = millis();

    // --- BUCLE DE ESPERA ACTIVA ---
    // Se queda aquí hasta que se acabe el tiempo O encuentre la posición
    while ((millis() - timer) < timeout) {

        // Pide los datos al módem. 'success' será true si el fix es válido.
        success = modem.getGPS(&latitude, &longitude, &speed, &alt, &vsat, &usat, &accuracy,
                               &year, &month, &day, &hour, &minute, &second);

        // Si se obtiene la posición, finaliza el bucle inmediatamente
        if (success) {
            break;
        }
    }

    // Si el bucle terminó sin éxito (timeout), retorna 'false'
    if (!success) {
        return false;
    }

    // --- Impresión de Datos (Si hubo éxito) ---
    SerialMon.println("--- ¡GPS FIX OBTENIDO! ---");
    Serial.print("Latitud: ");
    Serial.println((double)latitude, 6); // Imprime con 6 decimales

    Serial.print("Longitud: ");
    Serial.println((double)longitude, 6); // Imprime con 6 decimales

    Serial.print("Velocidad ( km/h ): ");
    Serial.println(speed);

    Serial.print("Altitud (mts): ");
    Serial.println(alt);

    Serial.print("Satelites (en vista): ");
    Serial.println(vsat);

    Serial.print("Satelites (utilizados): ");
    Serial.println(usat);

    Serial.print("Precision (mts): ");
    Serial.println(accuracy);

    Serial.print("Fecha/Hora: ");
    Serial.print(day); Serial.print("/"); Serial.print(month); Serial.print("/"); Serial.print(year);
    Serial.print(" ");
    Serial.print(hour); Serial.print(":"); Serial.print(minute); Serial.print(":"); Serial.println(second);
    SerialMon.println("---------------------------");

    // Marca que el coldboot ya pasó
    coldboot = false;
    return true;
}