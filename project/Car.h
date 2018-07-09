#ifndef CAR_H
#define CAR_H

#include "Mesh.h"
#include "Util.h"
#include <iostream>
#include <algorithm>
#include "Controller.h"
#include "Light.h"

// Oggetto contenete le mesh che servono a disegnare l'abitacolo con le ruote e i cerchioni,
// ed eventualmente anche l'ombra della macchina
class Car
{
private:
	SpotLight rightSpotLight; // Fari della macchina
	SpotLight leftSpotLight;

	// Ruote anteriori
	Mesh* frontRightWheel;
	Mesh* frontLeftWheel;
	// Ruote posteriori
	Mesh* backRightWheel;
	Mesh* backLeftWheel;
	// Cerchioni
	Mesh* frontRightWheelMetal;
	Mesh* frontLeftWheelMetal;
	Mesh* backRightWheelMetal;
	Mesh* backLeftWheelMetal;
	Mesh* body; // Chassis

	Point origin; // Origine - viene utilizzata per gli spostamenti
	Vector rotation; // Orientamento della macchina
	// Movimenti 
	bool isMovingForward;
	Vector velocity;
	Vector friction;
	float facing; // Orientamento espresso come angolo azimutale anziche' come vettore
	float grip;
	float steering;
	float frontWheelsRadius;
	float backWheelsRadius;
	// Lo stopwatch e' un cronometro che registra il tempo trascorso tra uno step fisico 
	// e l'altro
	Stopwatch stopwatch; 
	float deltaTime;
	void physicsStep();

public:
	bool castsShadow; // true: proietta l'ombra
	bool usesEnvMap;  // Uso una environment map per disegnare lo chassis
	bool drawAsWireframe; // Disegno la macchina come un wireframe
	bool onTurbo;

	Car(); // Costruttore - utilizza i file obj presenti nella directory

	Rect getBoundingBox() const;
	Point getOrigin() const;
	void setOrigin(const Point&);
	void setRotation(const Vector&);
	float getVelocity() const;
	void setVelocity(float);
	float getSteering() const;
	void setSteering(float);
	float getFacing() const;
	void setFacing(float);
	void draw(); // Disegna la macchina

	bool isMoving() const;
	void updatePhysics(); // Aggiorna la fisica
	void updateStopwatch();
};

#endif














