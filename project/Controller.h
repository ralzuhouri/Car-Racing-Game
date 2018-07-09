#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <vector>
#include <cstddef>
#include <iostream>

// La classe controller contiene un singleton in grado di memorizzare
// i comandi presi in input. Si puo' capire se un testo sta essendo premuto
// utilizzando il metodo isKeyPressed()
class Controller
{
private:
	std::vector<bool> keys; // vettore di booleani che specifica se un comando
						    // e' premuto per ogni carattere ASCII
	static Controller* controller; // Singleton
	Controller(); // Costruttore privato: per ottenere un'istanza si utilizza
				  // il metodo shared()
public:
	float joystickX;
	float joystickY;
	float joystickZ;
	bool isJoystickEnabled;
	bool isKeyUpPressed;
	bool isKeyDownPressed;
	bool isKeyRightPressed;
	bool isKeyLeftPressed;

	static Controller& shared(); // Ritorna il singleton

	void pressKey(unsigned char); // Premo un tasto
	void releaseKey(unsigned char); // Rilascio un tasto
	void releaseAllKeys(); // Rilascio tutti i tasti
	bool isKeyPressed(unsigned char); // Ritorna true se un tasto e' premuto
};

#endif
