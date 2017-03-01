/*
 * helicopter
 *
 *      Author: imerio spagnuolo

 */
#include "utility.h"
#include "helicopter.h"
#include "point3.h"
#include "mesh.h"
#include "objects.h"

// variabili globali di tipo mesh
Mesh base((char *) "./objects/base.obj");
Mesh carlinga((char *) "./objects/carlinga5.obj");
Mesh elica((char *) "./objects/elica1.obj");
Mesh elicaPR((char *) "./objects/elica2.obj");
Mesh sedile((char *) "./objects/seat.obj");
Mesh man((char *) "./objects/man.obj");
Mesh command((char *) "./objects/commandPlane.obj");
Mesh finestrinoDx((char *) "./objects/finestrinoDx.obj");
Mesh finestrinoSx((char *) "./objects/finestrinoSx.obj");
Mesh vetroAnteriore((char *) "./objects/vetroAnteriore.obj");
Mesh porta((char *) "./objects/door.obj");
//Mesh definite nella classe object
extern Mesh heliport;
extern Mesh h;
extern Mesh tabellone;

extern bool useShadow; // var globale esterna: per generare l'ombra
extern bool start;	//var che indica se il motore è spento o messo in moto
extern float mozzo; //var che permette la rotazione delle eliche
extern bool colori;	//var che indica se usare i colori o le texture
extern bool night;	//var che indica se modalità giorno o notte
extern float lightPosition[];	//posizione della luce principale
extern void glShadowProjection(float * l, float * e, float * n); //Funzione che genera la matrice ombra 4x4
		
// da invocare quando e' stato premuto/rilasciato il tasto numero "keycode"
void Controller::EatKey(int keycode, int* keymap, bool pressed_or_released)
{
	for (int i = 0; i < NKEYS; i++)
	{
		if (keycode == keymap[i])
			key[i] = pressed_or_released;
	}
}

//Inserisco la texture facendo sì che sia attaccata all'oggetto e non proiettata su di esso
void SetupTexture(Point3 min, Point3 max, GLuint textbind)
{
	glBindTexture(GL_TEXTURE_2D, textbind);
	glEnable(GL_TEXTURE_2D);
	//abilito la gen. automatica di coordinate
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_GEN_R);
	
	// ulilizzo le coordinate OGGETTO
	// cioe' le coordnate originali, PRIMA della moltiplicazione per la ModelView
	// in modo che la texture sia "attaccata" all'oggetto, e non "proiettata" su esso
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	float sz = 1.0 / (max.Z() - min.Z());
	float ty = 1.0 / (max.Y() - min.Y());
	float tx = 1.0 / (max.X() - min.X());
	float s[4] = { 0, 0, sz, -min.Z() * sz };
	float t[4] = { 0, ty, 0, -min.Y() * ty };
	float x[4] = { tx, 0, 0, -min.X() * tx };
	glTexGenfv(GL_S, GL_OBJECT_PLANE, s);
	glTexGenfv(GL_T, GL_OBJECT_PLANE, t);
	glTexGenfv(GL_R, GL_OBJECT_PLANE, x);
}

// DoStep: facciamo un passo di fisica (a delta_t costante)
// Indipendente dal rendering.
void Helicopter::DoStep()
{
	
	float vxm, vym, vzm; // velocita' in spazio elicottero
	// da vel frame mondo a vel frame elicottero
	float cosf = cos(facing * M_PI / 180.0);
	float sinf = sin(facing * M_PI / 180.0);
	vxm = +cosf * vx - sinf * vz;
	vym = vy;
	vzm = +sinf * vx + cosf * vz;
	
	// gestione dello sterzo
	if (controller.key[Controller::LEFT])
		sterzo += velSterzo;
	if (controller.key[Controller::RIGHT])
		sterzo -= velSterzo;
	if (controller.key[Controller::UP])
	{
		if (!start)	//accendo il motore
			start = true;
		py += velQuota; //Salgo di quota
	}
	if (controller.key[Controller::DOWN])
		py -= velQuota; //diminuisco di quota

	sterzo *= velRitornoSterzo; // ritorno a volante dritto
			
	if (controller.key[Controller::ACC] && start)
		vzm -= accMax; // accelerazione in avanti
	if (controller.key[Controller::DEC] && start)
		vzm += accMax; // accelerazione indietro
	if (abs(py - (base.bbmin.Y() * -0.02)) < 0.01) //L'elicottero va a terra
	{
		vzm = 0; // Elicottero a terra -> non può spostarsi.
		start = false; //se sta a terra vuol dire che non è partito o che si è fermato
	}
	//attirti (semplificando)
	vxm *= attritoX;
	vym *= attritoY;
	vzm *= attritoZ;
	
	// l'orientamento dell'elicottero' segue quello dello sterzo
	// (a seconda della velocita' sulla z)
	facing = facing - (vzm * grip) * sterzo;
	
	// ritorno a vel coord mondo
	vx = +cosf * vxm + sinf * vzm;
	vy = vym;
	vz = -sinf * vxm + cosf * vzm;
	
	// posizione = posizione + velocita * delta t (ma delta t e' costante)
	px += vx;
	py += vy;
	pz += vz;
	
	// imposto i limiti del mondo di gioco
	if (pz >= 65)
		pz = 65;
	if (pz <= -65)
		pz = -65;
	if (px >= 65)
		px = 65;
	if (px <= -65)
		px = -65;
	if (py >= 50)
		py = 50;
	//Collisioni
	CheckCollision();
}

void Helicopter::CheckCollision()
{
	//Collisione con il tabellone
	Point3 pos = Point3(px, py, pz);	//Posizione attuale dell'elicottero
	Point3 min = tabellone.bbmin.scala(Point3(0.02, 0.02, -0.02));//Scaliamo il minimo ed il massimo per 2%
	Point3 max = tabellone.bbmax.scala(Point3(0.02, 0.02, 0.02));
	Point3 posInit = Point3(860 * -0.02, 0, 2500 * -0.02);//Ottengo la posizione iniziale, pari alla traslazione effettuata nel momento in cui abbiamo disegnato il tabellone
	min = min.operator +(Point3(0, 0, posInit.Z()));//Aggiungo la coordinata Z non presente nel rettangolo
	max = max.operator +(Point3(0, 0, posInit.Z()));
	//Ottengo i 4 vertici del rettangolo sommando alla posizione iniziale le coordinate minime e massime della X e della Y.
	Point3 pos00 = posInit.operator +(Point3(min.coord[0], min.coord[1], 0));
	Point3 pos01 = posInit.operator +(Point3(min.coord[0], max.coord[1], 0));
	Point3 pos11 = posInit.operator +(Point3(max.coord[0], max.coord[1], 0));
	Point3 pos10 = posInit.operator +(Point3(max.coord[0], min.coord[1], 0));
	if (pos.X() >= pos00.X() && pos.Y() >= pos00.Y() && pos.X() >= pos01.X()
			&& pos.Y() <= pos01.Y() && pos.X() <= pos11.X()
			&& pos.Y() <= pos11.Y() && pos.X() <= pos10.X()
			&& pos.Y() >= pos10.Y() && pos.Z() >= min.Z() && pos.Z() <= max.Z())
	{
		//Se collido resto nella posizione attuale
		px -= vx;
		py -= vy;
		pz -= vz;
	}
	
	//controllo con il terreno
	if (py < -base.bbmin.Y() * 0.02)	//Viene scalato del 2%
		py = base.bbmin.Y() * -0.02; // l'elicottero non deve andare al di sotto del terreno
	//Controllo con l'eliporto
	pos = Point3(px * (-50), (py - 2.55) * 50, pz * (-50));	//riscaliamo nelle dimensioni originali
	min = heliport.bbmin;
	max = heliport.bbmax;
	
	if (min.X() < pos.X() && min.Y() < pos.Y() && min.Z() < pos.Z()
			&& max.X() > pos.X() && max.Y() > pos.Y() && max.Z() > pos.Z())
	{
		px = 0;
		py = (h.bbmin.Y() * 0.02) + 2.55;//trasliamo l'elicottero sopra l'H nella posizione iniziale
		pz = 0;
	}
	
}
/*
 * init del controller
 */
void Controller::Init()
{
	for (int i = 0; i < NKEYS; i++)
		key[i] = false;
}

/*
 * init della classe Helix
 */
void Helicopter::Init()
{
	// inizializzo lo stato dell'elicottero
	
	// posizione e orientamento
	px = 0;
//	py = 0;
	py = (h.bbmin.Y() * 0.02) + 2.55;//trasliamo l'elicottero sopra l'H nella posizione iniziale
	pz = 0;
	facing = 0;
	
	// stato
	sterzo = 0;
	
	// velocita' attuale
	vx = 0;
	vy = 0;
	vz = 0;
	
	// inizializzo la struttura di controllo
	controller.Init();
	
	velSterzo = 2.0;         // A
	velRitornoSterzo = 0.93; // B, sterzo massimo = A*B / (1-B)
	velQuota = 0.04;
	
	accMax = 0.0011;
	// attriti: percentuale di velocita' che viene mantenuta
	// 1 = no attrito
	// <<1 = attrito grande
	attritoZ = 0.991; // piccolo attrito sulla Z (nel senso di rotolamento delle ruote)
	attritoX = 0.8;  // grande attrito sulla X (per evitare slittamento)
	attritoY = 1.0;  // attrito sulla y nullo
			
	// Nota: vel max = accMax*attritoZ / (1-attritoZ)
	grip = 0.45;
	
}

// funzione che disegna tutti i pezzi dell'elicottero
// (carlinga + eliche + finestre e base)
// (da invocarsi due volte: per l'elicottero, e per la sua ombra)
// (Se si utilizzano i colori viene assegnato un colore ad ogni componente
// altrimenti si utilizza una texture)
void drawElica(bool shadow)
{
	glPushMatrix();
	if (!shadow)
	{
		if (colori)
			glColor3f(.1, .1, .1);
		else
		{
			glColor3f(1, 1, 1); // metto il colore neutro (viene moltiplicato col colore texture, componente per componente)
			SetupTexture(elica.bbmin, elica.bbmax, PROPELLER);
		}
	}
	
	if (start) //Se l'elicottero è a terra non si muovono
		glRotatef(25 * mozzo, 0, 1, 0);
	
	elica.RenderNxF();
	glPopMatrix();
	
	glPushMatrix();
	if (!shadow)
	{
		if (colori)
			glColor3f(.1, .1, .1);
		else
		{
			glColor3f(1, 1, 1); // metto il colore neutro (viene moltiplicato col colore texture, componente per componente)
			SetupTexture(elicaPR.bbmin, elicaPR.bbmax, PROPELLER);
		}
	}
	if (start) //Le eliche si muovono solo dopo che parte l'elicottero
	{
		glTranslatef(0, +elicaPR.Center().Y(), +elicaPR.Center().Z());
		glRotatef(20 * mozzo, 1, 0, 0);
		glTranslatef(0, -elicaPR.Center().Y(), -elicaPR.Center().Z());
	}
	
	elicaPR.RenderNxF();
	glDisable(GL_TEXTURE_2D);
	
	glPopMatrix();
}

void drawInterni(bool shadow)
{
	if (shadow) //Se sto disegnando l'ombra non li ridisegno
		return;
	glDisable(GL_LIGHTING);
	
	glPushMatrix(); //Sedile interno all'elicottero
	glColor3f(.5, .5, .5);
	sedile.RenderNxV();
	glPopMatrix();
	
	glPushMatrix(); //Persona seduta nell'elicottero
	glEnable(GL_LIGHTING);
	man.RenderNxV();
	glPopMatrix();
	
	glPushMatrix(); //Un pannello di comandi interno all'elicottero
	command.RenderNxV();
	glPopMatrix();
	glEnable(GL_LIGHTING);
}

void drawFinestrini(bool shadow)
{
	if (shadow)
		return;
	glPushMatrix(); //Finestrino destro
	if (colori)
	{
		glColor3f(.9, .9, .9);
//			glEnable(GL_LIGHTING);
	} else
	{
		glEnable(GL_TEXTURE_2D);
		glColor3f(1, 1, 1); // metto il colore neutro (viene moltiplicato col colore texture, componente per componente)
		SetupTexture(finestrinoDx.bbmin, finestrinoDx.bbmax, WINDOW);
	}
	
	finestrinoDx.RenderNxV();
	glPopMatrix();
	
	glPushMatrix(); //Finestrino sinistro
	if (colori)
	{
		glColor3f(.9, .9, .9);
//			glEnable(GL_LIGHTING);
		
	} else
	{
		glEnable(GL_TEXTURE_2D);
		glColor3f(1, 1, 1); // metto il colore neutro (viene moltiplicato col colore texture, componente per componente)
		SetupTexture(finestrinoSx.bbmin, finestrinoSx.bbmax, WINDOW);
	}
	
	finestrinoSx.RenderNxV();
	glPopMatrix();
	
	glPushMatrix(); //Vetro anteriore
	glDisable(GL_TEXTURE_2D);
//		glEnable(GL_LIGHTING);
	glColor4f(.9, .9, .9, .4);
	
	glEnable(GL_BLEND); //Vetro trasparente
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	vetroAnteriore.RenderNxV();
	
	glPopMatrix();
	glDisable(GL_BLEND); //disabilito il blending
			
}

void drawCarlinga(bool shadow)
{
	
	glPushMatrix(); //Carlinga
	if (!shadow)
	{
		if (colori)
		{
			glColor3f(.9, .2, .0);
//			glEnable(GL_LIGHTING);
		} else
		{
			glColor3f(1, 1, 1); // metto il colore neutro (viene moltiplicato col colore texture, componente per componente)
			glEnable(GL_TEXTURE_2D);
			SetupTexture(carlinga.bbmin, carlinga.bbmax, HELICOPTER);
		}
	}
	carlinga.RenderNxV();
	glPopMatrix();
}

void drawBase(bool shadow)
{
	if (shadow)
		return;
	glPushMatrix(); //Disegna base
	glDisable(GL_TEXTURE_2D);
	glColor3f(0.4, 0.4, 0.4);
	base.RenderNxV();
	glPopMatrix();
}

void drawPorta(bool shadow)
{
	if (shadow)
		return;
	glPushMatrix(); //porta
	if (colori)
		glColor3f(.5, .5, .5);
	else
	{
		glEnable(GL_TEXTURE_2D);
		glColor3f(1, 1, 1); // metto il colore neutro (viene moltiplicato col colore texture, componente per componente)
		SetupTexture(porta.bbmin, porta.bbmax, DOOR);
	}
	porta.RenderNxV();
	
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	
}
//La variabile shadow serve per indicare se stiamo disegnando l'ombra o meno
void Helicopter::RenderAllParts(bool shadow) const
{
	
	// disegna l'elicottero con delle mesh
	glPushMatrix();
	// patch: riscaliamo le mesh di 1/10
	glScalef(-0.02, 0.02, -0.02);

//Costruisco l'elicottero composto da eliche, finestrini, base e carlinga
	drawElica(shadow);
	
	drawInterni(shadow);
	
	drawCarlinga(shadow);
	
	drawBase(shadow);
	
	drawFinestrini(shadow);
	
	drawPorta(shadow);
	
	glDisable(GL_TEXTURE_2D);
	
	glPopMatrix();
}

/*
 * disegna a schermo
 */
void Helicopter::Render() const
{
	// sono nello spazio mondo
	
	glPushMatrix();
	
	glTranslatef(px, py, pz);
	glRotatef(facing, 0, 1, 0);
	
	// sono nello spazio elicottero
	
	// disegna l'elicottero con delle mesh
	RenderAllParts(false);
	// ombra!
	
	if (useShadow && !night)
	{
		//Prima luce
		glPushMatrix();
		glColor3f(0.2, 0.2, 0.2);
		float e[] = { 0, (0.02 - py), 0 }; //posizione del piano dove far riflettere l'ombra
		float n[] = { 0, -1, 0 }; //normale del piano
		glShadowProjection(lightPosition, e, n);
		glDisable(GL_LIGHTING);
		RenderAllParts(true);
		glEnable(GL_LIGHTING);
		glPopMatrix();
		
	}
	glPopMatrix();
	
}
