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
void DrawPixel(SDL_Renderer* renderer, int x, int y, int size = pixelSize)
{
	SDL_Rect* r = new SDL_Rect();
	r->x = x * pixelSize - size / 2;
	r->y = y * pixelSize - size / 2;
	r->h = size;
	r->w = size;
	SDL_RenderFillRect(renderer, r);
}

void DrawPixel(SDL_Renderer* renderer, SDL_Point p, int size = pixelSize)
{
	DrawPixel(renderer, p.x, p.y, size);
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

SDL_Point GetPointOnBezierDeCasteljau(SDL_Point p1, SDL_Point t1, SDL_Point p2, SDL_Point t2, double t)
{
	SDL_Point oneToTwo = GetPointOnLine(p1, t1, t);
	SDL_Point twoTothree = GetPointOnLine(t1, t2, t);
	SDL_Point threeToFour = GetPointOnLine(t2, p2, t);

	SDL_Point oneToThree = GetPointOnLine(oneToTwo, twoTothree, t);
	SDL_Point twoToFour = GetPointOnLine(twoTothree, threeToFour, t);

	return GetPointOnLine(oneToThree, twoToFour, t);
}

SDL_Point GetPointOnBezierPolynomial(SDL_Point p1, SDL_Point t1, SDL_Point p2, SDL_Point t2, double t)
{
	SDL_Point p = SDL_Point();
	p.x = pow(( 1 - t ), 3) * p1.x
		+ 3 * pow(( 1 - t ), 2) * t * t1.x
		+ 3 * ( 1 - t ) * pow(t, 2) * t2.x
		+ pow(t, 3) * p2.x;
	p.y = pow(( 1 - t ), 3) * p1.y
		+ 3 * pow(( 1 - t ), 2) * t * t1.y
		+ 3 * ( 1 - t ) * pow(t, 2) * t2.y
		+ pow(t, 3) * p2.y;
	return p;
}

SDL_Point GetPointOnBezier(SDL_Point p1, SDL_Point t1, SDL_Point p2, SDL_Point t2, double t, int type = 0)
{
	if (type % 2 == 0)
		return GetPointOnBezierDeCasteljau(p1, t1, p2, t2, t);
	return GetPointOnBezierPolynomial(p1, t1, p2, t2, t);
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
double _t = 0;
bool pause = false;
double tStep = 0.05;
double _tStep = 0.01;
void Prikoling(SDL_Renderer* renderer)
{
#pragma region DrawDefault

#pragma region Handles
	for (int i = 0; i < pointsCount; i++)
	{
		BezierPoint* point = points[i];

		SDL_SetRenderDrawColor(renderer, 125, 255, 125, 1);
		DrawLine(renderer, point->Point, point->HandlePrev);
		DrawLine(renderer, point->Point, point->HandleNext);
	}
#pragma endregion


#pragma region ControlPoints
	for (int i = 0; i < pointsCount; i++)
	{
		BezierPoint* point = points[i];

		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 1);
		DrawPixel(renderer, point->Point);

		SDL_SetRenderDrawColor(renderer, 255, 155, 0, 1);
		DrawPixel(renderer, point->HandleNext);
		DrawPixel(renderer, point->HandlePrev);
	}
#pragma endregion

#pragma endregion

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 1);
	SDL_Point prevPixel = points[0]->Point;
	for (double t = 0; t <= _t; t += 0.05)
	{
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 1);

		/*SDL_Point p1 = GetPointOnBezier(points[0]->Point, points[0]->HandleNext, points[1]->Point, points[1]->HandlePrev, t);
		SDL_Point p2 = GetPointOnBezier(points[1]->Point, points[1]->HandleNext, points[2]->Point, points[2]->HandlePrev, t);
		SDL_Point t1 = GetPointOnBezier(points[2]->Point, points[2]->HandleNext, points[3]->Point, points[3]->HandlePrev, t);
		SDL_Point t2 = GetPointOnBezier(points[3]->Point, points[3]->HandleNext, points[0]->Point, points[0]->HandlePrev, t);
		SDL_Point pixel = GetPointOnBezier(p1, t1, p1, t2, t);

		DrawLine(renderer, pixel.x, pixel.y, prevPixel.x, prevPixel.y);

		prevPixel.x = pixel.x;
		prevPixel.y = pixel.y;*/
	}

	//last drawn pixel
	for (int i = 0; i < pointsCount; i++)
	{
		SDL_Point p1 = GetPointOnBezier(points[i]->Point, points[i]->HandleNext, points[( i + 1 ) % pointsCount]->Point, points[( i + 1 ) % pointsCount]->HandlePrev, _t);
		DrawPixel(renderer, p1, 10);
	}

	for (int i = 0; i < pointsCount; i++)
	{
		//first order lines
		SDL_Point oneToTwo = GetPointOnLine(points[i]->Point, points[i]->HandleNext, _t);
		SDL_Point twoToThree = GetPointOnLine(points[i]->HandleNext, points[( i + 1 ) % pointsCount]->HandlePrev, _t);
		SDL_Point threeToFour = GetPointOnLine(points[( i + 1 ) % pointsCount]->HandlePrev, points[( i + 1 ) % pointsCount]->Point, _t);

		SDL_SetRenderDrawColor(renderer, 255, 0, 255, 1);
		DrawLine(renderer, oneToTwo, twoToThree);
		DrawLine(renderer, twoToThree, threeToFour);

		//second order lines
		SDL_Point oneToThree = GetPointOnLine(oneToTwo, twoToThree, _t);
		SDL_Point twoToFour = GetPointOnLine(twoToThree, threeToFour, _t);

		SDL_SetRenderDrawColor(renderer, 0,0, 255, 1);
		DrawLine(renderer, oneToThree, twoToFour);

	}

#pragma region Curves
	for (int i = 0; i < pointsCount; i++)
	{
		BezierPoint* point = points[i];

		//skip drawing curve for last point
		/*if (i == pointsCount - 1)
			break;*/
		SDL_SetRenderDrawColor(renderer, 125, 125, 125, 1);
		SDL_Point prevPixel = point->Point;
		for (double t = 0; t <= _t; t += tStep)
		{
			SDL_Point pixel = GetPointOnBezier(point->Point, point->HandleNext, points[( i + 1 ) % pointsCount]->Point, points[( i + 1 ) % pointsCount]->HandlePrev, t);

			DrawLine(renderer, pixel.x, pixel.y, prevPixel.x, prevPixel.y);

			prevPixel.x = pixel.x;
			prevPixel.y = pixel.y;
		}
	}
#pragma endregion

	if (!pause)
		_t += _tStep;
	if (_t < 0)
		_t = 1;
	if (_t > 1)
		_t = 0;

	std::cout << _t << std::endl;
#pragma region Events

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
		if (evnt.type == SDL_KEYDOWN)
		{
			switch (evnt.key.keysym.sym)
			{
			case SDLK_SPACE:
				pause = !pause;
				break;
			case SDLK_1:
				_t = 1;
				break;
			case SDLK_0:
				_t = 0;
				break;
			case SDLK_a:
				_t -= _tStep;
				break;
			case SDLK_d:
				_t += _tStep;
				break;
			default:
				break;
			}
		}
	}

	if (mousePressed && selected != nullptr)
	{
		selected->x = mousePos.x / pixelSize;
		selected->y = mousePos.y / pixelSize;
	}
#pragma endregion
}

int main(int argc, char* argv[])
{
	points[0] = &BezierPoint(300, 100, 200, 100, 400, 100);
	points[1] = &BezierPoint(400, 200, 400, 100, 400, 300);
	points[2] = &BezierPoint(300, 300, 400, 300, 200, 300);
	points[3] = &BezierPoint(200, 200, 200, 300, 200, 100);

	pointsCount = 4;

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