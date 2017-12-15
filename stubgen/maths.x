program MATH_PROG {

    version MATH_VER {
        float  SQR      (float)     = 1;
        float  EXP      (float)     = 2;
        float  LOG10    (float)     = 3;
    } = 1;

    version MATH_VER {
        float SQR       (float)     = 1;
        float EXP       (float)     = 2;
        float LOG10     (float)     = 3;
        float ABS       (float)     = 4;
    } = 2;

} = 0x20000001;
