/*  Author: Steve Gunn
 * Licence: This work is licensed under the Creative Commons Attribution License.
 *           View this license at http://creativecommons.org/about/licenses/
 */
 
#include <stdint.h>

#define ROTA	PE4
#define ROTB	PE5
#define SWC		PE7
#define SWN		PC2
#define SWE		PC3
#define SWS		PC4
#define SWW		PC5

extern volatile int8_t rotary;

void init_rotary();
int8_t get_rotary();
uint8_t get_switch();