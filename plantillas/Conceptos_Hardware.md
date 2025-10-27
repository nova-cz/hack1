# 🧠 Conceptos de Hardware

## 1. GPIO — Pines Digitales (solo entiende 1 y 0)

**Usos comunes:**
- Encender o apagar **LEDs**
- Leer **botones** (presionado / no presionado)
- Comunicación digital (**I2C**, **SPI**, **UART**)

### 🔧 Ejemplos:

```cpp
pinMode(PIN_NUMERO, OUTPUT);  // Para encender un LED.
pinMode(PIN_NUMERO, INPUT);   // Para leer un botón.
```

**Escribir:**
```cpp
digitalWrite(PIN_NUMERO, HIGH); // Envía 5V al pin.
```

**Leer:**
```cpp
int estadoBoton = digitalRead(PIN_NUMERO); // estadoBoton valdrá HIGH o LOW.
```

---

## 2. Pines Analógicos

**Función:** medir rangos de valores continuos.  
**Usos comunes:** sensores de **temperatura**, **humedad**, **luz**, etc.  
➡️ No necesitan `pinMode()`.

---

## 3. Protocolos de Comunicación (UART, I2C, SPI)

Se utilizan para comunicarse con **módulos externos** (GPS, pantallas, sensores, etc.).

---

### 🧩 UART

Principal uso:  
Enviar datos de **depuración (debug)** desde la placa al PC mediante `Serial.println`.

**Configuración en `setup()`:**
```cpp
Serial.begin(9600);  // 9600 es la velocidad de transmisión (baud rate)
```

**Escritura en `loop()`:**
```cpp
Serial.println("Hola mundo");
Serial.print("Valor del sensor: ");
Serial.println(valorSensor);
```

---

### 🔌 I2C — El "Bus" de Módulos

Permite conectar **múltiples dispositivos** (sensores, pantallas, etc.) usando solo **dos cables**:

- `SDA` → línea de datos  
- `SCL` → línea de reloj  

📘 Usa la librería `Wire.h` (incluida en Arduino).

Cada dispositivo I2C tiene una **dirección única** (por ejemplo, `0x27`).

```cpp
#include <Wire.h>

// Configurar en setup()
Wire.setPins(MIKROBUS_SDA, MIKROBUS_SCL);
Wire.begin();

// Comunicar en loop()
Wire.beginTransmission(DIRECCION_DEL_SENSOR);
Wire.write(DATO_A_ENVIAR);
Wire.endTransmission();
```

---

### ⚡ SPI (Serial Peripheral Interface)

Otro protocolo **serial** (como I2C y UART), pero **más rápido**.  
Ideal para **transferir grandes cantidades de datos** con periféricos de alta velocidad, como:

- Pantallas a color  
- Tarjetas SD  
- Sensores de alta frecuencia  

**Líneas principales:**
- `SCLK` (Clock): metrónomo controlado por el Maestro (MCU).  
- `MOSI` (Master Out, Slave In): MCU → Sensor.  
- `MISO` (Master In, Slave Out): Sensor → MCU.  
- `CS` (Chip Select) o `SS` (Slave Select): selecciona qué esclavo activa el Maestro.

---

### ⚙️ Nota

`GND` (**Ground / Tierra**)  
→ Es la referencia de **0V**.  
Todos los dispositivos deben **compartir un GND común** con la placa.
