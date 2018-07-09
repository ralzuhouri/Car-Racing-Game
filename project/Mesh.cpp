#include "Mesh.h"

/********************************* Buffer ******************************************/
Buffer::Buffer(BufferType type) 
{
	this->type = type;
	glGenBuffers(1, &this->buffer);

	switch(this->type) 
	{
	case VertexBufferType:
		this->target = GL_ARRAY_BUFFER;
		this->capability = GL_VERTEX_ARRAY;
		break;
	case ColorBufferType:
		this->target = GL_ARRAY_BUFFER;
		this->capability = GL_COLOR_ARRAY;
		break;
	case NormalBufferType:
		this->target = GL_ARRAY_BUFFER;
		this->capability = GL_NORMAL_ARRAY;
		break;
	case TexCoordBufferType:
		this->target = GL_ARRAY_BUFFER;
		this->capability = GL_TEXTURE_COORD_ARRAY;
	}

	this->data = 0;
	this->size = 0;
}

Buffer::Buffer(const Buffer& other)
{
	this->type = other.type;
	this->target = other.target;
	this->capability = other.capability;
	this->buffer = other.buffer;
	this->setData(other.data, other.size);
}

Buffer& Buffer::operator=(const Buffer& other)
{
	this->type = other.type;
	this->target = other.target;
	this->capability = other.capability;
	this->buffer = other.buffer;
	this->setData(other.data, other.size);

	return *this;
}

Buffer::~Buffer()
{
	glDeleteBuffers(1, &this->buffer);

	if(this->data) delete[] data;
}

void Buffer::bind()
{
	glBindBuffer(this->target, this->buffer);
}

void Buffer::unbind()
{
	glBindBuffer(this->target, 0);
}

void Buffer::enable()
{
	glEnableClientState(this->capability);
}

void Buffer::disable()
{
	glDisableClientState(this->capability);
}

BufferType Buffer::getType() const 
{
	return this->type;
}

GLfloat Buffer::getData(unsigned int index) const {
	return this->data[index];
}

// Invia i dati al buffer
void Buffer::setData(GLfloat* data, unsigned int size)
{
    // Se sono gia' presenti dei dati li cancello
	if(this->data) delete[] data;
	if(!data) { // Azzero i dati nel caso che l'argomento 'data' sia NULL
		this->data = 0;
		this->size = 0;
		return;
	}

	this->data = new GLfloat[size];
	memcpy(this->data, data, size * sizeof(GLfloat)); // Copio i dati
	this->size = size;

	this->bind(); // Collego il buffer ed invio i dati
	glBufferData(this->target, this->size * sizeof(GLfloat), this->data, GL_STATIC_DRAW);
	this->unbind();
}

// Speficia la posizione e il formato dei dati
void Buffer::pointer()
{
	switch(this->type)
	{
	case VertexBufferType:
		glVertexPointer(3, GL_FLOAT, 0, 0);
		break;
	case ColorBufferType:
		glColorPointer(3, GL_FLOAT, 0, 0);
		break;
	case NormalBufferType:
		glNormalPointer(GL_FLOAT, 0, 0);
		break;
	case TexCoordBufferType:
		glTexCoordPointer(2, GL_FLOAT, 0, 0);
		break;
	}
}

unsigned int Buffer::getSize() const {
	return this->size;
}

void Buffer::deleteData() {
	if(this->data) delete[] this->data;
	this->data = NULL;
}

VertexBuffer::VertexBuffer() : Buffer(VertexBufferType) {}
ColorBuffer::ColorBuffer() : Buffer(ColorBufferType) {}
NormalBuffer::NormalBuffer() : Buffer(NormalBufferType) {}
TexCoordBuffer::TexCoordBuffer() : Buffer(TexCoordBufferType) {}

/********************************* IndexBuffer ******************************************/

IndexBuffer::IndexBuffer() {
	glGenBuffers(1, &this->buffer);

	this->size = 0;
	this->indices = 0;
}

IndexBuffer::IndexBuffer(const IndexBuffer& other)
{
	this->buffer = other.buffer;
	this->setIndices(other.indices, other.size);
}

IndexBuffer& IndexBuffer::operator=(const IndexBuffer& other)
{
	this->buffer = other.buffer;
	this->setIndices(other.indices, other.size);

	return *this;
}

IndexBuffer::~IndexBuffer() {
	if(this->indices) {
		delete[] this->indices;
	}

	glDeleteBuffers(1, &this->buffer);
}

// Faccio la bind del buffer
void IndexBuffer::bind() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->buffer);
}

// Faccio la unbind del buffer
void IndexBuffer::unbind() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->buffer);
}

GLfloat IndexBuffer::getIndex(unsigned int index) const {
	return indices[index];
}

// Invia i dati al buffer
void IndexBuffer::setIndices(GLuint* indices, unsigned int size) {
	if(this->indices) { // Cancello i dati se sono gia' presenti
		delete[] this->indices;
	}
	if(!indices) {
		this->indices = 0;
		this->size = 0;
		return;
	}

	this->size = size;
	this->indices = new GLuint[size];
	memcpy(this->indices, indices, size * sizeof(GLuint)); // Copio i dati

	this->bind(); // Invio i dati al buffer
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * this->size, this->indices, GL_STATIC_DRAW);
	this->unbind();
}

unsigned int IndexBuffer::getSize() const {
	return this->size;
}

void IndexBuffer::deleteIndices() {
	if(this->indices) delete[] this->indices;
	this->indices = NULL;
}

/********************************* Mesh ******************************************/

// Costruttore senza parametri: costruisco una mesh vuota
Mesh::Mesh() {
	this->mode = DrawWithColor;
	this->texture = -1;
	this->scale = Vector(1,1,1);
	this->translation = Vector();
	this->rotation = Vector();
	this->boundingBox = Rect(Point(), Vector());
	this->drawAsWireframe = false;
	this->color = White;
}

// Costruisco una mesh utilizzando un file obj
// Versione semplice: non carica le normali
// Le normali per vertice possono essere calcolate automaticamente 
// utilizzando la funzione computeNormals()
Mesh::Mesh(const char* filename, MeshLoadingOptions options) {
	this->mode = DrawWithColor;
	this->texture = -1;
	this->scale = Vector(1,1,1);
	this->translation = Vector();
	this->rotation = Vector();
	this->drawAsWireframe = false;
	this->color = White;

	GLfloat minFloat = std::numeric_limits<float>::min();
	GLfloat maxFloat = std::numeric_limits<float>::max();
	// Memorizzo due punti che determinano il bounding box della mesh
	Point minPoint = Point(maxFloat, maxFloat, maxFloat);
	Point maxPoint = Point(minFloat, minFloat, minFloat);

	std::ifstream file(filename);
	std::string line;

	std::vector<GLfloat> vertexData;
	std::vector<GLfloat> colorData;
	std::vector<GLuint> indexData;

	if(file.is_open()) {
		while(std::getline(file, line)) // Leggo il file riga per riga
		{
			// Il metodo split divide una stringa in un array di stringhe utilizzando un carattere separatore
			std::vector<std::string> components = split(line, ' ');

			if(components.size() == 4 && components[0] == "v") {
				// ******************************** Aggiungo il vertice **************************************
				GLfloat x = atof(components[1].c_str());
				GLfloat y = atof(components[2].c_str());
				GLfloat z = atof(components[3].c_str());
				if(options == FlipZ || options == FlipZThenRecenter || options == FlipZThenRecenterThenFlipX) z = -z; 
				
				vertexData.push_back(x);
				vertexData.push_back(y);
				vertexData.push_back(z);

				if(x < minPoint.x) minPoint.x = x;
				if(y < minPoint.y) minPoint.y = y;
				if(z < minPoint.z) minPoint.z = z;

				if(x > maxPoint.x) maxPoint.x = x;
				if(y > maxPoint.y) maxPoint.y = y;
				if(z > maxPoint.z) maxPoint.z = z;
			} else if(components.size() == 4 && components[0] == "vn") {
				// Le normali vengono ignorate. E' possibile calcolarle automaticamente con la funzione computeNormals()
				/*Vector normal = Vector(atof(components[1].c_str()), atof(components[2].c_str()), atof(components[3].c_str()));
				this->normals.push_back(normal);*/
			} else if(components.size() >= 3 && components[0] == "f") {
				// ******************************** Aggiungo la faccia ***************************************
				std::vector<unsigned int> indices;
				for(unsigned int i=1; i<components.size(); i++) {
					std::vector<std::string> pair = split(components[i], '/');
					int index = atoi(pair[0].c_str());
					indices.push_back(index - 1);

					/*if(pair.size() < 2) continue;
					int normal = atoi(pair[1].c_str());
					faceNormals.push_back(normal - 1); // Normale della faccia*/
				}

				if(indices.size() == 3) { // Aggiungo una faccia triangolare (3 indici)
					indexData.push_back(indices[0]);
					indexData.push_back(indices[1]);
					indexData.push_back(indices[2]);
				} else if(indices.size() == 4) {
					// Se la faccia e' rettangolare, la scompongo in due triangoli
					indexData.push_back(indices[0]);
					indexData.push_back(indices[1]);
					indexData.push_back(indices[2]);

					indexData.push_back(indices[0]);
					indexData.push_back(indices[2]);
					indexData.push_back(indices[3]);
				} else {
					std::cout << "Mesh: found face with " << indices.size() << " vertices" << std::endl;
				}
			}
		}

		this->boundingBox = Rect(minPoint, Vector(maxPoint.x - minPoint.x, maxPoint.y - minPoint.y, maxPoint.z - minPoint.z));

		// Opzioni per ricentrare la mesh e invertire le componenti
		if(options == Recenter || options == RecenterThenFlipZ || options == FlipZThenRecenter || options == RecenterThenFlipZAndX || options == FlipZThenRecenterThenFlipX) {
			Vector sub = Vector(this->boundingBox.origin.x + this->boundingBox.dimension.dx / 2.0, this->boundingBox.origin.y + this->boundingBox.dimension.dy / 2.0, this->boundingBox.origin.z + + this->boundingBox.dimension.dz / 2.0);

			for(unsigned int i=0; i<vertexData.size() / 3; i++) {
				GLfloat x = vertexData[i * 3];
				GLfloat y = vertexData[i * 3 + 1];
				GLfloat z = vertexData[i * 3 + 2];

				x -= sub.dx;
				y -= sub.dy;
				z -= sub.dz;

				vertexData[i * 3] = x;
				vertexData[i * 3 + 1] = y;
				vertexData[i * 3 + 2] = z;
			}

			this->boundingBox.origin = Point(-this->boundingBox.dimension.dx / 2.0, -this->boundingBox.dimension.dy / 2.0, -this->boundingBox.dimension.dz / 2.0);
			this->translation = sub;
		}

		if(options == RecenterThenFlipZ || options == RecenterThenFlipZAndX || options == FlipZThenRecenterThenFlipX) {
			for(unsigned int i=0; i<vertexData.size() / 3; i++) {
				if(options == RecenterThenFlipZ || options == RecenterThenFlipZAndX) {
					GLfloat z = vertexData[i * 3 + 2];
					z = -z;
					vertexData[i * 3 + 2] = z;
				}

				if(options == RecenterThenFlipZAndX || options == FlipZThenRecenterThenFlipX) {
					GLfloat x = vertexData[i * 3];
					x = -x;
					vertexData[i * 3] = x;
				}
			}

			if(options == RecenterThenFlipZ || options == RecenterThenFlipZAndX) this->translation.dz = -this->translation.dz;
			if(options == RecenterThenFlipZAndX || options == FlipZThenRecenterThenFlipX) this->translation.dx = -this->translation.dx;	
		}

		this->vertexBuffer.setData(vertexData.data(), vertexData.size());
		this->colorBuffer.setData(colorData.data(), colorData.size());	
		this->indexBuffer.setIndices(indexData.data(), indexData.size());		

		file.close();
	} else {
		throw std::invalid_argument("Invalid filename used in Mesh::Mesh(const char*)");
	}

}


void Mesh::setVertexData(GLfloat* data, unsigned int size) {
	this->vertexBuffer.setData(data, size);
}

void Mesh::setColorData(GLfloat* data, unsigned int size) {
	this->colorBuffer.setData(data, size);
}

void Mesh::setNormalData(GLfloat* data, unsigned int size) {
	this->normalBuffer.setData(data, size);
}

void Mesh::setIndexData(GLuint* data, unsigned int size) {
	this->indexBuffer.setIndices(data, size);
}

void Mesh::setTexCoordData(GLfloat* data, unsigned int size) {
	this->texCoordBuffer.setData(data, size);
}

// Setta automaticamente i dati nel buffer delle coordinate texture utilizzando una funzione
// che chiede in input un texel (coordinate s,t) per ogni punto
void Mesh::setTexCoordData(void (*setTexelFunc) (GLfloat, GLfloat, GLfloat, GLfloat*, GLfloat*)) {

	GLfloat* texCoords = new GLfloat[this->getNumberOfVertices() * 2];

	for(unsigned int i=0; i<this->getNumberOfVertices(); i++) {
		GLfloat px = this->vertexBuffer.getData(i * 3);	
		GLfloat py = this->vertexBuffer.getData(i * 3 + 1);
		GLfloat pz = this->vertexBuffer.getData(i * 3 + 2);
		GLfloat s, t;

		setTexelFunc(px, py, pz, &s, &t);
		texCoords[i * 2] = s;
		texCoords[i * 2 + 1] = t;
	}

	this->texCoordBuffer.setData(texCoords, this->getNumberOfVertices() * 2);

	delete[] texCoords;
}

void Mesh::deleteData() {
	this->vertexBuffer.deleteData();
	this->colorBuffer.deleteData();
	this->normalBuffer.deleteData();
	this->indexBuffer.deleteIndices();
	this->texCoordBuffer.deleteData();
}

unsigned int Mesh::getNumberOfVertices() const {
	return this->vertexBuffer.getSize() / 3;
}

unsigned int Mesh::getNumberOfColors() const {
	return this->colorBuffer.getSize() / 3;
}

unsigned int Mesh::getNumberOfNormals() const {
	return this->normalBuffer.getSize() / 3;
}

unsigned int Mesh::getNumberOfFaces() const {
	return this->indexBuffer.getSize() / 3;
}

unsigned int Mesh::getNumberOfTexCoords() const {
	return this->texCoordBuffer.getSize() / 2;
}

// Calcolo le normali per vertice
void Mesh::computeNormals(bool flipX, bool flipY, bool flipZ) {
	GLfloat* normalData = new GLfloat[this->vertexBuffer.getSize()];
	for(unsigned int i=0; i<this->vertexBuffer.getSize(); i++) {
		normalData[i] = 0.0;
	}

	for(unsigned int i=0; i<this->getNumberOfFaces(); i++) {
		GLuint index1 = this->indexBuffer.getIndex(i * 3);
		GLuint index2 = this->indexBuffer.getIndex(i * 3 + 1);
		GLuint index3 = this->indexBuffer.getIndex(i * 3 + 2);

		GLfloat p0x = this->vertexBuffer.getData(index1 * 3);
		GLfloat p0y = this->vertexBuffer.getData(index1 * 3 + 1);
		GLfloat p0z = this->vertexBuffer.getData(index1 * 3 + 2);

		GLfloat p1x = this->vertexBuffer.getData(index2 * 3);
		GLfloat p1y = this->vertexBuffer.getData(index2 * 3 + 1);
		GLfloat p1z = this->vertexBuffer.getData(index2 * 3 + 2);

		GLfloat p2x = this->vertexBuffer.getData(index3 * 3);
		GLfloat p2y = this->vertexBuffer.getData(index3 * 3 + 1);
		GLfloat p2z = this->vertexBuffer.getData(index3 * 3 + 2);

		GLfloat v1x = p1x - p0x;
		GLfloat v1y = p1y - p0y;
		GLfloat v1z = p1z - p0z;

		GLfloat v2x = p2x - p0x;
		GLfloat v2y = p2y - p0y;
		GLfloat v2z = p2z - p0z;

		GLfloat nx = v1y * v2z - v1z * v2y;
		GLfloat ny = v1z * v2x - v1x * v2z;
	 	GLfloat nz = v1x * v2y - v1y * v2x;
		GLfloat norm = sqrt(nx * nx + ny * ny + nz * nz);
		nx /= norm;
		ny /= norm;
		nz /= norm;

		normalData[index1 * 3] += nx;
		normalData[index2 * 3] += nx;
		normalData[index3 * 3] += nx;

		normalData[index1 * 3 + 1] += ny;
		normalData[index2 * 3 + 1] += ny;
		normalData[index3 * 3 + 1] += ny;

		normalData[index1 * 3 + 2] += nz;
		normalData[index2 * 3 + 2] += nz;
		normalData[index3 * 3 + 2] += nz;
	}

	for(unsigned int i=0; i<this->getNumberOfVertices(); i++) {
		GLfloat nx = normalData[i * 3];
		GLfloat ny = normalData[i * 3 + 1];
		GLfloat nz = normalData[i * 3 + 2];
		GLfloat norm = sqrt(nx * nx + ny * ny + nz * nz);
		nx /= norm;
		ny /= norm;
		nz /= norm;

		if(flipX) nx = -nx;
		if(flipY) ny = -ny;
		if(flipZ) nz = -nz;

		normalData[i * 3] = nx;
		normalData[i * 3 + 1] = ny;
		normalData[i * 3 + 2] = nz;
	}

	this->normalBuffer.setData(normalData, this->vertexBuffer.getSize());
	delete[] normalData;
}

// Disegno la mesh
void Mesh::draw() {
	glPushMatrix();

	this->vertexBuffer.enable();
	this->vertexBuffer.bind();
	this->vertexBuffer.pointer();
	this->vertexBuffer.unbind();

	// Ci sono varie modalita' di rendering
	if(this->mode == DrawWithColor) { // posso usare un singolo colore
		this->color.set();
	} else if(this->mode == DrawWithColorBuffer) {
		this->colorBuffer.enable(); // oppure un VBO contente i colori per ogni vertice
		this->colorBuffer.bind();
		this->colorBuffer.pointer();
	} else if(this->mode == DrawWithMaterial) {
		glEnable(GL_LIGHTING);
		this->material.set();

		this->normalBuffer.enable();
		this->normalBuffer.bind();
		this->normalBuffer.pointer();
		this->normalBuffer.unbind();
	// Posso utilizzare una texture e un materiale contemporaneamente
	} else if(this->mode == DrawWithTexture || this->mode == DrawWithTextureAndMaterial) {
		glEnable(GL_TEXTURE_2D); 
		glBindTexture(GL_TEXTURE_2D, this->texture);
		if(this->mode == DrawWithTexture) {
			glDisable(GL_LIGHTING);
			this->color.set();
		} else if (this->mode == DrawWithTextureAndMaterial) {
			glEnable(GL_LIGHTING);
			this->material.set();

			this->normalBuffer.enable();
			this->normalBuffer.bind();
			this->normalBuffer.pointer();
			this->normalBuffer.unbind();
		}
		this->texCoordBuffer.enable();
		this->texCoordBuffer.bind();
		this->texCoordBuffer.pointer();
		this->texCoordBuffer.unbind();
	} else if(this->mode == DrawWithEnvMap) { // In questo caso uso un environment map
		glEnable(GL_LIGHTING);
		SetupEnvmapTexture(this->texture);

		this->normalBuffer.enable();
		this->normalBuffer.bind();
		this->normalBuffer.pointer();
		this->normalBuffer.unbind();
	}

	if(this->drawAsWireframe) {
		glPolygonMode(GL_FRONT, GL_LINE);
		glPolygonMode(GL_BACK, GL_LINE);
	}

	// Applico le trasformazioni prima di disegnare la mesh
	glTranslatef(this->translation.dx, this->translation.dy, this->translation.dz);
	glRotatef(this->rotation.dz, 0.0, 0.0, 1.0);
	glRotatef(this->rotation.dy, 0.0, 1.0, 0.0);
	glRotatef(this->rotation.dx, 1.0, 0.0, 0.0);
	glScalef(this->scale.dx, this->scale.dy, this->scale.dz);

	this->indexBuffer.bind();
	// Qui disegno le facce (sono triangoli)
	glDrawElements(GL_TRIANGLES, this->indexBuffer.getSize() , GL_UNSIGNED_INT, (void*)0);
	this->indexBuffer.unbind();

	if(this->mode == DrawWithColor) {
		this->colorBuffer.unbind();
		this->colorBuffer.disable();
	} else if(this->mode == DrawWithMaterial) {
		glDisable(GL_LIGHTING);
		this->normalBuffer.disable();
	} else if(this->mode == DrawWithTexture || this->mode == DrawWithTextureAndMaterial) {
		this->texCoordBuffer.disable();
		glDisable(GL_TEXTURE_2D);
		if(this->mode == DrawWithTextureAndMaterial) {
			this->normalBuffer.disable();
			glDisable(GL_LIGHTING);
		}
	} else if(this->mode == DrawWithEnvMap) {
		DisableEnvmapTexture();	
		this->normalBuffer.disable();
	}

	this->vertexBuffer.disable();

	glPolygonMode(GL_BACK, GL_FILL);
	glPolygonMode(GL_FRONT, GL_FILL);
	glPopMatrix();
}

// Disegno 2D della mesh
void Mesh::draw2D() {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 680, 0, 680, -1, 10);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glDisable(GL_CULL_FACE);
	glClear(GL_DEPTH_BUFFER_BIT);

	this->draw();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

// Costruisce un piano
Mesh* Mesh::plane(GLfloat width, GLfloat depth) {
	Mesh* mesh = new Mesh();
	
	GLfloat vertices[12] = {static_cast<GLfloat>(-width / 2.0), 0.0, static_cast<GLfloat>(-depth / 2.0),  
							static_cast<GLfloat>(width / 2.0), 0.0, static_cast<GLfloat>(-depth / 2.0),  
							static_cast<GLfloat>(width / 2.0), 0.0, static_cast<GLfloat>(depth / 2.0), 
							static_cast<GLfloat>(-width / 2.0), 0.0, static_cast<GLfloat>(depth / 2.0) };
	GLuint indices[6] = {0, 1, 2,  2, 3, 0};
	mesh->setVertexData(vertices, 12);
	mesh->setIndexData(indices, 6);

	return mesh;
}













