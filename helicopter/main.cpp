/*
 * Main principale.
 * la funzione rendering esegue il render dell'intera scena.
 * SDL_GL_DrawText si occupa della scrittura del testo.
 * drawsky permette di disegnare il cielo
 * initTexture esegue il caricamento di tutte le texture usate nel programma
 */
#include "utility.h"
#include "helicopter.h"
#include "sphere.h"
#include "objects.h"

float viewAlpha = 20;
float viewBeta = 40; // angoli che definiscono la vista
float eyeDist = 5.0; // distanza dell'occhio dall'origine
bool useWireframe = false; //wireframe
bool useShadow = true; //ombre
bool start = false; //indica l'accensione del motore dell'elicottero
bool night = false; //giorno o notte
int cameraType = 0; //var che indica la camera attualmente in esecuzione
float mozzo = 0; // var che permette alle eliche di girare
int windowWidth = 750;
int windowHeight = 750;
float lightPosition[4] = { 0, 50, 0, 1 }; // Pos della luce. ultima comp=0 -> luce direzionale
Helicopter helicopter; // l'elicottero
Sphere sphere1(SMILE), sphere2(SMILEPLUS), sphere3(ANGRY); //I tre bersagli
Objects objects; //Gli altri oggetti
int nstep = 0; // numero di passi di FISICA fatti fin'ora
const int PHYS_SAMPLING_STEP = 10; // numero di millisec che un passo di fisica simula

// Frames Per Seconds
const int fpsSampling = 3000; // lunghezza intervallo di calcolo fps
float fps = 0; // valore di fps dell'intervallo precedente
int fpsNow = 0; // quanti fotogrammi ho disegnato fin'ora nell'intervallo attuale
Uint32 timeLastInterval = 0; // quando e' cominciato l'ultimo intervallo

int punteggio = 0; // punteggio del giocatore

extern bool colori; //Si utilizzano i colori o le texture

/* qualita del testo scritto e id della realtiva texture */
enum textquality
{
	solid, shaded, blended
};
uint font_id = -1;

//tipo di font utilizzato
const char *fontFileName = "personal.ttf";
//nome del programma
const char *progName = "Helicopter";
/*
 * Funzione per la scrittura del testo
 */
void SDL_GL_DrawText(TTF_Font *font, // font
		char fgR, char fgG, char fgB, char fgA, // colore testo
		char bgR, char bgG, char bgB, char bgA, // colore background
		char *text, int x, int y, // testo e posizione
		enum textquality quality) // qualità del testo
{
	SDL_Color tmpfontcolor = { fgR, fgG, fgB, fgA };
	SDL_Color tmpfontbgcolor = { bgR, bgG, bgB, bgA };
	SDL_Surface *initial;
	SDL_Surface *intermediary;
	SDL_Rect location;
	int w, h;

	/* Usiamo SDL_TTF per il rendering del testo */
	initial = NULL;
	if (quality == solid)
		initial = TTF_RenderText_Solid(font, text, tmpfontcolor);
	else if (quality == shaded)
		initial = TTF_RenderText_Shaded(font, text, tmpfontcolor,
				tmpfontbgcolor);
	else if (quality == blended)
		initial = TTF_RenderText_Blended(font, text, tmpfontcolor);

	/* Convertiamo il testo in un formato conosciuto */
	w = initial->w;
	h = initial->h;

	/* Allochiamo una nuova surface RGB */
	intermediary = SDL_CreateRGBSurface(0, w, h, 32, 0x000000ff, 0x0000ff00,
			0x00ff0000, 0xff000000);

	/* Copiamo il contenuto dalla prima alla seconda surface */
	SDL_BlitSurface(initial, 0, intermediary, 0);

	/* Informiamo GL della nuova texture */
	glBindTexture(GL_TEXTURE_2D, font_id);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA,
	GL_UNSIGNED_BYTE, intermediary->pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (initial != NULL)
	{
		location.x = x;
		location.y = y;
	}
	/* disegnamo la cornice nera */
	glLineWidth(2);
	glColor3f(0, 0, 0);
	glBegin(GL_LINE_LOOP);
	glVertex2f(location.x - 2, location.y - 2);
	glVertex2f(location.x + w + 2, location.y - 2);
	glVertex2f(location.x + w + 2, location.y + h + 2);
	glVertex2f(location.x - 2, location.y + h + 2);
	glEnd();

	/* prepariamoci al rendering del testo */
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, font_id);
	glColor3f(1.0f, 1.0f, 1.0f);

	/* Disegnamo un quads come location del testo */
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(location.x, location.y);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(location.x + w, location.y);
	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(location.x + w, location.y + h);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(location.x, location.y + h);
	glEnd();

	/* Disegnamo un contorno al quads */
	glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_LINE_STRIP);
	glVertex2f((GLfloat) location.x - 1, (GLfloat) location.y - 1);
	glVertex2f((GLfloat) location.x + w + 1, (GLfloat) location.y - 1);
	glVertex2f((GLfloat) location.x + w + 1, (GLfloat) location.y + h + 1);
	glVertex2f((GLfloat) location.x - 1, (GLfloat) location.y + h + 1);
	glVertex2f((GLfloat) location.x - 1, (GLfloat) location.y - 1);
	glEnd();

	/* Bad things happen if we delete the texture before it finishes */
	glFinish();

	/* return the deltas in the unused w,h part of the rect */
	location.w = initial->w;
	location.h = initial->h;

	/* Clean up */
	glDisable(GL_TEXTURE_2D);
	SDL_FreeSurface(initial);
	SDL_FreeSurface(intermediary);

}

/*
 * setta le matrici di trasformazione in modo
 * che le coordinate in spazio oggetto siano le coord 
 * del pixel sullo schemo (per disegno in 2D sopra la scena)
 */
void SetCoordToPixel()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-1, -1, 0);
	glScalef(2.0 / windowWidth, 2.0 / windowHeight, 1);
}

/*
 * Carica una texture da file ed esegui il bind con openGL.
 * Da eseguire al momento di inizializzare le texture nel sistema
 */
bool LoadTexture(int textbind, char *filename)
{
	SDL_Surface *s = IMG_Load(filename);
	if (!s)
		return false;

	glBindTexture(GL_TEXTURE_2D, textbind);
	gluBuild2DMipmaps(
	GL_TEXTURE_2D,
	GL_RGB, s->w, s->h,
	GL_RGB,
	GL_UNSIGNED_BYTE, s->pixels);
	glTexParameteri(
	GL_TEXTURE_2D,
	GL_TEXTURE_MAG_FILTER,
	GL_LINEAR);
	glTexParameteri(
	GL_TEXTURE_2D,
	GL_TEXTURE_MIN_FILTER,
	GL_LINEAR_MIPMAP_LINEAR);
	return true;
}

/*
 * Disegna una sfera che rappresenterà il cielo
 */
void drawSphere(double r, int lats, int longs)
{
	int i, j;
	for (i = 0; i <= lats; i++)
	{
		double lat0 = M_PI * (-0.5 + (double) (i - 1) / lats);
		double z0 = sin(lat0);
		double zr0 = cos(lat0);

		double lat1 = M_PI * (-0.5 + (double) i / lats);
		double z1 = sin(lat1);
		double zr1 = cos(lat1);

		glBegin(GL_QUAD_STRIP);
		for (j = 0; j <= longs; j++)
		{
			double lng = 2 * M_PI * (double) (j - 1) / longs; // circonferenza
			double x = cos(lng);
			double y = sin(lng);

			// le normali servono per l'EnvMap
			glNormal3f(x * zr0, y * zr0, z0);
			glVertex3f(r * x * zr0, r * y * zr0, r * z0);
			glNormal3f(x * zr1, y * zr1, z1);
			glVertex3f(r * x * zr1, r * y * zr1, r * z1);
		}
		glEnd();
	}
}

/*
 * setta la posizione della telecamera
 */
void setCamera()
{
	//preleva le coordinate dell'elicottero
	double px = helicopter.px;
	double py = helicopter.py;
	double pz = helicopter.pz;
	double angle = helicopter.facing;
	double cosf = cos(angle * M_PI / 180.0);
	double sinf = sin(angle * M_PI / 180.0);
	double camd, camh, ex, ey, ez, cx, cy, cz;
	double cosff, sinff;

	// controllo la posizione della camera a seconda dell'opzione selezionata
	switch (cameraType)
	{
		case CAMERA_BACK_HELICOPTER:
			camd = 15.5;
			camh = 7.0;
			ex = px + camd * sinf;
			ey = py + camh;
			ez = pz + camd * cosf;
			cx = px - camd * sinf;
			cy = py + camh;
			cz = pz - camd * cosf;
			gluLookAt(ex, ey, ez, cx, cy, cz, 0.0, 1.0, 0.0);
			break;
		case CAMERA_LATERAL_HELICOPTER:
			camd = 5.0;
			camh = 2.15;
			angle = helicopter.facing + 40.0;
			cosff = cos(angle * M_PI / 180.0);
			sinff = sin(angle * M_PI / 180.0);
			ex = px + camd * sinff;
			ey = py + camh;
			ez = pz + camd * cosff;
			cx = px - camd * sinf;
			cy = py + camh;
			cz = pz - camd * cosf;
			gluLookAt(ex, ey, ez, cx, cy, cz, 0.0, 1.0, 0.0);
			break;
		case CAMERA_TOP_HELICOPTER:
			camd = 5.5;
			camh = 1.0;
			ex = px + camd * sinf;
			ey = py + camh;
			ez = pz + camd * cosf;
			cx = px - camd * sinf;
			cy = py + camh;
			cz = pz - camd * cosf;
			gluLookAt(ex, ey + 5, ez, cx, cy, cz, 0.0, 1.0, 0.0);
			break;
		case CAMERA_DOWN_HELICOPTER:
			camd = 1.0;
			camh = -2;
			ex = px + camd * sinf;
			ey = py + camh;
			ez = pz + camd * cosf;
			cx = px - camd * sinf;
			cy = py + camh;
			cz = pz - camd * cosf;
			gluLookAt(ex, ey, ez, cx, cy, cz, 0.0, 1.0, 0.0);
			break;
		case CAMERA_INTERN_HELICOPTER:
			camd = 1.0;
			camh = 0.25;
			ex = px + camd * sinf;
			ey = py + camh;
			ez = pz + camd * cosf;
			cx = px - camd * sinf;
			cy = py + camh;
			cz = pz - camd * cosf;
			gluLookAt(ex, ey, ez, cx, cy, cz, 0.0, 1.0, 0.0);
			break;
		case CAMERA_MOUSE:
			glTranslatef(0, 0, -eyeDist);
			glRotatef(viewBeta, 1, 0, 0);
			glRotatef(viewAlpha, 0, 1, 0);
			break;
	}
}

/*
 * Disegna il cielo
 */
void drawSky()
{
	if (useWireframe)
	{
		glDisable(GL_TEXTURE_2D);
		glColor3f(0, 0, 0);
		glDisable(GL_LIGHTING);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		drawSphere(100.0, 20, 20);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glColor3f(1, 1, 1);
		glEnable(GL_LIGHTING);
	} else
	{
		if (night)
			glBindTexture(GL_TEXTURE_2D, NIGHT);
		else
			glBindTexture(GL_TEXTURE_2D, SKY);
		//applica la texture con calcolo automatico delle coordinate
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glEnable(GL_TEXTURE_GEN_R);

		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP); // Env map
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
		glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
		glColor3f(1, 1, 1);
		glDisable(GL_LIGHTING);

		drawSphere(100.0, 20, 20);

		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_TEXTURE_GEN_R);
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_LIGHTING);
	}

}

/*
 * Disegna una minimappa
 */
void drawMinimap()
{

	/* calcolo delle coordinate reali dell'oggetto su minimappa */
	float helicop_px;
	float helicop_pz;
	helicop_px = ((50 * helicopter.px) / 67) + 50 + 20;
	helicop_pz = ((50 * helicopter.pz) / 67) + 50 + windowHeight - 120;

	float cube_px;
	float cube_pz;
	if (sphere3.isShowing)
	{ /* Mostro nella minimappa la sfera distrattrice */
		cube_px = ((50 * sphere3.pos_x) / 67) + 50 + 20;
		cube_pz = ((50 * sphere3.pos_z) / 67) + 50 + windowHeight - 120;
	} else
	{
		cube_px = ((50 * sphere1.pos_x) / 67) + 50 + 20;
		cube_pz = ((50 * sphere1.pos_z) / 67) + 50 + windowHeight - 120;

	}
	/* disegna l'indicatore dell'elicottero */
	glColor3ub(0, 0, 255);
	glBegin(GL_QUADS);
	glVertex2d(helicop_px, helicop_pz + 3);
	glVertex2d(helicop_px + 3, helicop_pz);
	glVertex2d(helicop_px, helicop_pz - 3);
	glVertex2d(helicop_px - 3, helicop_pz);
	glEnd();

	/* disegno del target */
	glColor3ub(255, 0, 0);
	glBegin(GL_QUADS);
	glVertex2d(cube_px, cube_pz + 3);
	glVertex2d(cube_px + 3, cube_pz);
	glVertex2d(cube_px, cube_pz - 3);
	glVertex2d(cube_px - 3, cube_pz);
	glEnd();

	/* disegno minimappa */
	glColor3ub(210, 210, 210);
	glBegin(GL_POLYGON);
	glVertex2d(20, windowHeight - 120);
	glVertex2d(20, windowHeight - 20);
	glVertex2d(120, windowHeight - 20);
	glVertex2d(120, windowHeight - 120);
	glEnd();
	/* disegna della cornice */
	glColor3ub(0, 0, 0);
	glBegin(GL_LINE_LOOP);
	glVertex2d(20, windowHeight - 120);
	glVertex2d(20, windowHeight - 20);
	glVertex2d(120, windowHeight - 20);
	glVertex2d(120, windowHeight - 120);
	glEnd();

}

/*
 * disegna un rettangolo verticale con una texture personale
 * di fianco alla minimappa
 */
void drawSelfie()
{

	glPushMatrix();
	//carico texture
	glBindTexture(GL_TEXTURE_2D, SELFIE);
	glEnable(GL_TEXTURE_2D);
	//disabilito generazione automatica
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	//disegno del quads ed indico le coordinate texture che combaciono con i vertici del rettangolo
	glBegin(GL_QUADS);
	glColor3f(1, 1, 0);
	glTexCoord2f(0.0, 0.0);
	glVertex2d(130, windowHeight - 20 - 132);
	glTexCoord2f(0.0, 1.0);
	glVertex2d(208, windowHeight - 20 - 132);
	glTexCoord2f(1.0, 1.0);
	glVertex2d(208, windowHeight - 20);
	glTexCoord2f(1.0, 0.0);
	glVertex2d(130, windowHeight - 20);
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	//disegno della cornice
	glColor3f(0, 0, 0);
	glBegin(GL_LINE_LOOP);
	glColor3f(0, 0, 0);
	glVertex2d(130, windowHeight - 20 - 132);
	glVertex2d(208, windowHeight - 20 - 132);
	glVertex2d(208, windowHeight - 20);
	glVertex2d(130, windowHeight - 20);
	glEnd();
}

/*
 * esegue il rendering dell'intera scena
 */
void rendering(SDL_Window *win, double tempo_attuale, TTF_Font *font)
{
	// un frame in piu'
	fpsNow++;

	// impostiamo spessore linee
	glLineWidth(3);

	// settiamo il viewport
	glViewport(0, 0, windowWidth, windowHeight);

	// colore di sfondo (fuori dal mondo)
	glClearColor(1, 1, 1, 1);

	// settiamo la matrice di proiezione
	glMatrixMode( GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(70, // fovy,
			((float) windowWidth) / windowHeight, // aspect Y/X,
			0.2, // distanza del NEAR CLIPPING PLANE in coordinate vista
			1000 // distanza del FAR CLIPPING PLANE in coordinate vista
			);

	/* passiamo a lavorare sui modelli */
	glMatrixMode( GL_MODELVIEW);
	glLoadIdentity();

	// riempe tutto lo screen buffer di pixel color sfondo
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glCallList(LIGHT0); //Richiamo la displaylist della luce
	// settiamo la telecamera
	setCamera();
	//abilito il lighting
	glEnable(GL_LIGHTING);
	// disegna il cielo come sfondo
	drawSky();
	// disegna il suolo
	objects.DrawFloor();
	//Disegna i pali della luce
	objects.DrawLightPole();
	//Disegna le panchine
	objects.DrawBench();
	//Disegna i cestini
	objects.DrawBin();
	//Disegna il tabellone
	objects.DrawTabellone();
	// disegna la base dell'elicottero
	objects.DrawHeliport();

	// disegna il target
	objects.DrawSpherePoint(helicopter.px, helicopter.py, helicopter.pz);
	// disegna l'elicottero
	helicopter.Render();
	//disabilito il lighting
	glDisable(GL_LIGHTING);
	// impostiamo le matrici per poter disegnare in 2D
	SetCoordToPixel();

	// disegnamo i fps (frame x sec) come una barra a sinistra.
	// (vuota = 0 fps, piena = 100 fps)
	glBegin(GL_QUADS);
	float y = windowHeight * fps / 100;
	float ramp = fps / 100;
	glColor3f(1 - ramp, 0.8, ramp);
	glVertex2d(10, 0);
	glVertex2d(10, y);
	glVertex2d(0, y);
	glVertex2d(0, 0);
	glEnd();

	glLineWidth(1);

	// disegna la minimappa in alto a sx
	drawMinimap();

	// disegna un rettangolo di fianco alla minimappa
	drawSelfie();

	glDisable(GL_DEPTH_TEST);

	char str[3];
	sprintf(str, "%d", punteggio);
	char myword[] = "Punteggio: ";
	SDL_GL_DrawText(font, 0, 0, 0, 0, 210, 210, 210, 255, strcat(myword, str),
			windowWidth - 200, windowHeight - 50, shaded);
	int tempoRimanente = TOTALTIME - (tempo_attuale / CLOCKS_PER_SEC);
	sprintf(str, "%d", tempoRimanente);
	char myword2[] = "Tempo rimanente: ";
	SDL_GL_DrawText(font, 0, 0, 0, 0, 210, 210, 210, 255, strcat(myword2, str),
			windowWidth - 500, windowHeight - 50, shaded);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);

	glFinish();
	
	// ho finito: buffer di lavoro diventa visibile
	SDL_GL_SwapWindow(win);
}

void redraw()
{
	// ci automandiamo un messaggio che
	// ci fara' ridisegnare la finestra
	SDL_Event e;
	e.type = SDL_WINDOWEVENT;
	e.window.event = SDL_WINDOWEVENT_EXPOSED;
	SDL_PushEvent(&e);
}

/*
 * disegna la schermata di game over 
 */
void gameOver(SDL_Window *win, TTF_Font *font)
{
	// settiamo il viewport
	glViewport(0, 0, windowWidth, windowHeight);

	// colore di sfondo (fuori dal mondo)
	glClearColor(.0, .0, .4, 1);

	// riempe tutto lo screen buffer di pixel color sfondo
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	SetCoordToPixel();

	glLineWidth(2);

	// conversione della variabile punteggio
	char punti[3];
	sprintf(punti, "%d", punteggio);

	char stringa_punti[] = "Punteggio: ";
	char g_over[] = "GAME OVER";
	char continuare[] = "Premi un tasto per continuare";

//	int margine = 75;

	SDL_GL_DrawText(font, 0, 0, 0, 0, 210, 210, 210, 255,
			strcat(stringa_punti, punti), windowWidth / 2 - 75,
			windowHeight / 2 + 100, shaded);
	SDL_GL_DrawText(font, 0, 0, 0, 0, 210, 210, 210, 255, g_over,
			windowWidth / 2 - 80, windowHeight / 3 + 20, shaded);
	SDL_GL_DrawText(font, 0, 0, 0, 0, 210, 210, 210, 255, continuare,
			windowWidth / 2 - 150, windowHeight / 4 + 20, shaded);
	glFinish();

	SDL_GL_SwapWindow(win);
}

/*
 * Inizializzazione di tutte le texture usate nel programma
 */
int initTexture()
{

	if (!LoadTexture(SKY, (char *) "./images/sky_ok.jpg"))
		return -SKY;
	if (!LoadTexture(NIGHT, (char *) "./images/skyNight.jpg"))
		return -NIGHT;
	if (!LoadTexture(SMILE, (char *) "./images/smile2.jpg"))
		return -SMILEPLUS;
	if (!LoadTexture(SMILEPLUS, (char *) "./images/smile.jpg"))
		return -SMILE;
	if (!LoadTexture(GRASS, (char *) "./images/grass1.jpg"))
		return -GRASS;
	if (!LoadTexture(SELFIE, (char *) "./images/selfie.JPG"))
		return -SELFIE;
	if (!LoadTexture(DOOR, (char *) "./images/aircraft.jpg"))
		return -DOOR;
	if (!LoadTexture(HELICOPTER, (char *) "./images/textureHelicopter.jpg"))
		return -HELICOPTER;
	if (!LoadTexture(WINDOW, (char *) "./images/textureFinestrino.jpg"))
		return -WINDOW;
	if (!LoadTexture(PROPELLER, (char *) "./images/textureElica.jpg"))
		return -PROPELLER;
	if (!LoadTexture(ANGRY, (char *) "./images/angry.jpg"))
		return -ANGRY;
	if (!LoadTexture(ROAD, (char *) "./images/road.jpg"))
		return -ROAD;
	if (!LoadTexture(CREDITS, (char *) "./images/credits2.jpg"))
		return -CREDITS;
	if (!LoadTexture(BINRECT, (char *) "./images/binRect.png"))
		return -BINRECT;
	if (!LoadTexture(BENCH, (char *) "./images/binRect.png"))
		return -BENCH;
	if (!LoadTexture(LIGHTPOLE, (char *) "./images/lightpole.png"))
		return -LIGHTPOLE;

	return 1;
}

/*************************************/
/*       PROGRAMMA PRINCIPALE        */
/*************************************/

int main(int argc, char* argv[])
{
	SDL_Window *win;
	SDL_GLContext mainContext;
	Uint32 windowID;

	static int keymap[Controller::NKEYS] = { SDLK_a, SDLK_d, SDLK_w, SDLK_s,
			SDLK_q, SDLK_e };

	// inizializzazione di SDL
	SDL_Init( SDL_INIT_VIDEO);
	if (TTF_Init() < 0)
	{
		fprintf(stderr, "Impossibile inizializzare TTF: %s\n", SDL_GetError());
		SDL_Quit();
		return (2);
	}
	//Carico il font per la scrittura
	TTF_Font *font;
	font = TTF_OpenFont(fontFileName, 29);
	if (font == NULL)
	{
		fprintf(stderr, "Impossibile caricare il font.\n");
	}

	// settaggio dei buffer
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	// creazione di una finestra di windowWidth x windowHeight pixels
	win = SDL_CreateWindow(progName, 0, 0, windowWidth, windowHeight,
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	// creiamo il nostro contesto OpenGL e lo colleghiamo alla nostra window
	mainContext = SDL_GL_CreateContext(win);
	//abilito il depth test
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);	//luce del sole
	glEnable(GL_NORMALIZE); // opengl, per favore, rinormalizza le normali prima di usarle
	glFrontFace(GL_CW); // consideriamo Front Facing le facce ClockWise
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_POLYGON_OFFSET_FILL); // sposta i frammenti generati dalla
	glPolygonOffset(1, 1);             // rasterizzazione poligoni
									   // indietro di 1
	/*Inizializzo la displaylist per la luce generale */
	//DisplayList luce 1
	glNewList(LIGHT0, GL_COMPILE);
	/* imposto diversi parametri di illuminazione per una luce */
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

	// the ambient RGBA intensity of the light
	float tmpp[4] = { 0.3, 0.3, 0.3, 1 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, tmpp);

	static float tmpcol[4] = { 1, 1, 1, 1 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, tmpcol);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 127);
	glEndList();
	/* caricamento di tutte le textures necessarie nel programma */
	int res = initTexture();
	if (res < 0)
	{
		cout << "errore nel caricamento della texture numero: " << res << endl;
	}
	// variabili per il calcolo del tempo
	double time_start;
	double time_end;
	time_start = clock();
	double tempo_attuale;
	objects = Objects();
	/* CICLO DEGLI EVENTI */
	bool done = 0;
	while (!done)
	{

		SDL_Event e;

		// guardo se c'e' un evento:
		if (SDL_PollEvent(&e))
		{
			// se si: processa evento
			switch (e.type)
			{
				case SDL_KEYDOWN: // tasto abbassato
					helicopter.controller.EatKey(e.key.keysym.sym, keymap,
							true);
					switch (e.key.keysym.sym)
					{
						case SDLK_F1:
							cameraType = (cameraType + 1) % CAMERA_TYPE_MAX;
							break;
						case SDLK_F2:
							useWireframe = !useWireframe;
							break;
						case SDLK_F3:
							colori = !colori; //usa le texture o i colori
							break;
						case SDLK_F4:
							useShadow = !useShadow;
							break;
						case SDLK_F5:
							night = !night; //Giorno/Notte
							break;
					}
					break;
				case SDL_KEYUP: // tasto sollevato
					helicopter.controller.EatKey(e.key.keysym.sym, keymap,
							false);
					break;
				case SDL_QUIT: // quit
					done = 1;
					break;
				case SDL_WINDOWEVENT: // dobbiamo ridisegnare la finestra
					if (e.window.event == SDL_WINDOWEVENT_EXPOSED)
						rendering(win, tempo_attuale, font);
					else
					{
						windowID = SDL_GetWindowID(win);
						if (e.window.windowID == windowID)
						{
							switch (e.window.event)
							{
								case SDL_WINDOWEVENT_SIZE_CHANGED:
								{
									windowWidth = e.window.data1;
									windowHeight = e.window.data2;
									glViewport(0, 0, windowWidth, windowHeight);
									rendering(win, tempo_attuale, font);
									break;
								}
							}
						}
					}
					break;

				case SDL_MOUSEMOTION: /* movimento del mouse */
					if (e.motion.state
							& SDL_BUTTON(1) & cameraType == CAMERA_MOUSE)
					{
						viewAlpha += e.motion.xrel;
						viewBeta += e.motion.yrel;
						if (viewBeta < +5)
							viewBeta = +5;
						if (viewBeta > +90)
							viewBeta = +90;
					}
					break;

				case SDL_MOUSEWHEEL: /* movimento della rotellina del mouse */
					if (e.wheel.y < 0)
					{
						// avvicino il punto di vista (zoom in)
						eyeDist = eyeDist * 0.9;
						if (eyeDist < 1)
							eyeDist = 1;
					}
					if (e.wheel.y > 0)
					{
						// allontano il punto di vista (zoom out)
						eyeDist = eyeDist / 0.9;
					}
					if (eyeDist > 96) // impedisco all'utente di uscire con la visuale dal mondo per vedere
									  // cosa c'è al di fuori (lo sfondo bianco)
						eyeDist = 96;
					break;
			}
		} else
		{
			//decremento variabile mozzo che permette il movimento delle eliche,
			//usata insieme alla primitiva glRotate
			mozzo--;
			// nessun evento: siamo IDLE

			Uint32 timeNow = SDL_GetTicks(); // ora attuale
			// ritorniamo il numero di millisecondi da quando la libreria SDL è stata inizializzata

			if (timeLastInterval + fpsSampling < timeNow)
			{
				fps = 1000.0 * ((float) fpsNow) / (timeNow - timeLastInterval);
				fpsNow = 0;
				timeLastInterval = timeNow;
			}

			bool doneSomething = false;
			int guardia = 0; // sicurezza da loop infinito

			// finche' il tempo simulato e' rimasto indietro rispetto
			// al tempo reale facciamo i passi di fisica che ci mancano
			// e poi ridisegniamo la scena
			while ((nstep * PHYS_SAMPLING_STEP) < timeNow)
			{
				helicopter.DoStep();
				nstep++;
				doneSomething = true;
				timeNow = SDL_GetTicks();
				guardia++;
				if (guardia > 1000)
				{
					done = true;
					break;
				} // siamo troppo lenti!
			}

			if (doneSomething)
				rendering(win, tempo_attuale, font);

			// dopo due minuti il gioco termina
			time_end = clock();
			tempo_attuale = time_end - time_start;

			if (tempo_attuale / CLOCKS_PER_SEC >= TOTALTIME)
			{
				done = 1;
				printf("Hai totalizzato %d punti!\n", punteggio);

				int done1 = 0;

				// schermata di game over
				while (!done1)
				{
					if (SDL_PollEvent(&e))
					{
						switch (e.type)
						{
							case SDL_KEYDOWN:
								done1 = 1;
								break;
							case SDL_WINDOWEVENT:
								windowID = SDL_GetWindowID(win);
								if (e.window.windowID == windowID)
								{
									switch (e.window.event)
									{
										case SDL_WINDOWEVENT_SIZE_CHANGED:
										{
											windowWidth = e.window.data1;
											windowHeight = e.window.data2;
											glViewport(0, 0, windowWidth,
													windowHeight);
											break;
										}
									}
								}
						}
					} else
					{ // disegno la schermata di game over
						gameOver(win, font);
					} // fine else
				} // fine while
			} // fine if del tempo attuale

			else
			{
				// tempo libero!!!
			}
		}
	}

	SDL_GL_DeleteContext(mainContext);
	SDL_DestroyWindow(win);
	SDL_Quit();
	return (0);
}
