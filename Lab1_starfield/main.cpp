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
const int f = SCREEN_HEIGHT / 2;//focal length
const float velocity = 0.001;
int t;//time between two frames
SDL_Surface* screen;
vector<vec3> stars(1000);//stars
// --------------------------------------------------------
// FUNCTION DECLARATIONS

void Draw();
void Update();
// --------------------------------------------------------
// FUNCTION DEFINITIONS

int main(int argc, char* argv[])
{
	screen = InitializeSDL(SCREEN_WIDTH, SCREEN_HEIGHT);
	t = SDL_GetTicks();
	float r = float(rand()) / float(RAND_MAX);
	for (int i = 0; i < 1000; ++i)
	{
		float x_sign= float(rand()) / float(RAND_MAX);//the sign of x
		float y_sign = float(rand()) / float(RAND_MAX);// the sign of y
		if (x_sign>=0.5)
			stars[i].x = -float(rand()) / float(RAND_MAX);
		else
			stars[i].x = float(rand()) / float(RAND_MAX);
		if (y_sign >= 0.5)
			stars[i].y = -float(rand()) / float(RAND_MAX);
		else
			stars[i].y = float(rand()) / float(RAND_MAX);
		stars[i].z = float(rand()) / float(RAND_MAX);
	}

	while (NoQuitMessageSDL())
	{
		Update();
		Draw();
	}
	SDL_SaveBMP(screen, "screenshot.bmp");
	return 0;
}

void Draw()
{
	SDL_FillRect(screen, 0, 0);
	if (SDL_MUSTLOCK(screen))
		SDL_LockSurface(screen);
	for (size_t s = 0; s<stars.size(); ++s)
	{
		// Add code for projecting and drawing each star
		vec3 color = 0.2f * vec3(1, 1, 1) / (stars[s].z*stars[s].z);
		int u = f*(stars[s].x / stars[s].z) + SCREEN_WIDTH / 2;
		int v = f*(stars[s].y / stars[s].z) + SCREEN_HEIGHT / 2;
		PutPixelSDL(screen, u, v, color);
	}
	if (SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);
	SDL_UpdateRect(screen, 0, 0, 0, 0);
}
void Update()
{
	int t2 = SDL_GetTicks();
	float dt = float(t2 - t);
	t = t2;
	for (int i = 0; i < 1000; ++i)
	{
		stars[i].z = stars[i].z - velocity*dt;
	}
	for (int s = 0; s<stars.size(); ++s)
	{
		// Add code for update of stars
		if (stars[s].z <= 0)
			stars[s].z += 1;
		if (stars[s].z > 1)
			stars[s].z -= 1;
	}
}
