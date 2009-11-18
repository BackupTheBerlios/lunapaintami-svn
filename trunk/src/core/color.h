#ifndef _colorconversion_h_ 
#define _colorconversion_h_

#define MAXCOLOR 65535
#define MAXCOLOR_DBL 65535.0
#define TRANSCOLOR 0x4444444444440000ULL
#define TRANSVALUE 0x0000000000004444ULL


/*
    Some color structs
*/

typedef struct
{
    unsigned int r, g, b;
} rgbData;

typedef struct
{
    unsigned long long int r, g, b, a;
} rgbaDataL;

typedef struct
{
    unsigned int r, g, b, a;
} rgbaData;

typedef struct
{
    unsigned char r, g, b, a;
} rgba32;

typedef struct
{
    unsigned char r, g, b;
} rgb24;

typedef struct
{
    unsigned short r, g, b, a;
} rgba64;

typedef struct
{
    unsigned char b, g, r, a;
} bgra32;

typedef struct
{
    unsigned char a, b, g, r;
} abgr32;

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
    Convert a 16-bit pr color to a 8-bit pr color pixel
*/
unsigned int canvasToWindowPixel ( unsigned long long int source );

/*
    Convert to an ARGB format for export etc
*/
unsigned int canvasToARGBPixel ( unsigned long long int source );

#endif
