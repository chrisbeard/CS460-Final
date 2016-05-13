#include <GL/glut.h>
#include <stdlib.h>
#include <cmath>
#include <iostream>
#include <vector>


// Data Structures
// ----------------------------------------------------------------------------
class Point {
public:
    double x, y, z;
    Point() : x(0), y(0), z(0) {}
    Point(double xIn, double yIn, double zIn) : x(xIn), y(yIn), z(zIn) {}
    Point(const Point &p) : x(p.x), y(p.y), z(p.z) {}
    void display() {
        glVertex3f((float)x, (float)y, (float)z);
    }
    void print() {
        std::cout << "(" << x << ", " << y << ", " << z << ")\n";
    }
    void reset() {
        x = y = z = 0;
    }
    Point &operator +=(const Point &p) {
        x = x + p.x;
        y = y + p.y;
        z = z + p.z;
        return *this;
    }
};
Point operator*(Point p, double d) {
    return Point(p.x * d, p.y * d, p.z *d);
}
Point operator*(double d, Point p) {
    return Point(p.x * d, p.y * d, p.z *d);
}
// Point operator+(Point p1, Point p2) {
//     return Point(p1.x + p2.x, p1.y + p2.y, p1.z + p2.z);
// }

class Vector {
public:
    double x, y, z;
    Vector() : x(0), y(0), z(0) {}
    Vector(double xIn, double yIn, double zIn) : x(xIn), y(yIn), z(zIn) {}
    Vector(const Vector &v) : x(v.x), y(v.y), z (v.z) {}
    double length() {
        return sqrt(x*x + y*y + z*z);
    }
    Vector abs() {
        return Vector(fabs(x), fabs(y), fabs(z));
    }
    double dot_product(const Vector &v) {
        return (x * v.x) + (y * v.y) + (z * v.z);
    }
    void normalize() {
        double length = sqrt(x*x + y*y + z*z);
        x /= length;
        y /= length;
        z /= length;
    }
    void print() {
        std::cout << "vec (" << x << ", " << y << ", " << z << ")\n";
    }
    void add(const Vector &v) {
        x += v.x;
        y += v.y;
        z += v.z;
    }
};
Vector operator*(const Vector &v, double d) {
    return Vector(v.x * d, v.y * d, v.z * d);
}
Vector operator*(double d, const Vector &v) {
    return Vector(v.x * d, v.y * d, v.z *d);
}
Vector operator+(const Vector &v1, const Vector &v2) {
    return Vector(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}
Vector operator-(const Vector &v1, const Vector &v2) {
    return Vector(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}
Vector operator+(Point p1, Point p2) {
    return Vector(p1.x + p2.x, p1.y + p2.y, p1.z + p2.z);
}
Vector operator-(Point p1, Point p2) {
    return Vector(p1.x - p2.x, p1.y - p2.y, p1.z - p2.z);
}


class PhysicsBody {
public:
    Point position;
    Vector velocity;
    double mass;
    double size;

    PhysicsBody() : position(), velocity(), mass(0), size(0) {}

    void drawCircle() {
       glBegin(GL_LINE_LOOP);
       for (unsigned i = 0; i < 360; i++) {
          float degInRad = i * 180.0 / M_PI;
          glVertex2f(cos(degInRad)*size, sin(degInRad)*size);
       }
       glEnd();
    }
};

class PhysicsWorld {
public:
    std::vector<PhysicsBody> bodies;
};

// Globals and Constants
// ----------------------------------------------------------------------------
static int window_width = 900;
static int window_height = 600;

enum MODE {
    MODE_DEFAULT,
    MODE_ADD_OBJECTS
};
static int mode = MODE_DEFAULT;

static int reshape_index = -1;


void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // glMatrixMode(GL_MODELVIEW);
    // glLoadIdentity();
    // glTranslatef(window_width / 2, window_height / 2, 0);

    glFlush();
    glutSwapBuffers();
}


// Input Handling
// ----------------------------------------------------------------------------
void mouse(int button, int state, int x, int y)
{
    (void)button;
    (void)state;
    (void)x;
    (void)y;
    if (mode == MODE_ADD_OBJECTS && button == GLUT_LEFT_BUTTON) {
        std::cout << "Adding object\n";
        // TODO if inside object, set reshape_index
        // TODO begin/end adding object
    }
    // TODO select object to give velocity to
}
void mouseMove(int x, int y)
{
    (void)x;
    (void)y;
    if (reshape_index != -1) {
        // TODO reshape object
        // TODO give velocity
    }
}
void handleKey(unsigned char key, int x, int y)
{
    (void)x;
    (void)y;
    (void)key;
    glutPostRedisplay();
}
void specialKey(int key, int x, int y)
{
    (void)x;
    (void)y;
    switch(key) {
        case GLUT_KEY_UP:
            break;
        case GLUT_KEY_DOWN:
            break;
        case GLUT_KEY_RIGHT:
            break;
        case GLUT_KEY_LEFT:
            break;
    }
    glutPostRedisplay();
}

void mode_sub(int value)
{
    if (value == 0)
        mode = MODE_ADD_OBJECTS;
    glutPostRedisplay();
}
void menu(int value)
{
    if (value == 0)
        (void)value;
    glutPostRedisplay();
}

void make_menu(void)
{ 
    int mode_menu = glutCreateMenu(mode_sub);
    glutAddMenuEntry("Add Bodies", 0);

    /*MainMenu*/
    glutCreateMenu(menu);
    glutAddSubMenu("Choose Mode", mode_menu);

    glutAttachMenu(GLUT_MIDDLE_BUTTON);
}

// OpenGL Setup
// ----------------------------------------------------------------------------
void init(void)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, window_width, 0.0, window_height, -1000.0, 1000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    window_width = w;
    window_height = h;
    init();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(window_width, window_height);
    glutInitWindowPosition(100, 100);
    glutCreateWindow(argv[0]);
    init();
    glClearColor(0, 0, 0, 1.0);
	make_menu();

    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutPassiveMotionFunc(mouseMove);
    glutKeyboardFunc(handleKey);
    glutSpecialFunc(specialKey);
    glutDisplayFunc(display);

    glutMainLoop();
    return 0;
}
