#ifndef LIGHT_H
#define LIGHT_H

#include <GL/glut.h>
#include "Geometry.h"
#include <iostream>
#include <vector>

class Light;
class SpotLight;

struct Light // Luce direzionale
{
private:
	static int numLights; // Questa variabile statica tiene traccia del numero totale di luci
						  // serve a poter assegnare un numero univoco ad ogni luce creata
	int number; // Numero della luce
public:
	// Componenti della luce e posizione
	Color ambient;
	Color diffuse;
	Color specular;
	Point position;

	Light();
	int getNumber() const; // Numero della luce (0~9 GL_LIGHT0 ~ GL_LIGHT9)
	virtual void enable(); // Abilita la luce
	virtual void disable(); // Disabilita la luce

	friend class SpotLight;
};

// Luce spot
struct SpotLight: public Light 
{
public:
	Vector direction; // in aggiunta ha anche la direzione
	// In realta' ci sono anche altri parametri come SPOT_CUTOFF, SPOT_EXPONENT, ecc...
	// Ma hanno un valore fisso e vengono impostati nella funzione enable()

	SpotLight();
	void enable(); // Abilito la luce spot
};


#endif
