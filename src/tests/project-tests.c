#include "printf.h"
#include "uart.h"

/*
 * NOTE: All tests have been integrated into the ground.c and air.c files
 * to improve testing workflow. Please refer to those files for test functions.
 */

void main(void)
{
    uart_init();
    printf("Running tests from file %s\n", __FILE__);
    uart_putchar(EOT);
}
