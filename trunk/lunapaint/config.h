/****************************************************************************
*                                                                           *
* config.h -- Lunapaint, http://www.sub-ether.org/lunapaint                 *
* Copyright (C) 2006, 2007, Hogne Titlestad <hogga@sub-ether.org>           *
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

#define LUNA_VERSION "0.3.3"
#define LUNA_SCREEN_TITLE "Lunapaint v0.3.3, 30.07.2007"
#define LUNA_ABOUT_TEXT "Lunapaint v0.3.3\n\nAuthor: Hogne Titlestad (m0ns00n)\nE-Mail: hogga@sub-ether.org\nUrl: http://www.sub-ether.org/lunapaint\n\nSpecial thanks to:\n\n  * d980, Michal Schulz , Ola Jensen,\n    MinisterQ"
#define VERSIONSTRING "$VER: Lunapaint v0.3.3 (30.07.07) by Hogne (m0ns00n) Titlestad\n"

typedef struct LunapaintPrefs
{
    char ScreenmodeType;
    char LayerBackgroundMode;
} LunapaintPrefs;

LunapaintPrefs GlobalPrefs;

#endif
