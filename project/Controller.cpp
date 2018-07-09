#include "Controller.h"

Controller* Controller::controller = NULL;

Controller::Controller() 
{
	for(unsigned int i=0; i<256; i++) {
		this->keys.push_back(false);
	}
}

Controller& Controller::shared() {
	if(controller == NULL) {
		controller = new Controller();
		controller->joystickX = 0.0;
		controller->joystickY = 0.0;
		controller->joystickZ = 0.0;
		controller->isJoystickEnabled = false;
		controller->isKeyUpPressed = false;
		controller->isKeyDownPressed = false;
		controller->isKeyRightPressed = false;
		controller->isKeyLeftPressed = false;
	}
	return *controller;
}

void Controller::pressKey(unsigned char key) {
	this->keys[key] = true;
}

void Controller::releaseKey(unsigned char key) {
	this->keys[key] = false;
}

void Controller::releaseAllKeys() {
	for(unsigned int i=0; i<256; i++) {
		this->keys[i] = false;
	}
}

bool Controller::isKeyPressed(unsigned char key) {
	return this->keys[key];
}
