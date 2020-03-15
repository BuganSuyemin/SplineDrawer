#include <SDL.h>
#include <iostream>
//#include "BuganDrawing.cpp"

int max(int i, int j)
{
	if (i < j)
		return j;
	return i;
}

int min(int i, int j)
{
	if (i > j)
		return j;
	return i;
}

int pixelSize = 3;
void DrawPixel(SDL_Renderer* renderer, int x, int y)
{
	SDL_Rect* r = new SDL_Rect();
	r->x = x * pixelSize;
	r->y = y * pixelSize;
	r->h = pixelSize;
	r->w = pixelSize;
	SDL_RenderFillRect(renderer, r);
}

SDL_Point GetPointOnLine(int x0, int y0, int x1, int y1, double t)
{
	SDL_Point p = SDL_Point();
	p.x = x0 + ( x1 - x0 ) * t;
	p.y = y0 + ( y1 - y0 ) * t;

	return p;
}

void DrawLine(SDL_Renderer* renderer, int x0, int y0, int x1, int y1)
{
	if (x0 > x1)
	{
		int t = x0;
		x0 = x1;
		x1 = t;

		t = y0;
		y0 = y1;
		y1 = t;
	}

	if (x1 == x0)
	{
		for (int y = min(y0, y1); y < max(y0, y1) + 1; y++)
			DrawPixel(renderer, x0, y);
		return;
	}

	if (y1 == y0)
	{
		for (int x = x0; x < x1 + 1; x++)
			DrawPixel(renderer, x, y0);
		return;
	}

	double yPerX = (double)abs(y1 - y0) / ( x1 - x0 );
	int yDir = abs(y1 - y0) / ( y1 - y0 );
	double y = y0;

	for (int x = x0; x < x1; x++)
	{
		DrawPixel(renderer, x, y);
		for (int i = 1; i < yPerX; i++)
			DrawPixel(renderer, x, y + i * yDir);
		y += yPerX * yDir;
	}
}

bool PointsClose(SDL_Point p0, SDL_Point p1)
{
	if (pow(p0.x - p1.x, 2) + pow(p0.y - p1.y, 2) < 100)
		return true;
	return false;
}

SDL_Point p1 = SDL_Point();
SDL_Point p2 = SDL_Point();
SDL_Point t1 = SDL_Point();
SDL_Point t2 = SDL_Point();
SDL_Point* points[5];
int selected = 0;
bool mousePressed = false;
SDL_Point mousePos = SDL_Point();
double t = 0;
//SDL_Point prevPixel = SDL_Point();
void Prikoling(SDL_Renderer* renderer)
{
	SDL_SetRenderDrawColor(renderer, 125, 125, 125, 1);
	SDL_Point prevPixel = GetPointOnLine(p1.x, p1.y, t1.x, t1.y, 0.0);
	for (double i = 0; i < 1.01; i += 0.01)
	{
		SDL_Point oneToTwo = GetPointOnLine(p1.x, p1.y, t1.x, t1.y, i);
		SDL_Point twoTothree = GetPointOnLine(t1.x, t1.y, t2.x, t2.y, i);
		SDL_Point ThreeToFour = GetPointOnLine(t2.x, t2.y, p2.x, p2.y, i);

		SDL_Point oneToThree = GetPointOnLine(oneToTwo.x, oneToTwo.y, twoTothree.x, twoTothree.y, i);
		SDL_Point twoToFour = GetPointOnLine(twoTothree.x, twoTothree.y, ThreeToFour.x, ThreeToFour.y, i);

		SDL_Point pixel = GetPointOnLine(oneToThree.x, oneToThree.y, twoToFour.x, twoToFour.y, i);

		DrawLine(renderer, pixel.x, pixel.y, prevPixel.x, prevPixel.y);

		prevPixel.x = pixel.x;
		prevPixel.y = pixel.y;
	}

	SDL_SetRenderDrawColor(renderer, 0, 0, 255, 1);
	DrawLine(renderer, p1.x, p1.y, t1.x, t1.y);
	DrawLine(renderer, p2.x, p2.y, t2.x, t2.y);

	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 1);
	DrawPixel(renderer, p1.x, p1.y);
	DrawPixel(renderer, p2.x, p2.y);

	SDL_SetRenderDrawColor(renderer, 255, 155, 0, 1);
	DrawPixel(renderer, t1.x, t1.y);
	DrawPixel(renderer, t2.x, t2.y);


	SDL_Event evnt;
	while (SDL_PollEvent(&evnt))
	{
		if (evnt.type == SDL_MOUSEMOTION)
		{
			mousePos.x = evnt.motion.x;
			mousePos.y = evnt.motion.y;
		}



		if (evnt.type == SDL_MOUSEBUTTONUP)
		{
			std::cout << "UP" << std::endl;
			mousePressed = false;
			selected = 0;
		}
		if (evnt.type == SDL_MOUSEBUTTONDOWN)
		{

			mousePressed = true;
			std::cout << "DOWN" << std::endl;

			selected = -1;
			for (int i = 0; i < 4; i++)
			{
				SDL_Point tPoint = SDL_Point();
				tPoint.x = points[i]->x * pixelSize;
				tPoint.y = points[i]->y * pixelSize;
				if (PointsClose(mousePos, tPoint))
					selected = i;
			}
		}
	}

	if (mousePressed && selected != -1)
	{
		points[selected]->x = mousePos.x / pixelSize;
		points[selected]->y = mousePos.y / pixelSize;
	}


	/*t += 0.01;
	if (t > 1)
		t = 0;*/
}

int main(int argc, char* argv[])
{
	p1.x = 100;
	p1.y = 100;
	p2.x = 200;
	p2.y = 200;
	t1.x = 200;
	t1.y = 100;
	t2.x = 200;
	t2.y = 250;

	points[0] = &p1;
	points[1] = &p2;
	points[2] = &t1;
	points[3] = &t2;

	if (SDL_Init(SDL_INIT_VIDEO) == 0)
	{
		SDL_Window* window = NULL;
		SDL_Renderer* renderer = NULL;

		if (SDL_CreateWindowAndRenderer(1900, 1000, 0, &window, &renderer) == 0)
		{
			SDL_bool done = SDL_FALSE;

			while (!done)
			{
				SDL_Delay(10);
				SDL_Event event;

				SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
				SDL_RenderClear(renderer);
				Prikoling(renderer);
				SDL_RenderPresent(renderer);

				while (SDL_PollEvent(&event))
				{
					if (event.type == SDL_QUIT)
					{
						done = SDL_TRUE;
					}
				}
			}
		}

		if (renderer)
		{
			SDL_DestroyRenderer(renderer);
		}
		if (window)
		{
			SDL_DestroyWindow(window);
		}
	}
	SDL_Quit();

	return 0;
}