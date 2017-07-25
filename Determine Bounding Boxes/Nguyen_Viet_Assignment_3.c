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
	Mat source_image_gray,source_image_gray_left_eye_region,source_image_gray_right_eye_region, source_image_gray_mouth_region;
	CascadeClassifier face_cascade, left_eye_cascade, right_eye_cascade,nose_cascade, mouth_cascade;
	int bounding_box_face_x = 0, bounding_box_face_y = 0, bounding_box_face_width = 0, bounding_box_face_height = 0;
	int bounding_box_nose_x = 0, bounding_box_nose_y = 0, bounding_box_nose_width = 0, bounding_box_nose_height = 0;
	int bounding_box_mouth_x = 0, bounding_box_mouth_y = 0, bounding_box_mouth_width = 0, bounding_box_mouth_height = 0;
	int bounding_box_left_eye_x = 0, bounding_box_left_eye_y = 0, bounding_box_left_eye_width = 0, bounding_box_left_eye_height = 0;
	int bounding_box_right_eye_x= 0, bounding_box_right_eye_y = 0, bounding_box_right_eye_width = 0, bounding_box_right_eye_height = 0;
  	int maximum_width_left_eye, maximum_height_left_eye, maximum_width_right_eye, maximum_height_right_eye;
 	int minimum_width_left_eye, minimum_height_left_eye, minimum_width_right_eye, minimum_height_right_eye;
	int number_of_left_eyes_detected, number_of_right_eyes_detected;
	std::vector<Rect> face, left_eye, right_eye, nose, mouth;
	char videoid[1280];
	double scale_factor = 1.01;
	int min_neighbors = 10;

	strcpy (&videoid[0],argv[1]);
	//Converts to greyscale
	source_image_gray = imread ("test_image.png", CV_LOAD_IMAGE_GRAYSCALE); //Converts to greyscale
	if (!source_image_gray.empty())
    	{
		equalizeHist (source_image_gray, source_image_gray);
		//Loads and detects the face
		if(!face_cascade.load("./haarcascade_frontalface_alt2.xml"))
		{ 
			printf("Error Loading Face Cascade\n"); 
			return -1; 		
		}
		face_cascade.detectMultiScale (source_image_gray, face, scale_factor, min_neighbors, CV_HAAR_FIND_BIGGEST_OBJECT);
		if (face.size() > 0)
		{
			bounding_box_face_x      = face[0].x;
			bounding_box_face_y      = face[0].y;
			bounding_box_face_width  = face[0].width;
			bounding_box_face_height = face[0].height;
		}
		if ((bounding_box_face_x > 0) && (bounding_box_face_y > 0))
	 	{
			rectangle(source_image_gray, Point(bounding_box_face_x, bounding_box_face_y), Point(bounding_box_face_x + bounding_box_face_width, bounding_box_face_y + 		bounding_box_face_height), Scalar (255,255,255), 1, 8, 0); 
		}
		//End face detection

		//Loads and detects the left eye
		if(!left_eye_cascade.load("./left_eye.xml"))
		{ 
			printf("Error Loading Left Eye Cascade\n"); 
			return -1; 		
		}
		//Sets the left eye parameters
		minimum_width_left_eye =  0.18 * bounding_box_face_width;
       		minimum_height_left_eye = 0.14 * bounding_box_face_height;
        	if ((minimum_width_left_eye < 18) || (minimum_height_left_eye < 12))
        	{
        		minimum_width_left_eye   = 18;
        		minimum_height_left_eye  = 12;
	       	}
	        maximum_width_left_eye   = minimum_width_left_eye * 2;
	        maximum_height_left_eye  = minimum_height_left_eye * 2;
		//Ends the right eye parameters
		source_image_gray_left_eye_region = source_image_gray (Rect (bounding_box_face_x + bounding_box_face_width / 2, bounding_box_face_y + bounding_box_face_height / 5.5, bounding_box_face_width / 2, bounding_box_face_height / 3)); 
        left_eye_cascade.detectMultiScale (source_image_gray_left_eye_region, left_eye, scale_factor, min_neighbors, CV_HAAR_SCALE_IMAGE, Size(minimum_width_left_eye, minimum_height_left_eye), Size(maximum_width_left_eye, maximum_height_left_eye));
		number_of_left_eyes_detected = left_eye.size();
        	if (left_eye.size() > 0)
        	{
        		bounding_box_left_eye_x      = bounding_box_face_x + bounding_box_face_width / 2 + left_eye[0].x;
        		bounding_box_left_eye_y      = bounding_box_face_y + bounding_box_face_height / 5.5 + left_eye[0].y;
        		bounding_box_left_eye_width  = left_eye[0].width;
        		bounding_box_left_eye_height = left_eye[0].height;
        	}
  		if ((bounding_box_left_eye_x > 0) && (bounding_box_left_eye_y > 0))
  		{
    		rectangle (source_image_gray, Point(bounding_box_left_eye_x, bounding_box_left_eye_y), Point(bounding_box_left_eye_x + bounding_box_left_eye_width, bounding_box_left_eye_y + bounding_box_left_eye_height), Scalar (255,255,255), 1, 8, 0); 
  		}
		//Ends the left eye detection

		//Loads and detects the right eye
		if(!right_eye_cascade.load("./right_eye.xml"))
		{ 
			printf("Error Loading Right Eye Cascade\n"); 
			return -1; 		
		}
		//Sets the right eye parameters
		minimum_width_right_eye =  minimum_width_left_eye;
        	minimum_height_right_eye = minimum_height_left_eye;
		if ((minimum_width_right_eye < 18) || (minimum_height_right_eye < 12))
        	{
        		minimum_width_right_eye   = 18;
        		minimum_height_right_eye  = 12;
	       	}
		maximum_width_right_eye  = minimum_width_right_eye * 2;
        	maximum_height_right_eye = minimum_height_right_eye * 2;
		//Ends the right eye parameters
		source_image_gray_right_eye_region = source_image_gray (Rect (bounding_box_face_x, bounding_box_face_y + bounding_box_face_height / 5.5, bounding_box_face_width / 2, bounding_box_face_height / 3));
        	right_eye_cascade.detectMultiScale (source_image_gray_right_eye_region, right_eye, scale_factor, min_neighbors, CV_HAAR_SCALE_IMAGE, Size(minimum_width_right_eye, minimum_height_right_eye), Size(maximum_width_right_eye, maximum_height_right_eye));
		number_of_right_eyes_detected = right_eye.size();
        	if (right_eye.size() > 0)
        	{
        		bounding_box_right_eye_x      = bounding_box_face_x + right_eye[0].x;
        		bounding_box_right_eye_y      = bounding_box_face_y + bounding_box_face_height / 5.5 + right_eye[0].y;
        		bounding_box_right_eye_width  = right_eye[0].width;
        		bounding_box_right_eye_height = right_eye[0].height;
        	}
  		if ((bounding_box_right_eye_x > 0) && (bounding_box_right_eye_y > 0))
  		{
    		rectangle (source_image_gray, Point(bounding_box_right_eye_x, bounding_box_right_eye_y), Point(bounding_box_right_eye_x + bounding_box_right_eye_width, bounding_box_right_eye_y + bounding_box_right_eye_height), Scalar (255,255,255), 1, 8, 0); 
		}
		//Ends the right eye detection


		//Loads and detects the nose
		if(!nose_cascade.load("./nose.xml"))
		{ 
			printf("Error Loading Nose Cascade\n"); 
			return -1; 		
		}
		nose_cascade.detectMultiScale (source_image_gray, nose, scale_factor, min_neighbors, CV_HAAR_FIND_BIGGEST_OBJECT);
		if (nose.size() > 0)
		{
			bounding_box_nose_x      = nose[0].x;
			bounding_box_nose_y      = nose[0].y;
			bounding_box_nose_width  = nose[0].width;
			bounding_box_nose_height = nose[0].height - 30;
		}
		if ((bounding_box_nose_x > 0) && (bounding_box_nose_y > 0))
	 	{
			rectangle(source_image_gray, Point(bounding_box_nose_x, bounding_box_nose_y), Point(bounding_box_nose_x + bounding_box_nose_width, bounding_box_nose_y + 		bounding_box_nose_height), Scalar (255,255,255), 1, 8, 0); 
		}
		//End nose detection

		//Loads and detects the mouth
		if(!mouth_cascade.load("./mouth.xml"))
		{ 
			printf("Error Loading Mouth Cascade\n"); 
			return -1; 		
		}
		source_image_gray_mouth_region = source_image_gray (Rect (bounding_box_face_x, bounding_box_face_width, bounding_box_face_width , bounding_box_face_height/2 ));
		mouth_cascade.detectMultiScale (source_image_gray_mouth_region, mouth, scale_factor, min_neighbors, CV_HAAR_FIND_BIGGEST_OBJECT);
		if (mouth.size() > 0)
		{
			bounding_box_mouth_x      = bounding_box_face_x + mouth[0].x;
			bounding_box_mouth_y      = bounding_box_face_y + bounding_box_face_height/2 + mouth[0].y + 30;
			bounding_box_mouth_width  = mouth[0].width;
			bounding_box_mouth_height = mouth[0].height - 30;
		}
		if ((bounding_box_mouth_x > 0) && (bounding_box_mouth_y > 0))
	 	{
			rectangle(source_image_gray, Point(bounding_box_mouth_x, bounding_box_mouth_y), Point(bounding_box_mouth_x + bounding_box_mouth_width, bounding_box_mouth_y + 		bounding_box_mouth_height), Scalar (255,255,255), 1, 8, 0); 
		}
		//End mouth detection
	}
	
	imshow ("Image", source_image_gray);
	waitKey (0);
	//imwrite("output.png",source_image_gray);

//Finishes Detectiong

//Starts the Postgres saving
	//DATABASE PARAMETERS
	PGconn   *db_connection;
	PGresult *db_result;
	char db_statement[MAX_DB_STATEMENT_BUFFER_LENGTH];
	int row,num_rows;

	//CONNECTS TO THE DATABASE	
	db_connection = PQconnectdb("host = 'localhost' dbname = 'cs161project' user = 'userz' password = 'hello'");
	if (PQstatus(db_connection) != CONNECTION_OK)
	{
		printf ("Connection to database failed: %s", PQerrorMessage(db_connection));
		PQfinish (db_connection);
		exit (EXIT_FAILURE);
	}
	
	//Queries video_id
	char dbInput[1000] ="SELECT video_id, framecount, xresolution, yresolution, fps FROM generaldetails WHERE video_id = '";
	strcat(dbInput,videoid);
	char lastChar[5] ="'";
	strcat(dbInput,lastChar);
	db_result = PQexec(db_connection, dbInput);
	if (PQresultStatus(db_result) != PGRES_COMMAND_OK)
    	{
		printf ("VIDEO_ID: %s FRAMECOUNT: %s WIDTH: %s HEIGHT: %s\n", PQgetvalue (db_result, 0, 0), PQgetvalue (db_result, 0, 1), PQgetvalue (db_result, 0, 2),PQgetvalue (db_result, 0, 3));
	}
    	PQclear(db_result);
	
	//INPUTS FACE
	snprintf(dbInput, sizeof dbInput, "INSERT INTO bounding_box_face (video_id,frame,x,y,width,height) VALUES (%s,%s,%d,%d,%d,%d)",videoid,videoid,bounding_box_face_x,bounding_box_face_y,bounding_box_face_width,bounding_box_face_height);
	db_result = PQexec(db_connection, dbInput);
	if (PQresultStatus(db_result) != PGRES_COMMAND_OK)
    	{
        	fprintf(stderr, "INSERT face bounding failed: %s", PQerrorMessage(db_connection));
		
    	}
	PQclear(db_result);
	//END FACE INPUT
	
	//INPUTS LEFT EYE
	snprintf(dbInput, sizeof dbInput, "INSERT INTO bounding_box_left_eye (video_id,frame,x,y,width,height) VALUES (%s,%s,%d,%d,%d,%d)",videoid,videoid,bounding_box_left_eye_x,bounding_box_left_eye_y,bounding_box_left_eye_width,bounding_box_left_eye_height);
	db_result = PQexec(db_connection, dbInput);
	if (PQresultStatus(db_result) != PGRES_COMMAND_OK)
    	{
        	fprintf(stderr, "INSERT left eye bounding failed: %s", PQerrorMessage(db_connection));
		
    	}
	PQclear(db_result);
	//ENDS LEFT EYE

	//INPUTS RIGHT EYE
	snprintf(dbInput, sizeof dbInput, "INSERT INTO bounding_box_right_eye (video_id,frame,x,y,width,height) VALUES (%s,%s,%d,%d,%d,%d)",videoid,videoid,bounding_box_right_eye_x,bounding_box_right_eye_y,bounding_box_right_eye_width,bounding_box_right_eye_height);
	db_result = PQexec(db_connection, dbInput);
	if (PQresultStatus(db_result) != PGRES_COMMAND_OK)
    	{
        	fprintf(stderr, "INSERT right eye bounding failed: %s", PQerrorMessage(db_connection));
		
    	}
	PQclear(db_result);
	//END RIGHT EYE

	//INPUT NOSE
	snprintf(dbInput, sizeof dbInput, "INSERT INTO bounding_box_nose (video_id,frame,x,y,width,height) VALUES (%s,%s,%d,%d,%d,%d)",videoid,videoid,bounding_box_nose_x,bounding_box_nose_y,bounding_box_nose_width,bounding_box_nose_height);
	db_result = PQexec(db_connection, dbInput);
	if (PQresultStatus(db_result) != PGRES_COMMAND_OK)
    	{
        	fprintf(stderr, "INSERT nose bounding failed: %s", PQerrorMessage(db_connection));
		
    	}
	PQclear(db_result);
	//END NOSE

	//INPUT MOUTH
	snprintf(dbInput, sizeof dbInput, "INSERT INTO bounding_box_mouth (video_id,frame,x,y,width,height) VALUES (%s,%s,%d,%d,%d,%d)",videoid,videoid,bounding_box_mouth_x,bounding_box_mouth_y,bounding_box_mouth_width,bounding_box_mouth_height);
	db_result = PQexec(db_connection, dbInput);
	if (PQresultStatus(db_result) != PGRES_COMMAND_OK)
    	{
        	fprintf(stderr, "INSERT mouth bounding failed: %s", PQerrorMessage(db_connection));
    	}
	PQclear(db_result);
	//END MOUTH
}

