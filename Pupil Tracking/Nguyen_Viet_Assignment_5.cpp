// Viet Nguyen

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "/usr/local/include/opencv/cv.h"
#include "/usr/local/include/opencv/cvaux.h"
#include "/usr/local/include/opencv/highgui.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "/usr/include/postgresql/libpq-fe.h"

#define MAX_DB_STATEMENT_BUFFER_LENGTH 5000
#define TRUE 1
#define FALSE 0

using namespace cv;
using namespace std;
Point call_FEC(Mat face, Rect Eye); 
int main (int argc, char *argv[]) 
{
	PGconn   *db_connection;
	PGresult *db_result;
	char db_statement[MAX_DB_STATEMENT_BUFFER_LENGTH];
	int row,num_rows;
	int frames;
	int numFrames, videoWidth, videoHeight;
	char leftEyeX[100], leftEyeY[100], leftEyeWidth[100], leftEyeHeight[100];
	char rightEyeX[100], rightEyeY[100], rightEyeWidth[100], rightEyeHeight[100];
	int lEyeX, lEyeY, lEyeWidth, lEyeHeight;
	int rEyeX, rEyeY, rEyeWidth, rEyeHeight;
	char fps[1280];		
	char numFrame[100];
	char videoid[1280];
	char width[100];
	char height[100];
	char vidlocation[100];
	Mat source_image;

	//CONNECTS TO THE DATABASE	
	db_connection = PQconnectdb("host = 'localhost' dbname = 'cs161project' user = 'userz' password = 'hello'");
	if (PQstatus(db_connection) != CONNECTION_OK)
	{
		printf ("Connection to database failed: %s", PQerrorMessage(db_connection));
		PQfinish (db_connection);
		exit (EXIT_FAILURE);
	}

	//Queries franes, width. height
	strcpy (&videoid[0],argv[1]);
	char dbInput[1000] ="SELECT framecount, xresolution, yresolution FROM generaldetails WHERE video_id = '";
	strcat(dbInput,videoid);
	char lastChar[5] ="'";
	strcat(dbInput,lastChar);
	db_result = PQexec(db_connection, dbInput);
	if (PQresultStatus(db_result) != PGRES_COMMAND_OK )
	{
		snprintf(numFrame, sizeof numFrame, "%s",PQgetvalue (db_result, 0, 0));
		snprintf(width, sizeof width, "%s",PQgetvalue (db_result, 0, 1));
		snprintf(height, sizeof height, "%s",PQgetvalue (db_result, 0, 2));
	}
	PQclear(db_result);

	//Converts everything to int
	numFrames = atoi(numFrame);
	videoWidth = atoi(width);
	videoHeight = atoi(height);


	for(frames = 1; frames <= numFrames-1; frames++)
	{
		//Queries left eye bounding box
		strcpy (&videoid[0],argv[1]);
		char dbInput[1000];
		snprintf(dbInput, sizeof dbInput, "SELECT x, y, width, height FROM bounding_box_left_eye WHERE video_id = '%s' AND frame = '%d'",videoid,frames);
		db_result = PQexec(db_connection, dbInput);
		if (PQresultStatus(db_result) != PGRES_COMMAND_OK)
		{
			snprintf(leftEyeX, sizeof leftEyeX, "%s",PQgetvalue (db_result, 0, 0));
			snprintf(leftEyeY, sizeof leftEyeY, "%s",PQgetvalue (db_result, 0, 1));
			snprintf(leftEyeWidth, sizeof leftEyeWidth, "%s",PQgetvalue (db_result, 0, 2));
			snprintf(leftEyeHeight, sizeof leftEyeHeight, "%s",PQgetvalue (db_result, 0, 3));
		}
		PQclear(db_result);

		//Converts left eye bounding box to int and into a rectangle
		lEyeX = atoi(leftEyeX);
		lEyeY = atoi(leftEyeY);
		lEyeWidth = atoi(leftEyeWidth);
		lEyeHeight = atoi(leftEyeHeight);
		CvRect leftEye = cvRect(lEyeX, lEyeY, lEyeWidth, lEyeHeight);

		//Queries right eye bounding box
		snprintf(dbInput, sizeof dbInput, "SELECT x, y, width, height FROM bounding_box_right_eye WHERE video_id = '%s' AND frame = '%d'",videoid,frames);
		db_result = PQexec(db_connection, dbInput);
		if (PQresultStatus(db_result) != PGRES_COMMAND_OK)
		{
			snprintf(leftEyeX, sizeof rightEyeX, "%s",PQgetvalue (db_result, 0, 0));
			snprintf(leftEyeY, sizeof rightEyeY, "%s",PQgetvalue (db_result, 0, 1));
			snprintf(leftEyeWidth, sizeof rightEyeWidth, "%s",PQgetvalue (db_result, 0, 2));
			snprintf(leftEyeHeight, sizeof rightEyeHeight, "%s",PQgetvalue (db_result, 0, 3));
		}
		PQclear(db_result);t

		//Converts right eye bounding box to int and into a rectangle
		rEyeX = atoi(rightEyeX);
		rEyeY = atoi(rightEyeY);
		rEyeWidth = atoi(rightEyeWidth);
		rEyeHeight = atoi(rightEyeHeight);
		CvRect rightEye = cvRect(rEyeX, rEyeY, rEyeWidth, rEyeHeight);

		snprintf(vidlocation, sizeof vidlocation, "%s/out%d.png",videoid,frames);
		source_image = imread(vidlocation, CV_LOAD_IMAGE_COLOR);
		Point leftEyePoint = call_FEC(source_image,leftEye);
		Point rightEyePoint = call_FEC(source_image,rightEye);

		//Insert bounding box for left eye
		snprintf(dbInput, sizeof dbInput, "INSERT  INTO left_pupil (video_id,frame,x,y) VALUES (%s,%d,%d,%d)",videoid,frames,leftEyePoint.x,leftEyePoint.y);
		db_result = PQexec(db_connection, dbInput);
		if (PQresultStatus(db_result) != PGRES_COMMAND_OK)
	    	{
			fprintf(stderr, "INSERT left Eye bounding failed: %s", PQerrorMessage(db_connection));
	    	}
		PQclear(db_result);

		//Insert bounding box for right eye
		snprintf(dbInput, sizeof dbInput, "INSERT  INTO right_pupil (video_id,frame,x,y) VALUES (%s,%d,%d,%d)",videoid,frames,rightEyePoint.x,rightEyePoint.y);
		db_result = PQexec(db_connection, dbInput);
		if (PQresultStatus(db_result) != PGRES_COMMAND_OK)
	    	{
			fprintf(stderr, "INSERT right eye bounding failed: %s", PQerrorMessage(db_connection));
	    	}
		PQclear(db_result);
	}
}

