/***************************************************************
*																					*
* Lunapaint																		*
* (c) 2006, Hogne Titlestad, hogga@sub-ether.org					*
* http://www.sub-ether.org/lunapaint									*
*																					*
***************************************************************/

#ifndef _math_h_
#define _math_h_

#include <stdio.h>
#include <math.h>

/*
    Get the distance between two points in double units
*/
float getDistance ( int x1, int y1, int x2, int y2 );

/*
    Get the radian angle between two points
*/
float getAngle ( int x1, int y1, int x2, int y2 );

#endif
