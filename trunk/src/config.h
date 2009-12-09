/****************************************************************************
*                                                                           *
* config.h -- Lunapaint,                                                    *
*    http://developer.berlios.de/projects/lunapaintami/                     *
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

#ifndef _lunapaint_config_
#define _lunapaint_config_

#define LUNA_VERSION "0.4"
#define LUNA_SCREEN_TITLE "Lunapaint 0.4, 2009-11-18"
#define LUNA_ABOUT_TEXT "Lunapaint 0.4\n\n" \
    "See http://developer.berlios.de/projects/lunapaintami/\n" \
    "for more information\n\n" \
    "Special thanks to:\n\n  * d980, Michal Schulz , Ola Jensen,\n    MinisterQ"
#define VERSIONSTRING "$VER: Lunapaint 0.4 (18.11.2009) by LunaPaint Development Team\n"

struct LunapaintPrefs
{
    char ScreenmodeType;
    char LayerBackgroundMode;
};

extern struct LunapaintPrefs GlobalPrefs;

#endif
