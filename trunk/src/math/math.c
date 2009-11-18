/****************************************************************************
*                                                                           *
* tool.c -- Lunapaint, http://www.sub-ether.org/lunapaint                   *
* Copyright (C) 2006, 2007, Hogne Titlestad <hogga@sub-ether.org>           *
* Copyright (C) 2009 LunaPaint Development Team                             *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License for more details.                              *
*                                                                           *
* You should have received a copy of the GNU General Public License         *
* along with this program; if not, write to the Free Software Foundation,   *
* Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.            *
*                                                                           *
****************************************************************************/

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
