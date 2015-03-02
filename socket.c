/* Socket Code From: */
/* Beginning Linux Programming, 4th edition, byt Matthew and Stone. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "graphics.h"

    /*Server Socket Variable*/
int server_sockfd, client_sockfd;
int server_len, client_len;
struct sockaddr_in server_address;
struct sockaddr_in client_address;

    /*Client Socket Variable*/
int sockfd;
int len;
struct sockaddr_in address;

    /* Landscape seed */
extern int landSeed;

    /* projectile Information */
extern float projectile[10][10];  //dx, dy, velocity
extern float projNumber;

   /*Projectile Calculation Function*/
extern void nextProjLoc(float *, float *, float, float, int );
extern float nextProjHeight(float , float, float *);

	/* viewpoint control */
extern void setViewPosition(float, float, float);
extern void getViewPosition(float *, float *, float *);
extern void getOldViewPosition(float *, float *, float *);
extern void setViewOrientation(float, float, float);
extern void getViewOrientation(float *, float *, float *);

	/* mob controls */
extern void createMob(int, float, float, float, float);
extern void setMobPosition(int, float, float, float, float);
extern void hideMob(int);
extern void showMob(int);
 
/*Client Thread Program*/
void *clientThread(void *arg) {
    while(1) {
        readSocket();
    }
}

/*Open the socket as a server*/
/*Reference: Beginning Linux Programming, 4th edition, byt Matthew and Stone. */
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
}

/*Open the socket as a client*/
/*Reference: Beginning Linux Programming, 4th edition, byt Matthew and Stone. */
void openSocketClient() {
    int result;
    
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
        perror("Unable to connect to server.");
        exit(1);
    }
}

/*Write information to socket that is sent to the client*/
void writeSocket() {
    static oldX, oldY, oldZ;
    float x, y, z;
    
    /*Send the server's current position*/
    sendViewPos();
    
    /*Determine if the orientation is any different*/
    getViewOrientation(&x,&y,&z);   //Get the current view orientation
    
    if (oldX != x || oldY != y  || oldZ != z ) {
        /*Send server's current view orientation*/
        sendViewOrient();
        
        /*Save the updated orientation*/
        oldX = x;
        oldY = y;
        oldZ = z;
    }
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
    
    /*Send server's current position*/
    getViewPosition(&x, &y, &z);
    
    /*Convert to a positive number*/
    x = x * -1;
    y = y * -1;
    z = z * -1;
        
    /*Convert coordinates to a string*/
    sprintf(strX, "%f", x);    //Convert integer to a string
    sprintf(strY, "%f", y);    //Convert integer to a string
    sprintf(strZ, "%f", z);    //Convert integer to a string
    
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
    
    /*Convert coordinates to a string*/
    sprintf(strX, "%f", x);
    sprintf(strY, "%f", y);
    
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
}

/*Send information about projectile fired*/
void sendProjectile(char *speedStr, char *angleStr) {
    char ch = 'M';
    int cordLen = 6;
    int msgLen = 12;    //Might want to increment by one (13)
    
    char  msgStr[msgLen];
    
    /*Inform the client that information being sent is viewPosition*/
    write(client_sockfd, &ch, 1);
        
    /*Format string*/
    convertProjNumDigit(speedStr);
    convertProjNumDigit(angleStr);
    
    /*Concate the message*/
    strcpy(msgStr,""); //Set up message
    strcat(msgStr, speedStr);
    strcat(msgStr, ",");
    strcat(msgStr, angleStr);
    
    /*Send message to client*/
    write(client_sockfd, &msgStr, msgLen);
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

/*Converts the orientation to 5 digit number*/
void convertProjNumDigit(char * str) {
    int strLen = 6;
    int i = 0, numZero = 0;  //Loop counter
    char newStr[strLen];
    
    /*Look at the 3 character (position 2) of the string. Determine if it's a decimal*/
    if (str[2] != '.') {
        /*Determine how many zeros to add to the front*/
        for (i = 0; i < 2; i++) {
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
        
        /*Get the message*/
        read(sockfd, &msg, msgLen);
                
        /*Set position to be same as server*/
        parseViewPos(msg);
        
    }
    else if (ch == 'O') {
        int msgLen = 26;    //msg = aaaa.bbbbbb,xxxx.yyyyyy
        char msg[msgLen];
        
        /*Get the message*/
        read(sockfd, &msg, msgLen);
        
        /*Parse the orientation information from the server*/
        parseOrientPos(msg);
        
    }
    else if (ch == 'M') {
        int msgLen = 12;
        char msg[msgLen];
        
        /*Get the message*/
        read(sockfd, &msg, msgLen);
        
        /*Parse the projectile information*/
        parseProjectInfo(msg);
    }
}

/*Parse the view position sent from the server*/
void parseViewPos(char *msg) {
    float x = 0.0, y = 0.0, z = 0.0;
    int numMsg = 3;
    int msgLen = 10;
    
    char ** msgSplit;
    
    /*Parse the message to the three coordinates*/
    msgSplit = splitNumMsgInfo(msg, numMsg, msgLen);
    
    /*Convert string to an integer*/
    x = atof(msgSplit[0]) * -1;
    y = atof(msgSplit[1]) * -1;
    z = atof(msgSplit[2]) * -1;
    
    /*Set the client location based on the server's*/
    setViewPosition(x, y, z);
    
    /*Free memory for array*/
    free(*msgSplit);
    free(msgSplit);
}

/*Parse the orientation information from the server*/
void parseOrientPos(char *msg) {
    //int x = 0, y = 0, z = 0;
    float x = 0.0, y = 0.0, z = 0.0;
    int numMsg = 3;
    int msgLen = 10;
    
    char ** msgSplit;
    
    /*Parse the message to the two coordinates*/
    msgSplit = splitNumMsgInfo(msg, numMsg, msgLen);
    
    /*Convert string to an integer*/
    x = atof(msgSplit[0]);
    y = atof(msgSplit[1]);
    
    /*Set the client location based on the server's*/
    setViewOrientation(x, y, z);
    
    /*Free memory for array*/
    free(*msgSplit);
    free(msgSplit);
    
}

/*Parse the information about the projectile*/
void parseProjectInfo(char *msg) {
    int numMsg = 3;
    int msgLen = 10;
    
    float speed = 0.0, angle = 0.0;
    
    char ** msgSplit;
    
    /*Parse the message to the three coordinates*/
    msgSplit = splitNumMsgInfo(msg, numMsg, msgLen);
    
    /*Convert string to an integer*/
    speed = atof(msgSplit[0]);
    angle = atof(msgSplit[1]);
    
    /*Create the projectile*/
    createClientProj(speed, angle);
    
    /*Free memory for array*/
    free(*msgSplit);
    free(msgSplit);
}

/*Create new projectile in the client world*/
void createClientProj(float speed, float angle) {
    float xPos, yPos, zPos;
    float xaxis, yaxis, zaxis;
    int reminder, projNum;
    float hor, height;
    float dx, dz;
    int orientAngle;
    float radian;
    
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
}

/*Split the message to coordinate*/
char ** splitNumMsgInfo(char * msg, int numMsg, int msgLen) {
    int i = 0;    //loop counters
    char delim[2] = ",";
    char ** msgSplit;
    
    msgSplit = (char**)malloc(msgLen * sizeof(char**)); //Set up the array
    
    char* token = strtok(msg, delim);   //Get the first token
    
    /*Go through all the tokens*/
    while (token != NULL) {
        if (token != NULL) {
            msgSplit[i] = (char*)malloc(sizeof(char*) * sizeof(token) + 2);
            strcpy(msgSplit[i], token);    //Save the message
            
            i++;
        }
        
        token = strtok(NULL, delim);
    }
    
    return msgSplit;
}




