#include "Light.h"

int Light::numLights = 0;

/*********************************** Light ******************************************************/

Light::Light() {
	this->number = Light::numLights++;
	this->ambient = Color(0.1, 0.1, 0.1, 1.0);
	this->diffuse = White;
	this->specular = White;
	this->position = Point();
}

int Light::getNumber() const {
	return this->number;
}

void Light::enable() {
	glEnable(GL_LIGHT0 + this->number);

	float buffer[4] = {0, 0, 0, 1};
	this->position.copyToBuffer(buffer);
	glLightfv(GL_LIGHT0 + this->number, GL_POSITION, buffer);

	this->ambient.copyToBuffer(buffer);
	glLightfv(GL_LIGHT0 + this->number, GL_AMBIENT, buffer);

	this->diffuse.copyToBuffer(buffer);
	glLightfv(GL_LIGHT0 + this->number, GL_DIFFUSE, buffer);

	this->specular.copyToBuffer(buffer);
	glLightfv(GL_LIGHT0 + this->number, GL_SPECULAR, buffer);
}

void Light::disable() {
	glDisable(GL_LIGHT0 + this->number);
}

/*********************************** SpotLight ******************************************************/

SpotLight::SpotLight() {
	this->ambient = Color(0.1, 0.1, 0.0, 1.0);
	this->diffuse = Yellow;
	this->specular = Yellow;
	this->direction = Vector(0, 0, -1);
}

void SpotLight::enable() {
	Light::enable();

	float buffer[4] = {this->direction.dx, this->direction.dy, this->direction.dz, 1.0};
	glLightfv(GL_LIGHT0 + this->number, GL_SPOT_DIRECTION, buffer);

	// Utilizzo dei valori di default per gli altri parametri
	glLightf(GL_LIGHT0 + this->number, GL_QUADRATIC_ATTENUATION, 0.5);
	glLightf(GL_LIGHT0 + this->number, GL_SPOT_CUTOFF, 20.0);
	glLightf(GL_LIGHT0 + this->number, GL_SPOT_EXPONENT, 1.0);
}













