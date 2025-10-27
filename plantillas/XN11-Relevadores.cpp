void writeXN11(uint8_t relay, uint8_t stat)
{
    uint8_t reg = 0x00;
    uint8_t data = 0x00;

    if ( stat ){
        data = 0x01;
    } else {
        data = 0x00;
    }

    switch( relay ){
        case 1:
        case 2:
        reg = relay;
        break;
        default:
        return;
    }

    // Comunicacion con XN11
    Wire.beginTransmission(11);
    // Valor de salidas
    Wire.write( reg );
    Wire.write(data);
    Wire.endTransmission();
}

void setup() {

    // Configuración de comunicaciones seriales
    Serial.begin(115200);
    Wire.setPins( MIKROBUS_SDA, MIKROBUS_SCL );
    Wire.begin();

    writeXN11( 1, LOW );
    writeXN11( 2, LOW );
}

void loop() {

    writeXN11( 1, HIGH );
    delay( 5000UL );
    writeXN11( 2, HIGH );
    delay( 5000UL );

    

    writeXN11( 1, LOW );
    writeXN11( 2, LOW );
    delay( 5000UL );

}
