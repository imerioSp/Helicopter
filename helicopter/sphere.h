/*
 * Sphere.h
 *
 *  Created on: 10/set/2016
 *      Author: imerio
 */

class Sphere
{
	void drawSphere(bool shadow);

public:

	Sphere(textures text)
	{
		idTexture = text;
		pos_x = (rand() % 60 + 1) - 30;
		pos_y = (rand() % 30 + 2);
		pos_z = (rand() % 60 + 1) - 30;
		regenerate = false;
		isShowing = false;
	}
	void Render(float px, float py, float pz); // disegna a schermo
	void RegenerateSphere();

	int pos_x, pos_y, pos_z;
	bool regenerate;
	textures idTexture;
	bool isShowing;
};

