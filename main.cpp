#include <GL/glut.h>
#include <stdlib.h>
#include <cmath>
#include <iostream>
#include <vector>
#include <stdio.h>


// Globals and Constants
// ----------------------------------------------------------------------------
static int window_width = 900;
static int window_height = 600;

enum MODE {
    MODE_DEFAULT,
    MODE_ADD_OBJECTS,
    MODE_CHANGE_VELOCITY,
    MODE_MOVE,
    MODE_DELETE
};
static int mode = MODE_ADD_OBJECTS;

static bool friendly_fire = false;
static int mask = 1;

enum MOUSE_STATE {
    MOUSE_DEFAULT,
    MOUSE_DRAWING
};
static int mouse_state = MOUSE_DEFAULT;

// Index of cricle currently being modified
static int reshape_index = -1;

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
        return Point(x + v.x, y + v.y);
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
    return hypot(p2.x - p1.x, p2.y - p1.y);
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
    int mask;

    PhysicsBody() : position(), velocity(), mass(1), radius(0), mask(1) {}
    PhysicsBody(double xIn, double yIn, int maskIn) : position(xIn, yIn), velocity(), mass(1), radius(0), mask(maskIn) {}

    void drawCircle() {
        glLineWidth(2.0);
        glBegin(GL_LINE_LOOP);
        if (!friendly_fire)
            glColor3f(1.0, 1.0, 1.0); 
        else if (mask == 1)
            glColor3f(1.0, 0.0, 0.0); 
        else
            glColor3f(0.0, 0.3, 1.0); 
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
    void print() {
        printf("x: %g, y: %g  dx: %g, dy: %g,  rad: %f,  mass: %g\n", position.x, position.y, velocity.x, velocity.y, radius, mass);
    }
};

class PhysicsWorld {
public:
    std::vector<PhysicsBody> bodies;
};


// Global physics world
PhysicsWorld world;


int get_circle_for_point(const Point &p)
{
    for (size_t i = 0; i < world.bodies.size(); ++i) {
        if (distance(p, world.bodies[i].position) < world.bodies[i].radius)
            return i;
    }
    return -1;
}

static bool collide(const Point &p1, double r1, const Point &p2, double r2)
{
    return distance(p1, p2) - (r1 + r2) < 0.5;
}
// Binary search to find the mtd
double bs_mtd(size_t i, size_t j)
{
    Point p1 = world.bodies[i].position;
    Point p2 = world.bodies[j].position;
    Vector v = world.bodies[i].velocity;
    double r1 = world.bodies[i].radius;
    double r2 = world.bodies[j].radius;

    double factor = 0.5;
    double diff = factor / 2;

    while (diff > 0.01) {
        if (collide(p1 + factor * v, r1, p2, r2))
            factor -= diff;
        else
            factor += diff;
        diff /= 2.0;
    }

    // If we can't move close enough, signal to move in direction
    // of the new velocity, post-collision
    if (collide(p1 + factor * v, r1, p2, r2))
        return factor - 1;

    return factor;
}

// Display functions
// ----------------------------------------------------------------------------
void update_bodies()
{
    if (mode != MODE_MOVE)
        return;

    // Check for ball collision
    for (size_t i = 0; i < world.bodies.size(); ++i) {

        Point new_position = world.bodies[i].position + world.bodies[i].velocity;
        bool collision = false;
        double mtd = 1;
        int index = -1;
        Vector v1p, v2p;

        for (size_t j = 0; j < world.bodies.size(); ++j) {
            if (i == j)  continue;

            // Friendly fire
            if (friendly_fire && (world.bodies[i].mask & world.bodies[j].mask)) {
                continue;
            }
            double d = distance(new_position, world.bodies[j].position);
            double rad_sum = world.bodies[i].radius + world.bodies[j].radius;
            if (d - rad_sum < 0.5) {
                collision = true;

                double mtd_temp = bs_mtd(i, j);
                printf("mtd: %g  i:%zu j:%zu\n", mtd_temp, i, j);

                if (mtd_temp < mtd) {
                    mtd = mtd_temp;
                    index = j;

                    Vector v1 = world.bodies[i].velocity;
                    Vector v2 = world.bodies[j].velocity;
                    double m1 = world.bodies[i].mass;
                    double m2 = world.bodies[j].mass;
                    Point p1 = world.bodies[i].position;
                    Point p2 = world.bodies[j].position;

                    v1p = v1 - ((2*m2) / (m1+m2)) * ((v1-v2).dot_product(p1-p2) / pow((p1-p2).length(), 2)) * (p1-p2);
                    v2p = v2 - ((2*m1) / (m1+m2)) * ((v2-v1).dot_product(p2-p1) / pow((p2-p1).length(), 2)) * (p2-p1);
                }
            }
        }
        // If no collision, check wall collision
        if (collision) {
            // We can move close to the other ball
            if (mtd > 0)
                world.bodies[i].position += mtd * world.bodies[i].velocity;

            world.bodies[i].velocity = v1p;
            world.bodies[index].velocity = v2p;

            // Can't move close, move in our new direction
            if (mtd < 0)
                world.bodies[i].position += fabs(mtd) * world.bodies[i].velocity;
        } else {
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
                new_position.y += 1 * -(new_position.y - radius);
                world.bodies[i].velocity.y *= -1;
            }
            // Bottom wall
            if (new_position.y + radius > window_height) {
                new_position.y -= 1 * (new_position.y + radius - window_height);
                world.bodies[i].velocity.y *= -1;
            }
            world.bodies[i].position = new_position;
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
                world.bodies.push_back(PhysicsBody(x, y, mask));
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
    // Remove objects
    else if (mode == MODE_DELETE && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if ((reshape_index = get_circle_for_point(p)) != -1) {
            world.bodies.erase(world.bodies.begin() + reshape_index);
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
    else if (key == 'f' || key == 'F') {
        friendly_fire = !friendly_fire;
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
    mode = MODE_ADD_OBJECTS;
    mask = value;
    glutPostRedisplay();
}
void menu(int value)
{
    if (value == 1)
        mode = MODE_CHANGE_VELOCITY;
    else if (value == 2)
        friendly_fire = !friendly_fire;
    else if (value == 3)
        world.bodies.clear();
    else if  (value == 4)
        mode = MODE_DELETE;
    glutPostRedisplay();
}

void make_menu(void)
{ 
    int mode_menu = glutCreateMenu(mode_sub);
    glutAddMenuEntry("Red", 1);
    glutAddMenuEntry("Blue", 2);

    /*MainMenu*/
    glutCreateMenu(menu);
    glutAddSubMenu("Add Bodies", mode_menu);
    glutAddMenuEntry("Change Velocity", 1);
    glutAddMenuEntry("Toggle Friendly Fire", 2);
    glutAddMenuEntry("Reset World", 3);
    glutAddMenuEntry("Remove Body", 4);

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
