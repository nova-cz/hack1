// Modulo XN01
//                             Input es el led a encender
uint8_t readXN01Input( uint8_t input ) {
    uint8_t inputs = 0;

    // Inicia la comunicación con el XN(01)
    Wire.beginTransmission(1);
    // Selecciona el registro de las entradas
    Wire.write( 0x01 );
    // Finaliza la comunicación con el nodo
    Wire.endTransmission();

    // Lee 1 byte con la información de las entradas
    Wire.requestFrom( 1, 1);
        
    inputs = Wire.read();

    if ( input > 8 || input < 1 )
        return 255;
    
    return ( inputs >> (input - 1) ) & 0x01;
}

void setup(){
    // Le decimos al LED que va a estar en modo salida
    pinMode(BOARD_LED, OUTPUT);

}

void loop(){

}

