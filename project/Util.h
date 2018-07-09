#ifndef UTIL_H
#define UTIL_H

#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

bool LoadTexture(int textbind, char *filename); // Carico una texture
void SetupEnvmapTexture(int textbind); // Fa il setup d una environment map
void DisableEnvmapTexture(); // Disabilita l'environment mapping

// Fa il setup della texture specificando i punti che definiscono il volume nel quale
// la texture deve essere applicata  
void SetupTexture(float minX, float minY, float minZ, float maxX, float maxY, float maxZ, int bindtext);
void DisableTexture(); // Disabilita il texture mapping
void drawSky(bool useTexture = false); // Disegna il cielo in una sfera utilizzando una texture

// Divide una stringa in un array di sottostringhe utilizzando un separatore
std::vector<std::string> split(std::string string, char separator);

class Timer;

/***************************************** Stopwatch *************************************/
// E' un cronometro in grado di registrare gli intervalli di tempo trascorsi
// tra un lapse e l'altro
class Stopwatch
{
private:
	bool started;
	float lastTime;
public:
	Stopwatch();
	bool isStarted() const;
	float getCurrentTime() const;
	float lapse(); // Il metodo lapse() ritorna l'intervallo di tempo tempo trascorso
				   // dal lapse() precedente, o dall'avvio del cronometro nel caso
				   // che il metodo lapse() venga chiamato per la prima volta
	void start(); // Avvia il cronometro
	void stop();  // Ferma il cronometro
};

/***************************************** Timer *************************************/
// Chiama un callback ad intervalli di tempo costanti

typedef void (*TimerFunc) (const Timer&);

class Timer
{
private:
	Stopwatch stopwatch;
	float elapsedTime;
	float timeout;
	bool repeats;
	TimerFunc func;
public:
	// func: callback da chiamare ad ogni timeout
	// timeout: intervallo di tempo
	// repeats: se true continua a chiamare i callback ad ogni intervallo finche'
	// il timer non viene invalidato
	Timer(TimerFunc func, float timeout, bool repeats);
	void schedule(); // Fa partire il timer
	void invalidate(); // Invalida il timer
	void update(); // E' necessario chiamare la update nel main loop per far 
				   // partire i callback
};

/***************************************** Text *************************************/


// Testo bidimensionale che puo' essere disegnato con il metodo draw()
class Text
{
public:
	std::string characters;
	float x,y;
	int size; // Dimensione del testo
			  // Calori validi: (1) piccolo, (2) medio, (3) grande
			  // Ogni altro valore verra' ignorato (di default il testo
			  // viene disegnato con caraterri di dimensiona media)

	Text(std::string);
	void draw();	
};


#endif










