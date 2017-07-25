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
#include "../stasm/stasm_lib.h"

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
	int numFrames;
	char fps[1280];		
	char numFrame[100];
	char videoid[1280];
	char vidlocation[1280];

	//CONNECTS TO THE DATABASE	
	db_connection = PQconnectdb("host = 'localhost' dbname = 'cs161project' user = 'userz' password = 'hello'");
	if (PQstatus(db_connection) != CONNECTION_OK)
	{
		printf ("Connection to database failed: %s", PQerrorMessage(db_connection));
		PQfinish (db_connection);
		exit (EXIT_FAILURE);
	}
	//Queries video_id
	strcpy (&videoid[0],argv[1]);
	char dbInput[1000] ="SELECT framecount FROM generaldetails WHERE video_id = '";
	strcat(dbInput,videoid);
	char lastChar[5] ="'";
	strcat(dbInput,lastChar);
	db_result = PQexec(db_connection, dbInput);
	if (PQresultStatus(db_result) != PGRES_COMMAND_OK)
    	{
		snprintf(numFrame, sizeof numFrame, "%s",PQgetvalue (db_result, 0, 0));
	}
    	PQclear(db_result);

	int frames = 0;
	Mat source_image;
	numFrames = atoi(numFrame);
	for(frames = 1; frames <= numFrames-1; frames++)
	{
		snprintf(vidlocation, sizeof vidlocation, "%s/out%d.png",videoid,frames);
		
		cv::Mat_<unsigned char> img(cv::imread(vidlocation, CV_LOAD_IMAGE_GRAYSCALE));

    		if (!img.data)
    		{
        		printf("Cannot load %s\n", vidlocation);
        		exit(1);
    		}

    		int foundface;
    		float landmarks[2 * stasm_NLANDMARKS]; // x,y coords (note the 2)
		if (!stasm_search_single(&foundface, landmarks, (const char*)img.data, img.cols, img.rows, vidlocation, "../data"))
		{
			printf("Error in stasm_search_single: %s\n", stasm_lasterr());
			exit(1);
		}

		for(int stasm = 0; stasm < stasm_NLANDMARKS; stasm++)
		{
			snprintf(dbInput, sizeof dbInput, "INSERT  INTO stasm (video_id,frame,stasm_index,stasm_point) VALUES (%s,%d,%d,POINT(%d,%d))",videoid,frames,stasm,cvRound(landmarks[stasm*2+1]),cvRound(landmarks[stasm*2]));
			db_result = PQexec(db_connection, dbInput);
			if (PQresultStatus(db_result) != PGRES_COMMAND_OK)
	    		{
				fprintf(stderr, "INSERT Stasm failed: %s", PQerrorMessage(db_connection));
	    		}
			PQclear(db_result);
		}
	}
}

