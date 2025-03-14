/*
 * File:   pi_serial.c
 * Author: nd159473 (Nickolay Dalmatov, Sun Microsystems)
 * modified by Dmitry V Irtegov, NSU
 *
 * Created on March 20, 2007, 6:33 PM
 *
 */

#include <stdio.h>
#include <stdlib.h>

 /*
  *
  */

#define num_steps 200000000

int
main(int argc, char** argv) {

    double pi = 0;
    int i;

    for (i = 0; i < num_steps; i++) {

        pi += 1.0 / (i * 4.0 + 1.0);
        pi -= 1.0 / (i * 4.0 + 3.0);
    }

    pi = pi * 4.0;
    printf("pi done - %.15g \n", pi);

    return (EXIT_SUCCESS);
}
