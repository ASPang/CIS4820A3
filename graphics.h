
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>

#include <time.h>

        /* world size and storage array */
#define WORLDX 100
#define WORLDY 50
#define WORLDZ 100
GLubyte  world[WORLDX][WORLDY][WORLDZ];

#define MAX_DISPLAY_LIST 500000

/*Functions for CIS4820 Assignment 1*/
int climbCube(int, int, int);
void gameWall();

void gravity();
void avoidCubeEdge();

void landscape();
void grassLand();
void waterFlow();
void mountainTops();
void perlinNoise();
void fillMountain(int, int, int);

void cloudFloat();
void cloudShape();
void createCloud(int, int, int, int *, int);
void moveCloud();
void updateCloudLoc();


/*Functions for CIS4820 Assignment 2*/
void worldOrientation();
void moveProjectile();
void initProjectiles();
void updateProjectiles();
void objectCollision();
void clearProjectile(int);
void nextProjLoc(float *, float *, float, float, int );
float nextProjHeight(float , float, float *);

float calSpeed(int, int);
float calAngle(int, int);
