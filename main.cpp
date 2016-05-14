#include <GL/glut.h>
#include <stdlib.h>
#include <cmath>
#include <iostream>
#include <vector>
#include <stdio.h>


// Data Structures
// ----------------------------------------------------------------------------
class Vector {
public:
    double x, y;
    Vector() : x(0), y(0) {}
    Vector(double xIn, double yIn) : x(xIn), y(yIn) {}
    Vector(const Vector &v) : x(v.x), y(v.y) {}
    double length() {
        return sqrt(x*x + y*y);
    }
    Vector abs() {
        return Vector(fabs(x), fabs(y));
    }
    double dot_product(const Vector &v) {
        return (x * v.x) + (y * v.y);
    }
    void normalize() {
        double length = sqrt(x*x + y*y);
        x /= length;
        y /= length;
    }
    void print() {
        std::cout << "vec (" << x << ", " << y << ")\n";
    }
    void add(const Vector &v) {
        x += v.x;
        y += v.y;
    }
};
class Point {
public:
    double x, y;
    Point() : x(0), y(0) {}
    Point(double xIn, double yIn) : x(xIn), y(yIn) {}
    Point(const Point &p) : x(p.x), y(p.y) {}
    void display() {
        glVertex3f((float)x, (float)y, 0);
    }
    void print() {
        std::cout << "(" << x << ", " << y << ")\n";
    }
    void reset() {
        x = y = 0;
    }
    Point &operator +=(const Point &p) {
        x += p.x;
        y += p.y;
        return *this;
    }
    Point &operator +=(const Vector &v) {
        x += v.x;
        y += v.y;
        return *this;
    }
    Point operator +(const Vector &v) {
        x += v.x;
        y += v.y;
        return *this;
    }
};
// Point operators
Point operator*(Point p, double d) {
    return Point(p.x * d, p.y * d);
}
Point operator*(double d, Point p) {
    return Point(p.x * d, p.y * d);
}
double distance(const Point &p1, const Point &p2) {
    return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
}

// Vector operators
Vector operator*(const Vector &v, double d) {
    return Vector(v.x * d, v.y * d);
}
Vector operator*(double d, const Vector &v) {
    return Vector(v.x * d, v.y * d);
}
Vector operator/(const Vector &v, double d) {
    return Vector(v.x / d, v.y / d);
}
Vector operator/(double d, const Vector &v) {
    return Vector(v.x / d, v.y / d);
}
Vector operator+(const Vector &v1, const Vector &v2) {
    return Vector(v1.x + v2.x, v1.y + v2.y);
}
Vector operator-(const Vector &v1, const Vector &v2) {
    return Vector(v1.x - v2.x, v1.y - v2.y);
}
Vector operator+(Point p1, Point p2) {
    return Vector(p1.x + p2.x, p1.y + p2.y);
}
Vector operator-(Point p1, Point p2) {
    return Vector(p1.x - p2.x, p1.y - p2.y);
}


class PhysicsBody {
public:
    Point position;
    Vector velocity;
    double mass;
    double radius;

    PhysicsBody() : position(), velocity(), mass(1), radius(0) {}
    PhysicsBody(double xIn, double yIn) : position(xIn, yIn), velocity(), mass(1), radius(0) {}

    void drawCircle() {
        glLineWidth(2.0);
        glBegin(GL_LINE_LOOP);
        for (unsigned i = 0; i < 360; ++i) {
            double degInRad = i * M_PI / 180.0;
            glVertex2f(position.x + cos(degInRad)*radius, position.y + sin(degInRad)*radius);
        }
        glEnd();
    }
    void drawVelocity() {
        glLineWidth(2.0);
        glBegin(GL_LINES);
        glVertex2f(position.x, position.y);
        glVertex2f(position.x + 30.0 * velocity.x, position.y + 30.0 * velocity.y);
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
    MODE_ADD_OBJECTS,
    MODE_CHANGE_VELOCITY,
    MODE_MOVE
};
static int mode = MODE_DEFAULT;

enum MOUSE_STATE {
    MOUSE_DEFAULT,
    MOUSE_DRAWING
};
static int mouse_state = MOUSE_DEFAULT;

// Index of cricle currently being modified
static int reshape_index = -1;

PhysicsWorld world;


int get_circle_for_point(const Point &p)
{
    for (size_t i = 0; i < world.bodies.size(); ++i) {
        if (distance(p, world.bodies[i].position) < world.bodies[i].radius)
            return i;
    }
    return -1;
}

// Display functions
// ----------------------------------------------------------------------------
void update_bodies()
{
    if (mode != MODE_MOVE)
        return;

    // Check for wall collsions
    for (size_t i = 0; i < world.bodies.size(); ++i) {
        Point old_position = world.bodies[i].position;
        Vector velocity = world.bodies[i].velocity;
        Point new_position = world.bodies[i].position + velocity;
        double radius = world.bodies[i].radius;

        // Left wall
        if (new_position.x - radius < 0) {
            new_position.x += 1 * -(new_position.x - radius);
            world.bodies[i].velocity.x *= -1;
        }
        // Right wall
        if (new_position.x + radius > window_width) {
            new_position.x -= 1 * (new_position.x + radius - window_width);
            world.bodies[i].velocity.x *= -1;
        }
        // Top wall
        if (new_position.y - radius < 0) {
            new_position.y += 2 * -(new_position.y - radius);
            world.bodies[i].velocity.y *= -1;
        }
        // Bottom wall
        if (new_position.y + radius > window_height) {
            new_position.y -= 2 * (new_position.y + radius - window_height);
            world.bodies[i].velocity.y *= -1;
        }
        world.bodies[i].position = new_position;
    }

    std::vector<bool> did_collide(world.bodies.size(), false);

    // Check for ball collisions
    bool collisions = true;
    while (collisions) {
        collisions = false;
        for (size_t i = 0; i < world.bodies.size() - 1; ++i) {
            for (size_t j = i + 1; j < world.bodies.size(); ++j) {
                // TODO different collision mechanisms
                double d = fabs(distance(world.bodies[i].position, world.bodies[j].position));
                double rad_sum = world.bodies[i].radius + world.bodies[j].radius;
                if (d < rad_sum) {
                    collisions = true;
                    Vector v1 = world.bodies[i].velocity;
                    Vector v2 = world.bodies[j].velocity;
                    double m1 = world.bodies[i].mass;
                    double m2 = world.bodies[j].mass;
                    Point p1 = world.bodies[i].position;
                    Point p2 = world.bodies[j].position;

                    Vector v1p = v1 - ((2*m2) / (m1+m2)) * ((v1-v2).dot_product(p1-p2) / pow((p1-p2).length(), 2)) * (p1-p2);
                    Vector v2p = v2 - ((2*m1) / (m1+m2)) * ((v2-v1).dot_product(p2-p1) / pow((p2-p1).length(), 2)) * (p2-p1);

                    world.bodies[i].velocity = v1p;
                    world.bodies[j].velocity = v2p;

                    did_collide[i] = true;
                    did_collide[j] = true;
                }
            }
        }
        for (size_t i = 0; i < world.bodies.size(); ++i) {
            if (did_collide[i])
                world.bodies[i].position += world.bodies[i].velocity;
        }
    }
}

void draw_bodies()
{
    for (size_t i = 0; i < world.bodies.size(); ++i) {
        world.bodies[i].drawCircle();
        if (mode != MODE_MOVE)
            world.bodies[i].drawVelocity();
    }
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    update_bodies();
    draw_bodies();

    glFlush();
    glutSwapBuffers();

    if (mode == MODE_MOVE)
        glutPostRedisplay();
}


// Input Handling
// ----------------------------------------------------------------------------
void mouse(int button, int state, int x, int y)
{
    Point p(x, y);
    // Add objects to the physics world
    if (mode == MODE_ADD_OBJECTS && button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN && mouse_state == MOUSE_DEFAULT) {
            if ((reshape_index = get_circle_for_point(p)) == -1) {
                world.bodies.push_back(PhysicsBody(x, y));
                reshape_index = world.bodies.size() - 1;
                mouse_state = MOUSE_DRAWING;
            } else {
                mouse_state = MOUSE_DRAWING;
            }
        } else if (state == GLUT_DOWN && mouse_state == MOUSE_DRAWING) {
            world.bodies[reshape_index].radius = distance(world.bodies[reshape_index].position, p);
            world.bodies[reshape_index].mass = pow(world.bodies[reshape_index].radius / 10, 2);
            reshape_index = -1;
            mouse_state = MOUSE_DEFAULT;
        }
    }
    // Give velocities to objects in the physics world
    else if (mode == MODE_CHANGE_VELOCITY && button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN && mouse_state == MOUSE_DEFAULT) {
            if ((reshape_index = get_circle_for_point(p)) != -1) {
                mouse_state = MOUSE_DRAWING;
            }
        } else if (state == GLUT_DOWN && mouse_state == MOUSE_DRAWING) {
            world.bodies[reshape_index].velocity = (p - world.bodies[reshape_index].position) / 30.0;
            mouse_state = MOUSE_DEFAULT;
        }
    }
    
    glutPostRedisplay();
}
void mouseMove(int x, int y)
{
    Point p(x, y);
    if (reshape_index != -1) {
        if (mode == MODE_ADD_OBJECTS && mouse_state == MOUSE_DRAWING) {
            world.bodies[reshape_index].radius = distance(world.bodies[reshape_index].position, p);
        }
        else if (mode == MODE_CHANGE_VELOCITY && mouse_state == MOUSE_DRAWING) {
            world.bodies[reshape_index].velocity = (p - world.bodies[reshape_index].position) / 30.0;
        }
    }
    glutPostRedisplay();
}
void handleKey(unsigned char key, int x, int y)
{
    (void)x;
    (void)y;
    if (key == ' ') {
        mode = (mode != MODE_MOVE) ? MODE_MOVE : MODE_DEFAULT;
    }
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
    else if (value == 1)
        mode = MODE_CHANGE_VELOCITY;
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
    glutAddMenuEntry("Change Velocity", 1);

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
    glOrtho(0.0, window_width, window_height, 0.0, -1000.0, 1000.0);
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

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);

    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutPassiveMotionFunc(mouseMove);
    glutKeyboardFunc(handleKey);
    glutSpecialFunc(specialKey);
    glutDisplayFunc(display);

    glutMainLoop();
    return 0;
}
