//VIET NGUYEN
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
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>


#define MAX_DB_STATEMENT_BUFFER_LENGTH 5000

using namespace cv;
using namespace std;

typedef struct face_landmark_node 
{
	int frame;
	int indice;
	float x;
	float y;
	struct face_landmark_node *next;
} face_landmark_node;

static void draw_point (Mat &img, Point2f fp, Scalar color)
{
	circle (img, fp, 1, color, CV_FILLED, CV_AA, 0);
}
 
static void draw_delaunay (Mat &img, Subdiv2D &subdiv, Scalar delaunay_color)
{
	vector<Vec6f> triangleList;
	subdiv.getTriangleList(triangleList);
	vector<Point> pt(3);
	Size size = img.size();
	Rect rect(0,0, size.width, size.height);
 
	for (size_t i = 0; i < triangleList.size(); i++)
	{
		Vec6f t = triangleList[i];
		pt[0] = Point(cvRound(t[0]), cvRound(t[1]));
		pt[1] = Point(cvRound(t[2]), cvRound(t[3]));
		pt[2] = Point(cvRound(t[4]), cvRound(t[5]));
         	// Draw rectangles completely inside the image.
    		if (rect.contains(pt[0]) && rect.contains(pt[1]) && rect.contains(pt[2]))
    		{
      			line (img, pt[0], pt[1], delaunay_color, 1, CV_AA, 0);
      			line (img, pt[1], pt[2], delaunay_color, 1, CV_AA, 0);
      			line (img, pt[2], pt[0], delaunay_color, 1, CV_AA, 0);
    		}
  	}
}

static void run (face_landmark_node *face_landmark_list_head, int videoid, int frame)
{
	face_landmark_node *face_landmark_element;
	Scalar delaunay_color(255,0,0), points_color(0, 0, 255); // Note: delaunay_color and points_color are in BGR (BLUE, GREEN, RED) format
	Mat source_image;
  	Size source_image_resolution;
	char vidlocation[1280];
	char outlocation[1280];
	snprintf(vidlocation, sizeof vidlocation, "%d/out%d.png",videoid,frame);
	snprintf(outlocation, sizeof outlocation, "output/%d/out%d.png",videoid,frame);
    	source_image = imread (vidlocation);

    	if (!source_image.empty())
    	{
      		source_image_resolution = source_image.size();
     	 	Rect rect(0, 0, source_image_resolution.width, source_image_resolution.height);
      		Subdiv2D subdiv(rect);

      		face_landmark_element = face_landmark_list_head;
      		while (face_landmark_element != NULL)
      		{
        		subdiv.insert(Point2f(face_landmark_element->x, face_landmark_element->y));
        		face_landmark_element = face_landmark_element->next;
      		}
      		draw_delaunay (source_image, subdiv, delaunay_color);
      		face_landmark_element = face_landmark_list_head;
      		while (face_landmark_element != NULL)
      		{
        		draw_point (source_image, Point2f(face_landmark_element->x, face_landmark_element->y), points_color);
        		face_landmark_element = face_landmark_element->next;
      		}
      		imwrite (outlocation, source_image);
    	}


}

face_landmark_node * add_face_landmark_element (face_landmark_node *face_landmark_list_head, int frame, int indice, float pixel_location_x, float pixel_location_y)
{
	face_landmark_node *new_face_landmark_element, *face_landmark_element, *previous_face_landmark_element;
  	new_face_landmark_element = (face_landmark_node *) malloc (sizeof (face_landmark_node));		
  	if (new_face_landmark_element != NULL)
  	{
    		new_face_landmark_element->frame = frame;
    		new_face_landmark_element->indice = indice;
    		new_face_landmark_element->x = pixel_location_x;
    		new_face_landmark_element->y = pixel_location_y;
    		new_face_landmark_element->next = NULL;
    		if (face_landmark_list_head != NULL)
    		{
      			face_landmark_element = face_landmark_list_head;
      			while (face_landmark_element->next != NULL) 
      			{
        			face_landmark_element = face_landmark_element->next;
      			}
      			face_landmark_element->next = new_face_landmark_element;
    		}
    		else
    		{
      			face_landmark_list_head = new_face_landmark_element;
    		}
  	}
  	return face_landmark_list_head;
}





face_landmark_node * load_face_landmark_data (face_landmark_node *face_landmark_list_head, int videoidnum, int frame)
{
	int x;
	char point[100];
	PGconn   *db_connection;
	PGresult *db_result;
	char db_statement[MAX_DB_STATEMENT_BUFFER_LENGTH];
	char dbInput[1280];
	float stasm_x,stasm_y;
	//CONNECTS TO THE DATABASE	
	db_connection = PQconnectdb("host = 'localhost' dbname = 'cs161project' user = 'userz' password = 'hello'");
	if (PQstatus(db_connection) != CONNECTION_OK)
	{
		printf ("Connection to database failed: %s", PQerrorMessage(db_connection));
		PQfinish (db_connection);
		exit (EXIT_FAILURE);
	}

	for(x = 0; x < 77; x++)
	{
		//Queries franes, width. height
		snprintf(dbInput, sizeof dbInput, "SELECT stasm_point FROM stasm WHERE video_id = '%d' AND frame = '%d' AND stasm_index = '%d'",videoidnum,frame,x);
		db_result = PQexec(db_connection, dbInput);
		if (PQresultStatus(db_result) != PGRES_COMMAND_OK )
		{
			snprintf(point, sizeof point, "%s",PQgetvalue (db_result, 0, 0));
			sscanf(point,"(%f,%f)",&stasm_x,&stasm_y);
			face_landmark_list_head = add_face_landmark_element (face_landmark_list_head, frame, x, stasm_x, stasm_y);
		}
	}
	return face_landmark_list_head;
}

int main (int argc, char *argv[]) 
{
	PGconn   *db_connection;
	PGresult *db_result;
	char db_statement[MAX_DB_STATEMENT_BUFFER_LENGTH];
	int row,num_rows;
	int fps;
	int frames,videoidnum;
	int leftEyeX,leftEyeY,rightEyeX,rightEyeY;
	int numFrames, videoWidth, videoHeight;
	int dirCheck;
	char rightX[100];
	char rightY[100];
	char leftX[100];
	char leftY[100];
	char videoid[1280];
	char vidlocation[100];
	char width[100];
	char height[100];
	char numFrame[100];
	char dbInput[1280];
	char outputlocation[1280];
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
		fps = atoi(PQgetvalue (db_result, 0, 4));
		frames = atoi(PQgetvalue(db_result,0,1));
	}
	PQclear(db_result);

	dirCheck = mkdir("output",ACCESSPERMS);
	if(!dirCheck)
		printf("CREATED OUTPUT DIRECTORY");

	snprintf(outputlocation, sizeof outputlocation, "output/%s", videoid);
	dirCheck = mkdir(outputlocation,ACCESSPERMS);
	if(!dirCheck)
		printf("CREATED OUTPUT DIRECTORY");

	int count;
	for(count = 1; count < frames; count++)
	{
		face_landmark_node *face_landmark_list_head, *face_landmark_element;
  		face_landmark_list_head = NULL;
  		face_landmark_list_head = load_face_landmark_data (face_landmark_list_head,videoidnum,count);
  		run(face_landmark_list_head,videoidnum,count);
	  	while (face_landmark_list_head != NULL)
	  	{
	    		face_landmark_element = face_landmark_list_head;
	    		face_landmark_list_head = face_landmark_list_head->next;
	    		free (face_landmark_element);
	    		face_landmark_element = NULL;
	  	}

	}
	char video[1280];
	char inputloc[1280];
	char hello[1280] = "%d.png";
	snprintf(inputloc, sizeof inputloc, "output/%s/out",videoid);
	strcat(inputloc,hello);
	FILE*p=NULL;
	snprintf(video, sizeof video, "ffmpeg -r %d -i %s -vcodec libx264 STASM_movie_%s.mp4",fps,inputloc,videoid);
	p=popen(video, "r"); 
	pclose(p); 


}
