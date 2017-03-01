/*
 * objects.h
 *
 *  Created on: 02/set/2016
 *      Author: imerio
 */
/*
 * funzione globale per la generazione della matrice d'ombra
 */
void glShadowProjection(float * l, float * e, float * n);

class Objects
{

public:
	//Variabili

	float lightPos1[3];
	float lightPos2[3];
	float lightPos3[3];
	float e[3];
	float n[3];
	Objects()
	{
		Init();
	}
	void Init();
	void DrawFloor();
	void DrawTabellone();
	void DrawHeliport();
	void DrawBin();
	void DrawBench();
	void DrawLightPole();
	void DrawSpherePoint(float px, float py, float pz);
};
