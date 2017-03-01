/*
 * CLASSE CONTROLLER
 */
class Controller
{
public:
	enum
	{
		//Girare a sx e dx, in avanti ed indietro, prendere quota (su) e scendere di quota
		LEFT = 0,
		RIGHT = 1,
		ACC = 2,
		DEC = 3,
		UP = 4,
		DOWN = 5,
		NKEYS = 6
	};
	bool key[NKEYS];

	void Init();
	void EatKey(int keycode, int* keymap, bool pressed_or_released);
	Controller()
	{
		Init();
	} // costruttore
};

/*
 * CLASSE HELICOPTER
 */
class Helicopter
{
	//Disegna tutte le parti dell'elicottero
	void RenderAllParts(bool shadow) const;

	void CheckCollision(); //Gestisci le collisioni

public:
	// Metodi
	void Init(); // inizializza variabili
	void Render() const; // disegna a schermo
	void DoStep(); // computa un passo del motore fisico
	Helicopter()
	{
		Init();
	} // costruttore

	Controller controller;

	// STATO DELL'ELICOTTERO
	// (DoStep fa evolvere queste variabili nel tempo)
	float px, py, pz, facing; // posizione e orientamento
	float mozzoA, mozzoP, sterzo; // stato interno
	float vx, vy, vz; // velocita' attuale
	//float volante;

	// STATO DELL'ELICOTTERO
	float velSterzo, velRitornoSterzo, accMax, grip, attritoX, attritoY,
			attritoZ, velQuota;//velQuota = velocità di quota, velRotation = velocità con cui si muovono le eliche
};
