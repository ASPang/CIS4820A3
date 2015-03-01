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

 

