// Introduction lab that covers:
// * C++
// * SDL
// * 2D graphics
// * Plotting pixels
// * Video memory
// * Color representation
// * Linear interpolation
// * glm::vec3 and std::vector

#include "SDL.h"
#include <iostream>
#include <glm/glm.hpp>
#include <vector>
#include "SDLauxiliary.h"

using namespace std;
using glm::vec3;

// --------------------------------------------------------
// GLOBAL VARIABLES

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
SDL_Surface* screen;

// --------------------------------------------------------
// FUNCTION DECLARATIONS

void Draw();
void Interpolate(vec3 a, vec3 b, vector<vec3>& result);

// --------------------------------------------------------
// FUNCTION DEFINITIONS

int main(int argc, char* argv[])
{
	screen = InitializeSDL(SCREEN_WIDTH, SCREEN_HEIGHT);
	while (NoQuitMessageSDL())
	{
		Draw();
	}
	SDL_SaveBMP(screen, "screenshot.bmp");
	return 0;
}

void Draw()
{
	//four corners
	vec3 topLeft(1, 0, 0); // red
	vec3 topRight(0, 0, 1); // blue
	vec3 bottomLeft(0, 1, 0); // green
	vec3 bottomRight(1, 1, 0); // yellow
	//
	vector<vec3> leftSide(SCREEN_HEIGHT);
	vector<vec3> rightSide(SCREEN_HEIGHT);
	Interpolate(topLeft, bottomLeft, leftSide);
	Interpolate(topRight, bottomRight, rightSide);
	for (int y = 0; y<SCREEN_HEIGHT; ++y)
	{
		vector<vec3> rows(SCREEN_WIDTH);
		Interpolate(leftSide[y], rightSide[y], rows);
		for (int x = 0; x<SCREEN_WIDTH; ++x)
		{

			PutPixelSDL(screen, x, y, rows[x]);
		}
	}

	if (SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);

	SDL_UpdateRect(screen, 0, 0, 0, 0);
}
void Interpolate(vec3 a, vec3 b, vector<vec3>& result)
{
	float distance_r;
	float distance_g;
	float distance_b;
	distance_r = (b.r - a.r) / (result.size() - 1);
	distance_g = (b.g - a.g) / (result.size() - 1);
	distance_b = (b.b - a.b) / (result.size() - 1);
	for (int i = 0; i < result.size(); ++i)
	{
		result[i].r = a.r + distance_r*i;
		result[i].g = a.g + distance_g*i;
		result[i].b = a.b + distance_b*i;

	}


}
