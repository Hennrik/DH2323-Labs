#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModel.h"

using namespace std;
using glm::vec3;
using glm::ivec2;
using glm::mat3;
using glm::vec2;

// ----------------------------------------------------------------------------
// GLOBAL VARIABLES

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 500;
const float focalLength = SCREEN_HEIGHT;
vec3 cameraPos(0, 0, -3.001);
vec3 currentColor;
vec3 lightPos(0, -0.5, -0.7);
vec3 lightPower = 14.1f*vec3(1, 1, 1);
vec3 indirectLightPowerPerArea = 0.5f*vec3(1, 1, 1);
SDL_Surface* screen;
int t;
vector<Triangle> triangles;
mat3 Ry;//rotation matrix y
mat3 Rx;//rotation matrix x
float yaw = 0;// Yaw angle controlling camera rotation around y-axis
float xaw = 0; // xaw angle controlling camera rotation around x - axis
float depthBuffer[SCREEN_HEIGHT][SCREEN_WIDTH];//store inverse depth 
vec3 reflectance(1, 1, 1);
vec3 currentNormal;
vec3 currentReflectance;
//-----------------------------------------------------------------------------
//data structer
struct Pixel
{
	int x;
	int y;
	float zinv;//inverse depth in 3d space
//	vec3 illumination;
	vec3 pos3d;
};
struct Vertex
{
	vec3 position;
//	vec3 normal;
//	vec3 reflectance;
};
// ----------------------------------------------------------------------------
// FUNCTIONS

void Update();
void Draw();
//void VertexShader(const vec3& v, ivec2& p);//3d to 2d image
void VertexShader(const Vertex& v, Pixel& p);
//void DrawPolygonEdges(const vector<vec3>& vertices);//draw Polygon edges
//void DrawLineSDL(SDL_Surface* surface, ivec2 a, ivec2 b, vec3 color);//draw lines
//void Interpolate(ivec2 a, ivec2 b, vector<ivec2>& result);
void Interpolate(Pixel a, Pixel b, vector<Pixel>& result);
//void ComputePolygonRows(const vector<ivec2>& vertexPixels, vector<ivec2>& leftPixels, vector<ivec2>& rightPixels);
void ComputePolygonRows(const vector<Pixel>& vertexPixels, vector<Pixel>& leftPixels, vector<Pixel>& rightPixels);
//void DrawPolygonRows(const vector<ivec2>& leftPixels, const vector<ivec2>& rightPixels);
void DrawPolygonRows(const vector<Pixel>& leftPixels, const vector<Pixel>& rightPixels);
void DrawPolygon(const vector<Vertex>& vertices);
void PixelShader(const Pixel& p);

int main(int argc, char* argv[])
{
	LoadTestModel(triangles);
	screen = InitializeSDL(SCREEN_WIDTH, SCREEN_HEIGHT);
	t = SDL_GetTicks();	// Set start value for timer.
	while (NoQuitMessageSDL())
	{
		Update();
		Draw();
	}

	SDL_SaveBMP(screen, "screenshot.bmp");
	return 0;
}

void Update()
{
	// Compute frame time:
	int t2 = SDL_GetTicks();
	float dt = float(t2 - t);
	t = t2;
	cout << "Render time: " << dt << " ms." << endl;

	Uint8* keystate = SDL_GetKeyState(0);
	float theta = 0.01;

	if (keystate[SDLK_UP])
		cameraPos.z += theta;

	if (keystate[SDLK_DOWN])
		cameraPos.z -= theta;;

	if (keystate[SDLK_RIGHT])
		yaw -= theta;

	if (keystate[SDLK_LEFT])
		yaw += theta;

	if (keystate[SDLK_RSHIFT])
		xaw += theta;//rotate around x

	if (keystate[SDLK_RCTRL])
		xaw -= theta;;//rotate around x

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
	Ry = mat3(cos(yaw), 0, sin(yaw),
		0, 1, 0,
		-sin(yaw), 0, cos(yaw));
	Rx = mat3(1, 0, 0,
		0, cos(xaw), -sin(xaw),
		0, sin(xaw), cos(xaw));

	int dx;
	int dy;
	SDL_GetRelativeMouseState(&dx, &dy);

	//SDL_PumpEvents();
	if (SDL_GetMouseState(NULL, NULL)&SDL_BUTTON(1))
	{
		xaw -= (float)dy / 200;
		yaw += (float)dx / 200;
	}
}

void Draw()
{
	SDL_FillRect(screen, 0, 0);
	if (SDL_MUSTLOCK(screen))
		SDL_LockSurface(screen);
	for (int y = 0; y < SCREEN_HEIGHT; ++y)
	{
		for (int x = 0; x<SCREEN_WIDTH; ++x)
			depthBuffer[y][x] = 0;

	}
	for (int i = 0; i<triangles.size(); ++i)
	{
		currentColor = triangles[i].color;
		vector<Vertex> vertices(3);
		vertices[0].position = triangles[i].v0;
		vertices[1].position = triangles[i].v1;
		vertices[2].position = triangles[i].v2;
		currentNormal = triangles[i].normal;
		currentReflectance = vec3(1, 1, 1);
		DrawPolygon(vertices);
		/*for (int v = 0; v<3; ++v)
		{
		ivec2 projPos;
		VertexShader(vertices[v], projPos);
		vec3 color(1, 1, 1);
		PutPixelSDL(screen, projPos.x, projPos.y, color);
		}*/
	}
	if (SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);
	SDL_UpdateRect(screen, 0, 0, 0, 0);
}






//-----------------------------lab3_part2-----------------
void DrawPolygon(const vector<Vertex>& vertices)
{
	int V = vertices.size();
	vector<Pixel> vertexPixels(V);
	for (int i = 0; i<V; ++i)
		VertexShader(vertices[i], vertexPixels[i]);
	vector<Pixel> leftPixels(3);
	vector<Pixel> rightPixels(3);
	ComputePolygonRows(vertexPixels, leftPixels, rightPixels);
	DrawPolygonRows(leftPixels, rightPixels);
}

void VertexShader(const Vertex& v, Pixel& p)
{
	vec3 b = (v.position - cameraPos)*Ry*Rx;
	p.x = focalLength*float(b.x / b.z) + SCREEN_WIDTH / 2;
	p.y = focalLength*float(b.y / b.z) + SCREEN_HEIGHT / 2;
	p.zinv = 1.0 /b.z;
	p.pos3d = v.position;
	/*float r = glm::distance(lightPos, v.position);
	vec3 r_hat= glm::normalize(lightPos - v.position);
	vec3 n_hat = v.normal;
	float product = glm::dot(r_hat, n_hat);
	if (product < 0)
		product = 0;
	vec3 D = lightPower*product / (4 * 3.1415926f*r*r);
	p.illumination = (D + indirectLightPowerPerArea);*/

}

void ComputePolygonRows(const vector<Pixel>& vertexPixels, vector<Pixel>& leftPixels, vector<Pixel>& rightPixels)
{


	// 1. Find max and min y-value of the polygon
	// and compute the number of rows it occupies.
	int max_y = glm::max(glm::max(vertexPixels[0].y, vertexPixels[1].y), vertexPixels[2].y);
	int min_y = glm::min(glm::min(vertexPixels[0].y, vertexPixels[1].y), vertexPixels[2].y);
	int ROWS = max_y - min_y + 1;
	// 2. Resize leftPixels and rightPixels
	// so that they have an element for each row.
	leftPixels.resize(ROWS);
	rightPixels.resize(ROWS);
	// 3. Initialize the x-coordinates in leftPixels
	// to some really large value and the x-coordinates
	// in rightPixels to some really small value.
	for (int i = 0; i<ROWS; ++i)
	{
		leftPixels[i].x = +numeric_limits<int>::max();
		leftPixels[i].y = i + min_y;
		rightPixels[i].x = -numeric_limits<int>::max();
		rightPixels[i].y = i + min_y;
	}

	// 4. Loop through all edges of the polygon and use
	// linear interpolation to find the x-coordinate for
	// each row it occupies. Update the corresponding
	// values in rightPixels and leftPixels.
	for (int i = 0; i<3; ++i)
	{
		int j = (i + 1) % 3; // The next vertex
		//vec3 color(1, 1, 1);
		ivec2 delta;
		delta.x = glm::abs(vertexPixels[i].x - vertexPixels[j].x);
		delta.y = glm::abs(vertexPixels[i].y - vertexPixels[j].y);
		int pixels = glm::max(delta.x, delta.y) + 1;
		vector<Pixel> line(pixels);
		Interpolate(vertexPixels[i], vertexPixels[j], line);
		for (int l = 0; l < line.size(); l++)
		{
			for (int c = 0; c < ROWS; c++)
			{
				if (leftPixels[c].y == line[l].y)
				{
					if (leftPixels[c].x>line[l].x)
					{
						leftPixels[c].x = line[l].x;
						leftPixels[c].zinv = line[l].zinv;
						leftPixels[c].pos3d = line[l].pos3d;
					}
					if (rightPixels[c].x < line[l].x)
					{
						rightPixels[c].x = line[l].x;
						rightPixels[c].zinv = line[l].zinv;
						rightPixels[c].pos3d = line[l].pos3d;
					}
					break;
				}
			}
		}
	}


}
void Interpolate(Pixel a, Pixel b, vector<Pixel>& result)
{
	int N = result.size();
	vec3 step;
	step.x = float(b.x - a.x) / float(glm::max(N - 1, 1));
	step.y = float(b.y - a.y) / float(glm::max(N - 1, 1));
	step.z = float(b.zinv - a.zinv) / float(glm::max(N - 1, 1));
	vec3 pos3dStep = (b.pos3d - a.pos3d) / float(glm::max(N - 1, 1));
	Pixel current(a);
	for (int i = 0; i<N; ++i)
	{
		result[i] = current;
		current.x = a.x + (i+1)*step.x;
		current.y = a.y+(i+1)*step.y;
		current.zinv = a.zinv+(i+1)*step.z;
		current.pos3d += pos3dStep;
	}
}

void DrawPolygonRows(const vector<Pixel>& leftPixels, const vector<Pixel>& rightPixels)
{
	for (int i = 0; i < leftPixels.size(); i++)
	{
		Pixel delta;
		delta.x = glm::abs(leftPixels[i].x - rightPixels[i].x);
		delta.y = glm::abs(leftPixels[i].y - rightPixels[i].y);
		int pixels = glm::max(delta.x, delta.y) + 1;
		vector<Pixel> line(pixels);
		Interpolate(leftPixels[i], rightPixels[i], line);
		for (int i = 0; i < pixels; i++)
		{
			if (line[i].x >= 0 && line[i].y >= 0 && line[i].x < SCREEN_WIDTH && line[i].y < SCREEN_HEIGHT)
			{
				PixelShader(line[i]);
			}
		}
	}

}

void PixelShader(const Pixel& p)
{
	int x = p.x;
	int y = p.y;
	float r = glm::distance(lightPos, p.pos3d);
	vec3 r_hat = glm::normalize(lightPos - p.pos3d);
	vec3 n_hat = currentNormal;
	float product = glm::dot(r_hat, n_hat);
	if (product < 0)
		product = 0;
	vec3 D = lightPower*product / (4 * 3.1415926f*r*r);
	vec3 illumination = (D*currentReflectance + indirectLightPowerPerArea);
	if (p.zinv > depthBuffer[y][x])
	{
		depthBuffer[y][x] = p.zinv;
		PutPixelSDL(screen, x, y, currentColor*illumination);
	}
}

//-------------------------------lab3_part1------------
/*void DrawPolygonEdges(const vector<vec3>& vertices)
{
int L= vertices.size();
vector<ivec2> projectedVertices(L);
for (int v = 0; v < L; ++v)
{
VertexShader(vertices[v], projectedVertices[v]);
}



for (int i = 0; i<L; ++i)
{
int j = (i + 1) % L; // The next vertex
vec3 color(1, 1, 1);
DrawLineSDL(screen, projectedVertices[i], projectedVertices[j], color);
}

}

void DrawLineSDL(SDL_Surface* surface, ivec2 a, ivec2 b, vec3 color)
{
ivec2 delta = glm::abs(a - b);
int pixels = glm::max(delta.x, delta.y) + 1;
vector<ivec2> line(pixels);
Interpolate(a, b, line);
for (int i = 0; i < pixels; i++)
{

PutPixelSDL(screen, line[i].x, line[i].y, color);
}

}*/
/*void VertexShader(const vec3& v, ivec2& p)
{
vec3 b = (v - cameraPos)*Ry*Rx;
p.x = focalLength*float(b.x / b.z) + SCREEN_WIDTH / 2;
p.y = focalLength*float(b.y / b.z) + SCREEN_HEIGHT / 2;
}
void Interpolate(ivec2 a, ivec2 b, vector<ivec2>& result)
{
int N = result.size();
vec2 step = vec2(b - a) / float(glm::max(N - 1, 1));
vec2 current(a);
for (int i = 0; i<N; ++i)
{
result[i] = current;
current += step;
}
}



void ComputePolygonRows(const vector<ivec2>& vertexPixels, vector<ivec2>& leftPixels, vector<ivec2>& rightPixels)
{



// 1. Find max and min y-value of the polygon
// and compute the number of rows it occupies.
int max_y = glm::max(glm::max(vertexPixels[0].y, vertexPixels[1].y), vertexPixels[2].y);
int min_y = glm::min(glm::min(vertexPixels[0].y, vertexPixels[1].y), vertexPixels[2].y);
int ROWS = max_y - min_y + 1;
// 2. Resize leftPixels and rightPixels
// so that they have an element for each row.
leftPixels.resize(ROWS);
rightPixels.resize(ROWS);
// 3. Initialize the x-coordinates in leftPixels
// to some really large value and the x-coordinates
// in rightPixels to some really small value.
for (int i = 0; i<ROWS; ++i)
{
leftPixels[i].x = +numeric_limits<int>::max();
leftPixels[i].y = i + min_y;
rightPixels[i].x = -numeric_limits<int>::max();
rightPixels[i].y = i + min_y;
}

// 4. Loop through all edges of the polygon and use
// linear interpolation to find the x-coordinate for
// each row it occupies. Update the corresponding
// values in rightPixels and leftPixels.
for (int i = 0; i<3; ++i)
{
int j = (i + 1) % 3; // The next vertex
//vec3 color(1, 1, 1);
ivec2 delta = glm::abs(vertexPixels[i] - vertexPixels[j]);
int pixels = glm::max(delta.x, delta.y) + 1;
vector<ivec2> line(pixels);
Interpolate(vertexPixels[i], vertexPixels[j], line);
for (int l = 0; l < line.size(); l++)
{
for (int c = 0; c < ROWS; c++)
{
if (leftPixels[c].y == line[l].y)
{
if (leftPixels[c].x>line[l].x)
leftPixels[c].x = line[l].x;
if (rightPixels[c].x<line[l].x)
rightPixels[c].x = line[l].x;
break;
}
}
}
}


}




void DrawPolygon(const vector<vec3>& vertices)
{
int V = vertices.size();
vector<ivec2> vertexPixels(V);
for (int i = 0; i<V; ++i)
VertexShader(vertices[i], vertexPixels[i]);
vector<ivec2> leftPixels(3);
vector<ivec2> rightPixels(3);
ComputePolygonRows(vertexPixels, leftPixels, rightPixels);
DrawPolygonRows(leftPixels, rightPixels);
}


void DrawPolygonRows(const vector<ivec2>& leftPixels, const vector<ivec2>& rightPixels)
{
for (int i = 0; i < leftPixels.size(); i++)
{
ivec2 delta = glm::abs(leftPixels[i] - rightPixels[i]);
int pixels = glm::max(delta.x, delta.y) + 1;
vector<ivec2> line(pixels);
Interpolate(leftPixels[i], rightPixels[i], line);
for (int i = 0; i < pixels; i++)
{

PutPixelSDL(screen, line[i].x, line[i].y, currentColor);
}
}

}*/

