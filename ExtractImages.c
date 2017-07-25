/* Viet Nguyen CS161 Assignment 2*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "/usr/include/postgresql/libpq-fe.h"

#define MAX_DB_STATEMENT_BUFFER_LENGTH 5000
#define TRUE 1
#define FALSE 0


int main (int argc, char *argv[])
{
	//DATABASE PARAMETERS
	PGconn   *db_connection;
	PGresult *db_result;
	char db_statement[MAX_DB_STATEMENT_BUFFER_LENGTH];
	int row,num_rows;

	//FFPROBE FFMPEG PARAMETERS
	char input_video_filename[1280],loc[100];
	int i,x;
	int location;

	//DIRECTORY PARAMETERS
	struct stat file_stat;
	char remote_filename[1280]; // 1024+256 (1024 bytes for path and 256 bytes for filename)
  	int directory_exists;
	
	//CONNECTS TO THE DATABASE	
	db_connection = PQconnectdb("host = 'localhost' dbname = 'cs161project' user = 'userz' password = 'hello'");
	if (PQstatus(db_connection) != CONNECTION_OK)
	{
		printf ("Connection to database failed: %s", PQerrorMessage(db_connection));
		PQfinish (db_connection);
		exit (EXIT_FAILURE);
	}

	//RETRIEVES OPEN SPACE IN DATABASE
	char dbRetrieve[1000] ="SELECT video_id FROM generaldetails";
	db_result = PQexec(db_connection, dbRetrieve);
    	if (PQresultStatus(db_result) == PGRES_TUPLES_OK)
	{
		num_rows = PQntuples(db_result);
		if (num_rows == 0)
		{
			strcpy (&loc[0], "0");
		}
		else
		{
			for (row = 0; row < num_rows; row++)
				strcpy (&loc[0],PQgetvalue(db_result,row,0));
			sscanf(loc, "%d", &location);
			location++;
			sprintf(loc,"%d",location);
		}  
		PQclear (db_result);
	}

	//MAKES DIRECTORY
  	directory_exists = FALSE;
  	if (stat (&loc[0], &file_stat) == 0)
  	{
    		directory_exists = TRUE;
  	}
  	else
  	{
    		if (mkdir (loc, 0755) == 0)
    		{
      			directory_exists = TRUE;
    		}
    		else
    		{
      			printf ("mkdir() failed creating %s\n", remote_filename);
  	  	}
  	}
	
	//FFPROBE 
 	strcpy (&input_video_filename[0], argv[1]);
	char ffprobeOutput[200] ="ffprobe -v error -count_frames -select_streams v:0 -show_entries stream=height,width,avg_frame_rate,nb_read_frames -of default=nokey=1:noprint_wrappers=1 ";
	strcat(ffprobeOutput,input_video_filename); 
	FILE*p=NULL; 
	char buf[1024];
	char xRes[100],yRes[100],fps[100],frames[100];
	p=popen(ffprobeOutput, "r"); 
	while(fgets(buf, 1024, p) !=NULL)
	{
		if(x == 0)
			strcpy(xRes,buf);
		if(x == 1)
			strcpy(yRes,buf);
		if(x == 2)
		{
			strtok(buf,"/");
			strcpy(fps,buf);
		}
		if(x == 3)
			strcpy(frames,buf);
		x++; 
	}
	char dbInput[1000] ="INSERT INTO generaldetails (frame_count,xresolution,yresolution,fps) VALUES (";
	char comma[5] = ",";
	char ending[5] = ")";
	strcat(dbInput,frames);
	strcat(dbInput,comma);
	strcat(dbInput,xRes);
	strcat(dbInput,comma);
	strcat(dbInput,yRes);
	strcat(dbInput,comma);
	strcat(dbInput,fps);
	strcat(dbInput,ending);
	db_result = PQexec(db_connection, dbInput);
    	if (PQresultStatus(db_result) != PGRES_COMMAND_OK)
    	{
        	fprintf(stderr, "INSERT failed: %s", PQerrorMessage(db_connection));
    	}
    	PQclear(db_result);

	//FFMPEG
	char ffmpegInput[100] = "ffmpeg -i ";
	char ffmpegPara[100] = " -nostats -loglevel 0 -vf fps=";
	char ffmpegOutput[100] = "/out%d.png";
	strcat(ffmpegInput,input_video_filename);
	strcat(ffmpegInput,ffmpegPara);
	strcat(ffmpegInput,fps);
	strcat(ffmpegInput," ");
	strcat(ffmpegInput,loc);
	strcat(ffmpegInput,ffmpegOutput);
	char ffmpegFPS[100]; 
	p=popen(ffmpegInput, "r"); 
	pclose(p); 

}
