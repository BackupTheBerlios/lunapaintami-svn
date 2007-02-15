/****************************************************************************
*                                                                           *
* drawtools.c -- Lunapaint, http://www.sub-ether.org/lunapaint              *
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

#include "drawtools.h"

BOOL fPop ( int *x, int *y, int pitch, int *fStack )
{
	if ( fStackPointer > 0 )
	{
		int p = fStack[ fStackPointer ];
		*x = p % pitch;
		*y = p / pitch;
		fStackPointer--;
		return 1;
	}
	return 0;
}

void fPush ( int x, int y, int limit, int pitch, int *fStack )
{
	if ( fStackPointer + 1 < limit )
	{
		fStackPointer++;
		fStack[ fStackPointer ] =  pitch * y + x;
	}
}

Affrect drawLine ( 
	double x1, double y1, double x2, double y2, 
	int bufferwidth, int bufferheight,
	unsigned long long int *buffer, BOOL subtract_endpoint
)
{				
	// Find the area drawn on and paint
	
	double X = 0, Y = 0, W = 0, H = 0;
	
	if ( x2 > x1 )
	{
		X = x1; W = x2 - x1;
	}
	else
	{
		X = x2; W = x1 - x2;
	}
	if ( y2 > y1 )
	{
		Y = y1; H = y2 - y1;
	}
	else
	{
		Y = y2; H = y1 - y2;
	}
	
	double dist;

	float slopex = 0.0, slopey = 0.0;
	
	if ( W > H )
	{
		slopex = 1.0;
		slopey = ( float )H / ( float )W;
		dist = W;
	}
	else if ( H > W )
	{
		slopex = ( float )W / ( float )H;
		slopey = 1.0;
		dist = H;
	}
	else
	{
		slopex = 1.0;
		slopey = 1.0;
		dist = W;
	}

	// If we are to subtract endpoint
	dist -= ( subtract_endpoint ? ( 1.0 / globalActiveCanvas->zoom ) : 0.0 );
	
	double coordx = x1, coordy = y1;
	
	if ( x1 > x2 ) slopex = -slopex;
	if ( y1 > y2 ) slopey = -slopey;
	
	int i = 0; for ( ; i <= dist; i++ )
	{
		if ( i % brushTool.step == 0 )
			plotBrush ( coordx, coordy, bufferwidth, bufferheight, buffer );
		coordx += slopex; coordy += slopey;
	}
	
	// Return affected coorinates
	int offx = ( double )brushTool.width / 2;
	int offy = ( double )brushTool.height / 2;
	Affrect rect = { 
		( int )( X - 1 - offx ), ( int )( Y - 1 - offy ), 
		( int )( W + brushTool.width + 2 ), ( int )( H + brushTool.height + 2 )
	};
	return rect;
}

void drawLineCharbuf ( 
	double x1, double y1, double x2, double y2, 
	int bufferwidth, int bufferheight,
	unsigned char *buffer, unsigned char value
)
{				
	// Find the area drawn on and paint
	
	double X = 0.0, Y = 0.0, W = 0.0, H = 0.0;
	
	if ( x2 > x1 )
	{
		X = x1; W = x2 - x1;
	}
	else
	{
		X = x2; W = x1 - x2;
	}
	if ( y2 > y1 )
	{
		Y = y1; H = y2 - y1;
	}
	else
	{
		Y = y2; H = y1 - y2;
	}
	
	double dist;

	float slopex, slopey;
	
	if ( W > H )
	{
		slopex = 1.0; slopey = ( float )H / ( float )W; dist = W;
	}
	else if ( H > W )
	{
		slopex = ( float )W / ( float )H; slopey = 1.0; dist = H;
	}
	else
	{
		slopex = 1.0; slopey = 1.0; dist = W;
	}
	
	double coordx = x1, coordy = y1;
	
	if ( x1 > x2 ) slopex = -slopex;
	if ( y1 > y2 ) slopey = -slopey;
	
	int i = 0; for ( ; i <= dist; i++ )
	{
		if ( 
			( int )coordy >= 0 && ( int )coordy < bufferheight &&
			( int )coordx >= 0 && ( int )coordx < bufferwidth
		)
		{
			int coord = ( int )coordy * bufferwidth + ( int )coordx;
			buffer[ coord ] = value;
		}
		coordx += slopex; coordy += slopey;
	}
}

Affrect plotBrush ( 
	double x, double y,
	int bufferwidth, int bufferheight,
	unsigned long long int *buffer
)
{
	// Offset of brush top/left drawing in relation to x/y
	double offx = 0.0, offy = 0.0;
	
	// Special case for 1x1 brushes
	if ( brushTool.width == 1 && brushTool.height == 1 )
	{
		rgba64 sourcecol = *( rgba64 *)brushTool.buffer;
		if ( x < 0 || x >= bufferwidth || y < 0 || y >= bufferheight )
			return ( Affrect ){ 0, 0, 0, 0 };
		if ( brushTool.antialias )
			pixelAntialias ( x, y, sourcecol, bufferwidth, bufferheight, buffer );
		else
			pixelPlain ( ( int )x, ( int )y, sourcecol, bufferwidth, bufferheight, buffer );
	}
	else
	{
		// Precision to center the brush on x,y coordinate
		offx = ( double )brushTool.width / 2;
		offy = ( double )brushTool.height / 2;	
		int size = brushTool.width * brushTool.height;
		int i = 0; for ( ; i < size; i++ )
		{
			int bx = i % brushTool.width;
			int by = i / brushTool.width;
			double px = bx + x - offx;
			double py = by + y - offy;
			rgba64 col2 = *( rgba64 *)&brushTool.buffer[ i ];
			if ( brushTool.antialias ) pixelAntialias ( px, py, col2, bufferwidth, bufferheight, buffer );
			else pixelPlain ( ( int )px, ( int )py, col2, bufferwidth, bufferheight, buffer );
		}
	}
	
	// Return the affected area (x,y,w,h)
	Affrect rect = { ( int )( x - offx - 1 ), ( int )( y - offy - 1 ), brushTool.width + 2, brushTool.height + 2 };
	return rect;
}

inline void pixelAntialias ( 
	double x, double y, rgba64 paintcol, 
	int bw, int bh, unsigned long long int *buf
)
{
	int ox = ( int )x;
	int oy = ( int )y;
	   
	int any = 0; for ( ;any < 2; any++ )
	{
		int datay = oy + any; 
		if ( datay < 0 || datay >= bh ) continue;	
		int yml = datay * bw;
		int anx = 0; for ( ;anx < 2; anx++ )
		{
			int datax = ox + anx;
			if ( datax < 0 || datax >= bw ) continue;
			int dataoffset = yml + datax;
			rgba64 sourcecol = *( rgba64 *)&buf[ dataoffset ];			
			buf[ dataoffset ] = processPixel ( 
				sourcecol, paintcol, datax, datay, x - datax, y - datay
			);
		}
	}
}

inline void pixelPlain ( int x, int y, rgba64 paintcol, int bw, int bh, unsigned long long int *buf )
{
	if ( y < 0 || y >= bh || x < 0 || x >= bw ) return;
	int dataoffset = ( y * bw ) + x;
	rgba64 sourcecol = *( rgba64 *)&buf[ dataoffset ];
	buf[ dataoffset ] = processPixel ( sourcecol, paintcol, x, y, x, y );
}

inline unsigned long long int processPixel ( 
	rgba64 origCol, rgba64 paintCol, 
	double x, double y, double diffx, double diffy 
)
{	
	// Align colors correctly ( SHOULD BE FIXED IN THE FUTURE! )
	int r1 = origCol.a, 	g1 = origCol.b, 	b1 = origCol.g, 	a1 = origCol.r,
    	r2 = paintCol.a, 	g2 = paintCol.b, 	b2 = paintCol.g, 	a2 = paintCol.r;
	
	// Take into account the brush tool opacity and power
	a2 = ( ( double )a2 / 255 * brushTool.opacity ) / 100 * brushTool.power;
	
	// Calculate alpha
	double alpha = ( double )a2 / MAXCOLOR;
	
	// Decrease alpha with antialiasing calc.
	if ( brushTool.antialias )
	{
		double dist = sqrt ( ( diffx * diffx ) + ( diffy * diffy ) );
		alpha -= dist * alpha;
	}
	if ( alpha <= 0 )
		return *( unsigned long long int *)&origCol;
	else if ( alpha > 1 ) alpha = 1;
	
	switch ( brushTool.paintmode )
	{
		case LUNA_PAINTMODE_NORMAL:
			
			// Wonderful blending exception
			if ( a1 <= 0 )
			{ 
				r1 = r2; 
				g1 = g2; 
				b1 = b2; 
				if ( brushTool.opacitymode == LUNA_OPACITYMODE_ADD )
					a1 = a2 * alpha;
			}
			else
			{
				r1 -= ( r1 - r2 ) * alpha;
				g1 -= ( g1 - g2 ) * alpha;
				b1 -= ( b1 - b2 ) * alpha;
				
				if ( brushTool.opacitymode == LUNA_OPACITYMODE_ADD )
				{ 
					a1 += a2 * alpha; 
					if ( a1 > MAXCOLOR ) a1 = MAXCOLOR; 
				}
			}
			origCol.r = r1; origCol.g = g1; 
			origCol.b = b1; origCol.a = a1;
		
			break;
			
		case LUNA_PAINTMODE_COLOR:
			
			{
				rgba64 cols = PaletteToRgba64 ( globalPalette[ currColor ] );
				if ( a1 <= 0 )
				{ 
					r1 = cols.r; 
					g1 = cols.g; 
					b1 = cols.b; 
					if ( brushTool.opacitymode == LUNA_OPACITYMODE_ADD )
						a1 = a2 * alpha;
				}
				else
				{
					r1 -= ( r1 - cols.r ) * alpha;
					g1 -= ( g1 - cols.g ) * alpha;
					b1 -= ( b1 - cols.b ) * alpha;
					if ( brushTool.opacitymode == LUNA_OPACITYMODE_ADD )
					{
						a1 += a2 * alpha; 
						if ( a1 > MAXCOLOR ) a1 = MAXCOLOR;
					}
				}
				origCol.r = r1; origCol.g = g1; 
				origCol.b = b1; origCol.a = a1;
			}
		
			break;
			
		case LUNA_PAINTMODE_LIGHTEN:
			
			if ( a1 >= 0 )
			{
				r1 += ( r1 + 255 ) * alpha;
				g1 += ( g1 + 255 ) * alpha;
				b1 += ( b1 + 255 ) * alpha;
				if ( r1 > MAXCOLOR ) r1 = MAXCOLOR;
				if ( g1 > MAXCOLOR ) g1 = MAXCOLOR;
				if ( b1 > MAXCOLOR ) b1 = MAXCOLOR;
				origCol.r = r1; origCol.g = g1; 
				origCol.b = b1; origCol.a = a1;
			}
		
			break;
		
		case LUNA_PAINTMODE_DARKEN:
			
			if ( a1 >= 0 )
			{
				r1 -= ( 255 + r1 ) * alpha;
				g1 -= ( 255 + g1 ) * alpha;
				b1 -= ( 255 + b1 ) * alpha;
				if ( r1 < 0 ) r1 = 0;
				if ( g1 < 0 ) g1 = 0;
				if ( b1 < 0 ) b1 = 0;
				origCol.r = r1; origCol.g = g1; 
				origCol.b = b1; origCol.a = a1;
			}
		
			break;
			
		case LUNA_PAINTMODE_COLORIZE:
			
			if ( a1 >= 0 )
			{
				// Different product of each color
				int r3 = 0, g3 = 0, b3 = 0;
 				
 				double toneO = ( ( r1 + g1 + b1 ) / 3 );
 				double toneC = ( ( r2 + g2 + b2 ) / 3 );
 				
			
				r3 = ( r2 * toneO / toneC ); if ( r3 > MAXCOLOR ) r3 = MAXCOLOR; if ( r3 < 0 ) r3 = 0;
				g3 = ( g2 * toneO / toneC ); if ( g3 > MAXCOLOR ) g3 = MAXCOLOR; if ( g3 < 0 ) g3 = 0;
				b3 = ( b2 * toneO / toneC ); if ( b3 > MAXCOLOR ) b3 = MAXCOLOR; if ( b3 < 0 ) b3 = 0;
				
				r1 -= ( r1 - r3 ) * alpha;
				g1 -= ( g1 - g3 ) * alpha;
				b1 -= ( b1 - b3 ) * alpha;
				
				origCol.r = r1; origCol.g = g1; 
				origCol.b = b1; origCol.a = a1;
			}
		
			break;
		
		case LUNA_PAINTMODE_BLUR:
			
			{
				// TODO: Make power ajustable?
				int power = 1;
				
				int r = 0, g = 0, b = 0, a = 0;
				int samples = 0;
				
				int sy3 = 0; for ( sy3 = -power; sy3 <= power; sy3++ )
				{
					int yoffset = ( int )( y + sy3 ) * globalActiveCanvas->width;
					int sx3 = 0; for ( sx3 = -power; sx3 <= power; sx3++ )
					{
						if ( 
							x + sx3 >= 0 && x + sx3 < globalActiveCanvas->width && 
							y + sy3 >= 0 && y + sy3 < globalActiveCanvas->height )
						{
							int offset = yoffset + ( int )( x + sx3 );
							rgba64 tmp = *( rgba64 *)&globalActiveCanvas->activebuffer[ offset ];
							r += tmp.a;
							g += tmp.b;
							b += tmp.g;
							a += tmp.r;
							samples++;
						}
					}
				}
				
				if ( samples > 0 )
				{
					r = ( ( double )r / samples );
					g = ( ( double )g / samples );
					b = ( ( double )b / samples );
					
					r1 -= ( ( r1 - r ) * alpha );
					g1 -= ( ( g1 - g ) * alpha );
					b1 -= ( ( b1 - b ) * alpha );
					
					if ( brushTool.opacitymode == LUNA_OPACITYMODE_ADD )
					{
						a = ( ( double )a / samples );
						a1 -= ( ( a1 - a ) * alpha );
					}
				}
				
				origCol.r = r1; origCol.g = g1; 
				origCol.b = b1; origCol.a = a1;
			}
			break;
			
		case LUNA_PAINTMODE_SMUDGE:
		
			{
				// TODO: pick the color that is in front of the mouse movement
				int diffx = pMouseX - dMouseX;
				int diffy = pMouseY - dMouseY;
			
			
				// TODO: Make power ajustable?
				int power = 1;
				
				int r = 0, g = 0, b = 0, a = 0;
				int samples = 0;
				
				int sy3; for ( sy3 = -power + diffy; sy3 <= power + diffy; sy3++ )
				{
					int yoffset =  ( int )( y + sy3 ) * globalActiveCanvas->width;
					int sx3; for ( sx3 = -power + diffx; sx3 <= power + diffx; sx3++ )
					{
						if ( 
							x + sx3 >= 0 && x + sx3 < globalActiveCanvas->width && 
							y + sy3 >= 0 && y + sy3 < globalActiveCanvas->height )
						{
							int offset = yoffset + ( int )( x + sx3 );
							rgba64 tmp = *( rgba64 *)&globalActiveCanvas->activebuffer[ offset ];
							r += tmp.a;
							g += tmp.b;
							b += tmp.g;
							a += tmp.r;
							samples++;
						}
					}
				}
				if ( samples > 0 )
				{
					r = ( ( double )r / samples );
					g = ( ( double )g / samples );
					b = ( ( double )b / samples );

					r1 -= ( ( r1 - r ) * alpha );
					g1 -= ( ( g1 - g ) * alpha );
					b1 -= ( ( b1 - b ) * alpha );
					
					if ( brushTool.opacitymode == LUNA_OPACITYMODE_ADD )
					{
						a = ( ( double )a / samples );
						a1 -= ( ( a1 - a ) * alpha );
					}
				}	
				origCol.r = r1; origCol.g = g1; 
				origCol.b = b1; origCol.a = a1;
			}
			break;
		
		case LUNA_PAINTMODE_ERASE:
			
			if ( a1 >= 0 )
			{
				a1 -= a2 * alpha; if ( a1 < 0 ) a1 = 0;
				origCol.r = r1; origCol.g = g1; 
				origCol.b = b1; origCol.a = a1;
			}
			break;
		
		case LUNA_PAINTMODE_UNERASE:
			
			if ( a1 >= 0 )
			{
				a1 += a2 * alpha; if ( a1 > MAXCOLOR ) a1 = MAXCOLOR;
				origCol.r = r1; origCol.g = g1; 
				origCol.b = b1; origCol.a = a1;
			}
			break;
		
	}
		
	// Resulting 64bit pixel
	rgba64 result = { origCol.a, origCol.b, origCol.g, origCol.r };
	
	// Snap to colors
	if ( globalColorMode == LUNA_COLORMODE_SNAP )
		result = snapToPalette ( result, FALSE );
	
	return *( unsigned long long int * )&result;
}

Affrect floodFill (
	int x, int y,
 	int bufferwidth, int bufferheight,
	unsigned long long int color, 
	unsigned long long int clickColor,
	unsigned long long int *buffer,
	unsigned int threshold
)
{	
	// Exit if out of bounds for filling or otherwise we're not allowed
	if ( x < 0 || y < 0 || x >= bufferwidth || y >= bufferheight || clickColor == color || isFilling == TRUE )
	{
		return ( Affrect ){ 0, 0, 0, 0 };
	}
	
	isFilling = TRUE;
	
	// redraw all rect =)
	Affrect rect = { 0, 0, bufferwidth, bufferheight };
	
	unsigned int click8 = bufferToScreenColor ( clickColor );
	unsigned int clicka = ( click8 << 24 ) >> 24; // a part of rgba
	
	// Max stacksize
	int stacksize = bufferwidth * bufferheight;
	int *fStack = AllocVec ( 4 * stacksize, MEMF_ANY|MEMF_CLEAR );
	fStackPointer = 0;
	int fx = x, fy = y;
	
	// Add first pixel where the mouse clicked
	fPush ( fx, fy, stacksize, bufferwidth, fStack );
	
	// Do - until there's no more pixels to fill in the stack
	while ( fPop ( &fx, &fy, bufferwidth, fStack ) )
	{	
		// Watch for buffer overflow
		if ( fStackPointer >= stacksize - 1 ) 
		{
			FreeVec ( fStack );
			return rect;
		}
		
		// rowpos here
		int boffset = ( fy * bufferwidth );
		
		// Fill
		buffer[ boffset + fx ] = color;
		
		// The colors to the n,e,s,w directions
		unsigned long long int topcol = 0ULL, botcol = 0ULL, lefcol = 0ULL, rigcol = 0ULL;
		unsigned int rigcol8 = 0U, lefcol8 = 0U, botcol8 = 0U, topcol8 = 0U;
		unsigned int riga = 0U, lefa = 0U, bota = 0U, topa = 0U; // <- a part of rgba
		
		// Make sure the coords bound to box
		BOOL tz = TRUE, bz = TRUE, lz = TRUE, rz = TRUE;
		if ( fx + 1 < bufferwidth )
		{
			rigcol = buffer[ boffset + fx + 1 ];
			rigcol8 = bufferToScreenColor ( rigcol );
			riga = ( rigcol8 << 24 ) >> 24;
		}
		else rz = FALSE;
		if ( fx - 1 >= 0 )
		{
			lefcol = buffer[ boffset + fx - 1 ];
			lefcol8 = bufferToScreenColor ( lefcol );
			lefa = ( lefcol8 << 24 ) >> 24;
		}
		else lz = FALSE;
		if ( fy + 1 < bufferheight )
		{
			botcol = buffer[ ( ( fy + 1 ) * bufferwidth ) + fx ];
			botcol8 = bufferToScreenColor ( botcol );
			bota = ( botcol8 << 24 ) >> 24;
		}
		else bz = FALSE;
		if ( fy - 1 >= 0 )
		{
			topcol = buffer[ ( ( fy - 1 ) * bufferwidth ) + fx ];
			topcol8 = bufferToScreenColor ( topcol );
			topa = ( topcol8 << 24 ) >> 24;
		}
		else tz = FALSE;
		
		// Add pixels within bounds 64-bit
		/*if ( rz && clickColor == rigcol )
			fPush ( fx + 1, fy, stacksize, bufferwidth, fStack );
		if ( lz && clickColor == lefcol )
			fPush ( fx - 1, fy, stacksize, bufferwidth, fStack );
		if ( bz && clickColor == botcol )
			fPush ( fx, fy + 1, stacksize, bufferwidth, fStack );
		if ( tz && clickColor == topcol )
			fPush ( fx, fy - 1, stacksize, bufferwidth, fStack );*/
			
		// Add pixels within bounds 24-bit
		// Fill if clickcolor or that clickcolor alpha = 0 and destcolor alpha = 0
		if ( rz && ( click8 == rigcol8 || ( !riga && !clicka ) ) )
		{
			fPush ( fx + 1, fy, stacksize, bufferwidth, fStack );
		}
		if ( lz && ( click8 == lefcol8 || ( !lefa && !clicka ) ) )
		{
			fPush ( fx - 1, fy, stacksize, bufferwidth, fStack );
		}
		if ( bz && ( click8 == botcol8 || ( !bota && !clicka ) ) )
		{
			fPush ( fx, fy + 1, stacksize, bufferwidth, fStack );
		}
		if ( tz && ( click8 == topcol8 || ( !topa && !clicka ) ) )
		{
			fPush ( fx, fy - 1, stacksize, bufferwidth, fStack );
		}
	}
	FreeVec ( fStack );
	isFilling = FALSE;
	return rect;
}

void fillCharbuf (
	int x, int y, int bufferwidth, int bufferheight, 
	unsigned char *buffer, unsigned char value
)
{
	
	// Max stacksize
	int stacksize = bufferwidth * bufferheight;
	int *fStack = AllocVec ( stacksize * 4, MEMF_ANY|MEMF_CLEAR );
	fStackPointer = 0;
	int fx = x, fy = y;
	unsigned char clickColor = buffer[ y * bufferwidth + x ];
	if ( clickColor == value ) return;
	
	// Exit if out of bounds for filling!
	if ( x < 0 || y < 0 || x >= bufferwidth || y >= bufferheight )
	{
		FreeVec ( fStack ); return;
	}
	
	// Add first pixel where the mouse clicked
	fPush ( fx, fy, stacksize, bufferwidth, fStack );
	
	// Do - until there's no more pixels to fill in the stack
	while ( fPop ( &fx, &fy, bufferwidth, fStack ) )
	{
		// Watch for buffer overflow
		if ( fStackPointer >= stacksize - 1 ) 
		{
			FreeVec ( fStack ); return;
		}
		
		// rowpos here
		int boffset = ( fy * bufferwidth );
		
		// Fill
		buffer[ boffset + fx ] = value;
		
		// The colors to the n,e,s,w directions
		unsigned char topcol = 0, botcol = 0, lefcol = 0, rigcol = 0;
		
		// Make sure the coords bound to box
		BOOL tz = TRUE, bz = TRUE, lz = TRUE, rz = TRUE;
		if ( fx + 1 < bufferwidth )
			rigcol = buffer[ boffset + fx + 1 ];
		else rz = FALSE;
		if ( fx - 1 >= 0 )
			lefcol = buffer[ boffset + fx - 1 ];
		else lz = FALSE;
		if ( fy + 1 < bufferheight )
			botcol = buffer[ ( ( fy + 1 ) * bufferwidth ) + fx ];
		else bz = FALSE;
		if ( fy - 1 >= 0 )
			topcol = buffer[ ( ( fy - 1 ) * bufferwidth ) + fx ];
		else tz = FALSE;
		
		// Add pixels within bounds
		if ( rz && clickColor == rigcol )
			fPush ( fx + 1, fy, stacksize, bufferwidth, fStack );
		if ( lz && clickColor == lefcol )
			fPush ( fx - 1, fy, stacksize, bufferwidth, fStack );
		if ( bz && clickColor == botcol )
			fPush ( fx, fy + 1, stacksize, bufferwidth, fStack );
		if ( tz && clickColor == topcol )
			fPush ( fx, fy - 1, stacksize, bufferwidth, fStack );
	}
	FreeVec ( fStack );
}


Affrect drawCircle (
	double x, double y, double w, double h,
	unsigned int bufferwidth, unsigned int bufferheight,
	unsigned long long int *buffer
)
{
	int offx = ( double )brushTool.width / 2;
	int offy = ( double )brushTool.height / 2;

	double prevx = cos ( RAD ) * w + x;
	double prevy = sin ( RAD ) * h + y;		
	
	if ( filledDrawing != DRAW_DRAW )
	{
		// Setup data
		int size = h > w ? h : w;
		int size2 = size * 2 + 1;
		int datalen = size2 * size2;
		double fillradius = size;
		unsigned long long int *tmpBuf = AllocVec ( datalen * 8, MEMF_ANY );
		
		// Make a uniform circular shape
		int i = 0; for ( ; i < datalen; i++ )
		{
			int yc = i / size2;
			int xc = i % size2;
			int tmpoff = yc * size2 + xc;
			if ( getDistance ( xc, yc, size, size ) < fillradius )
				tmpBuf[ tmpoff ] = 0xffffffffffffffffULL;
			else
				tmpBuf[ tmpoff ] = 0x0000000000000000ULL;
		}
		
		// Copy the tmpbuf data scaled to fit the wanted shape
		int sx = x - w;
		int dx = x + w;
		int sy = y - h;
		int dy = y + h;
		int w2 = w * 2;
		int h2 = h * 2;
		
		BOOL ba = brushTool.antialias;
		brushTool.antialias = FALSE;
		
		int yc = sy; for ( ; yc <= dy; yc++ )
		{
			int xc = sx; for ( ; xc <= dx; xc++ )
			{	
				int prx = ( xc - x + w ) / w2 * size2;
				int pry = ( yc - y + h ) / h2 * size2;
				int offset = pry * size2 + prx;
				if ( offset < 0 || offset >= datalen ) continue;
				if ( tmpBuf[ offset ] != 0x0000000000000000ULL )
					plotBrush ( xc, yc, bufferwidth, bufferheight, buffer );
			}
		}
		FreeVec ( tmpBuf );
		brushTool.antialias = ba;
	}
	
	if ( filledDrawing == DRAW_DRAW || filledDrawing == DRAW_DRAWFILLED )
	{
		int a = 0; for ( ; a <= 361; a++ )
		{
			double phase = a * RAD;
			double curx = cos ( phase ) * w + x;
			double cury = sin ( phase ) * h + y;
					
			if ( floor ( prevx ) != floor ( curx ) || floor ( prevy ) != floor ( cury ) )
			{
				drawLine ( 
					prevx, prevy,
					curx, cury,
					bufferwidth, bufferheight,
					buffer, FALSE
				);
				prevx = curx;
				prevy = cury;
			}
		}
	}
	Affrect rect = { 
		x - w - offx - 1, y - h - offy - 1, 
		x + w + offx + 2, y + h + offy + 2 
	};
	return rect;
}

rgba64 snapToPalette ( rgba64 color, BOOL selectIndex )
{
	int rating = 1024; // high number
	int index = 0;
	rgba32 testcol = { color.r / 256, color.g / 256, color.b / 256, color.a / 256 };
	
	// find the colors that are the most alike
	// giving the best diff rating (the least different)
	int c = 0; for ( ; c < 256; c++ )
	{
		rgba32 curcol = *( rgba32 *)&globalPalette[ c ];
		int test =
			abs( curcol.r - testcol.a ) +
			abs( curcol.g - testcol.b ) +
			abs( curcol.b - testcol.g );
		if ( test < rating )
		{
			index = c;
			rating = test;
		}
	}
	// Return the result
	rgba32 rescol = *( rgba32 *)&globalPalette[ index ];
	
	if ( selectIndex )
	{
		currColor = index;
		DoMethod ( tbxAreaPalette, MUIM_Draw );
	}
	
	rgba64 res = { 
		MAXCOLOR,
		( unsigned long long int )( ( double )rescol.b / 255 * MAXCOLOR ),
		( unsigned long long int )( ( double )rescol.g / 255 * MAXCOLOR ),
		( unsigned long long int )( ( double )rescol.r / 255 * MAXCOLOR )
	};
	return res;
}
