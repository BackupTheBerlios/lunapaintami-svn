/****************************************************************************
*                                                                           *
* new_image.c -- Lunapaint,                                                 *
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
#include  <SDI_hook.h>
#include "new_image.h"

Object *nwWindow;
Object *nwBtnOk;
Object *nwBtnCancel;
Object *nwStringWidth;
Object *nwStringHeight;
Object *nwStringFrames;
Object *nwStringPrjName;
Object *nwTemplates;
struct Hook template_hook;


HOOKPROTONHNO(template_func, void, int *param)
{

    int selection = XGET ( nwTemplates, MUIA_Cycle_Active );

    int templates[ 9 ][ 2 ] = {
        { 32, 32 }, { 48, 48 }, { 64, 64 }, { 160, 120 },
        { 320, 240 }, { 640, 480 }, { 800, 600 },
        { 1024, 768 }, { 1280, 1024 }
    };

    set ( nwStringWidth, MUIA_String_Integer, ( IPTR )templates[ selection ][ 0 ] );
    set ( nwStringHeight, MUIA_String_Integer, ( IPTR )templates[ selection ][ 1 ] );

}


void nwNewWindow ( )
{
    static const char *templates[] = {
        "32x32", "48x48", "64x64", "160x120",
        "320x240", "640x480", "800x600",
        "1024x768", "1280x1024", NULL
    };

    nwWindow = WindowObject,
        MUIA_Window_ScreenTitle, ( IPTR )"New Lunapaint Project",
        MUIA_Window_Title, ( IPTR )"New Lunapaint Project",
        MUIA_Window_CloseGadget, FALSE,
        MUIA_Window_Screen, ( IPTR )lunaPubScreen,
        WindowContents, ( IPTR )VGroup,
            Child, ( IPTR )GroupObject,
                MUIA_Frame, MUIV_Frame_Group,
                Child, ( IPTR )HGroup,
                    Child, ( IPTR )VGroup,
                        Child, ( IPTR )HGroup,
                            Child, ( IPTR )TextObject,
                                MUIA_Text_Contents, ( IPTR )"Project name:",
                            End,
                            Child, ( IPTR )( nwStringPrjName = StringObject,
                                MUIA_String_MaxLen, 128,
                                MUIA_String_Format, MUIV_String_Format_Left,
                                MUIA_Frame, MUIV_Frame_String,
                                MUIA_String_Contents, ( IPTR )"Unnamed project",
                                MUIA_CycleChain, 1,
                            End ),
                        End,
                        Child, ( IPTR )HGroup,
                            Child, ( IPTR )TextObject,
                                MUIA_Text_Contents, ( IPTR )"Size templates:",
                            End,
                            Child, ( IPTR )( nwTemplates = CycleObject,
                                MUIA_Cycle_Entries, ( IPTR )templates,
                                MUIA_Cycle_Active, 5,
                                MUIA_CycleChain, 1,
                            End ),
                        End,
                        Child, ( IPTR )HGroup,
                            Child, ( IPTR )TextObject,
                                MUIA_Text_Contents, ( IPTR )"Canvas width:",
                            End,
                            Child, ( IPTR )( nwStringWidth = StringObject,
                                MUIA_String_MaxLen, 5,
                                MUIA_String_Format, MUIV_String_Format_Right,
                                MUIA_Frame, MUIV_Frame_String,
                                MUIA_String_Integer, 640,
                                MUIA_CycleChain, 1,
                            End ),
                        End,
                        Child, ( IPTR )HGroup,
                            Child, ( IPTR )TextObject,
                                MUIA_Text_Contents, ( IPTR )"Canvas height:",
                            End,
                            Child, ( IPTR )( nwStringHeight = StringObject,
                                MUIA_String_MaxLen, 5,
                                MUIA_String_Format, MUIV_String_Format_Right,
                                MUIA_Frame, MUIV_Frame_String,
                                MUIA_String_Integer, 480,
                                MUIA_CycleChain, 1,
                            End ),
                        End,
                        Child, ( IPTR )HGroup,
                            Child, ( IPTR )TextObject,
                                MUIA_Text_Contents, ( IPTR )"Number of frames:",
                            End,
                            Child, ( IPTR )( nwStringFrames = StringObject,
                                MUIA_String_MaxLen, 5,
                                MUIA_String_Format, MUIV_String_Format_Right,
                                MUIA_Frame, MUIV_Frame_String,
                                MUIA_String_Integer, 1,
                                MUIA_CycleChain, 1,
                            End ),
                        End,
                    End,
                End,
            End,
            Child, ( IPTR )HGroup,
                Child, ( IPTR )( nwBtnOk = SimpleButton ( ( IPTR )"Ok" ) ),
                Child, ( IPTR )( nwBtnCancel = SimpleButton ( ( IPTR )"Cancel" ) ),
            End,
        End,
    End;
}

void Init_NewProjectMethods ( )
{
    // Do Methods...
    DoMethod (
        nwBtnOk, MUIM_Notify, MUIA_Pressed, FALSE,
        tbxAreaPalette, 1, MUIM_Luna_NewProject
    );
    DoMethod (
        nwBtnCancel, MUIM_Notify, MUIA_Pressed, FALSE,
        nwWindow, 3, MUIM_Set, MUIA_Window_Open, FALSE
    );

    template_hook.h_Entry = ( HOOKFUNC )&template_func;

    DoMethod (
        nwTemplates, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
        nwTemplates, 2, MUIM_CallHook, &template_hook
    );

    // Disable the keyboard when this window is activated
    DoMethod (
        nwWindow, MUIM_Notify, MUIA_Window_Activate, TRUE,
        nwWindow, 2, MUIM_CallHook, &DisableKeyboard_hook
    );
    // Enable the keyboard when this window is deactivated
    DoMethod (
        nwWindow, MUIM_Notify, MUIA_Window_Activate, FALSE,
        nwWindow, 2, MUIM_CallHook, &EnableKeyboard_hook
    );
}
