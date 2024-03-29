#include <GL/glut.h>
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<algorithm>
#include<time.h>
#include<vector>
using namespace std;

#define pi (2*acos(0.0))

double const EPS = 1e-9;
int const NUMBER_OF_CIRCLE = 5;
double cameraHeight;
double cameraAngle;
int drawgrid;
int drawaxes;
int const GRID_LEN = 500;
double keySensitivity = 10, angleSensitivity = 3;

// constants about shapes
const int slice = 50, stack = 50;
double const BigRadius = 40, smallRadius = 20, tailRadius = 15, bulletRadius = 5;
double const MAX_ANGLE = 60, MIN_ANGLE = -60;
double cylinderHeight = 150;
double entireRotationAngle = 0, halfSphereRotationAngle = 0;
double gunAxisRotationAngle = 0, gunRotationAngle = 0;
const double PlaneX = 400, PlaneY = 250, PlaneZ = 250;
const double bulletWidth = 5, diff = 5;

struct Point{
    double x{},y{},z{};
    Point(){}
    Point(double _x, double _y, double _z):x(_x), y(_y), z(_z){}

    Point& operator+=(const Point &p) {
        x += p.x;
        y += p.y;
        z += p.z;
        return *this;
    }
    Point& operator-=(const Point &p) {
        x -= p.x;
        y -= p.y;
        z -= p.z;
        return *this;
    }
    Point& operator*=(const double s) {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }
    Point& operator/=(const double s) {
        x /= s;
        y /= s;
        z /= s;
        return *this;
    }
    Point operator+(const Point& p) const {
        return Point(*this) += p;
    }

    Point operator-(const Point& p) const {
        return Point(*this) -= p;
    }

    Point operator*(const double t) const {
        return Point(*this) *= t;
    }

    Point operator/(const double t) const {
        return Point(*this) /= t;
    }

};

Point u, l, r, pos, gunRoot, gunDir;
vector<Point> bullets;

double dot(Point &a, Point &b){
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

double norm(Point p){
    return sqrt(dot(p, p));
}

Point normalize(Point p){
    return p / norm(p);
}

Point cross(Point a, Point b) {
    return Point(a.y * b.z - a.z * b.y,
                   a.z * b.x - a.x * b.z,
                   a.x * b.y - a.y * b.x);
}

/**
 *
 * @param axis -> rotating axis
 * @param rotatingVector -> the vector that needs to rotate
 * @param angle -> in degrees
 * @return
 */
Point rotateAroundAnAxis(Point axis, Point rotatingVector, double angle){
    angle = pi / 180.0 * angle;
    return rotatingVector * cos(angle) +
    cross(axis, rotatingVector) * sin(angle) +
     axis * dot(axis, rotatingVector) * (1 - cos(angle));
}

struct Circle{
    Point centre;
    double radius;
    Circle(){}
    Circle(Point c, double r):centre(c), radius(r){}

    bool isInside(const Circle &rhs) const {
        double centerDistance = norm(centre - rhs.centre);
        return fabs(radius - rhs.radius) + EPS > centerDistance;
    }

    bool doesTouchFromInside(const Circle &rhs) const {      // rhs is the bigger circle
        double centerDistance = norm(centre - rhs.centre);
        return  centerDistance + radius + EPS > rhs.radius;
    }

    bool doesTouchFromOutside(const Circle &rhs) const {
        double centerDistance = norm(centre - rhs.centre);
        return radius + rhs.radius >= centerDistance;
    }

};

void drawAxes() {
    if (drawaxes == 1) {
        glColor3f(1.0, 1.0, 1.0);
        glBegin(GL_LINES);
        {
            glVertex3f(100, 0, 0);
            glVertex3f(-100, 0, 0);

            glVertex3f(0, -100, 0);
            glVertex3f(0, 100, 0);

            glVertex3f(0, 0, 100);
            glVertex3f(0, 0, -100);
        }
        glEnd();
    }
}

void drawLine(Point a, Point b){
    glBegin(GL_LINES);{
        glVertex3f( a.x,a.y,a.z);
        glVertex3f(b.x, b.y, b.z);
    }glEnd();
}

void drawCircle(double radius,int segments)
{
    int i;
    Point points[100];
    for(i=0;i<=segments;i++)
    {
        points[i].x=radius*cos(((double)i/(double)segments)*2*pi);
        points[i].y=radius*sin(((double)i/(double)segments)*2*pi);
    }
    //draw segments using generated points
    for(i=0;i<segments;i++)
    {
        glBegin(GL_LINES);
        {
            glVertex3f(points[i].x,points[i].y,0);
            glVertex3f(points[i+1].x,points[i+1].y,0);
        }
        glEnd();
    }
}


void drawRectangle(){
    glColor3f(0, 1.0, 0);
    drawLine(Point(0, 0, 0), Point(0, GRID_LEN, 0));
    drawLine(Point(0, 0, 0), Point(GRID_LEN, 0, 0));
    drawLine(Point(GRID_LEN, 0, 0), Point(GRID_LEN, GRID_LEN, 0));
    drawLine(Point(0, GRID_LEN, 0), Point(GRID_LEN, GRID_LEN, 0));

}

void drawSphere(double radius,int slices,int stacks)
{
    Point points[100][100];
    int i,j;
    double h,r;
    //generate points
    for(i=0;i<=stacks;i++)
    {
        h=radius*sin(((double)i/(double)stacks)*(pi/2));
        r=radius*cos(((double)i/(double)stacks)*(pi/2));
        for(j=0;j<=slices;j++)
        {
            points[i][j].x=r*cos(((double)j/(double)slices)*pi - pi/2);
            points[i][j].y=h;
            points[i][j].z=r*sin(((double)j/(double)slices)*pi - pi/2);
        }
    }
    //draw quads using generated points
    for(i=0;i<stacks;i++)
    {
        for(j=0;j<slices;j++)
        {
            glBegin(GL_QUADS);{
                //upper hemisphere
                glColor3f(j&1 , j&1, j&1);
                glVertex3f(points[i][j].x,points[i][j].y,points[i][j].z);
                glVertex3f(points[i][j+1].x,points[i][j+1].y,points[i][j+1].z);
                glVertex3f(points[i+1][j+1].x,points[i+1][j+1].y,points[i+1][j+1].z);
                glVertex3f(points[i+1][j].x,points[i+1][j].y,points[i+1][j].z);
                //lower hemisphere
                glColor3f(1-j&1 , 1-j&1, 1-j&1);
                glVertex3f(-points[i][j].x,points[i][j].y,points[i][j].z);
                glVertex3f(-points[i][j+1].x,points[i][j+1].y,points[i][j+1].z);
                glVertex3f(-points[i+1][j+1].x,points[i+1][j+1].y,points[i+1][j+1].z);
                glVertex3f(-points[i+1][j].x,points[i+1][j].y,points[i+1][j].z);
            }glEnd();
        }
    }
}


void drawCylinder(double h, double r){
   Point points[100][110];
   double newSlice = slice * 2;
   for (int i = 0; i <= stack; i++){
       for (int j = 0; j <= newSlice; j++){
           points[i][j].x = (double) i * h/stack;
           points[i][j].y = r * cos((double )j/(double )newSlice * 2* pi);
           points[i][j].z = r * sin((double )j/ (double )newSlice * 2 * pi);
       }
   }

   for (int i = 0; i < stack; i++){
       for (int j = 0; j < newSlice; j++){
           glBegin(GL_QUADS);{
               //upper hemisphere
               glColor3f(j&1 , j&1, j&1);
               glVertex3f(points[i][j].x,points[i][j].y,points[i][j].z);
               glVertex3f(points[i][j+1].x,points[i][j+1].y,points[i][j+1].z);
               glVertex3f(points[i+1][j+1].x,points[i+1][j+1].y,points[i+1][j+1].z);
               glVertex3f(points[i+1][j].x,points[i+1][j].y,points[i+1][j].z);
           }glEnd();

       }
   }
}

void drawTail(){
    Point points[110][110];
    double newSlice = slice * 2;
    for (int i = 0; i <= stack; i++){
        double angle = (double )i / stack * pi / 2;
        double r = smallRadius + tailRadius - tailRadius * cos(angle);

        for (int j = 0; j <= newSlice; j++){
            points[i][j].x = (double)tailRadius * sin(angle);
            points[i][j].y = r * cos((double )j/(double )newSlice * 2* pi);
            points[i][j].z = r * sin((double )j/ (double )newSlice * 2 * pi);
        }
    }

    for (int i = 0; i < stack; i++){
        for (int j = 0; j < newSlice; j++){
            glBegin(GL_QUADS);{
                //upper hemisphere
                glColor3f(j&1 , j&1, j&1);
                glVertex3f(points[i][j].x,points[i][j].y,points[i][j].z);
                glVertex3f(points[i][j+1].x,points[i][j+1].y,points[i][j+1].z);
                glVertex3f(points[i+1][j+1].x,points[i+1][j+1].y,points[i+1][j+1].z);
                glVertex3f(points[i+1][j].x,points[i+1][j].y,points[i+1][j].z);
            }glEnd();

        }
    }
}

void positionBigSemiSphereLeft(){
    glPushMatrix();
    glRotatef(90, 0, 0, 1);
    //glRotatef(-180, 1, 0, 0);
    drawSphere(BigRadius, slice,  stack);
    glPopMatrix();
}

void positionBigSemiSphereRight(){
    glPushMatrix();
    glRotatef(90, 1, 0, 0);
    glRotatef(-90, 0, 0, 1);
    drawSphere(BigRadius, slice, stack);
    glPopMatrix();
}

void positionSmallSemiSphere(){
    glPushMatrix();
    glTranslatef(BigRadius + smallRadius, 0, 0);
    glRotatef(90, 0, 0, 1);
    drawSphere(smallRadius, slice, stack);
    glPopMatrix();
}

void positionCylinder(){
    glPushMatrix();
    glTranslatef(BigRadius + smallRadius, 0, 0);
    glRotatef(90, 1, 0, 0);
    drawCylinder(cylinderHeight, smallRadius);
    glPopMatrix();
}

void positionTail(){
    glPushMatrix();
    glTranslatef(BigRadius + smallRadius + cylinderHeight, 0, 0);
    glRotatef(90, 1, 0, 0);
    drawTail();
    glPopMatrix();
}

void positionPlane(){
    glPushMatrix();
    //glTranslatef(PlaneX, 0, 0);

    glBegin(GL_QUADS);{
        glColor3f(0.8, 0.8, 0.8);
        glVertex3f(PlaneX, PlaneY / 2, PlaneZ / 2);
        glVertex3f(PlaneX, -PlaneY / 2, PlaneZ / 2);
        glVertex3f(PlaneX, -PlaneY / 2, -PlaneZ / 2);
        glVertex3f(PlaneX, PlaneY / 2, -PlaneZ / 2);
    }glEnd();

    glPopMatrix();
}

Point findTail(Point lineVector, double z_angle, double y_angle){

    Point tmp = rotateAroundAnAxis(Point(0, 0, 1), lineVector, z_angle);
    tmp = rotateAroundAnAxis(Point(0, 1, 0), tmp, y_angle);
    return tmp;
}

void shotFire(){
    double y_angle = entireRotationAngle;
    double z_angle = gunRotationAngle + halfSphereRotationAngle;

    Point root = findTail(gunRoot,halfSphereRotationAngle,entireRotationAngle);
    Point other = findTail(gunDir, z_angle, y_angle);

    double t = (PlaneX - 5 - root.x)/ other.x;

    Point p = root + other * t;
    if (p.y >= -PlaneY/2 && p.y <= PlaneY/2 && p.z >= -PlaneZ/2 && p.z <= PlaneZ/2)
        bullets.push_back(p);
}

void drawScreen(){
    drawAxes();
    //drawRectangle()
    glPushMatrix();

    glRotatef(entireRotationAngle, 0, 1, 0);

    positionBigSemiSphereLeft();

    glRotatef(halfSphereRotationAngle, 0, 0, 1);

    positionBigSemiSphereRight();

    glTranslatef(+BigRadius, 0, 0);   // undo the translation done below
    glRotatef(gunRotationAngle, 0, 0, 1);  // apply the rotation on the origin
    glRotatef(gunAxisRotationAngle, 1, 0, 0);
    glTranslatef(-BigRadius, 0, 0);   // bring the top point of the semi-sphere to origin

    positionSmallSemiSphere();
    positionCylinder();
    positionTail();

    glPopMatrix();

    double y_angle = entireRotationAngle;
    double z_angle = gunRotationAngle + halfSphereRotationAngle;

    Point root = findTail(gunRoot,halfSphereRotationAngle,entireRotationAngle);
    Point other = findTail(gunDir, z_angle, y_angle);

    glColor3f(0, 0, 1);
    drawLine(root, root + other*400);

    positionPlane();

    glColor3f(1, 0, 0);
    for (Point p: bullets){

        glBegin(GL_QUADS);{
            glVertex3f(p.x,p.y +  bulletWidth / 2, p.z + bulletWidth / 2);
            glVertex3f(p.x,p.y -bulletWidth / 2, p.z + bulletWidth / 2);
            glVertex3f(p.x,p.y -bulletWidth / 2, p.z  - bulletWidth / 2);
            glVertex3f(p.x,p.y +  bulletWidth / 2, p.z  - bulletWidth / 2);
        }glEnd();
    }

}

void keyboardListener(unsigned char key, int x,int y){
    switch(key){
        case '1':
            l = rotateAroundAnAxis(u, l, angleSensitivity);
            r = rotateAroundAnAxis(u, r, angleSensitivity);
            break;
        case '2':
            l = rotateAroundAnAxis(u, l, -angleSensitivity);
            r = rotateAroundAnAxis(u, r, -angleSensitivity);
            break;
        case '3':
            l = rotateAroundAnAxis(r, l, angleSensitivity);
            u = rotateAroundAnAxis(r, u, angleSensitivity);
            break;
        case '4':
            l = rotateAroundAnAxis(r, l, -angleSensitivity);
            u = rotateAroundAnAxis(r, u, -angleSensitivity);
            break;
        case '5':
            r = rotateAroundAnAxis(l, r, angleSensitivity);
            u = rotateAroundAnAxis(l, u, angleSensitivity);
            break;
        case '6':
            r = rotateAroundAnAxis(l, r, -angleSensitivity);
            u = rotateAroundAnAxis(l, u, -angleSensitivity);
            break;
        case 'q':
            entireRotationAngle  += angleSensitivity;
            entireRotationAngle = min(entireRotationAngle, MAX_ANGLE);
            break;
        case 'w':
            entireRotationAngle -= angleSensitivity;
            entireRotationAngle = max(entireRotationAngle, MIN_ANGLE);
            break;
        case 'e':
            halfSphereRotationAngle += angleSensitivity;
            halfSphereRotationAngle = min(halfSphereRotationAngle, MAX_ANGLE);
            break;
        case 'r':
            halfSphereRotationAngle -= angleSensitivity;
            halfSphereRotationAngle = max(halfSphereRotationAngle, MIN_ANGLE);
            break;
        case 'a':
            gunRotationAngle += angleSensitivity;
            gunRotationAngle = min(gunRotationAngle, MAX_ANGLE);
            break;
        case 's':
            gunRotationAngle -= angleSensitivity;
            gunRotationAngle = max(gunRotationAngle, MIN_ANGLE);
            break;
        case 'd':
            gunAxisRotationAngle += angleSensitivity;
            if (gunAxisRotationAngle > 360)
                gunAxisRotationAngle -= 360;
            break;
        case 'f':
            gunAxisRotationAngle -= angleSensitivity;
            if (gunAxisRotationAngle < -360)
                gunAxisRotationAngle += 360;

            break;
        default:
            break;
    }
}


void specialKeyListener(int key, int x,int y){
    switch(key){
        case GLUT_KEY_DOWN:		//down arrow key
            pos -= l * keySensitivity;
            break;
        case GLUT_KEY_UP:		// up arrow key
            pos += l * keySensitivity;
            break;
        case GLUT_KEY_RIGHT:
            pos += r * keySensitivity;
            break;
        case GLUT_KEY_LEFT:
            pos -= r * keySensitivity;
            break;
        case GLUT_KEY_PAGE_UP:
            pos += u * keySensitivity;
            break;
        case GLUT_KEY_PAGE_DOWN:
            pos -= u * keySensitivity;
            break;
        case GLUT_KEY_INSERT:
            break;
        case GLUT_KEY_HOME:
            break;
        case GLUT_KEY_END:
            break;
        default:
            break;
    }
}


void mouseListener(int button, int state, int x, int y){	//x, y is the x-y of the screen (2D)
    switch(button){
        case GLUT_LEFT_BUTTON:
            if(state == GLUT_DOWN){		// 2 times?? in ONE click? -- solution is checking DOWN or UP
                shotFire();
            }
            break;
        case GLUT_RIGHT_BUTTON:
            if(state == GLUT_DOWN){
                drawaxes=1-drawaxes;
            }
            break;

        case GLUT_MIDDLE_BUTTON:
            //........
            break;

        default:
            break;
    }
}


void display(){

    //clear the display
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0,0,0,0);	//color black
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /********************
    / set-up camera here
    ********************/
    //load the correct matrix -- MODEL-VIEW matrix
    glMatrixMode(GL_MODELVIEW);

    //initialize the matrix
    glLoadIdentity();

    //now give three info
    //1. where is the camera (viewer)?
    //2. where is the camera looking?
    //3. Which direction is the camera's UP direction?

    //gluLookAt(100,100,100,	0,0,0,	0,0,1);
    //gluLookAt(200*cos(cameraAngle), 200*sin(cameraAngle), cameraHeight,		0,0,0,		0,0,1);
    Point lookAt = pos + l;

    gluLookAt(pos.x, pos.y, pos.z,
              lookAt.x, lookAt.y, lookAt.z,
              u.x, u.y, u.z);


    //again select MODEL-VIEW
    glMatrixMode(GL_MODELVIEW);


    /****************************
    / Add your objects from here
    ****************************/
    drawScreen();

    //ADD this line in the end --- if you use double buffer (i.e. GL_DOUBLE)
    glutSwapBuffers();
}


void animate(){
   glutPostRedisplay();
}

void init(){
    //codes for initialization
    drawgrid=0;
    drawaxes=1;
    pos = Point(0, 0, 200);
    u = Point(0, 1, 0);
    l = Point(0, 0, -1);
    r = Point(1, 0, 0);

    gunRoot = Point(BigRadius, 0, 0);
    gunDir = Point(1, 0, 0);

    //clear the screen
    glClearColor(0,0,0,0);

    /************************
    / set-up projection here
    ************************/
    //load the PROJECTION matrix
    glMatrixMode(GL_PROJECTION);

    //initialize the matrix
    glLoadIdentity();

    //give PERSPECTIVE parameters
    gluPerspective(80,	1,	1,	1000.0);
    //field of view in the Y (vertically)
    //aspect ratio that determines the field of view in the X direction (horizontally)
    //near distance
    //far distance
}

int main(int argc, char **argv){
    //srand(time(0));
    glutInit(&argc,argv);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(300, 250);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);	//Depth, Double buffer, RGB color

    glutCreateWindow("Task-1");

    init();

    glEnable(GL_DEPTH_TEST);	//enable Depth Testing

    glutDisplayFunc(display);	//display callback function
    glutIdleFunc(animate);		//what you want to do in the idle time (when no drawing is occurring)

    glutKeyboardFunc(keyboardListener);
    glutSpecialFunc(specialKeyListener);
    glutMouseFunc(mouseListener);

    glutMainLoop();		//The main loop of OpenGL

    return 0;
}