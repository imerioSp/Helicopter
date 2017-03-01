/*********************************************************/
/* CLASSE POINT3: un punto (o vettore) in 3 dimensioni   */
/* comprende le operazioni fra punti                     */
/*********************************************************/
class Point3
{
public:
	
	// l'UNICO campo: le coordinate x, y, z
	float coord[3];

	float X() const
	{
		return coord[0];
	}
	float Y() const
	{
		return coord[1];
	}
	float Z() const
	{
		return coord[2];
	}
	
	// costruttore 1
	Point3(float x, float y, float z)
	{
		coord[0] = x;
		coord[1] = y;
		coord[2] = z;
	}
	
	// costruttore senza parametri
	Point3()
	{
		coord[0] = 0;
		coord[1] = 0;
		coord[2] = 0;
	}
	
	// restituisce la versione di se stesso normalizzata
	Point3 Normalize() const
	{
		return (*this) / modulo();
	}
	
	// restituisce il modulo
	float modulo() const
	{
		return sqrt(
				coord[0] * coord[0] + coord[1] * coord[1] + coord[2] * coord[2]);
	}
	
	// overloading dell'operatore "/" binario: divisione per uno scalare (un float)
	Point3 operator/(float f) const
	{
		return Point3(coord[0] / f, coord[1] / f, coord[2] / f);
	}
	
	// overloading dell'operatore "-" unario: inversione del verso del vettore
	Point3 operator-() const
	{
		return Point3(-coord[0], -coord[1], -coord[2]);
	}
	
	// overloading dell'operatore "-" binario: differenza fra punti
	Point3 operator-(const Point3 &a) const
	{
		return Point3(coord[0] - a.coord[0], coord[1] - a.coord[1],
				coord[2] - a.coord[2]);
	}
	
	// overloading dell'operatore "+": somma fra vettori  
	Point3 operator+(const Point3 &a) const
	{
		return Point3(coord[0] + a.coord[0], coord[1] + a.coord[1],
				coord[2] + a.coord[2]);
	}
	
	// overloading dell'operatore binario "%" per fare il CROSS PRODUCT
	Point3 operator %(const Point3 &a) const
	{
		return Point3(coord[1] * a.coord[2] - coord[2] * a.coord[1],
				-(coord[0] * a.coord[2] - coord[2] * a.coord[0]),
				coord[0] * a.coord[1] - coord[1] * a.coord[0]);
	}
	
	// metodo per mandare il punto come vertice di OpenGl
	void SendAsVertex() const
	{
		glVertex3fv(coord);
	}
	
	// metodo per mandare il punto come normale di OpenGl
	void SendAsNormal() const
	{
		glNormal3fv(coord);
	}
	
	Point3 scala(const Point3 scala)
	{
		return Point3(coord[0] * scala.coord[0], coord[1] * scala.coord[1],
				coord[2] * scala.coord[2]);
	}
	
	Point3 trasla(const Point3 trasla)
	{
		return this->operator +(trasla);
	}
	
	Point3 rotateAsseY(float teta)
	{
		float x, y, z;
		x = coord[0] * cos(teta) - coord[2] * sin(teta);
		y = coord[1];
		z = coord[0] * sin(teta) + coord[2] * cos(teta);
		return Point3(x, y, z);
	}
	
	Point3 rotateAsseX(float teta)
	{
		float x, y, z;
		x = coord[1];
		y = coord[1] * cos(teta) + coord[2] * sin(teta);
		z = -coord[1] * sin(teta) + coord[2] * cos(teta);
		return Point3(x, y, z);
	}
	
	Point3 rotateAsseZ(float teta)
	{
		float x, y, z;
		x = coord[0] * cos(teta) + coord[1] * sin(teta);
		z = coord[2];
		y = -coord[0] * sin(teta) + coord[1] * cos(teta);
		return Point3(x, y, z);
	}
};

/******************************************************/
/* Esternamente alla classe Point3, definiamo Vector3 */
/* come SINONIMO di Point3, per chiarezza e per       */
/* poter distinguere nel codice fra punti e vettori   */
/******************************************************/

typedef Point3 Vector3;

