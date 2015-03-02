#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "graphics.h"

	/* 2D drawing functions */
extern void  draw2Dline(int, int, int, int, int);
extern void  draw2Dbox(int, int, int, int);
extern void  draw2Dtriangle(int, int, int, int, int, int);
extern void  set2Dcolour(float []);

   /* projectile Information */
extern float projectile[10][10];  //dx, dy, velocity
extern float projNumber;

   /*Projectile Calculation Function*/
extern void nextProjLoc(float *, float *, float, float, int );
extern float nextProjHeight(float , float, float *);

	/* viewpoint control */
extern void getViewPosition(float *, float *, float *);

	/* size of the window in pixels */
extern int screenWidth, screenHeight;

/*Draws the map area and its boarder*/
void drawMapArea(int mX1, int mY1, int mX2, int mY2, int mSize, int spaceBuf) {
   int lineWidth = 5;   //Map boarder width
   
   /*Colour Variables*/
   GLfloat black[] = {0.0, 0.0, 0.0, 0.8};
   GLfloat grey[] = {0.0, 0.0, 0.0, 0.5};
      
   /*Draw lines to indicate the boundary of the map*/
   set2Dcolour(black);
   //draw2Dline(int x1, int y1, int x2, int y2, int lineWidth)
   draw2Dline(mX1 - lineWidth, mY1, mX1 + (mSize - spaceBuf + lineWidth), mY1, lineWidth);   //Top boarder
   draw2Dline(mX1 - lineWidth, mY2, mX1 + (mSize - spaceBuf + lineWidth), mY2, lineWidth);   //Bottom boarder
   
   draw2Dline(mX1 - lineWidth/2, mY1, mX1 - lineWidth/2, mY1 - (mSize - spaceBuf), lineWidth);   //Left boarder
   draw2Dline(mX2 + (lineWidth/2 + 1), mY1, mX2 + (lineWidth/2 + 1), mY1 - (mSize - spaceBuf), lineWidth);   //Right boarder
   
   /*Draw the map area**/
   set2Dcolour(grey);
   draw2Dbox(mX1, mY1, mX2, mY2); //draw2Dbox(int x1, int y1, int x2, int y2)
   //printf("mx1, my1, x2,y2 = %d, %d, %d, %d \n", mX1, mY1, mX2, mY2);
}

/*Draw the player to the map*/
void drawPlayerToMap(int mX, int mY, int mSize) {
   /*Player Variables*/
   int pSize = screenWidth * 0.01; //Player size
   float x, y, z;
   int pX, pY;
   
   /*Colour Variable*/
   GLfloat green[] = {0.0, 0.5, 0.0, 0.5};
      
   /*Get player's current position*/
   getViewPosition(&x, &y, &z);
   
   /*Convert the location to positive integer by:
    *1)Convert cord to a percentage
    *2)Multiple it by map size
    *3)convert it to a positive number
    */
   pX = (int)floor(z/100 * mSize) * -1;
   pY = (int)floor(x/100 * mSize) * -1;
   
   /*Convert location to map*/
   mX += pX;
   mY += pY;
   
   /*Create a 20x20 pixel square and center it to represent the player*/
   set2Dcolour(green);
   draw2Dbox(mX - pSize, mY - pSize, mX + pSize, mY + pSize); //int x1, int y1, int x2, int y2
}

/*Draw any projectiles to the map*/
void drawProjToMap(int mX, int mY, int mSize) {
   /*Player Variables*/
   int pSize = screenWidth * 0.005; //projectile size
   float x, y, z;
   int pX, pY;
   int newX, newY;
   int i;   //Loop counters
   
   /*Colour Variable*/
   GLfloat blue[] = {0.0, 0.0, 0.5, 0.8};
      
   /*Go through all the projectiles*/   
   for (i = 0; i < 10; i++) {
      /*Get current projectile position*/
      x = projectile[i][0];
      y = projectile[i][1];
      z = projectile[i][2];

      if (x >= 0 && y >= 0 && z >= 0) {
         /*Convert the location to positive integer by:
          *1)Convert cord to a percentage
          *2)Multiple it by map size
          *3)convert it to a positive number
          */
         pX = (int)floor(z/100 * mSize);
         pY = (int)floor(x/100 * mSize);
         
         /*Convert location to map*/
         newX = mX + pX;
         newY = mY + pY;
         
         /*Draw a square and center it to represent the projectile*/
         set2Dcolour(blue);
         draw2Dbox(newX - pSize, newY - pSize, newX + pSize, newY + pSize); //int x1, int y1, int x2, int y2
      }            
   }
}




