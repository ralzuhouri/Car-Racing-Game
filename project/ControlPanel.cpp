#include "ControlPanel.h"

// Costruttore con parametri: titolo, posizione-x, posizione-y, larghezza, altezza del pulsante
ControlPanelButton::ControlPanelButton(std::string title, float x, float y, float width, float height) : text(title) {
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
	this->hoovering = false;
	this->color = Color(0.4, 0.4, 0.4, 1.0);
	this->panel = NULL;

	this->text.x = x + 5.0;
	this->text.y = y + 5.0;
	this->text.size = 2;

	GLfloat vertices[12] = {x,y,0.0,  x+width,y,0.0,  x+width,y+height,0.0,   x,y+height,0.0};
	GLuint indices[6] = {0,1,2,  2,3,0};
	this->box.setVertexData(vertices, 12);
	this->box.setIndexData(indices, 6);
	this->box.color = this->color;
	this->box.mode = DrawWithColor;
}

// Disegna il pulsante
void ControlPanelButton::draw() {
	float factor = 0.85;
	if(panel && panel->isMouseDown) factor = 0.6;
	if(this->hoovering) this->box.color = Color(this->color.red * factor, this->color.green * factor, this->color.blue * factor, 1.0);
	else this->box.color = this->color;

	this->box.draw2D();
	this->text.draw();
}

// Costruisce un oggetto del pannello di controllo
ControlPanelItem::ControlPanelItem(std::string str, int state) : text(str) {
	this->state = state;
	this->setText(str, state);
}

void ControlPanelItem::setText(std::string text, int state) {
	//this->statesMap.insert(std::pair<int,std::string>(state,text));
	this->statesMap[state] = text;
}

void ControlPanelItem::draw() { // Disegno l'oggetto del pannello di controllo
	std::string text = "?";
	std::map<int,std::string>::iterator i = this->statesMap.find(this->state);
	if(i != this->statesMap.end()) 
		text = i->second;

	this->text.characters = text;
	this->text.draw();
}

// Costruttore del pannello di controllo
ControlPanel::ControlPanel(GLfloat width, GLfloat height) {
	this->width = width;
	this->height = height;
	this->horizontalOffset = 0;
	this->verticalOffset = 0;
	this->backgroundColor = Gray;
	this->isMouseDown = false;
	this->callback = NULL;

	GLfloat vertices[12] = {0.0,0.0,0.0,  width,0.0,0.0,  width,height,0.0,   0.0,height,0.0};
	GLuint indices[6] = {0,1,2,  2,3,0};
	this->box = new Mesh();
	this->box->setVertexData(vertices,12);
	this->box->setIndexData(indices,6);
	this->box->color = this->backgroundColor;
	this->box->mode = DrawWithColor;
}

GLfloat ControlPanel::getWidth() const {
	return this->width;
}

GLfloat ControlPanel::getHeight() const {
	return this->height;
}

void ControlPanel::pushItem(std::string text, int state) { // Aggiunge un oggetto
	ControlPanelItem* item = new ControlPanelItem(text, state);
	item->text.y = this->verticalOffset + this->height - this->items.size() * 25 - 20.0;
	item->text.x = this->horizontalOffset + 20.0;
	this->items.push_back(item);
}

void ControlPanel::popItem() {
	ControlPanelItem* item = this->items[this->items.size() - 1];
	this->items.pop_back();
	delete item;
}

unsigned int ControlPanel::numberOfItems() const {
	return this->items.size();
}

ControlPanelItem* ControlPanel::getItem(unsigned int index) {
	return this->items[index];
}

// Aggiunge un pulsante
void ControlPanel::pushButton(std::string title) {
	float x = this->horizontalOffset + 20.0;
	float y = this->verticalOffset + 20.0 + 30.0 * this->buttons.size();
	float width = this->width - 40.0;
	float height = 25.0;
	ControlPanelButton* button = new ControlPanelButton(title, x, y, width, height);
	button->panel = this;
	this->buttons.push_back(button);
}

void ControlPanel::popButton() {
	ControlPanelButton* button = this->buttons[this->buttons.size() - 1];
	this->buttons.pop_back();
	delete button;
}

unsigned int ControlPanel::numberOfButtons() const {
	return this->buttons.size();
}

ControlPanelButton* ControlPanel::getButton(unsigned int index) {
	return this->buttons[index];
}

// Disegna il riquadro del pannello di controllo con tutti gli oggetti
void ControlPanel::draw() {
	this->box->translation = Vector(this->horizontalOffset, this->verticalOffset, 0.0);
	this->box->color = this->backgroundColor;

	this->box->draw2D();

	for(unsigned int i=0; i<this->items.size(); i++) {
		ControlPanelItem* item = this->items[i];
		item->draw();	
	}

	for(unsigned int i=0; i<this->buttons.size(); i++) {
		ControlPanelButton* button = this->buttons[i];
		button->draw();	
	}
}


// Mouse move: serve a colorare i pulsanti nel caso che il mouse ci sia sopra
void ControlPanel::mouseMove(float x, float y) {
	for(unsigned int i=0; i<this->buttons.size(); i++) {
		ControlPanelButton* button = this->buttons[i];
		bool h = x >= button->x && x <= (button->x + button->width);
		bool v = y >= button->y && y <= (button->y + button->height);
		bool hoovering = h && v;
		button->hoovering = hoovering;	
	}
}

void ControlPanel::mouseDown(float x, float y) {
	this->isMouseDown = true;
}

// Mouse up: serve a chiamare il callback nel caso che il mouse sia sopra
// ad un pulsante
void ControlPanel::mouseUp(float x, float y) {
	this->isMouseDown = false;

	for(unsigned int i=0; i<this->buttons.size(); i++) {
		ControlPanelButton* button = this->buttons[i];
		bool h = x >= button->x && x <= (button->x + button->width);
		bool v = y >= button->y && y <= (button->y + button->height);
		bool pressed = h && v;
		if(pressed && this->callback) {
			this->callback(i, button);
		}
	}
}

// Resetta gli eventi
void ControlPanel::resetEvents() {
	this->isMouseDown = false;
	for(unsigned int i=0; i<this->buttons.size(); i++) {
		ControlPanelButton* button = this->buttons[i];
		button->hoovering = false;
	}
}













