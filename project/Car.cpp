#include "Car.h"

void setFrontRightWheelTexel(GLfloat x, GLfloat y, GLfloat z, GLfloat* s, GLfloat* t) {
	*s = (1.0 - y / 7.01949) + 0.5;
	*t = z / 7.0242 + 0.5;
}

void setFrontLeftWheelTexel(GLfloat x, GLfloat y, GLfloat z, GLfloat* s, GLfloat* t) {
	*s = y / 7.01949 + 0.5;
	*t = z / 7.0242 + 0.5;
}

void setBackRightWheelTexel(GLfloat x, GLfloat y, GLfloat z, GLfloat* s, GLfloat* t) {
	*s = (1.0 - y / 7.01949) + 0.5;
	*t = z / 7.2146 + 0.5;
}

void setBackLeftWheelTexel(GLfloat x, GLfloat y, GLfloat z, GLfloat* s, GLfloat* t) {
	*s = y / 7.01949 + 0.5;
	*t = z / 7.2146 + 0.5;
}

Car::Car() { // Costruisco la macchina utilizzando i file obj presenti nella directory
	Material material; // Materiale dell'abitacolo
	// Inizialmente non viene utilizzato, ma premendo il tasto 'k' l'utente puo' disabilitare
	// l'environment map della macchina - in tal caso viene utilizzato il materiale
	material.ambient = Color(0.25,0,0,1);
	material.diffuse = Color(1,.75,.5,1);
	material.specular = White;
	material.shininess = 10;
	material.emission = Color(.75,.25,.25,1);

	this->usesEnvMap = true;
	this->drawAsWireframe = false;
	this->onTurbo = false;

	// Carico lo chassis
	this->body = new Mesh("Ferrari_chassis.obj", FlipZThenRecenter);
	this->body->computeNormals(true, true, true);
	this->body->mode = DrawWithEnvMap;
	this->body->material = material;
	this->body->texture = 0;
	char envmap[] = "envmap_flipped.jpg"; // Environment map
	LoadTexture(0, envmap);
	char logoTexture[] = "logo.jpg"; // Logo da disegnare sulle ruote
	LoadTexture(1, logoTexture);

	// Ruota destra anteriore
	this->backRightWheel = new Mesh("Ferrari_wheel_back_R.obj", FlipZThenRecenter);
	this->backRightWheel->computeNormals(true, true, true);
	this->backRightWheel->mode = DrawWithTextureAndMaterial;
	this->backRightWheel->texture = 1; // Texture per proiettare il logo
	this->backRightWheel->color = Black;
	this->backRightWheel->setTexCoordData(setBackRightWheelTexel);

	// Cerchione
	this->backRightWheelMetal = new Mesh("Ferrari_wheel_back_R_metal.obj", FlipZThenRecenter);
	this->backRightWheelMetal->mode = DrawWithColor;
	this->backRightWheelMetal->color = Metallic;

	this->backLeftWheel = new Mesh("Ferrari_wheel_back_R.obj", FlipZThenRecenterThenFlipX);
	this->backLeftWheel->computeNormals(false, false, false);
	this->backLeftWheel->mode = DrawWithTextureAndMaterial;
	this->backLeftWheel->texture = 1;
	this->backLeftWheel->color = Black;
	this->backLeftWheel->setTexCoordData(setBackLeftWheelTexel);

	// Cerchione 
	this->backLeftWheelMetal = new Mesh("Ferrari_wheel_back_R_metal.obj", FlipZThenRecenterThenFlipX);
	this->backLeftWheelMetal->mode = DrawWithColor;
	this->backLeftWheelMetal->color = Metallic; // Imposto un colore metallico

	// Ruota anteriore destra
	this->frontRightWheel = new Mesh("Ferrari_wheel_front_R.obj", RecenterThenFlipZ);
	this->frontRightWheel->computeNormals(true, true, true);
	this->frontRightWheel->color = Black;
	this->frontRightWheel->mode = DrawWithTextureAndMaterial;
	this->frontRightWheel->texture = 1;
	this->frontRightWheel->setTexCoordData(setFrontRightWheelTexel);

	// Cerchione
	this->frontRightWheelMetal = new Mesh("Ferrari_wheel_front_R_metal.obj", RecenterThenFlipZ);
	this->frontRightWheelMetal->color = Metallic;
	this->frontRightWheelMetal->mode = DrawWithColor;

	// Ruota anteriore sinistra
	this->frontLeftWheel = new Mesh("Ferrari_wheel_front_R.obj", RecenterThenFlipZAndX);
	this->frontLeftWheel->computeNormals(false, false, false);
	this->frontLeftWheel->color = Black;
	this->frontLeftWheel->mode = DrawWithTextureAndMaterial;
	this->frontLeftWheel->texture = 1;
	this->frontLeftWheel->setTexCoordData(setFrontLeftWheelTexel);

	// Cerchione
	this->frontLeftWheelMetal = new Mesh("Ferrari_wheel_front_R_metal.obj", RecenterThenFlipZAndX);
	this->frontLeftWheelMetal->color = Metallic;
	this->frontLeftWheelMetal->mode = DrawWithColor;

	Material wheelsMaterial; // Materiale delle ruote
	wheelsMaterial.ambient = Black;
	wheelsMaterial.diffuse = White;
	wheelsMaterial.specular = White;
	wheelsMaterial.emission = Color(0.25,0.25,0.25,1);
	wheelsMaterial.shininess = 10;
	this->frontRightWheel->material = wheelsMaterial;
	this->frontLeftWheel->material = wheelsMaterial;
	this->backRightWheel->material = wheelsMaterial;
	this->backLeftWheel->material = wheelsMaterial;

	// Parametri per la fisica
	this->frontWheelsRadius = std::fabs(this->frontRightWheel->boundingBox.dimension.dz / 2.0);
	this->backWheelsRadius = std::fabs(this->backRightWheel->boundingBox.dimension.dz / 2.0);
	this->facing = 0.0;
	this->friction = Vector(0.2, 0.0, 0.1);
	this->grip = 0.45;
	this->isMovingForward = true;

	float angle = M_PI / 9.0;
	Rect bodyBoundingBox = this->body->boundingBox;
	this->rightSpotLight.position = Point(bodyBoundingBox.origin.x + bodyBoundingBox.dimension.dx , bodyBoundingBox.origin.y + bodyBoundingBox.dimension.dy , bodyBoundingBox.origin.z);
	this->rightSpotLight.direction = Vector(0, -sin(angle), -cos(angle));

	this->leftSpotLight.position = Point(bodyBoundingBox.origin.x , bodyBoundingBox.origin.y + bodyBoundingBox.dimension.dy , bodyBoundingBox.origin.z);
	this->leftSpotLight.direction = Vector(0, -sin(angle), -cos(angle));

	this->castsShadow = false;

	this->body->deleteData();
	this->frontRightWheel->deleteData();
	this->frontLeftWheel->deleteData();
	this->backRightWheel->deleteData();
	this->backLeftWheel->deleteData();
	this->frontRightWheelMetal->deleteData();
	this->frontLeftWheelMetal->deleteData();
	this->backRightWheelMetal->deleteData();
	this->backLeftWheelMetal->deleteData();

	this->deltaTime = 0.0;
	this->stopwatch.start(); // Parte il cronometro che misura il tempo tra uno step fisico e l'altro
}

Rect Car::getBoundingBox() const {
	return this->body->boundingBox;
}

Point Car::getOrigin() const {
	return this->origin;
}

void Car::setOrigin(const Point& origin) {
	this->origin = origin;
}

void Car::setRotation(const Vector& rotation) {
	this->rotation = getRotationVector(rotation, Degrees);
}

float Car::getVelocity() const {
	if(this->isMovingForward) return this->velocity.norm();
	else return -this->velocity.norm();
}

// Imposto la velocita' della macchina, utilizzando uno scalare
// Utilizzando lo scalare velocita' e l'orientamento della macchina, sono
// in grado di calcolare il vettore velocita'
void Car::setVelocity(float velocity) {
	if(this->onTurbo) {
		velocity = std::min(velocity, 500.0f); 
		velocity = std::max(velocity, -150.0f); 
	} else {
		if(velocity > 275.0) {
			velocity *= 0.975;
		} else if (velocity < -82.5) {
			velocity *= 0.975;
		} else {
			velocity = std::min(velocity, 250.0f); // Velocita' massima: 180
			velocity = std::max(velocity, -75.0f); // Velocita' massima in retromarcia: 50
		}
	}
	// Qui calcolo le componenti del vettore velocita'
	this->velocity.dz = -cos(this->facing / 180.0 * M_PI) * velocity;
	this->velocity.dx = -sin(this->facing / 180.0 * M_PI) * velocity;

	this->isMovingForward = velocity >= 0.0;
}

float Car::getSteering() const {
	return this->steering;
}

void Car::setSteering(float steering) {
	this->steering = std::max(-45.0f, std::min(45.0f, steering));
}

float Car::getFacing() const {
	return this->facing;
}

void Car::setFacing(float facing) {
	this->facing = facing;
}

void Car::draw() { // Disegno la macchina
	glPushMatrix();
	glTranslatef(this->origin.x, this->origin.y, this->origin.z);
	glRotatef(this->rotation.dz, 0.0, 0.0, 1.0);
	glRotatef(this->rotation.dy, 0.0, 1.0, 0.0);
	glRotatef(this->rotation.dx, 1.0, 0.0, 0.0);

	this->rightSpotLight.enable();
	this->leftSpotLight.enable();

	if(this->usesEnvMap)
		this->body->mode = DrawWithEnvMap;
	else 
		this->body->mode = DrawWithMaterial;

	// Nel caso che la macchina debba essere disegnata come un wireframe, imposto 
	// la prorpieta' drawAsWireframe di ogni mesh
	this->body->drawAsWireframe = this->drawAsWireframe;
	this->frontRightWheel->drawAsWireframe = this->drawAsWireframe;
	this->frontLeftWheel->drawAsWireframe = this->drawAsWireframe;
	this->backRightWheel->drawAsWireframe = this->drawAsWireframe;
	this->backLeftWheel->drawAsWireframe = this->drawAsWireframe;
	this->frontRightWheelMetal->drawAsWireframe = this->drawAsWireframe;
	this->frontLeftWheelMetal->drawAsWireframe = this->drawAsWireframe;
	this->backRightWheelMetal->drawAsWireframe = this->drawAsWireframe;
	this->backLeftWheelMetal->drawAsWireframe = this->drawAsWireframe;

	// Disegno lo chassis e le ruote
	this->body->draw();
	this->frontRightWheel->draw();
	this->frontLeftWheel->draw();
	this->backRightWheel->draw();
	this->backLeftWheel->draw();

	// Disegno i cerchioni
	this->frontRightWheelMetal->draw();
	this->frontLeftWheelMetal->draw();
	this->backRightWheelMetal->draw();
	this->backLeftWheelMetal->draw();
	glPopMatrix();

    /*************************** Ombra *******************************/
	if(this->castsShadow) {
		glPushMatrix();
		// la shadowProjection() moltiplica la matrice corrente per un'opportuna matrice
		// in modo tale da proiettare la mesh sul terreno
		// La luce e' fissa in posizione (0,1000,0)
		// Passo un punto qualsiasi del piano (0,2,0)
		// La normale e' (0,-1,0)
		shadowProjection(Point(0,10000,0), Point(0, 2.0, 0), Vector(0,-1,0));	

		// Da qui in poi tutto cio' che disegno sara' proiettato sul terreno

		// Trasformazioni per spostare/ruotare la macchina
		glTranslatef(this->origin.x, this->origin.y, this->origin.z);
		glRotatef(this->rotation.dz, 0.0, 0.0, 1.0);
		glRotatef(this->rotation.dy, 0.0, 1.0, 0.0);
		glRotatef(this->rotation.dx, 1.0, 0.0, 0.0);

		this->body->color = Black;
		this->body->mode = DrawWithColor;
		this->body->draw();

		// Disabilito le texture
		// Quindi gli oggetti verranno "schiacciati", ovvero proiettati sul terreno
		// e disegnati di nero
		this->frontRightWheel->mode = DrawWithColor;
		//this->frontRightWheel->color = Black;
		this->frontRightWheel->draw();

		this->frontLeftWheel->mode = DrawWithColor;
		//this->frontLeftWheel->color = Black;
		this->frontLeftWheel->draw();

		this->backRightWheel->mode = DrawWithColor;
		//this->backRightWheel->color = Black;
		this->backRightWheel->draw();

		this->backLeftWheel->mode = DrawWithColor;
		//this->backLeftWheel->color = Black;
		this->backLeftWheel->draw();

		this->frontRightWheel->mode = DrawWithTextureAndMaterial;
		this->frontLeftWheel->mode = DrawWithTextureAndMaterial;
		this->backRightWheel->mode = DrawWithTextureAndMaterial;
		this->backLeftWheel->mode = DrawWithTextureAndMaterial;

		glPopMatrix();
	}
}

bool Car::isMoving() const {
	return this->stopwatch.isStarted() && this->getVelocity() != 0.0;
}

void Car::updatePhysics() {
	// Aggiorno la fisica
	// Normalmente questa funzione viene chiamata ogni 1/100
	// di secondo, pero'puo' succedere che il programma 
	// "rimanga indietro", nel senso che non e' in grado di 
	// calcolare la fisica ogni 1/100 di secondo perche' la
	// CPU o la GPU lavorano troppo. Per questo motivo, per
	// evitare di far rallentare la macchina in caso di
	// rallentamenti dell'hardware, posso eseguire piu' passi 
	// di fisica qualora sia trascorso piu' di 1/100 di secondo
	// tra una chiamata e l'altra di updatePhysics().
	this->deltaTime += this->stopwatch.lapse(); // Calcolo il tempo
												// trascorso
	while(this->deltaTime > 0.01) {
		// Aggiorno la fisica una volta per ogni 1/100 di
		// secondo trascorso
		this->physicsStep();
		this->deltaTime -= 0.01;
	}
}

void Car::physicsStep() { // Aggiorno la fisica (simulazione a passo costante)

	/**************************** Accelerazione e spostamenti **************************************/
	Controller& controller = Controller::shared();
	float speed = this->getVelocity();
	float steering = this->getSteering();

	float deltaSpeed = 4.0;
	if(this->onTurbo) deltaSpeed *= 1.5;
	float deltaSteering = 2.0;

	if(controller.isJoystickEnabled)  // Nel caso che l'utente stia utilizzando il joystick 
	{								  // (attivabile con il tasto 'j')
		float x = controller.joystickX;
		float y = controller.joystickY;
		
		// Modifico la velocita' e lo sterzo
		steering -= deltaSteering * x;
		speed -= deltaSpeed * y;
	} 
	else // Se l'utente sta utilizzando la tastiera
	{
		if(controller.isKeyPressed('w') || controller.isKeyUpPressed) {
			speed += deltaSpeed;
		}
		if(controller.isKeyPressed('s') || controller.isKeyDownPressed) {
			speed -= deltaSpeed;
		}
		if(controller.isKeyPressed('a') || controller.isKeyLeftPressed) {
			steering += deltaSteering;
		}
		if(controller.isKeyPressed('d') || controller.isKeyRightPressed) {
			steering -= deltaSteering;
		}
	}

	// Infine imposto la velocita' e lo sterzo precedentemente calcolati
	this->setVelocity(speed);
	this->setSteering(steering);

	/************ Calcolo della posizione e dell'orientamento dell'auto e delle ruote **************/
	float cosF = cos(this->facing / 180.0 * M_PI);
	float sinF = sin(this->facing / 180.0 * M_PI);
	// Vettore velocita' locale alla macchina
	Vector localVelocity = Vector(cosF * this->velocity.dx - sinF * this->velocity.dz, this->velocity.dy , sinF * this->velocity.dx + cosF * this->velocity.dz);
	localVelocity = Vector(localVelocity.dx * (1.0 - friction.dx / 100.0), localVelocity.dy * (1.0 - friction.dy / 100.0), localVelocity.dz * (1.0 - friction.dz / 100.0));

	this->facing = this->facing - (localVelocity.dz * this->grip) * this->steering / 3200.0; // Modifico lo sterzo
	this->facing = getRotationAngle(this->facing, Degrees);

	// Questo valore delta determina di quanto ruotano le ruote rispetto all'asse x
	float delta = 0.005 * (180.0 * localVelocity.dz) / (M_PI * frontWheelsRadius); 
	float angle = getRotationAngle(this->frontRightWheel->rotation.dx + delta, Degrees);
	// Rotazione delle ruote rispetto ai due assi x-y (rotazione - sterzo)
	this->frontRightWheel->rotation.dx = angle;
	this->frontLeftWheel->rotation.dx = angle;
	this->frontRightWheel->rotation.dy = this->steering;
	this->frontLeftWheel->rotation.dy = this->steering;
	this->frontRightWheelMetal->rotation.dx = angle;
	this->frontLeftWheelMetal->rotation.dx = angle;
	this->frontRightWheelMetal->rotation.dy = this->steering;
	this->frontLeftWheelMetal->rotation.dy = this->steering;
		
	// Stessa procedura con le ruote posteriori, solamente che le ruote posteriori non sterzano
	angle = getRotationAngle(this->backRightWheel->rotation.dx + delta, Degrees);
	this->backRightWheel->rotation.dx = angle;
	this->backLeftWheel->rotation.dx = angle;
	this->backRightWheelMetal->rotation.dx = angle;
	this->backLeftWheelMetal->rotation.dx = angle;

	Vector velocity = Vector(cosF * localVelocity.dx + sinF * localVelocity.dz , localVelocity.dy, -sinF * localVelocity.dx + cosF * localVelocity.dz);
	this->velocity = velocity; // Aggiorno la velocita'

	this->rotation = Vector(0, this->facing, 0); // Imposto l'orientamento
	this->origin = this->origin + Vector(this->velocity.dx / 100.0, this->velocity.dy / 100.0, this->velocity.dz / 100.0); // Spostamento della macchina
	this->steering = this->steering * 0.93; // Ritorno sterzo
}

void Car::updateStopwatch() {
	this->stopwatch.lapse();
}











