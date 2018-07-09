#ifndef BUFFER_H
#define BUFFER_H

#include <GL/glew.h>
#include <GL/glut.h>
#include <cstring>
#include <string>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <limits>

#include "Util.h"
#include "Geometry.h"


// Tipo di vertex buffer object
enum BufferType
{
	VertexBufferType, ColorBufferType, NormalBufferType, TexCoordBufferType
    // Array di vertici, colori, normali e coordinate texture
};

/********************************* Buffer ******************************************/

// Questa classe serve a gestire un vertex buffer object che puo' contenere dati come
// vertici, normali, colori e coordinate texture (per gli indici dei vertici da
// disegnare viene usata la classe IndexBuffer). Un VBO contiene un array di float, che
// possono essere usati per disegnare la scena piu' efficientemente rispetto alle primitive.
class Buffer
{
public:
	BufferType type; // Tipo di VBO
	GLuint buffer;   // Buffer contenente i dati
	// Valori da passare ad OpenGL
	GLenum target;   
	GLenum capability; // Enumerazione OpenGL che rappresenta il tipo di VBO
	GLfloat* data;     // Puntatore ai dati. Quando i dati vengono inviati al VBO, diventa
                       // ridondante mantenere un array di dati oltre al VBO. E' quindi
                       // possibile cancellare i dati con il metodo deleteData(). Verranno
					   // cancellati solamente i dati ridondanti, non quelli contenuti 
					   // nel VBO.
	unsigned int size; // Dimensione del buffer (numero di float)
                       // Per ottenere il numero di punti, devo dividere per 3

	void bind();       // Effettua la bind 
	void unbind();     // e la unbind del buffer

	void enable();     // Abilita la funzionalita' del buffer (esempio: abilita 
                       // gli array di vertici).
	void disable();    
public:
	Buffer(BufferType);
	Buffer(const Buffer&);
	Buffer& operator=(const Buffer&);
	~Buffer();

	BufferType getType() const;
	GLfloat getData(unsigned int index) const;
	void setData(GLfloat* data, unsigned int size); // Manda i dati al VBO
	void pointer();  // Specifica la locazione e il formato dei dati (va chiamata
                     // prima di disegnare gli elementi)

	unsigned int getSize() const;

	void deleteData(); // Cancella i dati nell'array "data", ma non quelli inviati 
                       // al VBO (serve solamente a risparmiare memoria)
};

// Array di vertici
class VertexBuffer : public Buffer
{
public:
	VertexBuffer();
};

// Array di colori
class ColorBuffer : public Buffer
{
public:
	ColorBuffer();
};

// Array di normali
class NormalBuffer : public Buffer
{
public:
	NormalBuffer();
};

// Array di coordinate texture
class TexCoordBuffer : public Buffer
{
public:
	TexCoordBuffer();
};

/********************************* IndexBuffer ******************************************/

// Array di indici. Serve ad indicizzare un array di vertici (utile per le mesh, serve a 
// risparmiare memoria).
class IndexBuffer
{
private:
	GLuint* indices;
	unsigned int size;
	GLuint buffer;
public:
	IndexBuffer();
	IndexBuffer(const IndexBuffer&);
	IndexBuffer& operator=(const IndexBuffer&);
	~IndexBuffer();

	void bind();
	void unbind();
	GLfloat getIndex(unsigned int index) const;
	void setIndices(GLuint* indices, unsigned int size); // Invia gli indici al buffer
	unsigned int getSize() const;
	void deleteIndices(); // Cancella gli indici memorizzati nell'array "indices"
  						  // ma non quelli inviati al VBO (e' un'ottimizzazione)
};

/********************************* Mesh ******************************************/

// Modalita' con cui puo' essere disegnata una mesh
// Si puo' utilizzare solamente un colore, oppure un color buffer, si puo' utilizzare
// un materiale, una texture, un environment map, oppure una texture e un materiale
// contemporaneamente.
enum DrawingMode {
	DrawWithColor, DrawWithColorBuffer, DrawWithMaterial, DrawWithTexture, DrawWithEnvMap, DrawWithTextureAndMaterial
};


// Opzioni di caricamento della mesh da file obj
enum MeshLoadingOptions {
	None, FlipZ, Recenter, FlipZThenRecenter, RecenterThenFlipZ, RecenterThenFlipZAndX, FlipZThenRecenterThenFlipX
};

// Classe mesh: contiene un array di vertici e un array di indici per disegnare la mesh efficientemente. Inoltre
// e' possibile utilizzare un array di normali, di coordinate texture e di colori. Per motivi di efficienza, la 
// mesh puo' contenere soltanto triangoli.
class Mesh
{
private:
	VertexBuffer vertexBuffer;
	ColorBuffer colorBuffer;
	NormalBuffer normalBuffer;
	IndexBuffer indexBuffer;
	TexCoordBuffer texCoordBuffer;
public:	
	Mesh();
	// Costruttore con il nome del file obj e le opzioni da applicare alla mesh
	// possono ricentrare la mesh ed invertire alcune componenti
	Mesh(const char* filename, MeshLoadingOptions options = None);
	void setVertexData(GLfloat* data, unsigned int size); // Invio i vertici al buffer
	void setColorData(GLfloat* data, unsigned int size);  // Invio i colori al buffer dei colori
	void setNormalData(GLfloat* data, unsigned int size); // Invio le normali al buffer
	void setIndexData(GLuint* data, unsigned int size);   // Invio gli indici
	void setTexCoordData(GLfloat* data, unsigned int size); // Invio le coordinate texture
	// Setta automaticamente i dati nel buffer delle coordinate texture utilizzando una funzione
	// che chiede in input un texel (coordinate s,t) per ogni punto
	void setTexCoordData(void (*setTexelFunc) (GLfloat, GLfloat, GLfloat, GLfloat*, GLfloat*));
	void deleteData(); // E' un'ottimizzazione: cancella i dati ridondanti dai buffer

	unsigned int getNumberOfVertices() const;
	unsigned int getNumberOfColors() const;
	unsigned int getNumberOfNormals() const;
	unsigned int getNumberOfFaces() const;
	unsigned int getNumberOfTexCoords() const;

    // Calcolo automatico delle normali (vengono calcolate le normali per vertice)
	void computeNormals(bool flipX = false, bool flipY = false, bool flipZ = false);

	Rect boundingBox; // Il bounding box viene calcolato in fase di caricamento della mesh
	Color color;
	// Proprieta' geometriche
	Vector scale;
	Vector translation;
	Vector rotation;
	GLuint texture;
	Material material;
	DrawingMode mode; // Modalita' con cui viene disegnata la mesh
	bool drawAsWireframe; // Se true, disegna la mesh come un wireframe
	void draw(); // Disegno 3D
	void draw2D(); // Disegno 2D

	static Mesh* plane(GLfloat width, GLfloat depth); // Genera un piano
};



#endif





















