/*
 * utility
 *
 *      Author: imerio spagnuolo
 *
 *  Definisco tutte le librerie da utilizzare nel codice.
 *  Tale file dev'essere incluso da tutti i file del progetto.
 *
 */
#include <time.h> 	//Libreria C che permette di accedere ad informazioni sul tempo
#include <math.h>	//Libreria C per funzioni matematiche
#include <stdio.h>
#include <string.h>	//Libreria C per lavorare con le stringhe
#include <GL/gl.h>	//Libreria GL
#include <GL/glu.h>	//Libreria GL
#include <SDL2/SDL.h>	//Libreria SDL
#include <SDL2/SDL_image.h>//SDL per gestire le immagini, caricare texture ecc
#include <SDL2/SDL_ttf.h>//SDL per gestire le scritte
#define TOTALTIME 120//dopo 2 minuti il gioco finisce
/* Tutti i tipi di camera */
#define CAMERA_BACK_HELICOPTER 0
#define CAMERA_LATERAL_HELICOPTER 1
#define CAMERA_TOP_HELICOPTER 2
#define CAMERA_DOWN_HELICOPTER 3
#define CAMERA_INTERN_HELICOPTER 5
#define CAMERA_MOUSE 4
#define CAMERA_TYPE_MAX 6

//Librerie C++
#include <vector>	//Libreria C++ per lavorare con i vettori
#include <iostream>	//Libreria C++ per operazioni di io
using namespace std;

//Enumerativo per numerare le texture utilizzate nel programma

enum textures
{
	SKY = 0,
	SMILE,
	SMILEPLUS,
	ANGRY,
	GRASS,
	SELFIE,
	DOOR,
	HELICOPTER,
	WINDOW,
	PROPELLER,
	ROAD,
	CREDITS,
	BINRECT,
	BENCH,
	LIGHTPOLE,
	NIGHT
};

enum displayList
{
	LIGHT0 = 1, LIGHT1, LIGHT2, LIGHT3
};

