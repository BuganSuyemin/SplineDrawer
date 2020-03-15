#include <SDL.h>
#include <iostream>
//#include "BuganDrawing.cpp"

struct BezierPoint
{
public:
	SDL_Point Point;
	SDL_Point HandlePrev;
	SDL_Point HandleNext;

	BezierPoint(SDL_Point point, SDL_Point handlePrev, SDL_Point handleNext)
	{
		Point = point;
		HandlePrev = handlePrev;
		HandleNext = handleNext;
	}

	BezierPoint(int px, int py, int hpx, int hpy, int hnx, int hny)
	{
		Point = SDL_Point();
		Point.x = px;
		Point.y = py;

		HandlePrev = SDL_Point();
		HandlePrev.x = hpx;
		HandlePrev.y = hpy;

		HandleNext = SDL_Point();
		HandleNext.x = hnx;
		HandleNext.y = hny;
	}
};

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

void DrawPixel(SDL_Renderer* renderer, SDL_Point p)
{
	DrawPixel(renderer, p.x, p.y);
}

SDL_Point GetPointOnLine(int x0, int y0, int x1, int y1, double t)
{
	SDL_Point p = SDL_Point();
	p.x = x0 + ( x1 - x0 ) * t;
	p.y = y0 + ( y1 - y0 ) * t;

	return p;
}

SDL_Point GetPointOnLine(SDL_Point p1, SDL_Point p2, double t)
{
	SDL_Point p = SDL_Point();
	p.x = p1.x + ( p2.x - p1.x ) * t;
	p.y = p1.y + ( p2.y - p1.y ) * t;

	return p;
}

SDL_Point GetPointOnBezier(SDL_Point p1, SDL_Point t1, SDL_Point p2, SDL_Point t2, double t)
{
	SDL_Point oneToTwo = GetPointOnLine(p1, t1, t);
	SDL_Point twoTothree = GetPointOnLine(t1, t2, t);
	SDL_Point ThreeToFour = GetPointOnLine(t2, p2, t);

	SDL_Point oneToThree = GetPointOnLine(oneToTwo, twoTothree, t);
	SDL_Point twoToFour = GetPointOnLine(twoTothree, ThreeToFour, t);

	return GetPointOnLine(oneToThree, twoToFour, t);
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

void DrawLine(SDL_Renderer* renderer, SDL_Point p1, SDL_Point p2)
{
	DrawLine(renderer, p1.x, p1.y, p2.x, p2.y);
}

bool PointsClose(SDL_Point p0, SDL_Point p1)
{
	if (pow(p0.x - p1.x, 2) + pow(p0.y - p1.y, 2) < 100)
		return true;
	return false;
}


BezierPoint* points[5];
int pointsCount = 0;
SDL_Point* selected = 0;
bool mousePressed = false;
SDL_Point mousePos = SDL_Point();
void Prikoling(SDL_Renderer* renderer)
{
	for (int i = 0; i < pointsCount; i++)
	{
		BezierPoint* point = points[i];

		SDL_SetRenderDrawColor(renderer, 0, 0, 255, 1);
		DrawLine(renderer, point->Point, point->HandlePrev);
		DrawLine(renderer, point->Point, point->HandleNext);

		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 1);
		DrawPixel(renderer, point->Point);

		SDL_SetRenderDrawColor(renderer, 255, 155, 0, 1);
		DrawPixel(renderer, point->HandleNext);
		DrawPixel(renderer, point->HandlePrev);

		//skip drawing curve for last point
		if (i == pointsCount - 1)
			break;

		SDL_SetRenderDrawColor(renderer, 125, 125, 125, 1);
		SDL_Point prevPixel = point->Point;
		for (double t = 0; t < 1.0; t += 0.001)
		{
			SDL_Point pixel = GetPointOnBezier(point->Point, point->HandleNext, points[i + 1]->Point, points[i + 1]->HandlePrev, t);

			DrawLine(renderer, pixel.x, pixel.y, prevPixel.x, prevPixel.y);

			prevPixel.x = pixel.x;
			prevPixel.y = pixel.y;
		}
	}

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

			selected = nullptr;
			for (int i = 0; i < pointsCount; i++)
			{
				SDL_Point tPoint = SDL_Point();
				tPoint.x = points[i]->Point.x * pixelSize;
				tPoint.y = points[i]->Point.y * pixelSize;
				if (PointsClose(mousePos, tPoint))
					selected = &points[i]->Point;

				tPoint.x = points[i]->HandlePrev.x * pixelSize;
				tPoint.y = points[i]->HandlePrev.y * pixelSize;
				if (PointsClose(mousePos, tPoint))
					selected = &points[i]->HandlePrev;

				tPoint.x = points[i]->HandleNext.x * pixelSize;
				tPoint.y = points[i]->HandleNext.y * pixelSize;
				if (PointsClose(mousePos, tPoint))
					selected = &points[i]->HandleNext;
			}
		}
	}

	if (mousePressed && selected != nullptr)
	{
		selected->x = mousePos.x / pixelSize;
		selected->y = mousePos.y / pixelSize;
	}
}

int main(int argc, char* argv[])
{
	points[0] = &BezierPoint(100, 100, 0, 100, 200, 100);
	points[1] = &BezierPoint(100, 200, 0, 200, 200, 200);
	points[2] = &BezierPoint(100, 300, 0, 300, 200, 300);

	pointsCount = 3;

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