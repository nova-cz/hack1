// Modulo XN04
float readXN04Temperature( ){

    float temperature;
    uint16_t temperature_int;
    // Comunicarse con XN04
    Wire.beginTransmission( 4 );
    // Registro de temperature
    Wire.write( 0x01 );
    Wire.endTransmission();

    // Pide 2 bytes al modulo 4 (XNO4)
    Wire.requestFrom( 4, 2 );
    
    temperature_int = ( (Wire.read() << 8) | Wire.read() );
    temperature = temperature_int/100.0f;

    return temperature;
}

float readXN04Humidity( ){

    float humidity;
    uint16_t humidity_int;
    // Comunicarse con XN04
    Wire.beginTransmission( 4 );
    // Registro de humidity
    Wire.write( 0x02 );
    Wire.endTransmission();

    Wire.requestFrom( 4, 2 );
    
    humidity_int = ( (Wire.read() << 8) | Wire.read() );
    humidity = humidity_int/100.0f;

    return humidity;

}

uint16_t readXN04Luminosity( ){

    uint16_t lux;
    // Comunicarse con XN04
    Wire.beginTransmission( 4 );
    // Registro de lux
    Wire.write( 0x03 );
    Wire.endTransmission();

    Wire.requestFrom( 4, 2 );
    
    lux = ( (Wire.read() << 8) | Wire.read() );

    return lux;

}