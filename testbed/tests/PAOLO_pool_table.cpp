// MIT License

// Copyright (c) 2019 Erin Catto

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "test.h"

#include <iostream>
#include <cstdlib>
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <mmsystem.h>
using namespace std;



/// This tests bullet collision and provides an example of a gameplay scenario.
class PoolTable : public Test
{
public:
	PoolTable()
	{
		//mciSendString("open \"C:\\Users\\dhume\\Documents\\GitHub\\box2d\\build\\testbed\\data\\audio.mp3\" type mpegvideo alias mp3", NULL, 0, NULL);
		//mciSendString("play mp3", NULL, 0, NULL);

		m_world->SetGravity(b2Vec2(0.0f, 0.0f));

		b2Body* ground;
		{
			b2BodyDef bd;
			bd.position.Set(0.0f, 0.0f);
			ground = m_world->CreateBody(&bd);

			b2EdgeShape shape;

			b2FixtureDef sd;
			sd.shape = &shape;
			sd.density = 1.0f;
			sd.restitution = 0.0f;

			// Left vertical
			shape.SetTwoSided(b2Vec2(-22.0f, 1.0f), b2Vec2(-22.0f, 38.0f));
			ground->CreateFixture(&sd);

			shape.SetTwoSided(b2Vec2(-22.0f, -38.0f), b2Vec2(-22.0f, -1.f));
			ground->CreateFixture(&sd);

			// Right vertical
			shape.SetTwoSided(b2Vec2(22.0f, 1.0f), b2Vec2(22.0f, 38.0f));
			ground->CreateFixture(&sd);

			shape.SetTwoSided(b2Vec2(22.0f, -38.0f), b2Vec2(22.0f, -1.0f));
			ground->CreateFixture(&sd);

			// Top horizontal
			shape.SetTwoSided(b2Vec2(-20.0f, 40.0f), b2Vec2(20.0f, 40.0f));
			ground->CreateFixture(&sd);

			// Bottom horizontal
			shape.SetTwoSided(b2Vec2(-20.0f, -40.0f), b2Vec2(20.0f, -40.0f));
			ground->CreateFixture(&sd);
		}

		float x[] = { -23, 23, -23, 23, -25, 25 };
		float y[] = { -41, 41, 41, -41, 0, 0 };

		for (int i = 0; i < sizeof(x) / sizeof(x[0]); ++i)
		{
			b2BodyDef bd;
			bd.position.Set(x[i], y[i]);
			b2Body* hole = m_world->CreateBody(&bd);

			b2CircleShape shape;
			shape.m_radius = 3.0f;

			b2FixtureDef fd;
			fd.shape = &shape;
			fd.density = 0.0f;	// will use as a marker to recognize this body is a hole.
			fd.friction = 1.0f;
			fd.restitution = 0.0f;
			hole->CreateFixture(&fd);
		}

		float x_pos[] = { -0,  0, -1, 1, -2, 0, 2, -3, -1, 1, 3 };
		float y_pos[] = { -25,  25 + 0, 25 + 1.5, 25 + 1.5, 25 + 3, 25 + 3, 25 + 3,  25 + 4.5, 25 + 4.5, 25 + 4.5, 25 + 4.5 };

		// Ball
		for (int i = 0; i < sizeof(x_pos) / sizeof(x_pos[0]); ++i)
		{
			b2BodyDef bd;
			bd.position.Set(x_pos[i], y_pos[i]);
			bd.type = b2_dynamicBody;
			bd.bullet = true;

			b2Body* ball = m_world->CreateBody(&bd);

			b2CircleShape shape;
			shape.m_radius = (i == 0) ? 0.6f : 0.8f;

			b2FixtureDef fd;
			fd.shape = &shape;
			fd.density = 1.0f;
			fd.friction = 0.2f;
			fd.restitution = 0.8f;
			ball->CreateFixture(&fd);

			b2FrictionJointDef jd;
			jd.localAnchorA.SetZero();
			jd.localAnchorB.SetZero();
			jd.bodyA = ground;
			jd.bodyB = ball;
			jd.collideConnected = true;
			jd.maxForce = 10.f;
			jd.maxTorque = 1.f;
			m_world->CreateJoint(&jd);
		}
	}

	void Step(Settings& settings) override
	{
		Test::Step(settings);

		g_debugDraw.DrawString(5, m_textLine, "Score = %d", m_score);
		m_textLine += m_textIncrement;

		// Traverse the contact results. Destroy bodies that touch the holes, instead teleport the smaller body (white ball)
		for (int32 i = 0; i < m_pointCount; ++i)
		{
			ContactPoint* point = m_points + i;

			b2Body* body1 = point->fixtureA->GetBody();
			b2Body* body2 = point->fixtureB->GetBody();

			b2Fixture* fl1 = body1->GetFixtureList();
			if (fl1[0].GetDensity() == 0)	// using 0 density to recognize we hit a hole
			{
				b2Fixture* fl2 = body2->GetFixtureList();
				b2Shape* sh = fl2[0].GetShape();
				if (sh->GetType() == b2Shape::e_circle)
				{
					b2CircleShape* csh = static_cast<b2CircleShape*>(sh);
					if (csh->m_radius > 0.7f)	// one of the regular balls "hit" a hole - remove the ball and add to the score
					{
						mciSendString("open \"C:\\Users\\dhume\\Documents\\GitHub\\box2d\\build\\testbed\\data\\woohoo.mp3\" type mpegvideo alias woohoo", NULL, 0, NULL);
						mciSendString("setaudio woohoo volume to 30", NULL, 0, NULL);
						mciSendString("play woohoo from 0", NULL, 0, NULL);

												
						m_score++;
						m_world->DestroyBody(body2);


					}
					else // the white ball "hit" a hole - reposition the white ball and apply the penalty
					{
						mciSendString("open \"C:\\Users\\dhume\\Documents\\GitHub\\box2d\\build\\testbed\\data\\boo.mp3\" type mpegvideo alias boo", NULL, 0, NULL);
						mciSendString("play boo from 0", NULL, 0, NULL);

												
						m_score -= 10;
						body2->SetTransform(b2Vec2(0.0f, -25.0f), 0.f);
						body2->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
						body2->SetAngularVelocity(0.f);


					}
				}
			}
		}
	}

	static Test* Create()
	{
		return new PoolTable;
	}

	int m_score = 0;
};

static int testIndex = RegisterTest("Examples", "PoolTable", PoolTable::Create);