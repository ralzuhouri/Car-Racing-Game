#include "Geometry.h"

// getRotationAngle: modifica il valore dell'angolo, renendolo un valore compreso nell' intervallo
// [0, 2 *pi] nel caso che il valore AngleUnit sia uguale a "Radians", oppure nell'intervallo [0,360]
// nel caso che venga passato un angolo in gradi (AngleUnit.Degrees)
float getRotationAngle(float angle, AngleUnit unit) {
	float circ;
	float result = angle;

	switch(unit) {
		case Degrees: 
			circ = 360.0;
			break;
		case Radians:
			circ = 2.0 * M_PI;
			break;	
	}

	while(result > circ) result -= circ;
	while(result < 0.0) result += circ;
	return result;
}

// Applica la getRotationAngle a tutte le componenti di un vettore
Vector getRotationVector(const Vector& vector, AngleUnit unit) {
	float dx = getRotationAngle(vector.dx, unit);
	float dy = getRotationAngle(vector.dy, unit);
	float dz = getRotationAngle(vector.dz, unit);
	return Vector(dx, dy, dz);
}

// Applica la shadow projection, moltiplicando la matrice corrente per una matrice tale che ogni 
// oggetto disegnato successivamente sia proiettato sul terreno 
// 
// - lightPos: posizione della luce
// - surfPoint: un punto qualsiasi della superficie
// - normal: il vettore normale alla superficie
void shadowProjection(Point lightPos, Point surfPoint, Vector normal) {

	float d = Vector::dot(normal, Vector(lightPos));
	float c = Vector::dot(Vector(surfPoint), normal) - d;
	float* matrix = new float[16];

	matrix[0] = lightPos.x * normal.dx + c;
	matrix[4] = normal.dy * lightPos.x;
	matrix[8] = normal.dz * lightPos.x;
	matrix[12] = -lightPos.x * c - lightPos.x * d;
	
	matrix[1] = normal.dx * lightPos.y;
	matrix[5] = lightPos.y * normal.dy + c;
	matrix[9] = normal.dz * lightPos.y;
	matrix[13] = -lightPos.y * c - lightPos.y * d;

	matrix[2] = normal.dx * lightPos.z;
	matrix[6] = normal.dy * lightPos.z;
	matrix[10] = lightPos.z * normal.dy + c;
	matrix[14] = -lightPos.z * c - lightPos.z * d;

	matrix[3] = normal.dx;
	matrix[7] = normal.dy;
	matrix[11] = normal.dz;
	matrix[15] = -d;

	glMultMatrixf(matrix);
	
	delete[] matrix;
}

/********************************* Point ******************************************/

Point::Point() {
	this->x = 0.0;
	this->y = 0.0;
	this->z = 0.0;
}

Point::Point(const Vector& vector) {
	this->x = vector.dx;
	this->y = vector.dy;
	this->z = vector.dz;
}

Point::Point(float x, float y, float z) {
	this->x = x;
	this->y =y;
	this->z = z;
}

void Point::draw() {
	glVertex3f(this->x, this->y, this->z);
}

Point Point::operator+(const Vector& v) const {
	return Point(this->x + v.dx, this->y + v.dy, this->z + v.dz);
}

Point Point::operator-(const Vector& v) const {
	return Point(this->x - v.dx, this->y - v.dy, this->z - v.dz);
}

Point Point::operator-() const {
	return Point(-this->x, -this->y, -this->z);
}

bool Point::operator== (const Point& other) const {
	return this->x == other.x && this->y == other.y && this->z == other.z;
}

void Point::copyToBuffer(float* buffer) const {
	buffer[0] = x;
	buffer[1] = y;
	buffer[2] = z;
}


/********************************* Color ******************************************/

Color::Color() {
	this->red = 0.0;
	this->green = 0.0;
	this->blue = 0.0;
	this->alpha = 0.0;
}

Color::Color(float red, float green, float blue, float alpha) {
	this->red = red;
	this->green = green;
	this->blue = blue;
	this->alpha = alpha;
}

Color Color::random() {
	std::vector<float> values;
	for(int i=0; i<3; i++) {
		int integer = rand() % 255;
		float value = integer / 255.0;
		values.push_back(value);
	}
	return Color(values[0], values[1], values[2], 1.0);
}

void Color::set() {
	glColor4f(red, green, blue, alpha);
}

void Color::copyToBuffer(float* buffer) const {
	buffer[0] = red;
	buffer[1] = green;
	buffer[2] = blue;
	buffer[3] = alpha;
}

Color Color::operator*(float factor) const {
	// Moltiplicazione per un coefficiente
	Color result;
	result.red = std::max(this->red * factor, 1.0f);    // Non puo' valere piu' di 1.0
	result.green = std::max(this->green * factor, 1.0f);
	result.blue = std::max(this->blue * factor, 1.0f);
	result.alpha = std::max(this->alpha * factor, 1.0f);
	return result;
}

/********************************* Vector ******************************************/

Vector::Vector() {
	this->dx = 0.0;
	this->dy = 0.0;
	this->dz = 0.0;
}

Vector::Vector(const Point& point) {
	this->dx = point.x;
	this->dy = point.y;
	this->dz = point.z;
}

Vector::Vector(float dx, float dy, float dz) {
	this->dx = dx;
	this->dy = dy;
	this->dz = dz;
}

float Vector::norm() const { // Norma vettoriale
	return sqrt(this->dx * this->dx + this->dy * this->dy + this->dz * this->dz);
}

// Ritorna il vettore normalizzato
Vector Vector::normalized() const {
	return *this / this->norm();
}

float Vector::dot(const Vector& v1, const Vector& v2) {
	return v1.dx * v2.dx + v1.dy * v2.dy + v1.dz * v2.dz;
}

// Prodotto vettoriale
Vector Vector::cross(const Vector& v1, const Vector& v2) {
	return Vector(v1.dy * v2.dz - v1.dz * v2.dy, v1.dz * v2.dx - v1.dx * v2.dz, v1.dx * v2.dy - v1.dy * v2.dx);
}

Vector Vector::operator*(float coeff) const {
	return Vector(this->dx * coeff, this->dy * coeff, this->dz * coeff);
}

Vector Vector::operator+(const Vector& other) const {
	return Vector(this->dx + other.dx, this->dy + other.dy, this->dz + other.dz);
}

Vector Vector::operator-(const Vector& other) const {
	return Vector(this->dx - other.dx, this->dy - other.dy, this->dz - other.dz);
}

Vector Vector::operator-() const {
	return Vector(-this->dx, -this->dy, -this->dz);
}

Vector Vector::operator/(float dividend) const {
	return Vector(this->dx / dividend, this->dy / dividend, this->dz / dividend);
}

bool Vector::operator== (const Vector& other) const {
	return this->dx == other.dx && this->dy == other.dy && this->dz == other.dz;
}

void Vector::copyToBuffer(float* buffer) const {
	buffer[0] = dx;
	buffer[1] = dy;
	buffer[2] = dz;
}

/********************************* Rect ******************************************/

Rect::Rect() {
	this->origin = Point();
	this->dimension = Vector();
}

Rect::Rect(const Point& origin, const Vector& dimension) {
	this->origin = origin;
	this->dimension = dimension;
}

Rect::Rect(const Point& origin, const Point& end) {
	this->origin = origin;
	this->dimension = Vector(end - origin);
}

// Ritorna il centro assoluto del volume
Point Rect::getCenter() const {
	return origin + (dimension / 2.0);
}

/********************************* Line ******************************************/

Line::Line(const Point& p1, const Point& p2) {
	this->p1 = p1;
	this->p2 = p2;
}

void Line::draw() {
	glLineWidth(1.5);
	this->color.set();
	glBegin(GL_LINES);
	this->p1.draw();
	this->p2.draw();
	glEnd();
}

Color Line::getColor() const {
	return this->color;
}

void Line::setColor(const Color& color) {
	this->color = color;
}

/********************************* Material ******************************************/

Material::Material() {
	this->ambient = Black;
	this->diffuse = Black;
	this->specular = White;
	this->emission = White;
	this->shininess = 1.0;
}

void Material::set() {
	float buffer[4] = {0, 0, 0, 0};
	this->ambient.copyToBuffer(buffer);
	glMaterialfv(GL_FRONT, GL_AMBIENT, buffer);

	this->diffuse.copyToBuffer(buffer);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, buffer);

	this->specular.copyToBuffer(buffer);
	glMaterialfv(GL_FRONT, GL_SPECULAR, buffer);

	this->emission.copyToBuffer(buffer);
	glMaterialfv(GL_FRONT, GL_EMISSION, buffer);

	glMaterialf(GL_FRONT, GL_SHININESS, this->shininess);
}

/********************************* Bar ******************************************/

Bar::Bar(float x, float y, float dx, float dy) {
	percentage = 0.0;
	this->targetFPS = 100.0;
	this->x = x;
	this->y = y;
	this->dx = dx;
	this->dy = dy;
}

void Bar::draw() {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 680, 0, 680, -1, 10);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glDisable(GL_CULL_FACE);
	glClear(GL_DEPTH_BUFFER_BIT);

	glBegin(GL_QUADS);
	glColor3f(0, 1, 0);
	glVertex2f(this->x, this->y);
	glVertex2f(this->x + this->dx, this->y);
	glVertex2f(this->x + this->dx, this->y + this->dy * this->percentage);
	glVertex2f(this->x, this->y + this->dy * this->percentage);

	glColor3f(1, 0, 0);
	glVertex2f(this->x, this->y + this->dy * this->percentage);
	glVertex2f(this->x + this->dx, this->y + this->dy * this->percentage);
	glVertex2f(this->x + this->dx, this->y + this->dy);
	glVertex2f(this->x, this->y + this->dy);
	glEnd();

	std::ostringstream os;
	os << int(this->percentage * this->targetFPS);
	std::string fpsString = os.str();
	fpsString = fpsString + " FPS";

	glColor3f(1, 1, 1);
	glRasterPos3f(this->x - this->dx / 2.0 + 30.0, this->y + 5, 0);
	for(unsigned int i=0; i<fpsString.length(); i++)
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, fpsString[i]);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

float Bar::getPercentage() const {
	return this->percentage;
}

void Bar::setPercentage(float percentage) {
	this->percentage = std::min(percentage, 1.0f);
}

/********************************* Circle ******************************************/

Circle::Circle(float radius) {
	this->radius = radius;
	this->x = 0.0;
	this->y = 0.0;
	this->fillColor = White;
	this->strokeColor = Black;
	this->stroke = true;
} 

void Circle::draw() {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 680, 0, 680, -1, 10);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glDisable(GL_CULL_FACE);
	glEnable(GL_LINE_SMOOTH);
	glClear(GL_DEPTH_BUFFER_BIT);

	glBegin(GL_POLYGON);
	this->fillColor.set();
	
	for(unsigned int i=0; i<16; i++) {
		float angle = 360.0 / 16.0 * float(i);
		float x = this->x + sin(angle / 180.0 * M_PI) * this->radius;
		float y = this->y + cos(angle / 180.0 * M_PI) * this->radius;
		glVertex2f(x,y);
	}

	glEnd();

	if(this->stroke) {
		this->strokeColor.set();
		glLineWidth(0.25);
		glBegin(GL_LINES);

		glVertex2f(this->x, this->y + this->radius);
		for(unsigned int i=1; i<16; i++) {
			float angle = 360.0 / 16.0 * float(i);
			float x = this->x + sin(angle / 180.0 * M_PI) * this->radius;
			float y = this->y + cos(angle / 180.0 * M_PI) * this->radius;
			glVertex2f(x,y);
			glVertex2f(x,y);
		}
		glVertex2f(this->x, this->y + this->radius);

		glEnd();
	}

	glDisable(GL_LINE_SMOOTH);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}






















