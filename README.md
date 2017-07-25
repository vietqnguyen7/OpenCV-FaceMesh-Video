# OpenCV-FaceMesh-Video
Set of programs to take an existing video and creating a face mesh around the face.

Order:
1. Use script for database on POSTGres to set up the database.
2. Run Extract Images to extract still images from a video.
3. Run Determine Bounding Boxes to save all of the important coordinates into the database.
4. Run Draw Bounding Boxes to draw a box around all the important facial features based off of the coordinates given.
5. Run Pupil Tracking to mark the pupil.
6. Run Pupil Drawing to mark a dot on the pupil.
7. Run Determine Facial Marks to run the frames into stasm to retrieve all of the facial points.
8. Run Face Mesh Drawing to connect all of the stasm points to create a face mesh and then merge all the images together to create a video.

Requires OpenCV to run.
