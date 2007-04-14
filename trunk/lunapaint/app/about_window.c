/****************************************************************************
*                                                                           *
* about_window.c -- Lunapaint, http://www.sub-ether.org/lunapaint           *
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


#include "about_window.h"

void Init_AboutWindow ( )
{
    aboutWindow = WindowObject,
        MUIA_Window_Title, ( IPTR )"About Lunapaint",
        MUIA_Window_ScreenTitle, ( IPTR )"About Lunapaint",
        MUIA_Window_Screen, ( IPTR )lunaPubScreen,
        MUIA_Window_CloseGadget, FALSE,
        WindowContents, ( IPTR )VGroup,
            Child, ( IPTR )GroupObject,
                InnerSpacing(0,0),
                MUIA_Frame, MUIV_Frame_Group,
                Child, ( IPTR )ImageObject,
                    MUIA_Frame, MUIV_Frame_Group,
                    MUIA_Image_Spec, (IPTR)"3:data/lunaabout_logo.png",
                End,
            End,
            Child, ( IPTR )GroupObject,
                MUIA_Frame, MUIV_Frame_Group,
                Child, ( IPTR )TextObject,
                    MUIA_Text_Contents, ( IPTR )LUNA_ABOUT_TEXT,
                End,
            End,
            Child, ( IPTR )( aboutWindowOk = SimpleButton ( ( IPTR )"Close window" ) ),
        End,
    End;
    DoMethod (
        aboutWindowOk, MUIM_Notify, MUIA_Pressed, FALSE, 
        aboutWindow, 3, MUIM_Set, MUIA_Window_Open, FALSE
    );
}
