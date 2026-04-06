/* 
    Tokyo3D\src\vec3.c
*/

//#include "tokyo3d.h"
#include "vec3.h"

// constants
const static float threehalfs = 1.5F;

float Q_rsqrt( float number ) // Quake code from Wikipedia cuz why not, not in header
{
	long i;
	float x2, y;
	x2 = number * 0.5F;
	y  = number;
	i  = * ( long * ) &y;                       // evil floating point bit level hacking
	i  = 0x5f3759df - ( i >> 1 );               // what the fuck?
	y  = * ( float * ) &i;
	y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
	return y;
}

// REMEMBER: Inline functions into the header