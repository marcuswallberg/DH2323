#include "SDL.h"
#include <iostream>
#include <glm/glm.hpp>
#include <vector>
#include "SDLauxiliary.h"
#include <stdlib.h>
#include <cstdlib>
#include <math.h>

using namespace std;
using glm::vec3;
using glm::mat3;
using glm::vec2;

// ----------------------------------------------------------------------------
// STRUCTS

//Structure for the nodes
struct Point
{
	vec3 pos;
	bool fixed = false;     // If the point is fixed or not
	int id;         // Id of the
	vector<int> neighbours;
	vec2 screenpos;
};

// --------------------------------------------------------
// GLOBAL VARIABLES

//Screen variables
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
SDL_Surface* screen;
int t;
SDL_Event event;

// Mouse events
int mousex;
int mousey;
bool ismousepressed = false;
bool wasTheMousePressedLastFrame = false;
vec2 mousePosition;
vec2 lastMousePosition;
Point closestNode;
float mousePower = 5e-5;
vector<int> closestNodes;
vector<float> closestNodesDist;

//The nodes
vector<Point> vertices;

//Camera and projection variables
vec3 cameraPos(0, 0, -3.001);
mat3 R;
float yaw = 0; // Yaw angle controlling camera rotation around y-axis
float speed = 0.5;

// Parameters for the mass spring physics
float gravitation = 9.82 * 1e-4;
float gForce = 0;
float spacing = 0.1;
float lineForce = 1;


// --------------------------------------------------------
// FUNCTION DECLARATIONS

void Draw();
void Update();
void UpdatePhysics();
//void createNet(int startX, int stopX, int startY, int stopY, int xled, int yled);
void createNet2(float startX, float stopX, float startY, float stopY, int xled, int yled);
void drawLines(Point p);
void DrawLineSDL(SDL_Surface* surface, Point a, Point b, vec3 color);
void Interpolate(Point a, Point b, vector<Point>& result);
void VertexShader(Point& v);
void checkMouseEvent();
void getMouseForce(vec3& force);
void findClosestNodes();


// --------------------------------------------------------
// FUNCTION DEFINITIONS

int main(int argc, char* argv[])
{
	screen = InitializeSDL(SCREEN_WIDTH, SCREEN_HEIGHT);

	//Creating the grid net
	createNet2(-1, 1, -1, 1, 17, 17);
	t = SDL_GetTicks();	// Set start value for timer.

						//pthread_t threads[2];
						//int rc = pthread_create(&threads[1], NULL, *checkMouseEvent, (void *) 1);

	while (NoQuitMessageSDL())
	{
		Update();
		Draw();
	}
	return 0;
}

//Updating the camera position and the physics
void Update()
{
	int t2 = SDL_GetTicks();
	float dt = float(t2 - t);
	t = t2;
	//cout << "Render time: " << dt << " ms." << endl;
	cout << "FPS: " << round(1000 / dt) << endl;

	// Getting the pressed button
	Uint8* keystate = SDL_GetKeyState(0);

	// Moving in positive direction from the camera direction (moving forward)
	if (keystate[SDLK_UP])
		cameraPos += R * vec3(0, 0, speed);

	// Moving in the negative direction from the camera direction (moving backward)
	if (keystate[SDLK_DOWN])
		cameraPos += R * vec3(0, 0, -speed);

	// Updating the rotation vector (rotating right)
	if (keystate[SDLK_RIGHT])
		yaw += -0.05;

	// Updating the rotation vector (rotating left)
	if (keystate[SDLK_LEFT])
		yaw += 0.05;

	//Checking if the space bar is pressed
	if (keystate[SDLK_SPACE]) {
		SDL_GetMouseState(&mousex, &mousey);
		findClosestNodes();
		ismousepressed = true;
	}
	else {
		ismousepressed = false;
	}

	UpdatePhysics();
}

//Updating the physics
void UpdatePhysics() {
	gForce = gravitation;

	//The force with which the mouse if pulling the cloth
	vec3 mouseforce;
	getMouseForce(mouseforce);

	// For each vertex
	for (int i = 0; i < vertices.size(); i++) {

		// If the point is not fixed, apply gravity
		Point& p = vertices[i];
		if (!p.fixed)
			p.pos.y += gForce;

		// Apply the force from the edges
		if (p.neighbours.size() > 0) {
			for (int j = 0; j < p.neighbours.size(); j++) {
				Point& n = vertices[p.neighbours[j]];
				float dist = glm::distance(p.pos, n.pos);

				// Check if the distance is far enough to apply the constraint
				if (dist > spacing) {
					float diff = (spacing - dist) / dist;
					float line = diff * lineForce * (1 - spacing / dist);

					vec3 vectorDiff = p.pos - n.pos;

					if (!p.fixed) {
						p.pos += (vectorDiff * line);
						for (int c = 0; c < closestNodes.size(); c++) {
							if (closestNodes[c] == p.id) {
								p.pos += mouseforce * mousePower * closestNodesDist[c];
							}
						}
					}
					if (!n.fixed) {
						n.pos -= (vectorDiff * line);
					}
				}
			}
		}
	}
}

//Getting the force with which the mouse is pulling the cloth
void getMouseForce(vec3& force) {
	//checkMouseEvent();  // Updates the bool ismousepressed
	if (ismousepressed)
	{
		if (!wasTheMousePressedLastFrame) {

			wasTheMousePressedLastFrame = true;

			//Getting the mouse position on the screen
			SDL_GetMouseState(&mousex, &mousey);
			mousePosition = vec2(mousex, mousey);

			force = vec3(0, 0, 0);

		}
		else {

			lastMousePosition = mousePosition;

			//Getting the mouse position on the screen
			SDL_GetMouseState(&mousex, &mousey);
			mousePosition = vec2(mousex, mousey);

			force = vec3(mousePosition - lastMousePosition, 0);
			//cout << "Force: " << force.x << force.y << endl;
		}
	}
	else
	{
		wasTheMousePressedLastFrame = false;
		force = vec3(0, 0, 0);
	}
}

//Checking if the mouse is being pressed
//Due to problems on one of our computers, this function is not used
//To move the cloth, press [SPACE BAR] + move the mouse/mousepad!
void checkMouseEvent()
{
	SDL_Event event;
	while (SDL_PollEvent(&event)) // check to see if an event has happened
	{
		switch (event.type)
		{
		//If the left mouse button is being pressed
		case SDL_MOUSEBUTTONDOWN: // if the event is mouse click*/
			if (event.type == SDL_MOUSEBUTTONDOWN)  // check if it is in the desired area
			{
				cout << "MOM, WE MADE IT!" << endl;
				ismousepressed = true;
			}
		//If the mouse button is being released
		case SDL_MOUSEBUTTONUP:
			if (event.type == SDL_MOUSEBUTTONUP)
			{
				cout << "FALSE!" << endl;
				ismousepressed = false;
			}
		}
	}
}

//Finding the nodes which are close to the mouse
void findClosestNodes()
{
	closestNodes.clear();
	closestNodesDist.clear();
	//Threshold distance to the mouse
	float dist = 100;
	for (int i = 0; i < vertices.size(); i++)
	{
		Point p = vertices[i];
		float length = sqrt(pow(p.screenpos.x - mousex, 2) + pow(p.screenpos.y - mousey, 2));
		if (length < dist)
		{
			//Saving the id and the distance to the nodes within the threshold radious to the mouse
			closestNodes.push_back(p.id);
			closestNodesDist.push_back(length);
		}
	}
}

//Drawing the cloth
void Draw()
{

	SDL_FillRect(screen, 0, 0);

	//Projection from 3D coordinates to 2D
	for (int i = 0; i<vertices.size(); ++i)
		VertexShader(vertices[i]);

	vec3 color(0, 0, 1);

	//Drawing the lines
	for (int i = 0; i < vertices.size(); i++)
	{
		Point p = vertices[i];
		drawLines(p);
	}

	//Drawing the nodes
	for (int i = 0; i < vertices.size(); i++)
	{
		Point p = vertices[i];
		//PutPixelSDL(screen, p.screenpos.x, p.screenpos.y, color);
		for (int i = -3; i < 3; i++) {
			for (int j = -3; j < 3; j++) {
				PutPixelSDL(screen, p.screenpos.x + i, p.screenpos.y + j, color);
			}
		}
	}

	if (SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);

	SDL_UpdateRect(screen, 0, 0, 0, 0);
}

//Creating the nodes and setting neighbours for the grid net
void createNet2(float startX, float stopX, float startY, float stopY, int xled, int yled)
{
	//Cloth size
	float width = stopX - startX;
	float height = stopY - startY;
	//Step size between nodes
	float xStep = width / (xled - 1);
	float yStep = height / (yled - 1);

	for (int i = 0; i < yled; i++)
	{
		float newy = startY + yStep * i;
		for (int j = 0; j < xled; j++)
		{
			float newx = startX + xStep * j;
			Point newPixel;
			float z = rand() % 30;
			z = z / 100 - 0.5;
			z = 0;
			newPixel.pos = vec3(newx, newy, z);
			newPixel.id = j + i * xled;

			//Setting neighbours
			if (j != 0)
				newPixel.neighbours.push_back(newPixel.id - 1);

			if (j != xled - 1)
				newPixel.neighbours.push_back(newPixel.id + 1);

			if (i != 0)
				newPixel.neighbours.push_back(newPixel.id - xled);

			if (i != yled - 1)
				newPixel.neighbours.push_back(newPixel.id + xled);

			//Setting the top nodes to a fixed position
			if (i == 0) {
				newPixel.fixed = true;
			}
			vertices.push_back(newPixel);
		}
	}
}

//Going through all the neighbours of the node to draw lines (springs)
void drawLines(Point p)
{
	for (int i = 0; i < p.neighbours.size(); i++)
	{
		Point n = vertices[p.neighbours[i]];
		vec3 color(1, 1, 1);
		DrawLineSDL(screen, n, p, color);
	}
}

//Drawing the lines between two nodes
void DrawLineSDL(SDL_Surface* surface, Point a, Point b, vec3 color)
{
	Point delta;
	delta.screenpos = glm::abs(a.screenpos - b.screenpos);
	int pixels = glm::max(delta.screenpos.x, delta.screenpos.y) + 1;
	vector<Point> line(pixels);
	Interpolate(a, b, line);

	for (int i = 0; i < pixels; i++) {
		Point pixel = line[i];
		PutPixelSDL(surface, pixel.screenpos.x, pixel.screenpos.y, color);
	}

}

//Interpolation between two nodes
void Interpolate(Point a, Point b, vector<Point>& result)
{
	int N = result.size();
	Point step;
	step.screenpos = ((b.screenpos - a.screenpos) / float(glm::max(N - 1, 1)));
	Point current;
	current.screenpos = vec2(a.screenpos);
	for (int i = 0; i < N; ++i)
	{
		result[i].screenpos = current.screenpos;
		current.screenpos += step.screenpos;
	}
}

//Projection from 3D coordinates to 2D
void VertexShader(Point& v)
{
	int f = SCREEN_HEIGHT;
	R = mat3(cos(yaw), 0, sin(yaw), 0, 1, 0, -sin(yaw), 0, cos(yaw));
	vec3 pPrim = (v.pos - cameraPos) *R;
	v.screenpos.x = (f * pPrim.x / pPrim.z) + SCREEN_WIDTH / 2;
	v.screenpos.y = (f * pPrim.y / pPrim.z) + SCREEN_HEIGHT / 2;
}
