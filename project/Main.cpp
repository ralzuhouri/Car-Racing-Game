#include <GL/glew.h>
#include <GL/glut.h>
#include <cstdlib>
#include <cstdio>
#include "Car.h"
#include <iostream>
#include "Controller.h"
#include "Light.h"
#include "ControlPanel.h"

/*  Progetto Car per il corso di grafica computerizzata 
	Studente: Ramy Al Zuhouri */

/************************* COMANDI *******************************************************

- Tasto 'esc': mette il gioco in pausa e apre il pannello di controllo
- Tasti 1~6: cambia la camera (1) frontale, (2) laterale - destra, (3) laterale - sinistra,
                              (4) dall'alto, (5) frontale - bassa, (6) posteriore
- Tasto 'j': abilita/disabilita il joystick 
- Asse x del joystick: gestisce lo sterzo
- Asse y del joystick: gestisce l'acceleratore
- Tasto 'o': abilita/disabilita le ombre
- Tasto 'k': abilita/disabilita l'environment map della macchina
- Tasto 'l': abilita/disabilita la visualizzazione come wireframe
- Tasto 'p': abilita/disabilita il pilota automatico
- Tasto 'w': accelera
- Tasto 's': decelera
- Tasto 'd': sterza a destra
- Tasto 'a': sterza a sinistra
- Tasti direzionali: possono essere usati in alternativa a w/a/s/d per accelerare/sterzare
- Rotellina del mouse: controlla lo zoom

****************************************************************************************/

// Un punteggio e' rappresentato da una coppia nome-tempo di completamento
struct Score {
public:
	unsigned int time;
	std::string name;
	Score(std::string name, unsigned int time) {
		this->name = name;
		this->time = time;
	}
};

Car* car = NULL;    // Macchina
Point startPoint = Point(-676.44, 3.0, 720.754);
Mesh* track = NULL; // Pista
Mesh* ground = NULL;  // Terreno
Mesh* sphere = NULL;
Timer* fpsTimer = NULL;  // Timer che serve a monitorare i frame al secondo
Timer* stepTimer = NULL; // Timer per calcolare la fisica
Light* sun = NULL;  // Luce 
Bar* bar = NULL;    // Barra che mostra i frame al secondo
int fps = 0;        // Numero di frame
float cameraDistance = 120;  // Distanza macchina-camera
int camera = 1;              // Tipo di camera (valori validi: 1~6)
bool autopilot = false;      // Booleano - true se il pilota automatico e' attivo
Text* autopilotText;         // Testo da mostrare quando il pilota automatico e' attivo
Text* gameStateText;         // Testo che mostra il numero di giri compiuti
Text* timeText;    	         // Testo che mostra il tempo di gioco
unsigned int seconds = 0;    // Tempo di gioco
bool useTextureToDrawSky = false;  // true: disegno il cielo utilizzando la texture.
							       // altrimenti utilizzo un colore
float updateFrequency = 100.0;     // Frequenza con la quale viene calcolata la fisica 
int wsize = 680;  			       // Dim. finestra
int windowId;			           // Id della finestra
// I checkpoint servono a calcolare che l'utente segua il percorso senza "barare" tagliando
// Si controlla che ad ogni giro vengano raggiunti tutti i checpoint
std::vector<Point> checkpoints;  
unsigned int currentCheckpoint = 0; // Ultimo checkpoint raggiunto

// Gli spawn points sono i punti dove le sfere possono apparire
// Ogni volta che una sfera viene generata, viene posizionata in uno spawn point
// casuale, al quale viene aggiunto un offset generato casualmente
std::vector<Point> spawnPoints; 
unsigned int currentSpawnPoint;
bool sphereSpawned = false; // Se true e' presente una sfera nella scena
unsigned int spawnTime = 0; // Questa variabile tiene conto del tempo di permanenza della
							// sfera nella scena
unsigned int turbo = 0;     // Numero di sfere collezionate
unsigned int turboTime = 0; // Quando si utilizza il turbo, viene conteggiato il tempo
Circle* circle = NULL;      // Cerchio che viene utlizzato per disegnare l'indicatore
					        // delle sfere in basso al centro

Mesh* map = NULL;  // Mesh 2D della mappa
Circle* mapDot = NULL;  // Puntino sulla mappa

bool gameOver = false;
bool gamePaused = false;
// Pannello di controllo e scoreboard
ControlPanel* controlPanel = NULL; 
ControlPanel* scoreboard = NULL;
std::vector<Score> scores; // Punteggi (nome - tempo)
std::string playerName = "";
bool insertingName = false;
int cpNavigationLevel = 0; // Livello di navigazione
					       // 0: pannello di controllo principale
						   // 1: scoreboard

// Forward declaration di alcune funzioni che verranno chiamate prima 
// della loro definizione
void setSphereTexCoords(GLfloat px, GLfloat py, GLfloat pz, GLfloat* s, GLfloat* t);
void spawnSphere();
void saveScores();
void insertScore(Score newScore);
void populateScoreboard();
void clearScoreboard();

// Formatta il tempo in formato hms
std::string formatTime(unsigned int sec) {
	unsigned int s = sec % 60;
	unsigned int m = (sec / 60) % 60;
	unsigned int h = sec / 3600;

	char buffer[64];
	if(h) {
		sprintf(buffer, "%uh%um%us", h, m, s);
	} else if(m) {
		sprintf(buffer, "%um%us", m, s);
	} else {
		sprintf(buffer, "%us", s);	
	}

	return std::string(buffer);
}

// Callback dei pulsanti del pannello di controllo
void CPButtonPressed(unsigned int index, ControlPanelButton* button) {

	if(cpNavigationLevel == 1) {
		clearScoreboard();
		cpNavigationLevel = 0;
		return;
	}

	if(index == 0) {
		// Quit
		glutDestroyWindow(windowId);
		exit(0);
	} else if(index == 1) {
		// Restart
		seconds = 0;
		currentCheckpoint = 0;
		turbo = 0;
		turboTime = 0;

		controlPanel->getItem(7)->state = 0;
		gameStateText->characters = "Giri: 0";

		car->setOrigin(startPoint);
		car->setFacing(90); 
		car->setRotation(Vector(0,90,0));
		car->setVelocity(0);

		if(insertingName) {
			controlPanel->popItem();
			controlPanel->popItem();
			controlPanel->popButton();
		} else if(controlPanel->numberOfItems() == 10) {
			controlPanel->popItem();
			controlPanel->popItem();
		}
  
		spawnSphere();
		gamePaused = false;
		gameOver = false;
		insertingName = false;
	} else if(index == 2) {
		// Scoreboard
		populateScoreboard();
		cpNavigationLevel = 1;
	} else if(index == 3 && insertingName) {
		// Salvataggio punteggio
		if(playerName.length() == 0) return;
		Score score = Score(playerName, seconds);
		insertScore(score);
		saveScores();
		insertingName = false;

		controlPanel->popButton();
		
		populateScoreboard();
		cpNavigationLevel = 1; // Mostro la scoreboard
	}
}

// Controlla le collisioni tra le sfere e la macchina
// Versione semplificata: fa un semplice controllo sulla distanza auto-sfera
void checkCollision() {

	if(!sphereSpawned) return;
	GLfloat distance = Vector(car->getOrigin() - sphere->translation).norm();

	if(distance < 30.0) {
		//std::cout << "Sphere caught!" << std::endl;
		sphereSpawned = false;

		turbo++;
		if(turbo > 3) turbo = 3;
		if(seconds >= 15) seconds -= 15;
		else seconds = 0;
	}
}

// Fa apparire una sfera in un punto casuale, applicando un offset
void spawnSphere() {
	sphereSpawned = true;
	spawnTime = 0;
	
	// Lo spawn point della sfera dipende dall'ultimo checkpoint raggiunto dall'auto, onde
	// evitare che la sfera compaia in una posizione tale che e' necessario tornare indietro
	unsigned int index = float(currentCheckpoint + 2) / checkpoints.size() * spawnPoints.size();
	index += rand() % 8; // Punto casuale
	Point point = spawnPoints[index % spawnPoints.size()];

	// Offset casuale
	int dx = rand() % 100 - 50; 
	int dz = rand() % 100 - 50;

	sphere->translation.dx = point.x + dx;
	sphere->translation.dz = point.z + dz;

	// Rotazione casuale
	int rotationY = rand() % 45 - 22;
	int rotationX = rand() % 45 - 22;

	sphere->rotation.dy = rotationY;
	sphere->rotation.dx = rotationX;
}

// Salva i punteggi, ovvero delle coppie nome-tempo
void saveScores() {
	std::ofstream file;
	file.open("Scores.txt");
	for(unsigned int i=0; i<scores.size(); i++) {
		Score score = scores[i];
		file << score.name << " " << score.time << "\n";	
	}
	file.close();
}

// Inserisce il punteggio nella giusta posizione del vettore (i punteggi
// sono ordinati per tempo, in ordine crescente)
void insertScore(Score newScore) {
	std::vector<Score> newScores;
	bool inserted = false;

	for(unsigned int i = 0; i < scores.size(); i++) {
		Score score = scores[i];
		if(newScore.time < score.time && !inserted) {
			newScores.push_back(newScore);
			inserted = true;
		}
		newScores.push_back(score);
	}

	if(newScores.size() == scores.size()) 
		newScores.push_back(newScore);

	scores = newScores;

	if(scores.size() > 10) {
		scores.pop_back();
	}
}

// Carica i punteggi dal file Scores.txt
std::vector<Score> loadScores() {
	std::vector<Score> scores;
	std::ifstream file("Scores.txt");
	std::string line;

	if(file.is_open()) {
		while(std::getline(file, line)) {
			std::vector<std::string> components = split(line, ' ');
			std::string name = components[0];
			unsigned int time = atol(components[1].c_str());
			scores.push_back(Score(name,time));
		}
		file.close();
	}

	return scores;
}

// Carica dei punti da un file
// Puo' essere usata per caricare i checkpoint e gli spawn point delle sfere
std::vector<Point> loadPoints(std::string filename) {
	std::vector<Point> points;
	std::ifstream file(filename.c_str());
	std::string line;

	if(file.is_open()) {
		while(std::getline(file, line)) {
			std::vector<std::string> components = split(line, ' ');
			GLfloat x = atof(components[0].c_str());
			GLfloat y = atof(components[1].c_str());
			GLfloat z = atof(components[2].c_str());
			points.push_back(Point(x,y,z));
		}
		file.close();
	}

	return points;
}

// Popola la scoreboard
void populateScoreboard() {
	scoreboard->pushItem("Migliori tempi:", 1);
	if(!scores.size()) scoreboard->pushItem("Nessun tempo salvato", 1);

	for(unsigned int i = 0; i < scores.size(); i++) {
		Score score = scores[i];
		std::string scoreText = score.name + " : " + formatTime(score.time);
		scoreboard->pushItem(scoreText, 1);
	}
}

// Cancella tutti gli item dalla scoreboard
void clearScoreboard() {
	while(scoreboard->numberOfItems()) 
		scoreboard->popItem();
}

// Aggiorna il checkpoint nel caso che l'auto abbia raggiunto
// il checkpoint successivo
void updateCurrentCheckpoint() {
	unsigned int index = (currentCheckpoint + 1) % checkpoints.size();
	Point next = checkpoints[index];
	Point position = car->getOrigin();

	float distance = Vector(next - Vector(position)).norm();
	if(distance < 200.0) {
		currentCheckpoint++;
		char buffer[32];
		unsigned int laps = currentCheckpoint / checkpoints.size();
		sprintf(buffer, "%u", laps);
		gameStateText->characters = "Giri: " + std::string(buffer);

		controlPanel->getItem(7)->state = laps;

		if(laps == 3) {
			gamePaused = true;
			gameOver = true;
			insertingName = true;
			cpNavigationLevel = 0;

			std::string time = formatTime(seconds);
			controlPanel->pushItem("Tempo di completamento: " + time, 1);
			controlPanel->pushItem("Nome giocatore: '" + playerName + "'", 1);
			controlPanel->pushButton("Salva");
		}
	}
}

// Questa funzione viene chiamata ogni secondo
// Aggiorna la barra dei frame al secondo
void fpsTimerFunc(const Timer& timer) 
{
	bar->setPercentage(fps / updateFrequency);
	//std::cout << "FPS: " << fps << std::endl;
	fps = 0;

	if(gamePaused) {
		glutPostRedisplay();
		return;
	}

	seconds++;

	timeText->characters = "Tempo: " + formatTime(seconds);
	timeText->x = 670 - timeText->characters.size() * 10;

	if(!sphereSpawned) {
		int random = rand() % 15;
		if(!random) {
			spawnSphere();
		}
	} else {
		spawnTime++;

		if(spawnTime > 30) {
			sphereSpawned = false;
		}
	}

	if(car->onTurbo) {
		turboTime++;
		if(turboTime == 10) {
			car->onTurbo = false;
		}
	}
}


// Questa funzione viene chiamata ad ogni passo di fisica
// Si e' deciso di calcolare la fisica ogni 1/100 di secondo
void stepTimerFunc(const Timer& timer) 
{
	if(gamePaused) {
		car->updateStopwatch();
		glutPostRedisplay();
		return;
	}

	float distance = Vector(car->getOrigin() - Point()).norm();
	if(distance > 4000.0) { 
		// Se esco dalla mappa riposiziono la macchina nel suo punto di partenza
		car->setOrigin(startPoint);
		car->setVelocity(0);
		car->setFacing(90);
		car->setRotation(Vector(0,90,0));
		seconds += 10;
	}

	if(autopilot) 
	{
		// Se il pilota automatico e' attivo, 'gioco' con l'acceleratore, premendolo e
        // rilasciandolo per tentare di mantenere una velocita' pari a 75
		float speed = car->getVelocity();
		if(speed > 150.0) {
			Controller::shared().releaseKey('w');
		} else if (speed < 150.0) {
			Controller::shared().pressKey('w');
		}
	}

	car->updatePhysics(); // Aggiorna la posizione, la velocita', direzione e sterzo della macchina
	checkCollision();
	updateCurrentCheckpoint();
	glutPostRedisplay();
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor (1.0, 1.0, 1,0);

	glViewport(0,0,wsize,wsize);
	glCullFace(GL_BACK);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity ();
	// Proiezione prospettica con zFar = 8000 (la lunghezza della mappa)
	gluPerspective(60.0, 1.0, 1.0, 8000.0);

	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	glShadeModel(GL_SMOOTH);
	
	Point origin = car->getOrigin();
	Vector viewup = Vector(0,1,0);
	float angle = M_PI / 6.0; // Questo e' l'angolo tra il vettore maccina-camera e l'orizzonte
                              // Se e' uguale a 90 gradi la camera e' esattamente sopra alla macchina
	float facing = car->getFacing() / 180.0 * M_PI;

	switch(camera) {
		// Sistemo l'angolo a seconda del tipo di camera scelto
		case 2:
			facing += M_PI / 6.0;
			break;
		case 3:
			facing -= M_PI / 6.0;
			break;
		case 4:
			angle += M_PI / 6.0;
			break;
		case 5:
			angle -= M_PI / 12.0;
			break;
		case 6:
			facing -= M_PI;
			break;
		default:
			break;
	} 

	// Calcolo la posizione dell'osservatore in base alla direzione della macchina e all'angolo scelto
	Point eye = origin + Vector(cameraDistance * sin(facing) * cos(angle), cameraDistance * sin(angle), cameraDistance * cos(facing) * cos(angle));
	gluLookAt(eye.x, eye.y, eye.z, origin.x, origin.y, origin.z, viewup.dx, viewup.dy, viewup.dz);

	sun->enable(); // Attiva la luce principale della scena
	drawSky(useTextureToDrawSky);     // Disegna il cielo (una sfera con una environment map)

	if(!gamePaused) car->draw();  

	if(sphereSpawned) sphere->draw();
	track->draw();  // Disegna la pista (utilizzando un modello obj)
	ground->draw(); // Disegna il terreno
	sun->disable();	

	// Disabilito l'illuminazione e disegno oggetti bidimensionali:
    // (a) la barra dei frame al secondo
    // (b) la scritta del pilota automatico (nel caso che sia abilitato)
	glDisable(GL_LIGHTING);
	bar->draw();
	
	// Disegno gli indicatori delle sfere
	if(turbo == 1) {
		circle->x = wsize / 2.0;
		circle->strokeColor = Color(0.4, 0.4, 0.4, 1.0);
		circle->draw();
	} else if(turbo == 2) {
		circle->x = wsize / 2.0 - 15.0;
		circle->strokeColor = Color(0.4, 0.4, 0.4, 1.0);
		circle->draw();
		circle->x = wsize / 2.0 + 15.0;
		circle->draw();
	} else if(turbo == 3) {
		circle->x = wsize / 2.0;
		circle->strokeColor = Color(1.0, 0.22, 0.0, 1.0);
		circle->draw();

		circle->x = wsize / 2.0 - 30.0;
		circle->draw();
		circle->x = wsize / 2.0 + 30.0;
		circle->draw();
	}

	if(autopilot && seconds % 2) {
		autopilotText->draw();
	}

	gameStateText->draw();
	timeText->draw();
	map->draw2D();

	// Calcolo la posizione sulla mappa
	GLfloat x = 10.0 + 50.0 - 100.0 / 8000.0 * car->getOrigin().z;
	GLfloat y = 10.0 + 50.0 - 100.0 / 8000.0 * car->getOrigin().x;
	mapDot->x = x;
	mapDot->y = y;
	mapDot->draw();

	if(gamePaused) {
		switch(cpNavigationLevel) {
			case 0:
				controlPanel->draw();
				break;
			case 1:
				scoreboard->draw();
				break;
		}
	}

    glutSwapBuffers();

	fps += 1;
}

void mouse(int btn, int state, int x, int y)
{
	// Gestisco i click
	if(btn == GLUT_LEFT_BUTTON) {
		if(state == GLUT_DOWN) {
			// Mouse down
			if(cpNavigationLevel == 0)
				controlPanel->mouseDown(x,wsize - y);
			else if(cpNavigationLevel == 1)
				scoreboard->mouseDown(x, wsize - y);
		} else if(state == GLUT_UP) {
			// Mouse up
			if(cpNavigationLevel == 0)
				controlPanel->mouseUp(x,wsize - y);
			else if(cpNavigationLevel == 1)
				scoreboard->mouseUp(x, wsize - y);
		}	
	}

	// Gestisco lo zoom con la rotellina del mouse
	if(btn == 3) {
		cameraDistance *= 0.9;
		cameraDistance = std::max(cameraDistance, 40.0f);
	} else if(btn == 4) {
		cameraDistance *= 1.1;
		cameraDistance = std::min(cameraDistance, 1000.0f);
	}
}

// Mouse in movimento
void motion(int x, int y) {
	if(cpNavigationLevel == 0)
		controlPanel->mouseMove(x, wsize - y);
	else if(cpNavigationLevel == 1)
		scoreboard->mouseMove(x, wsize - y);
}

void keydown(unsigned char key, int x, int y) 
{	
	if(insertingName) { // Se l'utente sta inserendo il nome a fine gioco...
		bool numeric = key >= '0' && key <= '9';
		bool character = key >= 'A' && key <= 'z';
		bool backspace = key == 8;
		if(numeric || character) {
			if(playerName.length() < 15)
				playerName = playerName + std::string(1,key);
		}
		else if(backspace && playerName.size())
			playerName.erase(playerName.size() - 1);
		std::string text = "Nome giocatore: '" + playerName + "'";
		controlPanel->getItem(controlPanel->numberOfItems() - 1)->setText(text, 1);
		return;
	}

	switch(key) {
		// Con i tasti 1~6 cambio il tipo di camera 
        // L'opportuna proiezione viene fatta nella funzione display
        // Leggendo la variabile 'camera'
		case '1': case '2': case '3': case '4': case '5': case '6':
			camera = key - '0';
			controlPanel->getItem(0)->state = camera;
			break;
		case ' ':
			if(turbo == 3 && !car->onTurbo) {
				turbo = 0;
				car->onTurbo = true;
				turboTime = 0;
			}
			break;
		case 'o':
  			// Abilito/disabilito l'ombra utilizzando la proprieta' Car::castsShadow,
            // che se settata a true proietta l'ombra
			car->castsShadow = !car->castsShadow;
			controlPanel->getItem(2)->state = car->castsShadow ? 1 : 0;
			break;
		case 'p':
			// Abilito/disabilito il pilota automatico
			// Se la variabile 'autopilot' e' settata a true, nella funzione stepTimerFunc -
  			// ovvero dove la fisica viene aggiornata - vengono dati gli opportuni comandi
			// per controllare la macchina in maniera automatica
			autopilot = !autopilot;
			Controller::shared().releaseAllKeys();
			controlPanel->getItem(5)->state = autopilot ? 1 : 0;
			break;
		case 'j':
			// Abilito/disabilito il joystick
			Controller::shared().isJoystickEnabled = !Controller::shared().isJoystickEnabled;
			controlPanel->getItem(1)->state = Controller::shared().isJoystickEnabled ? 1 : 0;
			break;
		case 'k':
			// Abilito/disabilito l'environment map della macchina
			car->usesEnvMap = !car->usesEnvMap;
			controlPanel->getItem(3)->state = car->usesEnvMap ? 1 : 0;
			break;
		case 'l':
			// Abilita/disabilita la visualizzazione come wireframe
			car->drawAsWireframe = !car->drawAsWireframe;
			controlPanel->getItem(4)->state = car->drawAsWireframe ? 1 : 0;
			break;
		case 't':
			useTextureToDrawSky = !useTextureToDrawSky;
			controlPanel->getItem(6)->state = useTextureToDrawSky ? 1 : 0;
			break;
		case 27:
			gamePaused = !gamePaused || gameOver;
			//controlPanel->resetEvents();
			break;
		default:
			// I comandi vengono registrati solamente se il pilota automatico e' disabilitato
			if(!autopilot) Controller::shared().pressKey(key);
			break;
	}
}

void keyup(unsigned char key, int x, int y)  {
	Controller::shared().releaseKey(key);
}

// Questa funzione viene  chiamata quando l'utente preme un tasto direzionale
void special(int key, int x, int y) {
	switch(key) {
		case GLUT_KEY_UP:
			Controller::shared().isKeyUpPressed = true;
			break;
		case GLUT_KEY_DOWN:
			Controller::shared().isKeyDownPressed = true;
			break;
		case GLUT_KEY_RIGHT:
			Controller::shared().isKeyRightPressed = true;
			break;
		case GLUT_KEY_LEFT:
			Controller::shared().isKeyLeftPressed = true;
			break;
		default:
			break;
	}
}

// Questa funzione viene  chiamata quando l'utente rilascia un tasto direzionale
void specialUp(int key, int x, int y) {
	switch(key) {
		case GLUT_KEY_UP:
			Controller::shared().isKeyUpPressed = false;
			break;
		case GLUT_KEY_DOWN:
			Controller::shared().isKeyDownPressed = false;
			break;
		case GLUT_KEY_RIGHT:
			Controller::shared().isKeyRightPressed = false;
			break;
		case GLUT_KEY_LEFT:
			Controller::shared().isKeyLeftPressed = false;
			break;
	}
}

void joystick(unsigned int buttonMask, int x, int y, int z) {
	// Registro i comandi dati dal joystick 
	Controller::shared().joystickX = x / 1000.0;
	Controller::shared().joystickY = y / 1000.0;
	Controller::shared().joystickZ = z / 1000.0;
}

void idle()
{
	// Aggiorno i timer che chiamano il callback per aggiornare i frame al secondo
	// e la funzione che aggiorna la fisica della macchina quando raggiungo il limite
	// di tempo imposto (1/100 di secondo per la fisica, 1 secondo per il timer
	// dei frame al secondo).
	fpsTimer->update();
	stepTimer->update();
}

void setSphereTexCoords(GLfloat px, GLfloat py, GLfloat pz, GLfloat* s, GLfloat* t) {
	GLfloat width = sphere->boundingBox.dimension.dx * 1.1;
	GLfloat height = sphere->boundingBox.dimension.dy * 1.1;
	*s = 0.5 - px / width;
	*t = 0.5 - py / height;
}

void setTrackTexCoords(GLfloat px, GLfloat py, GLfloat pz, GLfloat* s, GLfloat* t) {
	*s = px * 2.0;
	*t = pz * 2.0;
}

// Qua inizializzo gli oggetti della scena
void init()
{
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);

	car = new Car(); // Oggetto macchina
	// La posiziono al centro della pista con il giusto orientamento
	car->setOrigin(startPoint);
	car->setFacing(90); 
	car->setRotation(Vector(0,90,0));

	// Timer che viene chiamato ad ogni secondo
	// Mostra i frame al secondo
	fpsTimer = new Timer(fpsTimerFunc, 1.0, true);
	fpsTimer->schedule();

	// Timer che aggiorna la fisica ogni centesimo di secondo
	stepTimer = new Timer(stepTimerFunc, 1.0 / updateFrequency, true);
	stepTimer->schedule(); 

	// Barra dei frame al secondo
	// Mostra i frame al secondo ed una barra colorata di verde/rosso
	// La parte verde e' tanto piu' alta quanti sono i frame al secondo
	// Esempio: se ho 10 frame al secondo, il 10% della barra e' colorata di verde, il 90% di rosso 
	bar = new Bar(620, 15, 50, 120);
	bar->targetFPS = updateFrequency;

	// Luce principale della scena
	sun = new Light();
	sun->ambient = Color(0.1, 0.1, 0.1, 1.0);
	sun->diffuse = Color(0.25, 0.25, 0.25, 1.0);
	sun->specular = White;
	sun->position = Point(0, 10000, 0);

	// Carico la texture del cielo
	char texture[] = "sky_ok.jpg";
	LoadTexture(2, texture);

	// Mesh contenente la pista
	track = new Mesh("Pista.obj", Recenter);
	char trackTexture[] = "Asphalt.jpg";
	LoadTexture(4, trackTexture);
	track->texture = 4;
	track->mode = DrawWithTexture;
	track->setTexCoordData(setTrackTexCoords);
	track->scale = Vector(600,1,600);
	track->translation = Vector(0,1,0);
	track->deleteData();

	// Terreno
	ground = Mesh::plane(8000, 8000);
	char sandTexture[] = "Sand.jpg";
	LoadTexture(3, sandTexture);
	GLfloat maxTexCoord = 12.0;
	GLfloat texCoords[8] = {0.0, 0.0, maxTexCoord, 0.0,  maxTexCoord, maxTexCoord,  0.0, maxTexCoord};
	ground->setTexCoordData(texCoords, 8);
	ground->texture = 3;
	ground->mode = DrawWithTexture;

	// Testo che viene mostrato quando il pilota automatico e' abilitato
	autopilotText = new Text("Pilota Automatico");
	autopilotText->x = wsize / 2.0 - 90.0;
	autopilotText->y = wsize / 2.0 + 150.0;
	autopilotText->size = 3;

	gameStateText = new Text("Giri: 0");
	gameStateText->x = 10;
	gameStateText->y = 660;
	gameStateText->size = 2;

	timeText = new Text("Tempo: 0s");
	timeText->y = 660;
	timeText->x = 670 - timeText->characters.size() * 10;
	timeText->size = 2;

	// Sfera
	Material sphereMat;
	Color sphereColor = Color(0.6, 0.8, 1.0, 1.0);
	sphereMat.ambient = sphereColor;
	sphereMat.diffuse = sphereColor;
	sphereMat.specular = White;
	sphereMat.emission = sphereColor;
	sphereMat.shininess = 10;
	char photoTexture[] = "FotoPersonale.jpg";
	LoadTexture(5, photoTexture);
	sphere = new Mesh("Sphere.obj", Recenter);
	GLfloat scale = 20.0 / sphere->boundingBox.dimension.dz;
	sphere->scale = Vector(scale,scale,scale);
	sphere->computeNormals();
	sphere->translation.dy = sphere->boundingBox.dimension.dy * sphere->scale.dy / 2.0;
	sphere->texture = 5;
	sphere->material = sphereMat;
	sphere->mode = DrawWithTextureAndMaterial;
	sphere->setTexCoordData(setSphereTexCoords);

	// Indicatore sfera
	circle = new Circle(12);
	circle->x = wsize / 2.0;
	circle->y = 18;
	circle->fillColor = sphereColor;
	
	// Mappa
	map = new Mesh();
	map->translation = Vector(10,10,0);
	GLfloat mapWidth = 100.0, mapHeight = 100.0;
	GLfloat mapVertices[12] = {0.0,0.0,0.0,   mapWidth, 0.0, 0.0,   mapWidth, mapHeight, 0.0,   0.0, mapHeight, 0.0};
	map->setVertexData(mapVertices, 12);
	GLuint mapIndices[6] = {0,1,2,  2,3,0};
	map->setIndexData(mapIndices, 6);
	char mapTexture[] = "Map.jpg";
	LoadTexture(6, mapTexture);
	map->texture = 6;
	map->mode = DrawWithTexture;
	GLfloat mapTexCoords[8] = {0.0, 1.0, 1.0, 1.0,  1.0, 0.0,  0.0, 0.0};
	map->setTexCoordData(mapTexCoords, 8);

	mapDot = new Circle(3.0);
	mapDot->fillColor = Red;
	mapDot->stroke = false;

	checkpoints = loadPoints("Checkpoints.txt");
	spawnPoints = loadPoints("SpawnPoints.txt");

	scores = loadScores();

	// Pannello di controllo
	controlPanel = new ControlPanel(wsize * 0.8, wsize * 0.8);
	controlPanel->callback = CPButtonPressed;
	controlPanel->horizontalOffset = wsize * 0.1;
	controlPanel->verticalOffset = wsize * 0.1;
	controlPanel->pushItem("Camera: frontale (premere tasti 1-6 per cambiare)", 1);
	controlPanel->getItem(0)->setText("Camera: laterale destra (premere tasti 1-6 per cambiare)", 2);
	controlPanel->getItem(0)->setText("Camera: laterale sinistra (premere tasti 1-6 per cambiare)", 3);
	controlPanel->getItem(0)->setText("Camera: panoramica (premere tasti 1-6 per cambiare)", 4);
	controlPanel->getItem(0)->setText("Camera: orizzontale (premere tasti 1-6 per cambiare)", 5);
	controlPanel->getItem(0)->setText("Camera: posteriore (premere tasti 1-6 per cambiare)", 6);

	controlPanel->pushItem("Joystick: off (premere 'j' per abilitare)", 0);
	controlPanel->getItem(1)->setText("Joystick: on (premere 'j' per disabilitare)", 1);

	controlPanel->pushItem("Ombre: off (premere 'o' per abilitare)", 0);
	controlPanel->getItem(2)->setText("Ombre: on (premere 'o' per disabilitare)", 1);

	controlPanel->pushItem("Environment map auto: on (premere 'k' per disabilitare)", 1);
	controlPanel->getItem(3)->setText("Environment map auto: off (premere 'k' per abilitare)", 0);

	controlPanel->pushItem("Visualizzazione wireframe: off (premere 'l' per abilitare)", 0);
	controlPanel->getItem(4)->setText("Visualizzazione wireframe: on (premere 'l' per disabilitare)", 1);

	controlPanel->pushItem("Pilota automatico: off (premere 'p' per abilitare)", 0);
	controlPanel->getItem(5)->setText("Pilota automatico: on (premere 'p' per disabilitare)", 1);

	controlPanel->pushItem("Texture cielo: off (premere 't' per abilitare)", 0);
	controlPanel->getItem(6)->setText("Texture cielo: on (premere 't' per disabilitare)", 1);

	controlPanel->pushItem("Numero di giri compiuti: 0/3", 0);
	controlPanel->getItem(7)->setText("Numero di giri compiuti: 1/3", 1);
	controlPanel->getItem(7)->setText("Numero di giri compiuti: 2/3", 2);
	controlPanel->getItem(7)->setText("Numero di giri compiuti: 3/3", 3);

	controlPanel->pushButton("Esci dal gioco");
	controlPanel->pushButton("Ricomincia");
	controlPanel->pushButton("Scoreboard");

	scoreboard = new ControlPanel(wsize * 0.8, wsize * 0.8);
	scoreboard->callback = CPButtonPressed;
	scoreboard->horizontalOffset = wsize * 0.1;
	scoreboard->verticalOffset = wsize * 0.1;
	scoreboard->pushButton("Indietro");

	srand(time(NULL));
	spawnSphere();
}

void reshape(int width, int height) {
	glutReshapeWindow(wsize, wsize);
}

int main(int argc, char** argv)
{	
	// Inizializzazione di GLUT
 	glutInit(&argc,argv);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(wsize,wsize);
	glutInitWindowPosition(0,0);
	windowId = glutCreateWindow("Car");
	glewInit();
	init();
    glutMouseFunc(mouse);
	glutPassiveMotionFunc(motion);
	glutMotionFunc(motion);
	glutJoystickFunc(joystick, 25);
	glutSpecialFunc(special);
	glutSpecialUpFunc(specialUp);
	glutKeyboardFunc(keydown);
	glutKeyboardUpFunc(keyup);
	glutIgnoreKeyRepeat(1);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);
	glutDisplayFunc(display);
	glutMainLoop();
}















