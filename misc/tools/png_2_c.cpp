#include <SDL.h>
#include <SDL_image.h>

void writePixels( FILE* f, char* _symbolNameBase, SDL_Surface* image, int* _pTotalOutputSize, bool has_alpha = false)
{
    fprintf( f, "static const int %s_width = %d;\n", _symbolNameBase, image->w );
    fprintf( f, "static const int %s_height = %d;\n", _symbolNameBase, image->h );
    fprintf( f, "extern const uchar %s_pixels[];\n\n", _symbolNameBase );

	fprintf( f, "const uchar %s_pixels[] =\n{\n", _symbolNameBase );

	unsigned char* pixels = (unsigned char*)image->pixels;
	int x, y;
    unsigned char b, g, r, a;
	for( y = 0; y < image->h; y++ )
	{
		fprintf( f, "\t" );
		for( x = 0; x < image->w; x++ )
		{
			int rofs = ((y * image->w) + x) * (has_alpha ? 4 : 3);
			b = pixels[ rofs + 0 ];
			g = pixels[ rofs + 1 ];
			r = pixels[ rofs + 2 ];

			if (has_alpha)
            {
                a = pixels[ rofs + 3 ];
                fprintf( f, "0x%02x,0x%02x,0x%02x,0x%02x,", a, b, g, r );
            }
            else
            {
                fprintf( f, "0x%02x,0x%02x,0x%02x,", b, g, r );
            }
		}
		fprintf( f, "\n" );
	}
	
	*_pTotalOutputSize += image->w * image->h * (has_alpha ? 4 : 3);
	
	fprintf( f, "};\n\n" );
}


SDL_Surface* LoadImage( char* _fileName )
{
	SDL_Surface* image = IMG_Load( _fileName );
	//printf("Image=0x%016llx\n", (long long)image );
	bool isAlpha = SDL_ISPIXELFORMAT_ALPHA( image->format->format );
	bool isIndexed = SDL_ISPIXELFORMAT_INDEXED( image->format->format );

	return image;
}

FILE* openOutfileC( char* _baseOutFileName )
{
	char outname_c[ 2048 ];
	sprintf( outname_c, "%s.cpp", _baseOutFileName );
	FILE* f = fopen( outname_c, "w" );
	
	return f;
}

int main( int _numargs, char** _apszArgh )
{
	if( _numargs != 4 )
	{
		printf("Usage error: Program need 3 arguments:\n");
		printf("  png2c <in_file.png> <out_file_base> <symbol_name>\n");
		return -1;
	}

	char* pszInFileName = _apszArgh[ 1 ];
	char* pszOutFilenameBase = _apszArgh[ 2 ];
	char* pszSymbolNameBase = _apszArgh[ 3 ];
	
	//
	SDL_Surface* image = LoadImage( pszInFileName );

	//
	// Write cpp file
	//
	FILE* f = openOutfileC( pszOutFilenameBase );

	int totalOutputSize = 0;
	writePixels( f, pszSymbolNameBase, image, &totalOutputSize, SDL_ISPIXELFORMAT_ALPHA( image->format->format) );
	fclose( f );

	printf("Total output size: %i\n", totalOutputSize );
	return 0;
}
