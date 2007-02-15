/***************************************************************
*																					*
* Lunapaint																		*
* (c) 2006, Hogne Titlestad, hogga@sub-ether.org					*
* http://www.sub-ether.org/lunapaint									*
*																					*
***************************************************************/

#include "math.h"

float getDistance ( int x1, int y1, int x2, int y2 )
{
	int diffx = x1 - x2;
	int diffy = y1 - y2;
	return sqrt ( ( float )( diffx * diffx ) + ( float )( diffy * diffy ) );
}

float getAngle ( int x1, int y1, int x2, int y2 )
{
	int diffx = x1 - x2;
	int diffy = y1 - y2;
	return atan2 ( ( float )diffy, ( float )diffx );
}
