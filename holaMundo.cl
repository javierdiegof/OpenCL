// El kernel de las funciones de OpenCL

// El tipo "__global" indica que la variable se guarada en la memoeria compartida de todos los procesadores (unidades de procesamiento)
__kernel void hello(__global char *string){
    string[0] = 'H';
    string[1] = 'e';
    string[2] = 'l';
    string[3] = 'l';
    string[4] = 'o';
    string[5] = ' ';
    string[6] = 'W';
    string[7] = 'o';
    string[8] = 'r';
    string[9] = 'l';
    string[10] = 'd';
    string[11] = '!';
    string[12] = '\0';
} 