/* Slight modification of original source code. Now uses olcPixelGameEngine.h version 1.20.
All keyboard input removed. Use mouse left click followed by right click. Cursor visible only 
during input. Only bomb and adjacent gems explode on removal.
*/

/*
	Live 50K Sub Special - Matching Gems Game
	"Thanks everyone, it means so much..." - javidx9

	License (OLC-3)
	~~~~~~~~~~~~~~~

	Copyright 2018-2019 OneLoneCoder.com

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions
	are met:

	1. Redistributions or derivations of source code must retain the above
	copyright notice, this list of conditions and the following disclaimer.

	2. Redistributions or derivative works in binary form must reproduce
	the above copyright notice. This list of conditions and the following
	disclaimer must be reproduced in the documentation and/or other
	materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its
	contributors may be used to endorse or promote products derived
	from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
	A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
	HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
	LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Relevant Video: https://youtu.be/7y8Zg87rtjs

	Links
	~~~~~
	YouTube:	https://www.youtube.com/javidx9
				https://www.youtube.com/javidx9extra
	Discord:	https://discord.gg/WhwHUMV
	Twitter:	https://www.twitter.com/javidx9
	Twitch:		https://www.twitch.tv/javidx9
	GitHub:		https://www.github.com/onelonecoder
	Patreon:	https://www.patreon.com/javidx9
	Homepage:	https://www.onelonecoder.com

	Author
	~~~~~~
	David Barr, aka javidx9, �OneLoneCoder 2019
*/

// NOTE: THIS CODE IS PROVIDED AS IS FROM THE END OF THE
// LIVE STREAM. SO ITS A MESS. ENJOY!

#include <iostream>
#include <string>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h" //version 1.20

#undef min
#undef max
const int PXLEN = 8;

class Gems : public olc::PixelGameEngine
{
	boolean leftpressed = false;

public:
	Gems()
	{
		sAppName = "50K Subscriber Gems";
	}

	struct sGem
	{
		olc::Pixel colour;
		bool bExist;
		bool bRemove;
		bool bBomb;
		bool bBombed;
	};

	enum STATES
	{
		STATE_USER,
		STATE_SWAP,
		STATE_CHECK,
		STATE_ERASE,
		STATE_COMPRESS,
		STATE_NEWGEMS,
	} nState,
		nNextState;

	sGem gems[8][8];

	float fDelayTime = 0.0f;
	std::string sprGem, sprBomb;

	int nTotalGems = 0;

	int nCursorX = 0, nCursorY = 0;
	int nSwapX = 0, nSwapY = 0;

	bool bSwapFail = false;
	bool bGemsToRemove = false;

	struct sFragment
	{
		float x;
		float y;
		float vx;
		float vy;
		olc::Pixel colour;
	};

	std::list<sFragment> fragments;

	olc::Pixel randColour()
	{

		switch (rand() % 4)
		{
		case 0:
			return olc::RED;
		case 1:
			return olc::YELLOW;
		case 2:
			return olc::BLUE;
		case 3:
			return olc::DARK_GREEN;
		case 4:
			return olc::DARK_MAGENTA;
		default:
			return olc::WHITE;
		}
	}

public:
	bool OnUserCreate()
	{
		sprGem += "..XXXX..";
		sprGem += ".XXXXXX.";
		sprGem += "XXXXXXXX";
		sprGem += "XXX..XXX";
		sprGem += "XXX..XXX";
		sprGem += "XXXXXXXX";
		sprGem += ".XXXXXX.";
		sprGem += "..XXXX..";

		sprBomb += "...XX...";
		sprBomb += ".X.XX.X.";
		sprBomb += "..XXXX..";
		sprBomb += "XXXXXXXX";
		sprBomb += "XXXXXXXX";
		sprBomb += "..XXXX..";
		sprBomb += ".X.XX.X.";
		sprBomb += "...XX...";

		for (int x = 0; x < 8; x++)
		{
			for (int y = 0; y < 8; y++)
			{
				gems[x][y].bExist = false;
				gems[x][y].bRemove = false;
				gems[x][y].bBomb = false;
				gems[x][y].bBombed = false;
				gems[x][y].colour = randColour();
			}
		}

		nState = STATE_COMPRESS;
		nNextState = STATE_USER;

		srand(time(0));

		return true;
	}

	bool OnUserUpdate(float fElapsedTime)
	{
		if (fDelayTime > 0.0f)
		{
			fDelayTime -= fElapsedTime;
		}
		else
		{
			auto boom = [&](int x, int y, int size, olc::Pixel colour) {
				auto random_float = [&](float min, float max) {
					return ((float)rand() / (float)RAND_MAX) * (max - min) + min;
				};

				for (int i = 0; i < size; i++)
				{
					float a = random_float(0, 2.0f * 3.14159f);
					sFragment f = {(float)x, (float)y, cos(a) * random_float(30.0f, 75.0), sin(a) * random_float(30.0f, 75.0), colour};
					fragments.push_back(f);
				}
			};

			// Gameplay
			switch (nState)
			{
			case STATE_USER:
				if (nTotalGems < 64)
					nNextState = STATE_COMPRESS;
				else
				{
					if (GetMouse(0).bPressed)
					{
						nCursorX = GetMouseX() / PXLEN;
						nCursorY = GetMouseY() / PXLEN;
						leftpressed = true;
					}
					if (GetMouse(1).bPressed && leftpressed)
					{
						nSwapX = GetMouseX() / PXLEN;
						nSwapY = GetMouseY() / PXLEN;
						leftpressed = false;
						if (!(abs(nCursorX - nSwapX) > 1) && !(abs(nCursorY - nSwapY) > 1))
							nNextState = STATE_SWAP;
					}
				}
				break;
			case STATE_SWAP:
				bSwapFail = true;
				std::swap(gems[nCursorX][nCursorY], gems[nSwapX][nSwapY]);
				fDelayTime = 0.5f;
				nNextState = STATE_CHECK;
				break;
			case STATE_CHECK:

				bGemsToRemove = false;

				for (int x = 0; x < 8; x++)
				{
					for (int y = 0; y < 8; y++)
					{
						if (!gems[x][y].bRemove)
						{
							bool bPlaceBomb = false;

							// Check Horizontally
							int nChain = 1;
							while (gems[x][y].colour == gems[x + nChain][y].colour && (nChain + x) < 8)
								nChain++;
							if (nChain >= 3)
							{
								if (nChain >= 4)
									bPlaceBomb = true;

								while (nChain > 0)
								{
									gems[x + nChain - 1][y].bRemove = true;

									if (gems[x + nChain - 1][y].bBomb)
									{
										for (int i = -1; i < 2; i++)
										{
											for (int j = -1; j < 2; j++)
											{
												int m = std::min(std::max(i + (x + nChain - 1), 0), 7);
												int n = std::min(std::max(j + y, 0), 7);
												gems[m][n].bRemove = true;
												gems[m][n].bBombed = true;
											}
										}
									}

									nChain--;
									bSwapFail = false;
									bGemsToRemove = true;
								}
							}

							// Check Vertically
							nChain = 1;
							while (gems[x][y].colour == gems[x][y + nChain].colour && (nChain + y) < 8)
								nChain++;
							if (nChain >= 3)
							{
								if (nChain >= 4)
									bPlaceBomb = true;

								while (nChain > 0)
								{
									gems[x][y + nChain - 1].bRemove = true;

									if (gems[x][y + nChain - 1].bBomb)
									{
										for (int i = -1; i < 2; i++)
										{
											for (int j = -1; j < 2; j++)
											{
												int m = std::min(std::max(i + x, 0), 7);
												int n = std::min(std::max(j + (y + nChain - 1), 0), 7);
												gems[m][n].bRemove = true;
												gems[m][n].bBombed = true;
											}
										}
									}

									nChain--;
									bSwapFail = false;
									bGemsToRemove = true;
								}
							}

							if (bPlaceBomb && !gems[x][y].bBomb)
							{
								gems[x][y].bBomb = true;
								gems[x][y].bBombed = true;
								gems[x][y].bRemove = false;
							}
						}
					}
				}

				if (bGemsToRemove)
					fDelayTime = 0.75f;

				nNextState = STATE_ERASE;
				break;
			case STATE_ERASE:
				if (!bGemsToRemove)
				{
					if (bSwapFail)
					{
						std::swap(gems[nCursorX][nCursorY], gems[nSwapX][nSwapY]);
					}

					nNextState = STATE_USER;
				}
				else
				{
					for (int x = 0; x < 8; x++)
					{
						for (int y = 0; y < 8; y++)
						{
							if (gems[x][y].bRemove)
							{
								gems[x][y].bExist = false;
								//gems[x][y].bBomb = false;
								if (gems[x][y].bBombed)
									boom(x * 8 + 4, y * 8 + 4, 50, gems[x][y].colour);
								nTotalGems--;
							}
						}
					}
					nNextState = STATE_COMPRESS;
				}
				break;
			case STATE_COMPRESS:

				for (int y = 6; y >= 0; y--)
				{
					for (int x = 0; x < 8; x++)
					{
						if (gems[x][y].bExist && !gems[x][y + 1].bExist)
							std::swap(gems[x][y], gems[x][y + 1]);
					}
				}

				fDelayTime = 0.1f;
				nNextState = STATE_NEWGEMS;
				break;

			case STATE_NEWGEMS:
				for (int x = 0; x < 8; x++)
				{
					if (!gems[x][0].bExist)
					{
						gems[x][0].bExist = true;
						gems[x][0].bRemove = false;
						gems[x][0].bBomb = false;
						gems[x][0].bBombed = false;
						gems[x][0].colour = randColour();
						nTotalGems++;
					}
				}

				if (nTotalGems < 64)
				{
					fDelayTime = 0.1f;
					nNextState = STATE_COMPRESS;
				}
				else
					nNextState = STATE_CHECK;

				break;
			}

			nState = nNextState;

		} // End Gameplay

		// Rendering
		Clear(olc::BLACK);

		// Draw Field
		for (int x = 0; x < 8; x++)
		{
			for (int y = 0; y < 8; y++)
			{
				if (gems[x][y].bExist)
				{
					std::string &source = gems[x][y].bBomb ? sprBomb : sprGem;
					for (int i = 0; i < 8; i++)
					{
						for (int j = 0; j < 8; j++)
						{
							if (source[j * 8 + i] != '.' /* && !gems[x][y].bRemove */)
								Draw(x * 8 + i, y * 8 + j, gems[x][y].colour);
						}
					}
				}
			}
		}

		// Draw Cursor
		if (nState == STATE_USER && leftpressed)
			DrawRect(nCursorX * 8, nCursorY * 8, 7, 7, olc::WHITE);

		for (auto &f : fragments)
		{
			Draw(f.x, f.y, f.colour);
			f.x += f.vx * fElapsedTime;
			f.y += f.vy * fElapsedTime;
		}

		std::remove_if(fragments.begin(), fragments.end(),
					   [&](const sFragment &f) {
						   return f.x < 0 || f.x > ScreenWidth() || f.y < 0 || f.y > ScreenHeight();
					   });

		return true;
	}
};

int main()
{
	Gems game;
	if (game.Construct(64, 64, PXLEN, PXLEN))
		game.Start();

	return 0;
}