/*
Copyright 2012, Bas Fagginger Auer.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/*
Directly inspired by http://blog.wolfram.com/2009/04/24/droste-effect-with-mathematica/.
*/
#include <iostream>
#include <iomanip>
#include <fstream>
#include <exception>

#include <cmath>
#include <cassert>

#include <SDL.h>

using namespace std;

void escher(SDL_Surface *dest, SDL_Surface *src, const Uint32 clearColour, const float xOff, const float yOff, const float _alpha, const float _beta)
{
	assert(dest && src);
	assert(dest->w == src->w && dest->h == src->h && dest->format->BytesPerPixel == src->format->BytesPerPixel);
	
	//Perform Escher mapping on src surface.
	const int bpp = dest->format->BytesPerPixel;
	const float alpha = _alpha/(2.0f*M_PI);
	const float beta = _beta/(2.0f*M_PI);
	const float baseScale = expf(-2.0f*M_PI*alpha);
	const int maxTries = 32;
	
	assert(bpp == 3);
	
	for (int iy = 0; iy < dest->h; ++iy)
	{
		Uint8 *p = (Uint8 *)dest->pixels + iy*dest->pitch;
		
		for (int ix = 0; ix < dest->w; ++ix)
		{
			//Determine vector to current pixel.
			float x = (float)ix - xOff;
			float y = yOff - (float)iy;
			float r = sqrtf(x*x + y*y);
			
			//Return if we are too close to the centre.
			if (r < 1.0e-6)
			{
				p[0] = 0x0;
				p[1] = 0x0;
				p[2] = 0x0;
			}
			else
			{
				//Determine angle between 0 and 2*M_PI.
				float a = atan2f(y, x);
				
				//Scale by an angle-dependent factor.
				float tmp = r*0.5f*expf(-alpha*(a + M_PI));
				a -= beta*logf(r) + 0.125f*M_PI/4.0f;
				r = tmp;
				x = cos(a);
				y = sin(a);
				
				//Scale until we are in a forbidden area.
				int count = maxTries;
				
				while (count-- > 0)
				{
					const int ix2 = (int)(xOff + r*x);
					const int iy2 = (int)(yOff - r*y);
					
					if (ix2 < 0 || iy2 < 0 || ix2 >= src->w || iy2 >= src->h)
					{
						//We are outside of the source image.
						r *= baseScale;
					}
					else
					{
						//Have we reached a forbidden pixel?
						const Uint8 *q = (Uint8 *)src->pixels + iy2*src->pitch + ix2*bpp;
						
						if (((Uint32)q[0] | ((Uint32)q[1] << 8) | ((Uint32)q[2] << 16)) == clearColour)
						{
							count = 0;
						}
						else
						{
							r *= baseScale;
						}
					}
				}
				
				//Scale until we are in the picture.
				count = maxTries;
				
				while (count-- > 0)
				{
					const int ix2 = (int)(xOff + r*x);
					const int iy2 = (int)(yOff - r*y);
					
					if (ix2 < 0 || iy2 < 0 || ix2 >= src->w || iy2 >= src->h)
					{
						//We are outside of the source image.
						p[0] = 0xff;
						p[1] = 0x0;
						p[2] = 0x0;
						count = 0;
					}
					else
					{
						//Have we reached a forbidden pixel?
						const Uint8 *q = (Uint8 *)src->pixels + iy2*src->pitch + ix2*bpp;
						
						if (((Uint32)q[0] | ((Uint32)q[1] << 8) | ((Uint32)q[2] << 16)) != clearColour)
						{
							p[0] = q[0];
							p[1] = q[1];
							p[2] = q[2];
							count = 0;
						}
						else
						{
							r /= baseScale;
						}
					}
				}
			}
			
			p += bpp;
		}
	}
}

Uint32 getPixel(const SDL_Surface *image, const int x, const int y)
{
	assert(image);
	
	if (x < 0 || y < 0 || x >= image->w || y >= image->h) return 0;
	
	const Uint8 *q = (const Uint8 *)image->pixels + y*image->pitch + x*image->format->BytesPerPixel;
	
	return (Uint32)q[0] | ((Uint32)q[1] << 8) | ((Uint32)q[2] << 16);
}

int main(int argc, char **argv)
{
	int xOff = 0, yOff = 0;
	Uint32 clearColour = 0x00000000;
	float alpha = 1.0f, beta = 0.0f;
	
	//Start SDL.
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		cerr << "Unable to initialise SDL: " << SDL_GetError() << endl;
		return -1;
	}
	
	//Read image.
	SDL_Surface *image = 0;
	
	try
	{
		if (argc != 2)
		{
			cerr << "Usage: " << argv[0] << " foo.bmp" << endl;
			throw exception();
		}
		
		image = SDL_LoadBMP(argv[1]);
		
		if (!image)
		{
			cerr << "Unable to read image: " << SDL_GetError() << endl;
			throw exception();
		}
		
		if (image->w <= 0 || image->h <= 0 || image->format->BytesPerPixel != 3)
		{
			cerr << "Empty image or non-RGB image!" << endl;
			throw exception();
		}
		
		xOff = image->w/2;
		yOff = image->h/2;
		clearColour = getPixel(image, xOff, yOff);
		
		cerr << "Performing Escher on " << image->w << "x" << image->h << " image '" << argv[1] << "'..." << endl;
	}
	catch (exception &e)
	{
		cerr << "Invalid command line arguments or unable to read image!" << endl;
		return -1;
	}

	SDL_Surface *screen = SDL_SetVideoMode(2*image->w, image->h, 24, SDL_SWSURFACE);
	SDL_Rect imageRect = {0, 0, image->w, image->h};
	SDL_Rect escherRect = {image->w, 0, image->w, image->h};
	
	if (!screen)
	{
		cerr << "Unable to create window: " << SDL_GetError() << endl;
		return -1;
	}
	
	//Set caption and clear screen.
	SDL_WM_SetCaption("Escher", "Escher");
	SDL_FillRect(screen, &screen->clip_rect, 0x00000000);
	SDL_BlitSurface(image, 0, screen, &imageRect);
	SDL_Flip(screen);
	
	//Enter main loop.
	bool running = true;
	
	while (running)
	{
		//Take care of incoming events.
		bool changed = false;	
		SDL_Event event;
		
		while (SDL_PollEvent(&event))
		{
			//Handle keypresses.
			if (event.type == SDL_KEYDOWN)
			{
				const SDLKey k = event.key.keysym.sym;
				
				if (k == SDLK_ESCAPE)
				{
					running = false;
				}
				else if (k == SDLK_SPACE)
				{
					//Restore old image.
					SDL_BlitSurface(image, 0, screen, &imageRect);
					changed = true;
				}
				else if (k == SDLK_LEFT)
				{
					alpha -= 0.1f;
					changed = true;
				}
				else if (k == SDLK_RIGHT)
				{
					alpha += 0.1f;
					changed = true;
				}
				else if (k == SDLK_UP)
				{
					beta += 0.1f;
					changed = true;
				}
				else if (k == SDLK_DOWN)
				{
					beta -= 0.1f;
					changed = true;
				}
			}
			else if (event.type == SDL_MOUSEBUTTONUP)
			{
				//Has the user clicked inside the source image?
				if (event.button.x < image->w && event.button.y < image->h)
				{
					//Calculate Escher.
					xOff = event.button.x;
					yOff = event.button.y;
					clearColour = getPixel(image, xOff, yOff);
					changed = true;
				}
			}
			
			if (event.type == SDL_QUIT)
			{
				running = false;
			}
		}
		
		if (changed)
		{
			cerr << "Performing Escher at (" << xOff << ", " << yOff << "), colour " << clearColour << ", with alpha = " << alpha << ", and beta = " << beta << " divided by two pi..." << endl;
			
			SDL_Surface *dest = SDL_ConvertSurface(image, image->format, image->flags);
			
			assert(dest);
			escher(dest, image, clearColour, xOff, yOff, alpha, beta);
			SDL_SaveBMP(dest, "escher.bmp");
			SDL_BlitSurface(dest, 0, screen, &escherRect);
			SDL_FreeSurface(dest);
			SDL_Flip(screen);
		}
		
		SDL_Delay(25);
	}
	
	//Clean up.
	SDL_FreeSurface(image);
	
	SDL_Quit();
	
	cerr << "Bye." << endl;

	return 0;
}

