/*
 * object.cpp
 *
 *  Created on: 02/set/2016
 *      Author: imerio
 */
#include "utility.h"
#include "point3.h"
#include "objects.h"
#include "mesh.h"
#include "sphere.h"

/* caricamento degli oggetti mesh da file */
Mesh heliport((char*) "./objects/helipad3.obj");
Mesh h((char*) "./objects/H.obj");
Mesh tabellone((char *) "./objects/tabellone.obj");
Mesh tabelloneBase((char *) "./objects/tabelloneBase2.obj");
Mesh binRect((char *) "./objects/binRect.obj");
Mesh binStruct((char *) "./objects/binStruct.obj");
Mesh binInside((char *) "./objects/binIntern.obj");
Mesh bench((char *) "./objects/bench.obj");
Mesh benchStructure((char *) "./objects/benchStructure.obj");
Mesh lightPole((char *) "./objects/lightPole.obj");
Mesh lightPoleStructure((char *) "./objects/lightPoleStruct.obj");

bool colori = false; // se true si utilizzano gli oggetti colorati, altrimenti si usano le texture
extern Sphere sphere1, sphere2, sphere3;
extern bool useShadow;
extern int punteggio;
extern bool night;
extern bool useWireframe;
extern float lightPosition[];

void drawQuads(int startx, int startz, int end, textures flags, int S, int H,
		int K);
void drawTabellone(bool shadow);
void drawBin(Point3 translate, bool shadow);
void drawBench(Point3 translate, float angle, bool shadow);
void switchLightPole(float x, float y, float z, int gl_light);
void drawLightPole(Point3 translate, float angle, bool shadow);

void Objects::Init()
{
	e[0] = 0;
	e[1] = 0.02;
	e[2] = 0;
	n[0] = 0;
	n[1] = -1;
	n[2] = 0;
	//Inizializzo le displaylist
	Point3 lightPos = (lightPole.Center().rotateAsseY(90)).trasla(
			Point3(1100, 0, 183));
	glNewList(LIGHT1, GL_COMPILE);
	glPushMatrix();
	glScalef(-0.02, 0.02, -0.02);
	switchLightPole(lightPos.X() + 30, lightPos.Y(), lightPos.Z() - 110,
	GL_LIGHT1);
	lightPos1[0] = lightPos.X() + 30;
	lightPos1[1] = lightPos.Y();
	lightPos1[2] = lightPos.Z() - 110;
	glPopMatrix();
	glEndList();

	lightPos = (lightPole.Center().rotateAsseY(270)).trasla(
			Point3(2150, 0, -140));
	glNewList(LIGHT2, GL_COMPILE);
	glPushMatrix();
	glScalef(-0.02, 0.02, -0.02);
	switchLightPole(lightPos.X() - 61, lightPos.Y(), lightPos.Z() + 70,
	GL_LIGHT2);
	lightPos2[0] = lightPos.X() - 61;
	lightPos2[1] = lightPos.Y();
	lightPos2[2] = lightPos.Z() + 70;

	glPopMatrix();

	glEndList();

	//Terzo palo della luce
	lightPos = (lightPole.Center().rotateAsseY(90)).trasla(
			Point3(3300, 0, 183));
	glNewList(LIGHT3, GL_COMPILE);
	glPushMatrix();
	glScalef(-0.02, 0.02, -0.02);
	switchLightPole(lightPos.X() + 30, lightPos.Y(), lightPos.Z() - 110,
	GL_LIGHT3);
	lightPos3[0] = lightPos.X() + 30;
	lightPos3[1] = lightPos.Y();
	lightPos3[2] = lightPos.Z() - 110;
	glPopMatrix();
	glEndList();

}

/*
 * SHADOW
 */
/*
 * This is where the "magic" is done:
 *
 * Multiply the current ModelView-Matrix with a shadow-projetion
 * matrix.
 *
 * l is the position of the light source
 * e is a point on within the plane on which the shadow is to be
 *   projected.
 * n is the normal vector of the plane.
 *
 * Everything that is drawn after this call is "squashed" down
 * to the plane. Hint: Gray or black color and no lighting
 * looks good for shadows *g*
 */
void glShadowProjection(float * l, float * e, float * n)
{
	float d, c;
	float mat[16];

	// These are c and d (corresponding to the tutorial)

	d = n[0] * l[0] + n[1] * l[1] + n[2] * l[2];
	c = e[0] * n[0] + e[1] * n[1] + e[2] * n[2] - d;

	// Create the matrix. OpenGL uses column by column
	// ordering

	mat[0] = l[0] * n[0] + c;
	mat[4] = n[1] * l[0];
	mat[8] = n[2] * l[0];
	mat[12] = -l[0] * c - l[0] * d;

	mat[1] = n[0] * l[1];
	mat[5] = l[1] * n[1] + c;
	mat[9] = n[2] * l[1];
	mat[13] = -l[1] * c - l[1] * d;

	mat[2] = n[0] * l[2];
	mat[6] = n[1] * l[2];
	mat[10] = l[2] * n[2] + c;
	mat[14] = -l[2] * c - l[2] * d;

	mat[3] = n[0];
	mat[7] = n[1];
	mat[11] = n[2];
	mat[15] = -d;

	// Finally multiply the matrices together *plonk*
	glMultMatrixf(mat);
}
/*
 * Disegna il terreno
 */
void Objects::DrawFloor()
{
	const float S = 200; // size
	const float H = 0;   // altezza
	const int K = 75; //disegna K x K quads

	// ulilizzo le coordinate OGGETTO
	// cioe' le coordnate originali, PRIMA della moltiplicazione per la ModelView
	// in modo che la texture sia "attaccata" all'oggetto, e non "proiettata" su esso

	drawQuads(0, 0, K / 2, GRASS, S, H, K);	//disegna il prato nel primo quarto di mappa
	drawQuads(0, K / 2 + 1, K, GRASS, S, H, K);	//Disegna il prato nel
	drawQuads(K / 2, 0, K, GRASS, S, H, K);
	drawQuads(0, K / 2, K / 2 + 1, ROAD, S, H, K);//disegna una stradina per arrivare all'eliporto

}

void drawQuads(int startx, int startz, int end, textures flags, int S, int H,
		int K)
{
	if (!colori)
	{
		// disegno il terreno ripetendo una texture su di esso
		glBindTexture(GL_TEXTURE_2D, flags);
		glEnable(GL_TEXTURE_2D);
		//disabilito la gen. automatica di coord. texture
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		//glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0); // normale verticale uguale x tutti
	for (int x = startx; x < end; x++)
	{
		for (int z = startz; z < end; z++)
		{
			// scelgo il colore per quel quad
			if (colori)
			{
				if (z % 2 == 0 && x % 2 == 0)
					glColor3f(0.1, 0.4, 0.1);
				else if (z % 2 != 0 && x % 2 != 0)
					glColor3f(0.1, 0.43, 0.1);
				else if (z % 3 == 0 && x % 3 == 0)
					glColor3f(0.12, 0.4, 0.12);
				else
					glColor3f(0.15, 0.43, 0.15);
			}
			float x0 = -S + 2 * (x + 0) * S / K;
			float x1 = -S + 2 * (x + 1) * S / K;
			float z0 = -S + 2 * (z + 0) * S / K;
			float z1 = -S + 2 * (z + 1) * S / K;
			if (!colori)
				glTexCoord2f(0.0, 0.0);
			glVertex3d(x0, H, z0);
			if (!colori)
				glTexCoord2f(1.0, 0.0);
			glVertex3d(x1, H, z0);
			if (!colori)
				glTexCoord2f(1.0, 1.0);
			glVertex3d(x1, H, z1);
			if (!colori)
				glTexCoord2f(0.0, 1.0);
			glVertex3d(x0, H, z1);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

//Disegna l'eliporto con una H rossa al centro
void Objects::DrawHeliport()
{
	glPushMatrix();
	glScalef(-0.02, 0.02, -0.02);

	glPushMatrix();
	glColor3f(.5, .5, .4);
	glTranslatef(0, 5, 0);
	heliport.RenderNxF();
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.8, 0, 0);
	glTranslatef(0, heliport.bbmin.Y() * 0.02 + 5, 0); //trasliamo l'H sopra l'heliport
	h.RenderNxV();
	glPopMatrix();

	glPopMatrix();
}

//Disegna il Tabellone, crea un rettangolo impostando come vertici minimi e massimi i rispettivi vertici della mesh
//Inserisce una texture personale sfruttando i vertici del rettangolo
void Objects::DrawTabellone()
{
	drawTabellone(false);

	if (useShadow && !night) //ombra
	{
		glPushMatrix();

		glShadowProjection(lightPosition, e, n);
		glDisable(GL_LIGHTING);
		glColor3f(0.2, 0.2, 0.2);
		drawTabellone(true);
		glEnable(GL_LIGHTING);
		glPopMatrix();
	}

}

void drawTabellone(bool shadow)
{
	glPushMatrix();
	glScalef(-0.02, 0.02, -0.02);

	glPushMatrix();
	if (!shadow)
		glColor3f(.5, .5, .5);
	glTranslatef(860, 0, 2500);
	tabelloneBase.RenderNxV();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(860, 0, 2500);
	if (!shadow)
	{
		glColor3f(1, 1, 1);
		if (!useWireframe)
		{
			if (colori)
				glColor3f(.6, .8, .9);
			else
			{

				glBindTexture(GL_TEXTURE_2D, CREDITS);
				glEnable(GL_TEXTURE_2D);
				glDisable(GL_TEXTURE_GEN_S);
				glDisable(GL_TEXTURE_GEN_T);
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
				//crea un quads ed inserisci una texture le cui coordinate corrispondono
				//ai vertici del bounding box del tabellone
				glBegin(GL_QUADS);
				glTexCoord2f(0.0, 0.0);
				glVertex2d((tabellone.bbmin.X()), tabellone.bbmin.Y());
				glTexCoord2f(1.0, 0.0);
				glVertex2d((tabellone.bbmax.X()), tabellone.bbmin.Y());
				glTexCoord2f(1.0, 1.0);
				glVertex2d((tabellone.bbmax.X()), tabellone.bbmax.Y());
				glTexCoord2f(0.0, 1.0);
				glVertex2d((tabellone.bbmin.X()), tabellone.bbmax.Y());
				glEnd();
				glDisable(GL_TEXTURE_2D);

			}
		}
	}
	tabellone.RenderNxV();
	glPopMatrix();

	glPopMatrix();
}

//Disegna 3 cestini
void Objects::DrawBin()
{

	glPushMatrix();
	glScalef(-0.02, 0.02, -0.02);
	Point3 pos1 = Point3(681, 0, 183);
	Point3 pos2 = Point3(1700, 0, -140);
	Point3 pos3 = Point3(2900, 0, 183);
	drawBin(pos1, false);
	drawBin(pos2, false);
	drawBin(pos3, false);
	glPopMatrix();

	if (useShadow && !night)				//ombra
	{

		//Prima luce
		glPushMatrix();
		if (!night)
			glShadowProjection(lightPosition, e, n);
		else
			glShadowProjection(lightPos1, e, n);
		glDisable(GL_LIGHTING);
		glColor3f(0.2, 0.2, 0.2);
		glScalef(-0.02, 0.02, -0.02);
		drawBin(pos1, true);
		glPopMatrix();
		//Seconda luce
		glPushMatrix();
		if (!night)
			glShadowProjection(lightPosition, e, n);
		else
			glShadowProjection(lightPos2, e, n);
		glDisable(GL_LIGHTING);
		glColor3f(0.2, 0.2, 0.2);
		glScalef(-0.02, 0.02, -0.02);
		drawBin(pos2, true);
		glPopMatrix();
		//Terza luce
		glPushMatrix();
		if (!night)
			glShadowProjection(lightPosition, e, n);
		else
			glShadowProjection(lightPos3, e, n);
		glDisable(GL_LIGHTING);
		glColor3f(0.2, 0.2, 0.2);
		glScalef(-0.02, 0.02, -0.02);
		drawBin(pos3, true);
		glPopMatrix();
	}

}
//Disegna il cestino indicando la posizione
void drawBin(Point3 translate, bool shadow)
{
	glPushMatrix();
	if (!shadow)
		glColor3f(0, 0, 0);
	glTranslatef(translate.X(), translate.Y(), translate.Z());

	binStruct.RenderNxF();
	glPopMatrix();

	glPushMatrix();

	glTranslatef(translate.X(), translate.Y(), translate.Z());
	if (!shadow)
	{
		glColor3f(.8, .7, .0);
		if (!colori)
		{
			glBindTexture(GL_TEXTURE_2D, BINRECT);

			glEnable(GL_TEXTURE_2D);
			glEnable(GL_TEXTURE_GEN_S); // abilito la generazione automatica delle coord texture S e T
			glEnable(GL_TEXTURE_GEN_T);
			glEnable(GL_TEXTURE_GEN_R);

			glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR); //
			glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
			glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);

		}
	}
	binRect.RenderNxF();

	glDisable(GL_TEXTURE_2D);

	glPopMatrix();

	glPushMatrix();
	if (!shadow)
		glColor3f(0.1, 0.1, 0.1);
	glTranslatef(translate.X(), translate.Y(), translate.Z());

	binInside.RenderNxF();

	glPopMatrix();

}

//Disegna 3 panchine
void Objects::DrawBench()
{

	glPushMatrix();
	glScalef(-0.02, 0.02, -0.02);
	Point3 pos1 = Point3(881, 0, 183);
	Point3 pos2 = Point3(1900, 0, -140);
	Point3 pos3 = Point3(3100, 0, 183);
	drawBench(pos1, 270, false);
	drawBench(pos2, 90, false);
	drawBench(pos3, 270, false);
	glPopMatrix();

	if (useShadow && !night)
	{
		//Prima luce
		glPushMatrix();
		if (!night)
			glShadowProjection(lightPosition, e, n);
		else
			glShadowProjection(lightPos1, e, n);
		glDisable(GL_LIGHTING);
		glColor3f(0.2, 0.2, 0.2);
		glScalef(-0.02, 0.02, -0.02);
		drawBench(pos1, 270, true);
		glPopMatrix();
		//Seconda luce
		glPushMatrix();
		if (!night)
			glShadowProjection(lightPosition, e, n);
		else
			glShadowProjection(lightPos2, e, n);
		glDisable(GL_LIGHTING);
		glColor3f(0.2, 0.2, 0.2);
		glScalef(-0.02, 0.02, -0.02);
		drawBench(pos2, 90, true);
		glPopMatrix();
		//Terza luce
		glPushMatrix();
		if (!night)
			glShadowProjection(lightPosition, e, n);
		else
			glShadowProjection(lightPos3, e, n);
		glDisable(GL_LIGHTING);
		glColor3f(0.2, 0.2, 0.2);
		glScalef(-0.02, 0.02, -0.02);
		drawBench(pos3, 270, true);
		glPopMatrix();

	}
}
//Disegna la panchina indicando la posizione
void drawBench(Point3 translate, float angle, bool shadow)
{
	glPushMatrix();
	glTranslatef(translate.X(), translate.Y(), translate.Z());
	glRotatef(angle, 0, 1, 0);
	if (!shadow)
	{
		glColor3f(.8, .7, .0);
		if (!colori)
		{
			glBindTexture(GL_TEXTURE_2D, BENCH);

			glEnable(GL_TEXTURE_2D);
			glEnable(GL_TEXTURE_GEN_S); // abilito la generazione automatica delle coord texture S e T
			glEnable(GL_TEXTURE_GEN_T);
			glEnable(GL_TEXTURE_GEN_R);

			glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
			glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
			glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
		}
	}
	bench.RenderNxV();
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);

	glPushMatrix();
	if (!shadow)
		glColor3f(.0, .0, .0);
	glTranslatef(translate.X(), translate.Y(), translate.Z());
	glRotatef(angle, 0, 1, 0);
	benchStructure.RenderNxV();
	glPopMatrix();

}

//Disegna i pali della luce
void Objects::DrawLightPole()
{

	glPushMatrix();
	glScalef(-0.02, 0.02, -0.02);
	Point3 pos1 = Point3(1100, 0, 183);
	Point3 pos2 = Point3(2150, 0, -140);
	Point3 pos3 = Point3(3300, 0, 183);
	drawLightPole(pos1, 90, false); //Prima luce
	drawLightPole(pos2, 270, false);	//Seconda luce
	drawLightPole(pos3, 90, false);	//Terzo palo della luce
	glPopMatrix();
	if (night)
	{
		glEnable(GL_LIGHT1);
		glEnable(GL_LIGHT2);
		glEnable(GL_LIGHT3);

		glCallList(LIGHT1);
		glCallList(LIGHT2);
		glCallList(LIGHT3);
		glDisable(GL_LIGHT0);
	} else
	{
		glDisable(GL_LIGHT1);
		glDisable(GL_LIGHT2);
		glDisable(GL_LIGHT3);
		glEnable(GL_LIGHT0);
		glCallList(LIGHT0);
	}

	if (useShadow && !night)
	{
		//Prima luce
		glPushMatrix();
		if (!night)
			glShadowProjection(lightPosition, e, n);
		else
			glShadowProjection(lightPos1, e, n);

		glDisable(GL_LIGHTING);
		glColor3f(0.2, 0.2, 0.2);
		glScalef(-0.02, 0.02, -0.02);
		drawLightPole(pos1, 90, true);
		glPopMatrix();
		//Seconda luce
		glPushMatrix();
		if (!night)
			glShadowProjection(lightPosition, e, n);
		else
			glShadowProjection(lightPos2, e, n);
		glDisable(GL_LIGHTING);
		glColor3f(0.2, 0.2, 0.2);
		glScalef(-0.02, 0.02, -0.02);
		drawLightPole(pos2, 270, true);
		glPopMatrix();
		//Terza luce
		glPushMatrix();
		if (!night)
			glShadowProjection(lightPosition, e, n);
		else
			glShadowProjection(lightPos3, e, n);
		glDisable(GL_LIGHTING);
		glColor3f(0.2, 0.2, 0.2);
		glScalef(-0.02, 0.02, -0.02);
		drawLightPole(pos3, 90, true);
		glPopMatrix();

	}

}
//Disegna i pali della luce indicando la posizione
void drawLightPole(Point3 translate, float angle, bool shadow)
{
	glPushMatrix();
	if (!shadow)
	{
		glColor4f(.9, .9, .9, .7);
		glEnable(GL_BLEND); //Vetro trasparente
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	glTranslatef(translate.X(), translate.Y(), translate.Z());
	glRotatef(angle, 0, 1, 0);
	lightPole.RenderNxV();
	glDisable(GL_BLEND);
	glPopMatrix();

	glPushMatrix();
	if (!shadow)
		glColor3f(0, 0, 0);
	glTranslatef(translate.X(), translate.Y(), translate.Z());
	glRotatef(angle, 0, 1, 0);
	lightPoleStructure.RenderNxV();
	glPopMatrix();

}

//Accendi i lampioni -> tasto F6
void switchLightPole(float x, float y, float z, int gl_light)
{
	float col0[4] = { 0.8, 0.8, 0.8, 1 };
	glLightfv(gl_light, GL_DIFFUSE, col0);

	float col1[4] = { 0.5, 0.5, 0.0, 1 };
	glLightfv(gl_light, GL_AMBIENT, col1);

	float tmpPos[4] = { x, y, z, 1 };
	glLightfv(gl_light, GL_POSITION, tmpPos);

	float tmpDir[4] = { -2, -25, -2, 1 };
	glLightfv(gl_light, GL_SPOT_DIRECTION, tmpDir);
	//proprietà per far riflettere la luce solo nelle vicinanze del lampione
	glLightf(gl_light, GL_SPOT_CUTOFF, 70);
	glLightf(gl_light, GL_SPOT_EXPONENT, 5);

//	glLightf(gl_light, GL_CONSTANT_ATTENUATION, 0);
//	glLightf(gl_light, GL_LINEAR_ATTENUATION, 1);
}

//Disegna le 3 sfere
void Objects::DrawSpherePoint(float px, float py, float pz)
{
	sphere1.RegenerateSphere();
	sphere1.Render(px, py, pz);
	sphere1.isShowing = true;

	if ((punteggio + 1) % 6 == 0) // Ogni 5 punti compare la sfera da doppio punteggio
	{
		sphere2.RegenerateSphere();
		sphere2.Render(px, py, pz);
		sphere2.isShowing = true;
		sphere3.isShowing = false;
	} else if ((punteggio + 1) % 4 == 0) //Ogni 3 punti compare la sfera distrattore
	{
		sphere3.RegenerateSphere();
		sphere3.Render(px, py, pz);
		sphere3.isShowing = true;
		sphere2.isShowing = false;
	} else
	{
		sphere2.isShowing = false;
		sphere3.isShowing = false;
	}
}

