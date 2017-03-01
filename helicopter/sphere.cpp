/*
 * Sphere.cpp
 *
 *  Created on: 10/set/2016
 *      Author: imerio
 */
#include "utility.h"
#include "sphere.h"

extern bool useShadow; // var globale esterna: per generare l'ombra
extern float mozzo;
extern int punteggio;
extern bool colori;
extern bool useWireframe;
extern bool night;
extern float lightPosition[];
extern void glShadowProjection(float * l, float * e, float * n); //Funzione che genera la matrice ombra 4x4

void Sphere::RegenerateSphere()
{
	if (regenerate)
	{
		srand(time(0));
		pos_x = (rand() % 59 + 1) - 30;
		pos_y = (rand() % 29 + 1);
		pos_z = (rand() % 59 + 1) - 30;
		regenerate = false;
	}
}

void Sphere::Render(float px, float py, float pz)
{
	glPushMatrix();
//	if (useWireframe)
//	{
//		glDisable(GL_TEXTURE_2D);
//		glColor3f(0, 0, 0);
//		glDisable(GL_LIGHTING);
//		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//		drawSphere(100.0, 20, 20);
//		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//		glColor3f(1, 1, 1);
//		glEnable(GL_LIGHTING);
//	} else
//	{
	drawSphere(false);

	if (useShadow && !night)
	{
		glPushMatrix();
		glColor3f(0.2, 0.2, 0.2);
		float e[] = { 0, 0.02, 0 };
		float n[] = { 0, -1, 0 };
		glShadowProjection(lightPosition, e, n);
		glDisable(GL_LIGHTING);
		drawSphere(true);
		glEnable(GL_LIGHTING);
		glPopMatrix();

	}
//	}

	glPopMatrix();
	// se l'aereo ha catturato la sfera
	if (px >= pos_x - 4 && px <= pos_x + 4 && py >= pos_y - 4 && py <= pos_y + 4
			&& pz >= pos_z - 4 && pz <= pos_z + 4)
	{
		if (idTexture == SMILEPLUS)
			punteggio += 2;
		else if (idTexture == ANGRY)
			punteggio -= 2;
		else
			punteggio++;
		regenerate = true;
		isShowing = false;
	}
}

void Sphere::drawSphere(bool shadow)
{
	glPushMatrix();
	GLUquadricObj *quadric;
	quadric = gluNewQuadric();
	if (!shadow)
	{
		if (!useWireframe)
		{
			if (colori)
			{
				if (idTexture == SMILE)
					glColor3f(.8, .5, .2);
				else if (idTexture == SMILEPLUS)
					glColor3f(.2, .5, .8);
				else
					glColor3f(0.8, 0, 0);
			} else
			{
				glBindTexture(GL_TEXTURE_2D, idTexture); //Texture da applicare alla sfera
				glEnable(GL_TEXTURE_2D);
				glDisable(GL_TEXTURE_GEN_S);
				glDisable(GL_TEXTURE_GEN_T);
				glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
				//Applico la texture alla sfera
				gluQuadricTexture(quadric, true);
			}
		}
	}
	//Imposto la posizione generata ed una piccola rotazione
	glTranslatef(pos_x, pos_y, pos_z);
	glTranslatef(1, 1, 1);
	glRotatef(mozzo, 1, 1, 0);
	glTranslatef(-1, -1, -1);
	gluQuadricDrawStyle(quadric, GLU_FILL);		//Disegna una sfera piena
	if (idTexture == SMILE)
		gluSphere(quadric, 2, 36, 36);
	else if (idTexture == SMILEPLUS)
		gluSphere(quadric, 1.2, 36, 36);		//Più piccolina
	else
		gluSphere(quadric, 3, 36, 36);		//Più grande

	gluDeleteQuadric(quadric);

	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}
