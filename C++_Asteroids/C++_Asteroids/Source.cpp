#include <iostream>
#include <algorithm>
#include <string>
#include <vector>

using namespace std;

#include "olcConsoleGameEngine.h"

class Asteroids : public olcConsoleGameEngine
{
public:
	Asteroids()
	{
		m_sAppName = L"Asteroids";
	}

private:
	struct sSpaceObject
	{
		float x;
		float y;
		float dx;
		float dy;
		int nSize;
		float angle;

	};

	vector<sSpaceObject>vecAsteroids;
	vector<sSpaceObject> vecBullets;
	sSpaceObject player;
	int nScore = 0;
	bool bDead = false;


	vector<pair<float, float>> vecModelShip;
	vector<pair<float, float>> vecModelAsteroid;


protected:
	// Called by gameengine 
	virtual bool OnUserCreate()
	{

		// initializer list for ship 
		vecModelShip =
		{
			{ 0.0f, -5.0f },
			{ -2.5f, +2.5f },
			{ +2.5f, +2.5f }		
		};	// An Isoceles triangle 

		// algorithmically initialize asteroid 
		int verts = 20;			// each asteroid contains 20 points 
		for (int i = 0; i < verts; i++)
		{
			float radius = (float)rand() / (float)RAND_MAX * 0.4f + 0.8f;								// each point lies within a circle 
			float a = ((float)i / (float)verts) * 6.28318f;		// degree per segment (i/verts) * 2(PI)
			vecModelAsteroid.push_back(make_pair(radius * sinf(a), radius * cosf(a)));	// get coordinates 
		}

		ResetGame();

		return true; 
	}

	// collision detection 
	bool IsPointInsideCircle(float cx, float cy, float radius, float x, float y)
	{
		return sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy)) < radius;
	}

	// create new game 
	void ResetGame()
	{
		vecAsteroids.clear();
		vecBullets.clear();

		//initialize asteroids 
		vecAsteroids.push_back({ 20.0f, 20.0f, 8.0f, -6.0f, (int)16, 0.0f });
		vecAsteroids.push_back({ 100.0f, 20.0f, -5.0f, 3.0f, (int)16, 0.0f });


		// Initialize Player Position 
		player.x = ScreenWidth() / 2.0f;
		player.y = ScreenHeight() / 2.0f;
		player.dx = 0.0f;
		player.dy = 0.0f;
		player.angle = 0.0f;

		bDead = false;
		nScore = 0;
	}

	virtual bool OnUserUpdate(float fElapsedTime)
	{
		if (bDead)
			ResetGame();

		//clear screen
		Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, 0);

		// Steer player 
		if (m_keys[VK_LEFT].bHeld)
			player.angle -= 5.0f * fElapsedTime;
		if (m_keys[VK_RIGHT].bHeld)
			player.angle += 5.0f * fElapsedTime;

		// thrust 
		if (m_keys[VK_UP].bHeld)
		{
			// ACCLERATION changes VELOCITY (with respecr to time)
			player.dx += sin(player.angle) * 20.0f * fElapsedTime;
			player.dy += -cos(player.angle) * 20.0f * fElapsedTime;
		}

		//VELOCITY changes POSITION (with respect to time)
		player.x += player.dx * fElapsedTime;
		player.y += player.dy * fElapsedTime;

		// keep ship in gamespace 
		WrapCoordinates(player.x, player.y, player.x, player.y);

		// Check ship collision with asteroids 
		for (auto& a : vecAsteroids)
			if (IsPointInsideCircle(a.x, a.y, a.nSize, player.x, player.y))
				bDead = true;	// whoops 

		// Fire Bullet in direction of player
		if (m_keys[VK_SPACE].bReleased)
			vecBullets.push_back({ player.x, player.y, 50.0f * sinf(player.angle), -50.0f * cosf(player.angle), 0, 0});

		// Update and draw asteroids 
		for (auto& a : vecAsteroids)
		{
			a.x += a.dx * fElapsedTime;
			a.y += a.dy * fElapsedTime;
			a.angle += 0.5f * fElapsedTime;
			WrapCoordinates(a.x, a.y, a.x, a.y);
			DrawWireFrameModel(vecModelAsteroid, a.x, a.y, a.angle, a.nSize, FG_RED);
		}

		// temp vector to store new asteroids 
		vector<sSpaceObject> newAsteroids;		

		// update and draw bullets 
		for (auto& b : vecBullets)
		{
			b.x += b.dx * fElapsedTime;
			b.y += b.dy * fElapsedTime;
			WrapCoordinates(b.x, b.y, b.x, b.y);
			Draw(b.x, b.y);

			// Check for collision with asteroids
			for (auto& a : vecAsteroids)
			{
				if (IsPointInsideCircle(a.x, a.y, a.nSize, b.x, b.y))
				{
					// Asteroid hit 
					b.x = -100;		// make bullet go offscreen

					if (a.nSize > 4)
					{
						// create two child asteroids 
						float angle1 = ((float)rand() / (float)RAND_MAX) * 6.283185f;		// choose random angel between 0 & 2(PI)
						float angle2 = ((float)rand() / (float)RAND_MAX) * 6.283185f;
						newAsteroids.push_back({ a.x, a.y, 10.f * sinf(angle1), 10.0f * cosf(angle1), (int)a.nSize >> 1, 0.0f });
						newAsteroids.push_back({ a.x, a.y, 10.f * sinf(angle2), 10.0f * cosf(angle2), (int)a.nSize >> 1, 0.0f });
					}

					a.x = -100;		// remove asteroid after being hit 
					nScore += 100;	// increase player score 
				}
			}
		}

		// append new asteroids to existing vector 
		for (auto a : newAsteroids)
			vecAsteroids.push_back(a);

		// remove off screen bullets 
		if (vecBullets.size() > 0)
		{
			auto i = remove_if(vecBullets.begin(), vecBullets.end(),
				[&](sSpaceObject o) { return (o.x < 1 || o.y < 1 || o.x >= ScreenWidth() - 1 || o.y >= ScreenHeight() - 1); });
			if (i != vecBullets.end())
				vecBullets.erase(i);
		}

		// remove hit asteroid 
		if (vecAsteroids.size() > 0)
		{
			auto i = remove_if(vecAsteroids.begin(), vecAsteroids.end(), [&](sSpaceObject o) { return (o.x < 0); });
			if (i != vecAsteroids.end())
				vecAsteroids.erase(i);
		}

		if (vecAsteroids.empty())
		{
			nScore += 1000;
			vecAsteroids.clear();
			vecBullets.clear();

			// Add Two new asteroids but in a place where the player is not.
			// add them 90 degrees left and right to the player
			// their cooridnates will be wrapped by the next asteroid update 
			vecAsteroids.push_back({ 30.0f * sinf(player.angle - 3.14159f / 2.0f),
									 30.0f * cosf(player.angle - 3.14159f / 2.0f),
									 10.0f * sinf(player.angle),
									 10.0f * cosf(player.angle),
									 (int)16, 0.0f });

			vecAsteroids.push_back({ 30.0f * sinf(player.angle + 3.14159f / 2.0f),
									 30.0f * cosf(player.angle + 3.14159f / 2.0f),
									 10.0f * sinf(-player.angle),
									 10.0f * cosf(-player.angle),
									 (int)16, 0.0f });
		}


		// Draw ship
		DrawWireFrameModel(vecModelShip, player.x, player.y, player.angle);

		// Draw Score 
		DrawString(2, 2, L"SCORE: " + to_wstring(nScore));

		return true;
	}

	// draw any polygon 
	void DrawWireFrameModel(const vector<pair<float, float>>& vecModelCoordinates, float x, float y, float r = 0.0f, float s = 1.0f, short col = FG_WHITE)
	{
		// pair.first = x coordinate 
		// pair.second = y coordinate 

		// create translated model vecrtor of coordinate pairs
		vector<pair<float, float>> vecTransformedCoordinates;
		int verts = vecModelCoordinates.size();
		vecTransformedCoordinates.resize(verts);

		// Rotate 
		for (int i = 0; i < verts; i++)
		{
			vecTransformedCoordinates[i].first = vecModelCoordinates[i].first * cosf(r) - vecModelCoordinates[i].second * sinf(r);
			vecTransformedCoordinates[i].second = vecModelCoordinates[i].first * sinf(r) + vecModelCoordinates[i].second * cosf(r);
		}

		// Scale 
		for (int i = 0; i < verts; i++)
		{
			vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first * s;
			vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second * s;
		}

		// translate 
		for (int i = 0; i < verts; i++)
		{
			vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first + x;
			vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second + y;
		}

		// draw closed polygon 
		for (int i = 0; i < verts + 1; i++)
		{
			int j = (i + 1);
			DrawLine(vecTransformedCoordinates[i % verts].first, vecTransformedCoordinates[i % verts].second,
				vecTransformedCoordinates[j % verts].first, vecTransformedCoordinates[j % verts].second, PIXEL_SOLID, col);
		}
	}

	// wrap around the x & y axis 
	void WrapCoordinates(float ix, float iy, float& ox, float& oy)
	{
		// output to input 
		ox = ix;
		oy = iy;

		//  x-axis // check if input is beyond boundaries of array 
		if (ix < 0.0f) ox = ix + (float)ScreenWidth();
		if (ix >= (float)ScreenWidth()) ox = ix - (float)ScreenWidth();

		//  y-axis // check if input is beyond boundaries of array 
		if (iy < 0.0f) oy = iy + (float)ScreenHeight();
		if (iy >= (float)ScreenHeight()) oy = iy - (float)ScreenHeight();
	}

	// Override Draw Function so object fully wraps around axis 
	virtual void Draw(int x, int y, short c = 0x2588, short col = 0x000F)
	{
		float fx, fy;
		WrapCoordinates(x, y, fx, fy);
		olcConsoleGameEngine::Draw(fx, fy, c, col);
	}
};

int main()
{
	Asteroids game;
	game.ConstructConsole(160, 100, 8, 8);
	game.Start();
	return 0;
}