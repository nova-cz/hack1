void writeXN02( bool o1 = LOW, bool o2 = LOW, bool o3 = LOW, bool o4 = LOW, bool o5 = LOW, bool o6 = LOW, bool o7 = LOW, bool o8 = LOW ){
    uint8_t data = 0;

    data |= o1;
    data |= o2 << 1;
    data |= o3 << 2;
    data |= o4 << 3;
    data |= o5 << 4;
    data |= o6 << 5;
    data |= o7 << 6;
    data |= o8 << 7;

    writeXN02Binary( data );
}