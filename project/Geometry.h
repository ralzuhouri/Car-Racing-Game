#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <vector>
#include <GL/glut.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include "Util.h"
#include <limits>
#include <sstream>

struct Color;
struct Vector;
struct Point;
struct Material;
class Face;
class Mesh;
class Box;
class Floor;

struct TexCoord {
	float s,t;
};

typedef TexCoord (*TextureMappingFunc) (Point);

enum AngleUnit {
	Radians, Degrees
};

// getRotationAngle: modifica il valore dell'angolo, renendolo un valore compreso nell' intervallo
// [0, 2 *pi] nel caso che il valore AngleUnit sia uguale a "Radians", oppure nell'intervallo [0,360]
// nel caso che venga passato un angolo in gradi (AngleUnit.Degrees)
float getRotationAngle(float, AngleUnit); 

// Applica la getRotationAngle a tutte le componenti di un vettore
Vector getRotationVector(const Vector&, AngleUnit);

// Applica la shadow projection, moltiplicando la matrice corrente per una matrice tale che ogni 
// oggetto disegnato successivamente sia proiettato sul terreno 
// 
// - lightPos: posizione della luce
// - surfPoint: un punto qualsiasi della superficie
// - normal: il vettore normale alla superficie
void shadowProjection(Point lightPos, Point surfPoint, Vector normal);

/********************************* Point ******************************************/

struct Point // Rappresenta un punto tridimensionale
{
	float x,y,z;

public:
	Point();
	Point(const Vector&);
	Point(float, float , float);
	void draw(); // chiama la glVertex - da usare in una primitiva
	Point operator+(const Vector&) const; // Somma punto-vettore
	Point operator-(const Vector&) const; // Sottrazione punto-vettore
	Point operator-() const;  // Ritorna il punto con le componenti invertite di segno
	bool operator==(const Point&) const; 
	void copyToBuffer(float*) const; // Copia il punto su un buffer di 3 float
};

/********************************* Color ******************************************/

struct Color // Struttura che rappresenta un colore con le componenti rgba
{
	float red, green, blue, alpha;

public:
	Color();
	Color(float, float, float, float);
	static Color random(); // Colore casuale
	void set();            // Imposta il colore usando la glColor
	void copyToBuffer(float*) const; // Copia delle componenti in un buffer

	Color operator*(float) const; // Moltiplicazione per un coefficiente
};

// Qua definisco i colori principali
static Color Red = Color(1.0, 0.0, 0.0, 1.0);
static Color Green = Color(0.0, 1.0, 0.0, 1.0);
static Color Blue = Color(0.0, 0.0, 1.0, 1.0);
static Color Yellow = Color(1.0, 1.0, 0.0, 1.0);
static Color Orange = Color(1.0, 0.5, 0.0, 1.0);
static Color White = Color(1.0, 1.0, 1.0, 1.0);
static Color Black = Color(0.0, 0.0, 0.0, 1.0);
static Color Gray = Color(0.5, 0.5, 0.5, 1.0);
static Color Metallic = Color(188.0 / 255.0, 198.0 / 255.0, 204.0 / 255.0, 1.0);
static Color Asphalt = Color(40 / 255.0, 43 / 255.0, 42 / 255.0, 1.0);
static Color Sand = Color(194 / 255.0, 178 / 255.0, 128 / 255.0, 1.0);
static Color Clear = Color();

/********************************* Vector ******************************************/

struct Vector // Struttura che rappresenta un vettore tridimensionale
{
	float dx, dy, dz; // Componenti del vettore
	Vector();
	Vector(const Point&);
	Vector(float, float, float);
	float norm() const; // Norma vettoriale
	Vector normalized() const; // Ritorna il vettore normalizzato
	static float dot(const Vector&, const Vector&); // Prodotto scalare
	static Vector cross(const Vector&, const Vector&); // Prodotto vettoriale
    // Qua definisco i principali operatori
	Vector operator*(float) const; 
	Vector operator+(const Vector&) const;
	Vector operator-(const Vector&) const;
	Vector operator-() const;
	Vector operator/(float) const;
	bool operator==(const Vector&) const;
	void copyToBuffer(float*) const; // Copia le componenti in un buffer
};

/********************************* Rect ******************************************/

// Struttura rettangolo (in realta' il nome non e' molto esplicativo, perche' un
// rettangolo e' tridimensionale, ma questa classe definisce un volume,
// ovvero uno spazio tridimensionale)
struct Rect
{
    // Le origini e le dimensioni definiscono il volume
	Point origin;
	Vector dimension;

	Rect();
	Rect(const Point&, const Vector&);
	Rect(const Point&, const Point&);

	Point getCenter() const; // Ritorna il centro del volume
};

/********************************* Line ******************************************/

// Definisce una linea che puo. essere disegnata utilizzando il metodo draw()
class Line
{
private:
    // Definendo i due punti che formato la linea ed un colore 
    // e' possibile disegnarla
	Point p1;
	Point p2;
	Color color;
public:
	Line(const Point&, const Point&);
	virtual void draw(); // Disegna la linea
	Color getColor() const; // Getter del colore
	void setColor(const Color&); // Setter del colore
};

/********************************* Material ******************************************/

// Struttura che definisce un materiale che puo' essere impostato con il metodo
// set(), che chiama la glMaterial
struct Material
{
private:
public:
	Color ambient;
	Color diffuse;
	Color specular;
	Color emission;
	float shininess;

	Material();
	void set(); // Imposta tutte le componenti del materiale
};

/********************************* Bar ******************************************/

// Barra dei frame al secondo
// E' un oggetto bidimensionale
// Disegno la barra utilizzando la proprieta' "percentage"
// Se la percentuale e' uguale a 100% la disegno tutta verde
// Se la percentuale e' inferiore al 100%, la parte inferiore sara' verde,
// quella superiore rossa.
// Esempio: percentage = 40%. 4/10 della barra sono verdi. Gli altri 6/10 sono rossi
class Bar
{
private:
	float percentage; // Percentuale di frame al secondo
	float x;
	float y;
	float dx;
	float dy;
public:
	float targetFPS;

	Bar(float, float, float, float);
	void draw(); // Disegna la barra

	float getPercentage() const;
	void setPercentage(float percentage);
};

/********************************* Circle ******************************************/
class Circle
{
public:
	bool stroke;
	float radius;
	float x,y;
	Color fillColor;
	Color strokeColor;

	Circle(float);

	void draw();
};


#endif

















