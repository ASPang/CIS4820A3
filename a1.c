
/* Derived from scene.c in the The OpenGL Programming Guide */
/* Keyboard and mouse rotation taken from Swiftless Tutorials #23 Part 2 */
/* http://www.swiftless.com/tutorials/opengl/camera2.html */

/* Frames per second code taken from : */
/* http://www.lighthouse3d.com/opengl/glut/index.php?fps */

/* Perlin Noise taken from : */
/* REFERENCE: http://www.angelcode.com/dev/perlin/perlin.html */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "graphics.h"

    /* Program Mode */
char* gameMode;    //The game can be of one of two different states: -server or -client

    /* projectile Information */
float projectile[10][10];  //dx, dy, velocity
float projNumber=0;

    /* Landscape seed */
int landSeed = 580789;

	/* mouse function called by GLUT when a button is pressed or released */
void mouse(int, int, int, int);

	/* initialize graphics library */
extern void graphicsInit(int *, char **);

	/* lighting control */
extern void setLightPosition(GLfloat, GLfloat, GLfloat);
extern GLfloat* getLightPosition();

	/* viewpoint control */
extern void setViewPosition(float, float, float);
extern void getViewPosition(float *, float *, float *);
extern void getOldViewPosition(float *, float *, float *);
extern void setViewOrientation(float, float, float);
extern void getViewOrientation(float *, float *, float *);

	/* add cube to display list so it will be drawn */
extern int addDisplayList(int, int, int);

	/* mob controls */
extern void createMob(int, float, float, float, float);
extern void setMobPosition(int, float, float, float, float);
extern void hideMob(int);
extern void showMob(int);

	/* player controls */
extern void createPlayer(int, float, float, float, float);
extern void setPlayerPosition(int, float, float, float, float);
extern void hidePlayer(int);
extern void showPlayer(int);

	/* 2D drawing functions */
extern void  draw2Dline(int, int, int, int, int);
extern void  draw2Dbox(int, int, int, int);
extern void  draw2Dtriangle(int, int, int, int, int, int);
extern void  set2Dcolour(float []);


	/* flag which is set to 1 when flying behaviour is desired */
extern int flycontrol;
	/* flag used to indicate that the test world should be used */
extern int testWorld;
	/* flag to print out frames per second */
extern int fps;
	/* flag to indicate removal of cube the viewer is facing */
extern int dig;
	/* flag indicates the program is a client when set = 1 */
extern int netClient;
	/* flag indicates the program is a server when set = 1 */
extern int netServer; 
	/* size of the window in pixels */
extern int screenWidth, screenHeight;
	/* flag indicates if map is to be printed */
extern int displayMap;

	/* frustum corner coordinates, used for visibility determination  */
extern float corners[4][3];

	/* determine which cubes are visible e.g. in view frustum */
extern void ExtractFrustum();
extern void tree(float, float, float, float, float, float, int);

/********* end of extern variable declarations **************/


	/*** collisionResponse() ***/
	/* -performs collision detection and response */
	/*  sets new xyz  to position of the viewpoint after collision */
	/* -can also be used to implement gravity by updating y position of vp*/
	/* note that the world coordinates returned from getViewPosition()
	   will be the negative value of the array indices */
void collisionResponse() {
   int i;  //Loop counter
   float x, y, z; //Viewpoint coordinates
   float oldX, oldY, oldZ; //Old viewpoint coordinates
   float spaceBuffer = -0.2, spaceBuffer2 = 0.5;   //VP buffer space
   
   int vpLocValueX, vpLocValueZ; //tenth digit in the decimal number for the view position
   int objX, objY, objZ; //Object coordinate check #1
   int objX2 = 0, objY2 = 0, objZ2 = 0; //Object coordinate check #2
   
   int collisionFlag = 0; 

   /* your collision code goes here */
   gameWall(); //Determine if the player is at the edge of the world

   /*Convert location to an integer*/
   getViewPosition(&x, &y, &z);

   objX = (int)(x - spaceBuffer) * -1;
   objY = (int)(y - spaceBuffer) * -1;
   objZ = (int)(z - spaceBuffer) * -1;

   objX2 = (int)(x) * (- 1);
   objY2 = (int)(y) * (- 1);
   objZ2 = (int)(z) * (- 1);
   
   /*Get the first decimal place for x and z*/
   vpLocValueX = ( (int)(floor( fabs( x ) * 10 ) ) ) % 10;
   vpLocValueZ = ( (int)(floor( fabs( z ) * 10 ) ) ) % 10;

   /*Determine if the view points is below WORLDY height value*/
   if (objY < (WORLDY-1)) {
      /*Determine if there's a cube in front of the vp*/
      if (world[objX][(int)y*(-1)][objZ] != 0) { 
         /*Check the height of the cube wall*/
         if (climbCube(objX, objY, objZ) == 1) {
            /*Climb up the cube*/
            setViewPosition(x, (y) - 1, z);
         }
         else {
            /*Don't move*/
            getOldViewPosition(&oldX, &oldY, &oldZ);
            setViewPosition(oldX, oldY, oldZ);
         }
      }
      
      /*Determine if current position you're inside a cube*/
      if (vpLocValueX >= 7) {
         objX += 1;
         objX2 += 1;
      }
      
      if (vpLocValueZ >= 7) {
         objZ += 1;
         objZ2 += 1;
      }
      
      /*Determine if there's a cube in front of the vp*/
      if (world[objX2][(int)y*(-1)][objZ2] != 0 || world[objX][(int)y*(-1)][objZ] != 0) {
         if (climbCube(objX, objY, objZ) == 1) {
            /*Climb up the cube*/
            setViewPosition(x-0.01, (y) - 1.2, z);
         }
         else {
            /*Don't move*/
            getOldViewPosition(&oldX, &oldY, &oldZ);
            setViewPosition(oldX, oldY, oldZ);
         }
      }
   }  
}

/*Determine if there's a cube stacked on top*/
int climbCube(int cX, int cY, int cZ) {
   int cubeTop;
   float x, y, z;

   getViewPosition(&x, &y, &z);  //Get the current VP position
   cubeTop = world[cX][(int)(y-1)*(-1)][cZ]; //Get the block information in front of VP

   /*There's no cube at the top*/
   if (cubeTop == 0) {
      return 1;
   }
   
   /*There's a cube stacked on top*/
   return 0;
}

/*Prevents the user from going through the edge of the game wall (north, south, east, west)*/
void gameWall() {
   float x, y, z; //Viewpoint coordinates
   float spaceBuffer = 0.5;   //VP buffer space
   float xMax, yMax, zMax, min;  //Game wall boarder

   /*Calculate the game wall with 0.2 space buffer and covert it to negative value*/
   xMax = -(WORLDX - spaceBuffer);
   zMax = -(WORLDZ - spaceBuffer);
   yMax = -(WORLDY - spaceBuffer);
   min = -spaceBuffer;

   /*Get the VP current location*/
   getViewPosition(&x, &y, &z);

   /*Determine if VP is at the game wall*/
   if (x >= min) {
      setViewPosition(min, y, z);
   }
   else if (x <= xMax) {
      setViewPosition(xMax, y, z);
   }
   else if (z >= min) {
      setViewPosition(x, y, min);
   }
   else if (z <= zMax) {
      setViewPosition(x, y, zMax);
   }
   else if (y <= yMax) {
       setViewPosition(x, yMax, z);
   }
   else if (y >= min) {
       setViewPosition(x, min, z);
   }
}


	/******* draw2D() *******/
	/* draws 2D shapes on screen */
	/* use the following functions: 			*/
	/*	draw2Dline(int, int, int, int, int);		*/
	/*	draw2Dbox(int, int, int, int);			*/
	/*	draw2Dtriangle(int, int, int, int, int, int);	*/
	/*	set2Dcolour(float []); 				*/
	/* colour must be set before other functions are called	*/
void draw2D() {

   if (testWorld) {
		/* draw some sample 2d shapes */
      GLfloat green[] = {0.0, 0.5, 0.0, 0.5};
      set2Dcolour(green);
      draw2Dline(0, 0, 500, 500, 15);
      draw2Dtriangle(0, 0, 200, 200, 0, 200);

      GLfloat black[] = {0.0, 0.0, 0.0, 0.5};
      set2Dcolour(black);
      draw2Dbox(500, 380, 524, 388);
   } else {

	/* your code goes here */

   }

}


/* 
 * Calculate the amount of time that have passed
 * 1 = true
 * 0 = false
 */
/*
 * References: http://stackoverflow.com/questions/3756323/getting-the-current-time-in-milliseconds
 */
int checkUpdateTime() {
    static clock_t updateStart;
    clock_t updateEnd;
    static int resetTime = 1;
    int milsec = 10000; //Milliseconds;
    double diff;
    
    struct timeval  tv;
    double time_in_mill;
    
    gettimeofday(&tv, NULL);
    
    /*Determine if the timer has been set*/
    if (resetTime == 1) {
        /*Reset the timer*/
        resetTime = 0;
        
        time_in_mill = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ; // convert tv_sec & tv_usec to millisecond
        
        updateStart = time_in_mill;
    }
    else if (resetTime == 0) {
        /*Determine if 0.08 second has passed*/        
        time_in_mill = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ; // convert tv_sec & tv_usec to millisecond
        
        updateEnd = time_in_mill;
        diff = ((updateEnd - updateStart));
        
        if (diff >= 80) {
            resetTime = 1;  //Reset the timer
            return 1;    //Return true that 1 second has passed
        }
    }
    
    
    return 0;   //Don't update the function
}

	/*** update() ***/
	/* background process, it is called when there are no other events */
	/* -used to control animations and perform calculations while the  */
	/*  system is running */
	/* -gravity must also implemented here, duplicate collisionResponse */
void update() {
   int i, j, k;
   float *la;

   if (checkUpdateTime()) {
      /* sample animation for the test world, don't remove this code */
      /* -demo of animating mobs */
      if (testWorld) {
      /* sample of rotation and positioning of mob */
      /* coordinates for mob 0 */
         static float mob0x = 50.0, mob0y = 25.0, mob0z = 52.0;
         static float mob0ry = 0.0;
         static int increasingmob0 = 1;
      /* coordinates for mob 1 */
         static float mob1x = 50.0, mob1y = 25.0, mob1z = 52.0;
         static float mob1ry = 0.0;
         static int increasingmob1 = 1;

      /* move mob 0 and rotate */
      /* set mob 0 position */
         setMobPosition(0, mob0x, mob0y, mob0z, mob0ry);

      /* move mob 0 in the x axis */
          if (increasingmob0 == 1){
            mob0x += 0.2;
            mob0z += 0.2; }
         else  {
            mob0x -= 0.2;
             mob0z -= 0.2;}
         if (mob0x > 50) increasingmob0 = 0;
         if (mob0x < 30) increasingmob0 = 1;

      /* rotate mob 0 around the y axis */
         mob0ry += 1.0;
         if (mob0ry > 360.0) mob0ry -= 360.0;

      /* move mob 1 and rotate */
         setMobPosition(1, mob1x, mob1y, mob1z, mob1ry);

      /* move mob 1 in the z axis */
      /* when mob is moving away it is visible, when moving back it */
      /* is hidden */
         if (increasingmob1 == 1) {
            mob1z += 0.2;
            showMob(1);
         } else {
            mob1z -= 0.2;
            hideMob(1);
         }
         if (mob1z > 72) increasingmob1 = 0;
         if (mob1z < 52) increasingmob1 = 1;

      /* rotate mob 1 around the y axis */
         mob1ry += 1.0;
         if (mob1ry > 360.0) mob1ry -= 360.0;
       /* end testworld animation */
      } else {

      /* your code goes here */
         /*Determine if the fly control is on or off*/
         if (flycontrol == 0) {
            gravity();
         } 

         /*Move the clouds*/
         moveCloud();
          
         /*Update the projectile*/
          moveProjectile();
          objectCollision();
          
         /*Determine if the program is a server or client*/
          if (strcmp(gameMode,"-server") == 0) {
              /*Write to socket*/
              writeSocket();
          }
          else if(strcmp(gameMode,"-client") == 0) {
              /*Write to socket*/
              readSocket();
          }
         
      }
   }
}

/**/
/**/
/**/
/**/
/**/
/**/
/**/
/**/

/*Server Socket Variable*/
int server_sockfd, client_sockfd;
int server_len, client_len;
struct sockaddr_in server_address;
struct sockaddr_in client_address;

/*Client Socket Variable*/
int sockfd;
int len;
struct sockaddr_in address;

/*Open the socket as a server*/
void openSocketServer() {
    /*  Remove any old socket and create an unnamed socket for the server.  */
    
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    /*  Name the socket.  */
    
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(9734);
    server_len = sizeof(server_address);
    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
    
    /*  Create a connection queue and wait for clients.  */
    
    listen(server_sockfd, 5);
    
    /*  Accept a connection.  */
    
    client_len = sizeof(client_address);
    client_sockfd = accept(server_sockfd,(struct sockaddr *)&client_address, &client_len);
    
    //close(client_sockfd); //TESTING!!!!! - Should be moved to when the user hits the "q" key
}

/*Open the socket as a client*/
void openSocketClient() {
    int result;
    //char ch = 'A';
    
    /*  Create a socket for the client.  */
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    /*  Name the socket, as agreed with the server.  */
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(9734);
    len = sizeof(address);
    
    /*  Now connect our socket to the server's socket.  */
    
    result = connect(sockfd, (struct sockaddr *)&address, len);
    
    if(result == -1) {
        perror("oops: client3");
        exit(1);
    }
}

/*Write information to socket that is sent to the client*/
void writeSocket() {
    char ch = 'A';
    
    
    /*  Create a connection queue and wait for clients.  */
    
    //listen(server_sockfd, 5);
   
    /*  Accept a connection.  */
    //client_len = sizeof(client_address);
    //printf("client_len = %d \n", client_len);
    //client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);
    //read(client_sockfd, &ch, 1);
    

    /*Determine if this is the first time client has connected to the server*/
        /*Send the perlin noise seed to the client*/
    
    /*Send the server's current position*/
    sendViewPos();
    
    /*Send servers' current view orientation*/
    sendViewOrient();
    
    /*write to client*/
    //write(client_sockfd, &ch, 1);
    
   
    
    /*Send server's orientation position - will be done last since this only happens when the mouse moves*/
    
    /*Send any projectile information - again will be done last since this only happens when the mouse moves*/
    
    
    //printf("write a character to client %c\n", ch);
}

/*Send the server's current position*/
void sendViewPos() {
    char ch = 'P';
    float x, y, z;
    int cordLen = 10;
    int msgLen = 34;
    char strX[cordLen], strY[cordLen], strZ[cordLen], msgStr[msgLen];
    
    /*Inform the client that information being sent is viewPosition*/
    write(client_sockfd, &ch, 1);
    //write(client_sockfd, '\0', 1);
    
    /*Send server's current position*/
    getViewPosition(&x, &y, &z);
    
    /*Convert to a positive number*/
    x = x * -1;
    y = y * -1;
    z = z * -1;
    
    //printf("x,y,z = %f,%f,%f \n", x, y, z);
    
    /*Convert coordinates to a string*/
    sprintf(strX, "%f", x);    //Convert integer to a string
    sprintf(strY, "%f", y);    //Convert integer to a string
    sprintf(strZ, "%f", z);    //Convert integer to a string
    
    //printf("strX %s\n", strX);
    
    /*Convert the three values into two digit string numbers - example 2 = "02" */
    convertPosNumDigit(strX);
    convertPosNumDigit(strY);
    convertPosNumDigit(strZ);
    
    /*Concate the message*/
    strcpy(msgStr,""); //Set up message
    strcat(msgStr, strX);
    strcat(msgStr, ",");
    strcat(msgStr, strY);
    strcat(msgStr, ",");
    strcat(msgStr, strZ);
    
    /*Send the message to client*/
    write(client_sockfd, &msgStr, msgLen);
    
    printf("msgStr sent %s\n", msgStr);
}

/*Send the server's current view orientation*/
void sendViewOrient() {
    char ch = 'O';
    float x, y, z;
    int cordLen = 10;
    int msgLen = 26;
    char strX[cordLen], strY[cordLen], msgStr[msgLen];
    
    /*Inform the client that information being sent is viewPosition*/
    write(client_sockfd, &ch, 1);
    
    /*Send server's current position*/
    getViewOrientation(&x, &y, &z);
    
    printf("x,y,z = %f,%f,%f \n", x, y, z);
    
    /*Convert coordinates to a string*/
    sprintf(strX, "%f", x);    //Convert integer to a string
    sprintf(strY, "%f", y);    //Convert integer to a string
    
    //printf("strX %s\n", strX);
    
    /*Convert the three values into two digit string numbers - example 2 = "02" */
    convertOrientNumDigit(strX);
    convertOrientNumDigit(strY);
    
    /*Concate the message*/
    strcpy(msgStr,""); //Set up message
    strcat(msgStr, strX);
    strcat(msgStr, ",");
    strcat(msgStr, strY);    
    
    /*Send message to client*/
    write(client_sockfd, &msgStr, msgLen);
    printf("ori - msgStr sent %s\n", msgStr);
}

/*Convert the three values into two digit string numbers - example 2 = "02" */
void convertPosNumDigit(char * str) {
    int strLen = 10;
    int i = 0;  //Loop counter
    char newStr[strLen];
    
    /*Look at the 3 character (position 2) of the string. Determine if it's a decimal*/
    if (str[2] != '.') {
        /*Add zero at the front*/
        for (i = 0; i < strLen; i++) {
            if (i == 0) {
                newStr[i] = '0';
            }
            else {
                newStr[i] = str[i-1];
            }
        }
        
        /*Replace old string with the new one*/
        strcpy(str, newStr);
    }    
}

/*Converts the orientation to 6 digits*/
void convertOrientNumDigit(char * str) {
    int strLen = 12;
    int i = 0, numZero = 0;  //Loop counter
    char newStr[strLen];
    
    /*Look at the 3 character (position 2) of the string. Determine if it's a decimal*/
    if (str[4] != '.') {
        /*Determine how many zeros to add to the front*/
        for (i = 0; i < 4; i++) {
            if (str[i] != '.') {
               numZero++;
            }
            else {
               break;
            }
        }
        
        /*Add zero at the front*/
        for (i = 0; i < strLen; i++) {
            if (i < numZero) {
                newStr[i] = '0';
            }
            else {
                newStr[i] = str[i-numZero];
            }
        }
        
        /*Replace old string with the new one*/
        strcpy(str, newStr);
    }    
}

/*Read the message from the socket sent by the server*/
void readSocket() {
    char ch = 'A';

    read(sockfd, &ch, 1);
    
    if (ch == 'P') {
      int msgLen = 34;
      char msg[msgLen];
      printf("read a character from server %c\n", ch);
      /*Get the message*/
      read(sockfd, &msg, msgLen);
        printf("---msgfrom server %s\n", msg);
      /*Set position to be same as server*/
      parseViewPos(msg);
        
      
    }
    else if (ch == 'O') {
        int msgLen = 26;    //msg = aaaa.bbbbbb,xxxx.yyyyyy
        char msg[msgLen];
        printf("read a character from server %c\n", ch);
        /*Get the message*/
        read(sockfd, &msg, msgLen);
        printf("---msgfrom server %s\n", msg);
        
    }
    else {
    
        printf("in else - read a character from server %c\n", ch);
    }
    
    
    /*Get the server current position*/
    /*Convert to a negative number*/

}

/*Parse the view position sent from the server*/
void parseViewPos(char *msg) {
    //printf("parseViewPos = %s\n", msg);
    int x = 0, y = 0, z = 0;
    int numMsg = 3;
    int msgLen = 10;
    
    //char msgSplit[numMsg][msgLen];
    char ** msgSplit;
    
    /*Parse the message to the three coordinates*/
    msgSplit = splitNumMsgInfo(msg, numMsg, msgLen);
    
    /*TESTING
    int i = 0;
    for (i = 0; i < numMsg; i++) {
        //printf("msgSplit[i] = %s  \n", msgSplit[i]);
    }*/
        
    /*Convert string to an integer*/
    x = atoi(msgSplit[0]) * -1;
    y = atoi(msgSplit[1]) * -1;
    z = atoi(msgSplit[2]) * -1;
    
    printf("x,y,z = %d, %d, %d \n", x, y, z);
    
    /*Set the client location based on the server's*/
    setViewPosition(x, y, z);
    
    
    free(*msgSplit);
    free(msgSplit);
}

/*Split the message to coordinate*/
char ** splitNumMsgInfo(char * msg, int numMsg, int msgLen) {
   char** tokens;
   int i = 0;    //loop counters
   char delim[2] = ",";
    
    //printf("msgLen = %d \n", numMsg);
    
    
    char ** msgSplit;
    msgSplit = (char**)malloc(msgLen * sizeof(char**));
    
    //char msgSplit[numMsg][msgLen];
    
    //printf("HERE\n");
    
    char* token = strtok(msg, delim);
    //msgSplit[0] = (char*)malloc(msgLen*sizeof(token) + 10);
    //printf("first token = %s  \n ", token);
    
    /*Go through all the tokens*/
    while (token != NULL) {
        if (token != NULL) {
            //printf("--- token = %s (size:%d), ", token, (int)sizeof(token)+2);
            msgSplit[i] = (char*)malloc(sizeof(char*) * sizeof(token) + 2);
            strcpy(msgSplit[i], token);    //Save the message
            
            i++;
        }
        
        token = strtok(NULL, delim);
        
    }
    //printf("---END\n");
    
    return msgSplit;    
}


/**/
/**/
/**/
/**/
/**/
/**/
/**/

/*Pulls the view point(camera) down the gameworld*/
void gravity() {
   float x, y, z;   //Viewpoints 0=x,1=y,2=z    
   float oldX, oldY, oldZ; //Old viewpoint coordinates
   float spaceBuffer = -0.2;   //VP buffer space
   int objX, objY, objZ; //Object coordinate check #1

   /*Convert location to an integer*/
   getViewPosition(&x, &y, &z);

   objX = (int)(x - spaceBuffer) * -1;
   objY = (int)(y - spaceBuffer) * -1;
   objZ = (int)(z - spaceBuffer) * -1;
   
   /*Determine if there's an object based on vp current position*/
   if ((world[objX][objY][objZ] == 0 || y < ((WORLDY-1)*(-1))) && (objY > 0)) {
      avoidCubeEdge();
   }
}

/*Determine which way the user is jumping off the edge of a cube*/
void avoidCubeEdge() {
   float x, y, z;   //New Viewpoint coordinate
   float oldX, oldY, oldZ; //Old viewpoint coordinates
   float diffX, diffY, diffZ;   //Stores the difference of the old and new coordinate

   int objX, objY, objZ; //Object coordinate check #1
   int vpLocValueX, vpLocValueZ;   //Decimal place of vp position

   /*Convert location to an integer*/
   getViewPosition(&x, &y, &z);
   getOldViewPosition(&oldX, &oldY, &oldZ);

   /*Determine if VP is at the very edge of the cube*/   
   vpLocValueX = ( (int)(floor( fabs( x ) * 10 ) ) ) % 10;
   vpLocValueZ = ( (int)(floor( fabs( z ) * 10 ) ) ) % 10;
   
   /*Update the X axis based on the direction the user is going*/
   if (vpLocValueX < 3 || vpLocValueX > 6) {
      diffX = oldX - x;
        
      /*User is heading East*/
      if (diffX > 0) {
         x -= 0.2;   
      }
      /*User is heading West*/
      else if (diffX <0) {
         x += 0.2;
      }
   }

   /*Update the Y axis based on the direction the user is going*/
   if (vpLocValueZ < 3 || vpLocValueZ > 6) {
      diffZ = oldZ - z;
      
      /*User is heading North*/      
      if (diffZ > 0) {
         z -= 0.2;   
      }
      /*User is heading South*/      
      else if (diffZ < 0) {
         z += 0.2;
      }
   }

   /*Pull the player down the world by 0.1 unit*/
   setViewPosition(x, y + 0.2, z);
}

/*UPdates the projectile overtime*/
void moveProjectile() {
    updateProjectiles();
}

/*Show all the current projectils on the screen and upate their position*/
void updateProjectiles() {
    int i;
    float dx, dz, dy, angleX, angleY;
    float xPos, yPos, zPos;
    float speed, height, gravity;
    
    for (i = 0; i < 10; i++) {
        /*Get current projectile position*/
        xPos = projectile[i][0];
        yPos = projectile[i][1];
        zPos = projectile[i][2];
        
        if (xPos >= 0) {
            /*Update mob position - height*/        
            angleY = projectile[i][7];
            speed = projectile[i][8];
            gravity = projectile[i][9];
            height = nextProjHeight(angleY, speed, &gravity);
            yPos += height;
    
            /*Update mob position - plane*/        
            dx = projectile[i][3];
            dz = projectile[i][5];
            angleX = projectile[i][6];
    
            nextProjLoc(&xPos, &zPos, dx, dz, angleX);
            
            /*Update the mob position in the world*/
            setMobPosition(i, xPos, yPos, zPos, 0);
        
            /*Show mob*/
            showMob(i);
    
            /*Save mob configuration on the plane*/
            projectile[i][0] = xPos;
            projectile[i][2] = zPos;
    
            /*Save mob configuration in flight*/
            projectile[i][1] = yPos;
            projectile[i][9] = gravity;
        }            
    }
}


/*Determine the next location of the projectile*/
void nextProjLoc(float * xPos, float * zPos, float dx, float dz, int angle) {
    /*Determine what quadrant it's in*/
    if (angle == 360 || (angle >= 0 && angle <=90)) {
        /*In quadrant 1*/
        *xPos += dz;
        *zPos -= dx;
    }
    else if (angle > 90 && angle <= 180) {
        /*In quadrant 2*/
        *xPos += dx;
        *zPos += dz;
    }
    else if (angle > 180 && angle <= 270) {
        /*In quadrant 3*/
        *xPos -= dz;
        *zPos += dx;
        
    }
    else if (angle > 270 && angle < 360) {
        /*In quadrant 4*/
        *xPos -= dx;
        *zPos -= dz;
    }
}

/*Determine the projectiles height*/
float nextProjHeight(float angle, float speed, float * gravity) {
    float height;
    float radian;
    
    radian = angle * M_PI / 180.0f;     //Convert the degree to radian
    height = sin(radian) * speed + *gravity;    //Calculate the height
    
    *gravity -= 0.05;   //Increase gravity
   
    /*Return the height*/
    return height;
}

/*Determine if any of the projectiles has hit anything ingame*/
void objectCollision() {
    int xPos, yPos, zPos;
    int xMax, zMax, min;
    int cube;
    int i, c, newX, newZ;  //Loop counter
    int demoCordsTop[] = {-2,-2,   -2,-1,   -2,0,   -2,1,   -2,2,
                          -1,-2,   -1,-1,   -1,0,   -1,1,   -1,2,
                           0,-2,    0,-1,    0,0,    0,1,    0,2,
                           1,-2,    1,-1,    1,0,    1,1,    1,2,
                           2,-2,    2,-1,    2,0,    2,1,    2,2};  //First Row of destruction
    int demoCordsMid[] = {-1,-1,   -1,0,   -1,1,
                           0,-1,    0,0,    0,1,
                           1,-1,    1,0,    1,1};   //Second Row of destruction
                        
    
    for (i = 0; i < 10; i++) {
        /*Get current projectile position*/
        xPos = projectile[i][0];
        yPos = projectile[i][1];
        zPos = projectile[i][2];
        
        if (xPos >= 0) {    
            /*Determine if the projectile hit the boarder*/
            xMax = WORLDX;
            zMax = WORLDZ;
            min = 0;
        
            /*Determine if the projectile hit the game wall*/
            if (xPos <= min || xPos >= xMax || zPos <= min || zPos >= zMax || yPos < min) {
                hideMob(i);
                clearProjectile(i);
            }
        
    
            /*Determine if the projectile hit the ground*/
            if (yPos < 47) {
                cube = world[xPos][yPos][zPos];
                
                if (cube != 0) {
                    /*First level of destruction*/
                    for (c = 0; c < 50; c += 2) {
                        newX = xPos + demoCordsTop[c];
                        newZ = zPos + demoCordsTop[c+1];
                        if (newX >= min && newX < xMax && newZ >= min && newZ < zMax && yPos >= min) {                            world[newX][yPos][newZ] = 0;
                        }
                    }
                    
                    /*Second level of destruction*/
                    for (c = 0; c < 18; c += 2) {
                        newX = xPos + demoCordsTop[c];
                        newZ = zPos + demoCordsTop[c+1];
                        if (newX >= min && newX < xMax && newZ >= min && newZ < zMax && yPos - 1 >= min) {                            world[newX][yPos-1][newZ] = 0;
                        }
                    }
                    
                    /*Last cube destruction*/
                    if (yPos - 2 >= min) {
                        world[xPos][yPos-2][zPos] = 0;
                    }
                                
                    /*Reset projectile*/
                    hideMob(i);
                    clearProjectile(i);
                }
            }
        }
    }
}


	/* called by GLUT when a mouse button is pressed or released */
	/* -button indicates which button was pressed or released */
	/* -state indicates a button down or button up event */
	/* -x,y are the screen coordinates when the mouse is pressed or */
	/*  released */ 
void mouse(int button, int state, int x, int y) {
    static int oldMouPosX, oldMouPosY;
    static int oldX, oldY, oldZ;
    float xPos, yPos, zPos;
    float xaxis, yaxis, zaxis;
    int reminder, projNum;
    float hor, height;
    float dx, dz;
    static float speed, angle;
    int orientAngle;
    float radian;
    
    /*Determine the mouse action*/
    if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON) {        
        /*Get the current position*/
        getViewPosition(&xPos, &yPos, &zPos);
        
        /*Determine player orientation*/
        getViewOrientation(&xaxis, &yaxis, &zaxis);        
        reminder = abs((int)yaxis) % 360;
        
        /*Convert position to be positive for the projectile*/
        xPos *= -1;
        yPos *= -1;
        zPos *= -1;
        
        radian = angle * M_PI / 180.0f;  //convert to radian
        height = sin(radian)*(speed);
        hor = cos(radian)* (speed);
            
        /*Determine if the projectile angle is 90 degrees*/
        if (angle >= 90) {
            /*Orientation direction*/
            dz = 0;
            dx = 0;
        }
        else {
            orientAngle = reminder % 90;
            radian = orientAngle * M_PI / 180.0f;  //convert to radian
           
            /*Orientation direction*/
            dz = sin(radian) * hor;  
            dx = cos(radian) * hor;
           
        }
        
        /*Determine what quadrant it's in*/
        nextProjLoc(&xPos, &zPos, dx, dz, reminder);
        
        /*Create the mob*/
        //yPos += 0.2;
        projNum = projNumber;
        createMob(projNum, xPos, yPos, zPos, 0); 
        showMob(projNum);
        
        /*Save mob configuration on the plane*/        
        projectile[projNum][0] = xPos;
        projectile[projNum][2] = zPos;
        projectile[projNum][3] = dx;
        projectile[projNum][5] = dz;
        projectile[projNum][6] = (float)reminder;
        
        /*Save mob configuration in flight*/
        projectile[projNum][1] = yPos;
        projectile[projNum][7] = (float)angle;
        projectile[projNum][8] = (float)speed;
        projectile[projNum][9] = 0.0;
        
        /*Determine the number of projectiles in the world*/
        if (projNumber + 1 > 9) {
            projNumber = 0;
        }
        else {
            projNumber++;   //Increase projectile number
        }
        
        /*Inform the User of the new angle set*/
        printf("Shot Projectile at: \nAngle = %0.2f and Speed =%0.2f\n ------\n", angle, speed);  
    }
    else if (state == GLUT_DOWN && button == GLUT_RIGHT_BUTTON) {        
        /*Save the orientation information*/
        oldMouPosX = x;
        oldMouPosY = y;
             
        /*Reset speed and angle values*/
        speed = -1;
        angle = -1;
    }
    else if (state == GLUT_UP && button == GLUT_RIGHT_BUTTON){
        /*Determine the speed*/
        speed = calSpeed(oldMouPosX, x);
        
        /*Determine the angle*/
        angle = calAngle(oldMouPosY, y);
        
        /*Inform the User of the new angle set*/
        printf("Set Angle = %0.2f and Speed =%0.2f\n ------\n", angle, speed);        
    }
}

/*Get the projectile speed/velocity to be travelling at*/
float calSpeed(int oldX, int newX) {
    float xaxis, yaxis, zaxis;
    float speed;
    
    /*Calculate the speed*/
    speed = newX - oldX;
    
    /*Modify the speed to be between 0.0 and 1.0*/    
    if (speed >= 360) {
        speed = 1.0;
    }
    else if (speed >= 270) {
        speed = 0.9;
    }
    else if (speed >= 180) {
        speed = 0.8;
    }
    else if (speed >= 90) {
        speed = 0.7;
    }
    else if (speed >= 0) {
        speed = 0.6;
    }
    else if (speed >= -90) {
        speed = 0.5;
    }else if (speed >= -180) {
        speed = 0.4;
    }
    else if (speed >= -270) {
        speed = 0.3;
    }else if (speed >= -360) {
        speed = 0.2;
    }else if (speed < -360) {
        speed = 0.1;
    }
    
    /*Return the speed that was selected by the user*/
    return speed;
}

/*Get the projectile angle to be shot at*/
float calAngle(int oldY, int newY) {
    float xaxis, yaxis, zaxis;
    float angle;
    int reminder;
    
    /*Calculate the angle*/
    angle = (oldY - newY);
    
    /*Modify the angle if it's greater than 90*/
    if (angle > 90) {
        angle = 90;
    }
    else if (angle < 0) {
        angle = 0;
    }
    
    /*Return the angle selected by the user*/
    return angle;
}


/*Main function in the game which sets up the environment and how it looks*/
int main(int argc, char** argv)
{
int i, j, k;
	/* initialize the graphics system */
   graphicsInit(&argc, argv);

	/* the first part of this if statement builds a sample */
	/* world which will be used for testing */
	/* DO NOT remove this code. */
	/* Put your code in the else statment below */
	/* The testworld is only guaranteed to work with a world of
		with dimensions of 100,50,100. */
   if (testWorld == 1) {
	/* initialize world to empty */
      for(i=0; i<WORLDX; i++)
         for(j=0; j<WORLDY; j++)
            for(k=0; k<WORLDZ; k++)
               world[i][j][k] = 0;

	/* some sample objects */
	/* build a red platform */
      for(i=0; i<WORLDX; i++) {
         for(j=0; j<WORLDZ; j++) {
            world[i][24][j] = 3;
         }
      }
	/* create some green and blue cubes */
      world[50][25][50] = 1;
      world[49][25][50] = 1;
      world[49][26][50] = 1;
      world[52][25][52] = 2;
      world[52][26][52] = 2;

	/* blue box shows xy bounds of the world */
      for(i=0; i<WORLDX-1; i++) {
         world[i][25][0] = 2;
         world[i][25][WORLDZ-1] = 2;
      }
      for(i=0; i<WORLDZ-1; i++) {
         world[0][25][i] = 2;
         world[WORLDX-1][25][i] = 2;
      }

	/* create two sample mobs */
	/* these are animated in the update() function */
      createMob(0, 50.0, 25.0, 52.0, 0.0);
      createMob(1, 50.0, 25.0, 52.0, 0.0);

	/* create sample player */
      createPlayer(0, 52.0, 27.0, 52.0, 0.0);

   } else {

	/* your code to build the world goes here */
       printf("argc = %d\n", argc); //TESTING!!!!!
       if (argc > 1) {
           int seedLen = 6;
           char seed[seedLen];
           
           /*Set the flags according to if its a server or client*/
           gameMode = argv[1];
           printf("argv = %s and gamemode = %s \n", argv[1], gameMode); //TESTING!!!!!
           
           /*Determine if the program is a server or client*/
           if (strcmp(gameMode,"-server") == 0) {
               
               /*Open socket as a server*/
               openSocketServer();
               
               /*Generate a landscape seed*/
               landSeed = (rand() % landSeed) + 400000;
               
               sprintf(seed, "%d", landSeed);    //Convert integer to a string
               printf("Server - landscape seed = %d in str = %s\n", landSeed, seed);
               
               /*Send client the seed*/
               write(client_sockfd, &seed, seedLen);
           }
           else if(strcmp(gameMode,"-client") == 0) {
               /*Open socket as a client*/
               //readSocket();
               openSocketClient();
               
               /*Read the seed that was passed over*/
               read(sockfd, &seed, seedLen);
               landSeed = atoi(seed);   //Convert string to an integer
               
               printf("Client - landscape seed = %d in str = %s\n", landSeed, seed);
           }
       }
       
      /*Creates the game landscape*/
      landscape();
       
      /*Initiate projectile array*/
      initProjectiles();
       
       /*Open up server connection*/
       //openSocket();
       
       
       
   }


	/* starts the graphics processing loop */
	/* code after this will not run until the program exits */
   glutMainLoop();
   return 0; 
}


/*Create the game world environment*/
void landscape() {
   grassLand();   //Add a bottom to the world
   waterFlow();   //Add body of water
   mountainTops(); //Add terrain
   cloudFloat();  //Add clounds

   worldOrientation();
}

void worldOrientation() {
    int y = 40;
    
    /*NORTH*/
    world[80][y][50] = 2;   //blue
    world[80][y-1][50] = 2;   //blue
    world[80][y-2][50] = 2;   //blue
    
    /*EAST*/
    world[50][y][80] = 4;   //black
    world[51][y-1][80] = 4;   //black
    world[50][y-2][80] = 4;   //black
    
    /*SOUTH*/
    world[20][y][50] = 3;   //red
    world[20][y-1][50] = 3;   //red
    world[20][y-2][50] = 3;   //red
    
    /*WEST*/
    world[50][y][20] = 5;   //white
    world[51][y-1][20] = 5;   //white
    world[50][y-2][20] = 5;   //white
}

/*Create a floor in the game world*/
void grassLand() {
   int x, z, y = 0;      //Loop counter
      
   for (x = 0; x < WORLDX; x++) {
      for (z = 0; z < WORLDZ; z++) {
         world[x][0][z] = 1;
      }
   }
}

/*Add a river along with the edge of the game world*/
void waterFlow() {
   int x, z = 0, y = 0;      //Loop counter

   for (x = 0; x < WORLDX; x++) {
      world[x][y][z] = 2;
   }
}

/*Add mountain to the game world*/
void mountainTops() {
   /*Use Perlin noise to create a mountain terrain*/
   perlinNoise();
}

/* 
 * Title: Generating Perlin Noise
 * Author: Andreas Jonsson, February 2002
 * REFERENCE: http://www.angelcode.com/dev/perlin/perlin.html
 *
 * Generates a table that will be used to build the landscape.
 * Using the table which is then used to build the game world terrain.
 */
void perlinNoise() {
   int x, z;
   int i, j, p[WORLDX], nSwap;
   int SIZE = WORLDX;
   
   float gx[WORLDX], gy[WORLDX];
   float perX, perZ;
  
   /*Random Generator Seed*/
   srand(landSeed);

   // Initialize the permutation table
   for(i = 0; i < SIZE; i++)
      p[i] = i;

   for(i = 0; i < SIZE; i++){
      j = rand() % SIZE;

      nSwap = p[i];
      p[i]  = p[j];
      p[j]  = nSwap;
   }
     
   // Generate the gradient look-up tables
   for(i = 0; i < SIZE; i++) {
      gx[i] = (float)(rand())/(RAND_MAX/2) - 1.0f; 
      gy[i] = (float)(rand())/(RAND_MAX/2) - 1.0f;
   }

   /*Go through the world array and build up the game world*/
   for (x = 0; x < (WORLDX); x++) {
      for (z = 0; z < (WORLDZ); z++) {
         perX = x / 13.5; 
         perZ = z / 13.5; 

         // Compute the integer positions of the four surrounding points
         int qx0 = (int)floorf(perX);
         int qx1 = qx0 + 1;

         int qy0 = (int)floorf(perZ);
         int qy1 = qy0 + 1;

         // Permutate values to get indices to use with the gradient look-up tables
         int q00 = p[(qy0 + p[qx0 % SIZE]) % SIZE];
         int q01 = p[(qy0 + p[qx1 % SIZE]) % SIZE];

         int q10 = p[(qy1 + p[qx0 % SIZE]) % SIZE];
         int q11 = p[(qy1 + p[qx1 % SIZE]) % SIZE];

         // Computing vectors from the four points to the input point
         float tx0 = perX - floorf(perX);
         float tx1 = tx0 - 1;

         float ty0 = perZ - floorf(perZ);
         float ty1 = ty0 - 1;

         // Compute the dot-product between the vectors and the gradients
         float v00 = gx[q00]*tx0 + gy[q00]*ty0;
         float v01 = gx[q01]*tx1 + gy[q01]*ty0;

         float v10 = gx[q10]*tx0 + gy[q10]*ty1;
         float v11 = gx[q11]*tx1 + gy[q11]*ty1;

         // Do the bi-cubic interpolation to get the final value
         float wx = (3 - 2*tx0)*tx0*tx0;
         float v0 = v00 - wx*(v00 - v01);
         float v1 = v10 - wx*(v10 - v11);

         float wy = (3 - 2*ty0)*ty0*ty0;
         float v = v0 - wy*(v0 - v1);
         
         /*Height*/
         v = (10 - v * 21);
         
         /*Fill the mountain*/
         fillMountain(x, (int)v, z);
      }
   }
}
/*********END OF REFERENCED CODE*********/

/*Fills interior of the mountain*/
void fillMountain(int x, int y, int z) {
   int i, green = 1;
   
   /*Prevent the mountain from spawning in the sky*/
   if (y > 47 || y < 0) {
      y = 0;
   }
   
   /*Top of the mountain terrain*/
   world[x][y][z] = 1;

   /*Build the mountain bottom up*/
   for (i = 1; i < y; i++) {
      world[x][i][z] = 1;
   }
}


/*Generates small clouds*/
void cloudFloat() { 
   /*Generate the clouds*/
   int x=50, z = 50, y = 48;      //Coordinates
   int cloud = 5;

   /*placed the cloud into the world*/
   world[x][y][50] = cloud;
   world[x+1][y][50] = cloud;
   world[x+1][y][50] = cloud;

   /*Place larger clouds into the world*/
   cloudShape();
}

/*Generates larger shaped clouds*/
void cloudShape() {
   /*Create the shape of the cloud*/
   int arySize;

   /*Cloud type #1*/
   int cloudType1[] = {1,3,
                      2,1,  2,2,  2,3,  2,4,
                      3,2,  3,1,  3,-3,  1,-4,  
                      4,1};

   /*Place the type 1 cloud into the world*/
   arySize = (int)(sizeof(cloudType1)/sizeof(int));
   createCloud(50,47,47, cloudType1, arySize);
   createCloud(15,47,15, cloudType1, arySize);
   createCloud(27,48,32, cloudType1, arySize);
   createCloud(80,48,22, cloudType1, arySize);
   createCloud(55,48,8, cloudType1, arySize);

   /*Cloud type #2*/
   int cloudType2[] =  {1,1,  1,2,  1,3,  1,5,  1,6,  1,7,
                       2,2,  2,3,  2,4, 2,6,  2,7,  2,8,
                       3,3,  3,4,  3,5,  3,6,  3,7,
                       4,4,  4,5,  4,6,  4,9,
                       5,3,  5,4,  5,6,  5,7,  5,8};

   /*Place the type 2 cloud into the world*/
   arySize = (int)(sizeof(cloudType2)/sizeof(int));
   createCloud(20,47,7, cloudType2, arySize);
   createCloud(60,47,48, cloudType2, arySize);
   createCloud(20,47,60, cloudType2, arySize);
   createCloud(30,48,31, cloudType2, arySize);
   createCloud(20,48,7, cloudType2, arySize);
   createCloud(45,48,37, cloudType2, arySize);
   createCloud(25,48,25, cloudType2, arySize);

   /*Cloud type #3*/
   int cloudType3[] =  {1,1,  1,2,  1,3,  1,5,  1,6,  1,7,
                       2,2,  2,3,  2,4, 2,6,  2,7,  2,8,
                       3,3,  3,4,  3,5,  3,6,  3,7,
                       4,4,  4,5,  4,6,  4,9,
                       5,1,  5,3,  5,4,  5,5,  5,6,  5,7,  5,8,
                       6,4,  6,5,  6,6,  6,9,
                       7,2,  7,3,  7,7,  7,8,  7,9,
                       8,6,  8,7,  8,9,  8,10,
                       9,1,  9,3,  9,6,  9,7,  9,9};

   /*Place the type 3 cloud into the world*/                    
   arySize = (int)(sizeof(cloudType3)/sizeof(int));
   createCloud(30,47,57, cloudType3, arySize);
   createCloud(38,48,21, cloudType3, arySize);
   createCloud(21,48,80, cloudType3, arySize);
   createCloud(10,47,80, cloudType3, arySize);
   createCloud(10,48,10, cloudType3, arySize);
   createCloud(21,47,20, cloudType3, arySize);   
}

/*Puts the clouds into the world 3D array*/
void createCloud(int x, int y, int z, int cloud[], int arySize) {
   int cloudX, cloudZ;   //Cloud information
   int i = 0;
   
   /*Place first cube that makes up the cloud into the array*/
   world[x][y][z] = 5;

   /*Place second cube that makes up the cloud into the array*/
   cloudX = x + cloud[i];
   cloudZ = z + cloud[i + 1];
   world[cloudX][y][cloudZ] = 5;

   /*Go through the rest of the cloud array that'll be placed into the world*/
   for (i = 2; i < arySize; i+=2) {
      cloudX = x + cloud[i];
      cloudZ = z + cloud[i+1];
      world[cloudX][y][cloudZ] = 5;
   }
}

/*Move the clouds in the game every 0.35 seconds*/
void moveCloud() {
    static int resetTime = 1;
    int diff;
    int milsec = 10000;    //Milliseconds

    static clock_t start;
    clock_t end;

    /*Determine if timer needs to be reset*/
    if (resetTime == 1) {
        /*Reset Timer*/        
        start = clock();     //Get the current time
        resetTime = 0;       //Set timer flag
              
    }
    else if (resetTime == 0) {
        /*Determine number of seconds elapsed*/
        end = clock();
        diff = (int)((end - start) / milsec);

        /*Determine if 0.35 seconds have passed*/
        if (diff >= 35) {
            updateCloudLoc();   //Update cloud location
            resetTime = 1;   //Reset timer flag
        }
    }
}

/*Locate and update the cloud location*/
void updateCloudLoc() {
    int x, y, z;   //Stores the location 
    int cloudLine = 47;   //The height of where the clouds will be at

    /*Determine if the cube is a cloud*/
    for (y = cloudLine; y < WORLDY-1; y++) {
        for (z = WORLDZ - 1; z >= 0; z--) {
            for (x = WORLDX - 1; x >= 0; x--) {
                if (world[x][y][z] == 5) {
                    /*Remove old cloud*/
                    world[x][y][z] = 0;

                    /*Relocate the cloud*/
                    if (x+1 > WORLDX - 1) {
                        world[0][y][z] = 5;
                    }
                    else {
                        world[x+1][y][z] = 5;
                    }
                }
            }    
        }
    }
}

/*Set up the projectile array*/
void initProjectiles() {
    int i, k;
    
    for (i = 0; i < 10; i++) {
        for (k = 0; k < 10; k++) {
            projectile[i][k] = -1;
        }
    }
}

/*Reset the projectile*/
void clearProjectile(num) {
    int i;
    
    for (i = 0; i < 10; i++) {
        projectile[num][i] = -1;
        
    }
}
