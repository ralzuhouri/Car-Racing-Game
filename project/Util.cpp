#include "Util.h"

bool LoadTexture(int textbind,char *filename) {
	SDL_Surface *s = IMG_Load(filename);
	if (!s) return false;
  
	glBindTexture(GL_TEXTURE_2D, textbind);
	gluBuild2DMipmaps(
		GL_TEXTURE_2D, 
		GL_RGB,
    	s->w, s->h, 
    	GL_RGB,
    	GL_UNSIGNED_BYTE,
    	s->pixels
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	return true;
}

void SetupEnvmapTexture(int textbind)
{
	glBindTexture(GL_TEXTURE_2D, textbind);
   
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_GEN_S); // abilito la generazione automatica delle coord texture S e T
	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE , GL_SPHERE_MAP); // Env map
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE , GL_SPHERE_MAP);
	glColor3f(1,1,1); // metto il colore neutro (viene moltiplicato col colore texture, componente per componente)
	glDisable(GL_LIGHTING); // disabilito il lighting OpenGL standard (lo faccio con la texture)
}

void DisableEnvmapTexture() 
{
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
}

void SetupTexture(float minX, float minY, float minZ, float maxX, float maxY, float maxZ, int bindtext) {
	glBindTexture(GL_TEXTURE_2D, bindtext);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
  
  	// Utilizzo le coordinate OGGETTO
  	// cioe' le coordinate originali, PRIMA della moltiplicazione per la ModelView
  	// in modo che la texture sia "attaccata" all'oggetto, e non "proiettata" su esso
  	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE , GL_OBJECT_LINEAR);
  	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE , GL_OBJECT_LINEAR);
  	float sz=1.0/(maxZ - minZ);
  	float ty=1.0/(maxY - minY);
  	float s[4]={0,0,sz,  - minZ*sz };
  	float t[4]={0,ty,0,  - minY*ty };
  	glTexGenfv(GL_S, GL_OBJECT_PLANE, s); 
  	glTexGenfv(GL_T, GL_OBJECT_PLANE, t); 

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void DisableTexture() {
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
}

void drawSky(bool useTexture) {  // Disegna il cielo
	if(useTexture) {
	// E' una sfera al quale viene applicata la texture del cielo
		glBindTexture(GL_TEXTURE_2D, 2); // Bind della texture
		glEnable(GL_TEXTURE_2D); // Abilito le texture
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE , GL_SPHERE_MAP); 
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE , GL_SPHERE_MAP);
		glColor3f(1,1,1);
		glDisable(GL_LIGHTING);
	} else {
		glColor3f(0.5394, 0.8078, 0.9215);
	}

	glutSolidSphere(4000, 16, 16); // Disegno la sfera

	if(useTexture) {
		glDisable(GL_TEXTURE_GEN_S);
 		glDisable(GL_TEXTURE_GEN_T);
 		glDisable(GL_TEXTURE_2D);
 		glEnable(GL_LIGHTING);
	}
}

// Divide una stringa in un array di sottostringhe utilizzando un separatore
std::vector<std::string> split(std::string string, char separator) {
	std::string tmp;
	std::vector<std::string> components;
	for(std::string::iterator i = string.begin(), end = string.end(); i != end; ++i) {
		char c = *i;
		if(c != separator) {
			tmp += c;
		} else if (tmp.length() > 0) {
			components.push_back(tmp);
			tmp = "";
		}
	}

	if(tmp.length() > 0) components.push_back(tmp);
	return components;
}

/***************************************** Stopwatch *************************************/

Stopwatch::Stopwatch() {
	this->started = false;
	this->lastTime = 0.0;
}

bool Stopwatch::isStarted() const {
	return this->started;
}

float Stopwatch::getCurrentTime() const {
	int ms = glutGet(GLUT_ELAPSED_TIME);
	return ms / 1000.0;
}

float Stopwatch::lapse() {
	if(!started) { return 0.0; }

	float currentTime = this->getCurrentTime();
	float result = currentTime - lastTime;
	this->lastTime = currentTime;
	return result;
}

void Stopwatch::start() {
	this->started = true;
	lastTime = this->getCurrentTime();
}

void Stopwatch::stop() {
	this->started = false;
	lastTime = 0.0;
}

/***************************************** Timer *************************************/

Timer::Timer(TimerFunc func, float timeout, bool repeats) {
	this->func = func;
	this->timeout = timeout;
	this->repeats = repeats;
	this->elapsedTime = 0.0;
}

void Timer::schedule() {
	this->stopwatch.start();
}

void Timer::invalidate() {
	this->stopwatch.stop();
}

void Timer::update() {
	this->elapsedTime += this->stopwatch.lapse();
	if(this->elapsedTime >= this->timeout)
	{
		this->func(*this);
		if(this->repeats) this->elapsedTime = 0.0;
	}
}

/***************************************** Text *************************************/

Text::Text(std::string characters) {
	this->characters = characters;
	this->size = 2;
	this->x = 0.0;
	this->y = 0.0;
}

void Text::draw() {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 680, 0, 680, -1, 10);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glDisable(GL_CULL_FACE);
	glClear(GL_DEPTH_BUFFER_BIT);

	glColor3f(1,1,1);
	glRasterPos3f(x, y, 0);
	
	// Assegno il font da utilizzare a seconda della dimensione del
	// testo scelta.
	void* font = GLUT_BITMAP_HELVETICA_18;
	switch(this->size) {
	case 1: 
		font = GLUT_BITMAP_HELVETICA_10;
		break;
	case 2:
		font = GLUT_BITMAP_HELVETICA_18;
		break;
	case 3:
		font = GLUT_BITMAP_TIMES_ROMAN_24;
		break;
	}

	for(unsigned int i=0; i<characters.length(); i++)
		glutBitmapCharacter(font, characters[i]);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}





























