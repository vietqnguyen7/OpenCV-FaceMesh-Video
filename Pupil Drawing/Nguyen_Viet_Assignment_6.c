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

int main (int argc, char *argv[]) 
{
	PGconn   *db_connection;
	PGresult *db_result;
	char db_statement[MAX_DB_STATEMENT_BUFFER_LENGTH];
	int row,num_rows;
	int frames,videoidnum;
	int leftEyeX,leftEyeY,rightEyeX,rightEyeY;
	int numFrames, videoWidth, videoHeight;
	char rightX[100];
	char rightY[100];
	char leftX[100];
	char leftY[100];
	char videoid[1280];
	char vidlocation[100];
	char width[100];
	char height[100];
	char numFrame[100];
	char fps[100];
	char dbInput[1280];
	Mat source_image_color;

	//CONNECTS TO THE DATABASE	
	db_connection = PQconnectdb("host = 'localhost' dbname = 'cs161project' user = 'userz' password = 'hello'");
	if (PQstatus(db_connection) != CONNECTION_OK)
	{
		printf ("Connection to database failed: %s", PQerrorMessage(db_connection));
		PQfinish (db_connection);
		exit (EXIT_FAILURE);
	}

	strcpy (&videoid[0],argv[1]);
	videoidnum = atoi(videoid);
	snprintf(dbInput, sizeof dbInput, "SELECT video_id, framecount, xresolution, yresolution, fps FROM generaldetails WHERE video_id = '%d'",videoidnum);
	db_result = PQexec(db_connection, dbInput);
	if (PQresultStatus(db_result) != PGRES_COMMAND_OK)
	{
		printf ("VIDEO_ID: %s FRAMECOUNT: %s WIDTH: %s HEIGHT: %s\n", PQgetvalue (db_result, 0, 0), PQgetvalue (db_result, 0, 1), PQgetvalue (db_result, 0, 2),PQgetvalue (db_result, 0, 3));
		snprintf(fps, sizeof fps, "%s", PQgetvalue(db_result,0,4));
	}
	PQclear(db_result);
	
	

	//Queries franes, width. height
	snprintf(dbInput, sizeof dbInput, "SELECT framecount, xresolution, yresolution FROM generaldetails WHERE video_id = '%d'",videoidnum);
	db_result = PQexec(db_connection, dbInput);
	if (PQresultStatus(db_result) != PGRES_COMMAND_OK )
	{
		snprintf(numFrame, sizeof numFrame, "%s",PQgetvalue (db_result, 0, 0));
		snprintf(width, sizeof width, "%s",PQgetvalue (db_result, 0, 1));
		snprintf(height, sizeof height, "%s",PQgetvalue (db_result, 0, 2));
	}
	PQclear(db_result);
	numFrames = atoi(numFrame);
	videoWidth = atoi(width);
	videoHeight = atoi(height);

	for(frames = 1; frames <= numFrames-1; frames++)
	{
		//Queries left eye bounding box
		strcpy (&videoid[0],argv[1]);
		char dbInput[1000];
		snprintf(dbInput, sizeof dbInput, "SELECT x, y FROM left_pupil WHERE video_id = '%s' AND frame = '%d'",videoid,frames);
		db_result = PQexec(db_connection, dbInput);
		if (PQresultStatus(db_result) != PGRES_COMMAND_OK)
		{
			snprintf(leftX, sizeof leftX, "%s",PQgetvalue (db_result, 0, 0));
			snprintf(leftY, sizeof leftY, "%s",PQgetvalue (db_result, 0, 1));
		}
		PQclear(db_result);


		//Queries right pupil
		snprintf(dbInput, sizeof dbInput, "SELECT x, y FROM right_pupil WHERE video_id = '%s' AND frame = '%d'",videoid,frames);
		db_result = PQexec(db_connection, dbInput);
		if (PQresultStatus(db_result) != PGRES_COMMAND_OK)
		{
			snprintf(rightX, sizeof rightX, "%s",PQgetvalue (db_result, 0, 0));
			snprintf(rightY, sizeof rightY, "%s",PQgetvalue (db_result, 0, 1));
		}
		PQclear(db_result);
		leftEyeX = atoi(leftX);
		leftEyeY = atoi(leftY);
		rightEyeX = atoi(rightX);
		rightEyeY = atoi(rightY);
		snprintf(vidlocation, sizeof vidlocation, "%s/out%d.png",videoid,frames);
		source_image_color = imread(vidlocation, CV_LOAD_IMAGE_COLOR);		
		circle(source_image_color, Point(leftEyeX, leftEyeY),2,Scalar (0,255,255),-1);
		circle(source_image_color, Point(rightEyeX, rightEyeY),2,Scalar (0,255,255),-1);
		imwrite(vidlocation,source_image_color);
	}
	char video[1280];
	char inputloc[1280];
	char hello[1280] = "%d.png";
	snprintf(inputloc, sizeof inputloc, "%s/out",videoid);
	strcat(inputloc,hello);
	FILE*p=NULL;
	snprintf(video, sizeof video, "ffmpeg -r %s -i %s -vcodec libx264 crosshair_%s.mp4",fps,inputloc,videoid);
	p=popen(video, "r"); 
	pclose(p); 
}
