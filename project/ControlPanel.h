#ifndef CONTROL_PANEL_H
#define CONTROL_PANEL_H

#include "Mesh.h"
#include <string>
#include <vector>
#include <map>

class ControlPanel;
class ControlPanelButton;

// Callback dei pulsanti del pannello di controllo
typedef void (*ControlPanelCallback) (unsigned int, ControlPanelButton*);

// Pulsante del pannello di controllo
class ControlPanelButton
{
public:
	ControlPanel* panel;
	Text text;
	float x,y;
	float width,height;
	Mesh box;
	bool hoovering;
	Color color;

	ControlPanelButton(std::string title, float x, float y, float width, float height);
	void draw();
};

// Oggetto del pannello di controllo (contiene del testo)
// Inoltre e' possibile salvare una stringa differente per ogni stato
// Cambiando lo stato, la stringa visualizzata cambia
class ControlPanelItem
{
public:
	Text text;
	int state;
	std::map<int, std::string> statesMap;

	ControlPanelItem(std::string, int state);
	void setText(std::string, int);
	void draw();
};

// Pannello di controllo contenente oggetti (stringhe) e pulsanti
class ControlPanel
{
private:
	GLfloat width, height;
	Mesh* box;
	std::vector<ControlPanelItem*> items;
	std::vector<ControlPanelButton*> buttons;
	bool isMouseDown;
public:
	GLfloat horizontalOffset, verticalOffset;
	Color backgroundColor;
	ControlPanelCallback callback;

	ControlPanel(GLfloat,GLfloat);
	GLfloat getWidth() const;
	GLfloat getHeight() const;

	// Aggiunge un oggetto (gli oggetti vengono mostrati dall'alto
	// verso il basso)
	void pushItem(std::string text, int state); 
	void popItem(); // Toglie l'ultimo oggetto aggiunto
	unsigned int numberOfItems() const;
	ControlPanelItem* getItem(unsigned int index);

	// Aggiunge un pulsante (i pulsanti vengono aggiunti dal basso
	// (verso l'alto)
	void pushButton(std::string title);
	void popButton(); // Rimuove l'ultimo pulsante aggiunto
	unsigned int numberOfButtons() const;
	ControlPanelButton* getButton(unsigned int index);

	void draw();

	// Eventi del mouse: servono a colorare i pulsanti quando il mouse e' sopra
	// e a chiamare i callback
	void mouseMove(float x, float y);
	void mouseDown(float x, float y);
	void mouseUp(float x, float y);
	void resetEvents();

	friend class ControlPanelButton;
};

#endif









