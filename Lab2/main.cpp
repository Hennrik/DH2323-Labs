#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModel.h"

using namespace std;
using glm::vec3;
using glm::mat3;
using glm::cos;
using glm::sin;

// Intersection
struct Intersection
{
	vec3 position;
	float distance;
	int triangleIndex;
};
// ----------------------------------------------------------------------------
// GLOBAL VARIABLES

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT =500;
//const float FOCAL_RATIO = 3.0f / 2.0f;
//const float focalLength = SCREEN_HEIGHT * FOCAL_RATIO;
const float focalLength = SCREEN_HEIGHT;
float yaw = 0;//angle
mat3 R;//rotation matrix
vec3 cameraPos(0, 0, -(2*focalLength / SCREEN_HEIGHT+1) );
vector<Triangle> triangles;
//direct light
vec3 lightPos(0, -0.5, -0.7);
vec3 lightColor = 14.f * vec3(1, 1, 1);
vec3 black(0, 0, 0);
vec3 indirectLight = 0.5f*vec3(1, 1, 1);
SDL_Surface* screen;
int t;

// ----------------------------------------------------------------------------
// FUNCTIONS

void Update();
void Draw();
bool ClosestIntersection(
	vec3 start,
	vec3 dir,
	const vector<Triangle>& triangles,
	int conflict_triangle,
	Intersection& closestIntersection
	);
vec3 DirectLight(const Intersection& i);
int main(int argc, char* argv[])
{
	screen = InitializeSDL(SCREEN_WIDTH, SCREEN_HEIGHT);
	t = SDL_GetTicks();	// Set start value for timer.
	LoadTestModel(triangles);
	while (NoQuitMessageSDL())
	{
		Update();
		Draw();
	}

	SDL_SaveBMP(screen, "screenshot.bmp");
	return 0;
}

bool ClosestIntersection(
	vec3 start,
	vec3 dir,
	const vector<Triangle>& triangles,
	int conflict_triangle,
	Intersection& closestIntersection
	)
{
	closestIntersection.distance = std::numeric_limits<float>::max();
	bool if_intersection = false;
	for (int i = 0; i < triangles.size(); i++)
	{
		vec3 v0 = triangles[i].v0*R;
		vec3 v1 = triangles[i].v1*R;
		vec3 v2 = triangles[i].v2*R;
		vec3 e1 = v1 - v0;
		vec3 e2 = v2 - v0;
		vec3 b = start - v0;
		mat3 A(-dir, e1, e2);
		vec3 x = glm::inverse(A) * b;
		float t = x.x;
		float u = x.y;
		float v = x.z;
		vec3 r = v0 + u*e1 + v*e2;
		if (u >= 0 && v >= 0 && (u + v) <1 && t >0 && conflict_triangle!=i)
		{
			if_intersection = true;
			// if there are several intersection point, choose the nearest(distance with camera) one
			if (t < closestIntersection.distance)
			{
				closestIntersection.position = r;
				closestIntersection.distance = t;
				closestIntersection.triangleIndex = i;

			}

		}
	}
	return if_intersection;
}
void Update()
{
	// Compute frame time:
	int t2 = SDL_GetTicks();
	float dt = float(t2 - t);
	t = t2;
	cout << "Render time: " << dt << " ms." << endl;
	Uint8* keystate = SDL_GetKeyState(0);
	float theta = 0.1;
	if (keystate[SDLK_UP])
	{
		// Move camera forward
		cameraPos.z += theta;

	}
	if (keystate[SDLK_DOWN])
	{
		// Move camera backward
		cameraPos.z += theta;
	}
	if (keystate[SDLK_LEFT])
	{
		// Move camera to the left
		yaw += theta;
	}
	if (keystate[SDLK_RIGHT])
	{
		// Move camera to the right
		yaw -= theta;
	}
	if (keystate[SDLK_w])
	{
		//Move light forward
		lightPos.z += theta;
	}
	if (keystate[SDLK_s])
	{
		//Move light backward
		lightPos.z -= theta;
	}
	if (keystate[SDLK_a])
	{
		//Move light to the left
		lightPos.x -= theta;
	}
	if (keystate[SDLK_d])
	{
		//Move light to the right
		lightPos.x += theta;
	}
	if (keystate[SDLK_q])
	{
		//Move light up
		lightPos.y += theta;
	}
	if (keystate[SDLK_e])
	{
		//Move light down
		lightPos.y -= theta;
	}
	R = mat3(cos(yaw), 0, sin(yaw),
		            0, 1, 0,
		-sin(yaw), 0, cos(yaw));

}
void Draw()
{
	if (SDL_MUSTLOCK(screen))
		SDL_LockSurface(screen);
	for (int y = 0; y<SCREEN_HEIGHT; ++y)
	{
		for (int x = 0; x<SCREEN_WIDTH; ++x)
		{
			vec3 d(x - SCREEN_WIDTH/2, y - SCREEN_HEIGHT/2, focalLength);
			Intersection Inter_point;
			if (ClosestIntersection(cameraPos, d, triangles,-1,Inter_point))
			{
				vec3 color = triangles[Inter_point.triangleIndex].color;
				vec3 D = DirectLight(Inter_point);
				PutPixelSDL(screen, x, y, color*(D + indirectLight));
			}
			else
				PutPixelSDL(screen, x, y, black);
		}
	}

	if (SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);

	SDL_UpdateRect(screen, 0, 0, 0, 0);
}
vec3 DirectLight(const Intersection& i)
{
	float r = glm::distance(lightPos, i.position);
	vec3 r_hat = glm::normalize(lightPos - i.position);
	vec3 n_hat = triangles[i.triangleIndex].normal;
	float product = glm::dot(r_hat, n_hat);
	if (product < 0)
		product = 0;
	vec3 D = lightColor*product / (4 * 3.1415926f*r*r);
	Intersection inter_point;//inter_point and i are in the same line

	if (ClosestIntersection(i.position, r_hat, triangles,i.triangleIndex,inter_point))
	{
		float point2point_distance = glm::distance(inter_point.position, i.position);
		if (point2point_distance<r){
			return black;
		}
	}
	return D;
}
