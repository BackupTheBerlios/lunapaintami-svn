/****************************************************************************
*                                                                           *
* canvas.h -- Lunapaint, http://www.sub-ether.org/lunapaint                 *
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

#ifndef _canvas_h_
#define _canvas_h_
#define MAXLAYERS 8

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define AROS 1
#ifdef AROS
#include <proto/dos.h>
#endif

#define DEBUG 1
#include <aros/debug.h>

#include "definitions.h"
#include "../common.h"
#include "../math/math.h"

#define MAXCOLOR 65535
#define TRANSCOLOR 0x4444444444440000ULL

// Use this to interrupt redrawing
BOOL abortRedraw;
int redrawTimes; // how many redrawtimes

/*
	Draw tool structs
*/

typedef struct r_g_b_a_32
{
	unsigned char r, g, b, a;
} rgba32;

typedef struct r_g_b_24
{
	unsigned char r, g, b;
} rgb24;

typedef struct r_g_b_a_64
{
	unsigned short r, g, b, a;
} rgba64;

typedef struct b_g_r_a_32
{
	unsigned char b, g, r, a;
} bgra32;

typedef struct a_b_g_r_32
{
	unsigned char a, b, g, r;
} abgr32;


// Linked value list
typedef struct valuelist
{
	double x, y;
	struct valuelist *Next;
} ValueList;


// Tool Line
typedef struct tool_LineData
{
	int mode;									// The mode 
	double x;									// Draw from
	double y;									// -||-
	double dx, dy;								// Destination x, y (draw to)
	double ox;									// start offset x
	double oy;									// start offset y
	double w;									// Width of buffer rect
	double h;									// Height of buffer rect
	BOOL initialized;							// Are we initialized or not?
	unsigned long long int *buffer;		// pixelbuffer
} toolLineData;
toolLineData lineTool;

// Tool Brush
typedef struct tool_BrushData
{
	int width;									// Brush width
	int height;									// Brush height
	int opacity;								// Brush opacity
	int power;									// Brush draw power
	int step;									// Brush steps in between draw
	int paintmode;								// The current paintmode
	char opacitymode;							// How to use opacity
	ValueList *ContourBuffer; 				// For filled freehand drawing
	BOOL RecordContour;						// Record pixels for contour or not?
	double ContourMinX, ContourMinY;		// Smallest x,y
	double ContourMaxX, ContourMaxY;		// Largest x,y
	BOOL feather;								// Using feather or not?
	BOOL antialias;							// Using antialiasing or not?
	BOOL initialized;							// Are we initialized?
	unsigned long long int *buffer;		// Buffer for storing the brush
	unsigned long long int *tmpbuf;   	// Generic buffer for different things
} toolBrushData;
toolBrushData brushTool;

// Tool Circle
typedef struct tool_CircleData
{
	int mode;								// The mode 
	int x, y;								// Draw from
	int ox;									// start offset x
	int oy;									// start offset y
	int w;									// Width of buffer rect
	int h;									// Height of buffer rect
	int bufwidth;							// Buffer width
	int bufheight;							// buffer height
	BOOL initialized;				// Are we initialized or not?
	unsigned long long int *buffer;		// pixelbuffer
} toolCircleData;
toolCircleData circleTool;

// Tool Rectangle
typedef struct tool_RectangleData
{
	int mode;								// The mode 
	double x;									// Draw from
	double y;									// -||-
	double dx, dy;							// Destination x, y (draw to)
	double ox;									// start offset x
	double oy;									// start offset y
	double w;									// Width of buffer rect
	double h;									// Height of buffer rect
	BOOL initialized;				// Are we initialized or not?
	unsigned long long int *buffer;		// pixelbuffer
} toolRectData;
toolRectData rectangleTool;

// Tool Clipbrush
typedef struct tool_ClipBrush
{
	int mode;								// The mode 
	double x;									// Draw from
	double y;									// -||-
	double dx, dy;							// Destination x, y (draw to)
	double ox;									// start offset x
	double oy;									// start offset y
	double w;									// Width of buffer rect
	double h;									// Height of buffer rect
	BOOL initialized;				// Are we initialized or not?
	unsigned long long int *buffer;		// pixelbuffer
} toolClipBrushData;
toolClipBrushData clipbrushTool;

/*
	Initialize a canvas and return a struct with a canvas in it
*/
oCanvas* Init_Canvas ( 
	unsigned int w, unsigned int h, unsigned int layers, unsigned int frames, BOOL generateBuffers
);

/*
	Control the frames on a canvas
*/
void NextFrame ( oCanvas *canvas );
void PrevFrame ( oCanvas *canvas );
int GotoFrame ( oCanvas *canvas, unsigned int frame );
int NextLayer ( oCanvas *canvas );
int PrevLayer ( oCanvas *canvas );
int GotoLayer ( oCanvas *canvas, unsigned int layer );

/*
	Get a pixel from the canvas
*/
unsigned long long int GetPixelFromCanvas ( unsigned int x, unsigned int y );

/*
	Get a pixel from all current layers combined ( flattened )
*/
unsigned long long int GetFlatPixelFromCanvas ( unsigned int x, unsigned int y );

/*
	Check which layer / frame we are using and set the pointer to the active
	buffer...
*/
void setActiveBuffer ( oCanvas *canvas );

/*
	Return a pointer to the next frame (wrap around)
*/
unsigned long long int *getNextFrame ( oCanvas *canvas );

/*
	Return a pointer to the prev frame (wrap around)
*/
unsigned long long int *getPrevFrame ( oCanvas *canvas );

/*
	Return a pointer to the current gfxbuffer struct
*/
gfxbuffer *getActiveGfxbuffer ( oCanvas *canvas );

/*
    Return a pointer to a specified gfxbuffer
*/
gfxbuffer *getGfxbuffer ( oCanvas *canvas, int layer, int frame );
gfxbuffer *getGfxbufferFromList ( gfxbuffer *buf, int layer, int frame, int totallayers, int totalframes );

/*
	Free the memory in a canvas
*/
void Destroy_Canvas ( oCanvas *canvas );

/*
	Free memory in a canvas buffer
*/
void Destroy_Buffer ( oCanvas *canv );

/*
	Draw the layers etc and return an unsigned int buffer for use by
	redrawScreenbufferRect and others
*/
inline unsigned int *renderCanvas ( 
	oCanvas *canvas, unsigned int rx, unsigned int ry, 
	unsigned int rw, unsigned int rh, BOOL Transparent
);

/*
	Draw all layers onto the screenbuffer constrained by rect
*/
BOOL redrawScreenbufferRect ( 
	oCanvas *canvas, unsigned int rx, unsigned int ry, unsigned int rw, unsigned int rh,
	BOOL updateStorage
);

/*
	Draw all layers onto the screenbuffer in specified
	canvas data
*/
BOOL redrawScreenbuffer ( oCanvas *canvas );

/*
	Convert a 16-bit pr color to a 8-bit pr color pixel
*/
unsigned int canvasToWindowPixel ( unsigned long long int source );

/*
	Convert to an ARGB format for export etc
*/
unsigned int canvasToARGBPixel ( unsigned long long int source );

/*
	Initialize the palette with standard colors...
*/
void initPalette ( );

/*
	Take an unsigned int color in ABGR format and convert to a
	struct containing the R, G, B channels separately
*/
rgbData paletteColorToRGB ( unsigned int color );

/*
	Palettecolor to rgba64
*/
rgba64 PaletteToRgba64 ( unsigned int rgb );

/*
	Take unsigned long long int and extract rgba data
	in rgbaData
*/
rgbaData canvasColorToRGBA ( unsigned long long int color );

/*
	Take unsigned long long int and extract rgba and return
	in a struct typedef rgbaDataL
*/
rgbaDataL canvasColorToRGBA_ull ( unsigned long long int color );

/*
	Take unsigned int and return rgba
*/
rgbaData bufferToRGBA ( unsigned int color );

/*
	Take r, g, b separately and join them to a palette color of type
	unsigned int...
*/
unsigned int RGBtoPaletteColor ( unsigned int r, unsigned int g, unsigned int b );

/*
	Take unsigned long long int and return unsigned int
*/
unsigned int bufferToScreenColor ( unsigned long long int col );

/*
	Take a palette color and convert it to a buffer color
*/
unsigned long long int PaletteToBuffercolor ( unsigned int rgb );

/*
	Load the default palette
*/
void loadDefaultPalette ( );

/*
	Insert a new layer on each frame
*/
void addLayer ( oCanvas *canv );

/*
	Swap two layers
*/
void swapLayers ( oCanvas *canv );

/*
	Copy one layer to another
*/
void copyLayers ( oCanvas *canv );

/*
	Merge two laywers
*/
void mergeLayers ( oCanvas *canv );

/*
	Delete a layer
*/
void deleteLayer ( oCanvas *canv );

/*
	Copy data to swap
*/
void copyToSwap ( oCanvas *canv );

/*
	Swap with spare
*/
void swapCanvasBuffers ( oCanvas *canv );


/*
	Draw preview of current tool / hover version 
*/
inline unsigned int drawToolPreview ( int x, int y );

/*
	Use the value list
*/ 
void addListValue ( double x, double y, ValueList **lst );
void freeValueList ( ValueList **lst );

#endif

