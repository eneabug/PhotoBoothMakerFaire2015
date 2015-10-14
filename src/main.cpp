//============================================================================
// Name        : main.cpp
// Author      : Enea Bagalini - Politecnico of Torino
// Version     : 1.0
// Copyright   : Your copyright notice
// Description : Photo Booth for Maker Faire Rome 2015
//============================================================================

#include <opencv.hpp>
#include <stdio.h>
#include <gtk-2.0/gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define SCREEN_ROWS 	720
#define SCREEN_COLS		1280

using namespace cv;
using namespace std;

//send mail
char cmd[100];
char email[1024];
FILE *fd;

Mat myWarhol(Mat img);
void myfilter(Mat grayimg, Mat &fltimg, int col[4][3]);

gboolean update(GtkWidget *entry1, GdkEventKey *event, gpointer label1)
{
	strcpy(email,(char*)gtk_entry_get_text(GTK_ENTRY(entry1)));
}

void destroy(GtkWidget *widget, gpointer *data)
{
    gtk_main_quit ();
}

void sendmail(GtkWidget *widget, gpointer *data)
{
	sprintf(cmd,"mutt -s \"Great Photo Shot from Freescale/Mouser booth @ Maker Faire Rome 2015!\" %s < ./../mail.txt -a ./WarholImg.jpg",email);
	system(cmd);
	g_print(email);
}

static void destroy_event(GtkWidget *widget, gpointer data)
{
	gtk_main_quit();
}

static gboolean delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	gtk_main_quit();
	return FALSE;
}

int main(int argc, char** argv)
{
	Mat src;
	Mat WarholImg;

	cvNamedWindow("Warholize your face with UDOO Neo", CV_WINDOW_AUTOSIZE);
	cvMoveWindow("Warholize your face with UDOO Neo",(SCREEN_COLS-640)/2,(SCREEN_ROWS-480)/2-10);

	VideoCapture cap(1); // open the default camera
	if(!cap.isOpened())  // check if we succeeded
		return -1;

	cap >> src;

	while(1)
	{
		while(1)
		{
			cap >> src; // get a new frame from camera

			imshow("Warholize your face with UDOO Neo", src);
			if(waitKey(30) >= 0) break;
		}

		WarholImg = myWarhol(src);
		imshow("Warholize your face with UDOO Neo", WarholImg);

		char s[1000];
		sprintf(s,"With compliments from Freescale and Mouser");
		putText(WarholImg, s, cvPoint(135,WarholImg.rows-10), FONT_HERSHEY_DUPLEX,0.5, cvScalar(255, 255, 255),1, CV_AA);

		imwrite( "./WarholImg.jpg", WarholImg );

		//GTK
		GtkWidget *window;
		GtkWidget *entry;
		GtkWidget *box;
		GtkWidget * button_send;

		gtk_init (&argc, &argv);

		window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
		gtk_window_set_default_size(GTK_WINDOW(window), 400, 40);
		gtk_window_set_title(GTK_WINDOW(window), "       Send e-mail");

		button_send = gtk_button_new_with_label("Send");

		entry = gtk_entry_new();
		gtk_widget_grab_focus(entry);

		box=gtk_vbox_new(TRUE, 5);
		gtk_box_pack_start(GTK_BOX(box), entry, TRUE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(box), button_send, TRUE, TRUE, 0);

		gtk_container_add(GTK_CONTAINER (window),box);
		gtk_widget_show_all(window);

		g_signal_connect_swapped(G_OBJECT(window), "destroy-event",G_CALLBACK(destroy_event), NULL);
		g_signal_connect_swapped(G_OBJECT(window), "delete-event",G_CALLBACK(delete_event), NULL);
		gtk_signal_connect (GTK_OBJECT (button_send), "clicked", GTK_SIGNAL_FUNC (sendmail), NULL);
		g_signal_connect(G_OBJECT(entry), "key-release-event", G_CALLBACK(update), NULL);

		gtk_main ();
	}

	return 0;
}

/**
          myWarhol filter
*/
Mat myWarhol(Mat img) {
    Mat grayimg, fltimg, tileimg;
    int col1[4][3] = {{0,0,255},{0,255,0},{255,0,0},{255,255,0}};//R,G,B
    int col2[4][3] = {{0,100,255},{165,255,0},{255,0,132},{255,215,140}};
    int col3[4][3] = {{19,31,24},{76,76,76},{65,0,89},{255,125,18}};
    int col4[4][3] = {{145,132,98},{184,45,63},{25,78,125},{25,25,47}};

    Size size(img.cols/2,img.rows/2);
    resize(img,img,size);

    //change color-image to gray-image
    cvtColor (img, grayimg, CV_BGR2GRAY);
    //filter the gray-image
    fltimg = img.clone();
    myfilter(grayimg, fltimg, col1);
    //instantiate the smaller sized image

    //create tileimg with zeros matrix
    tileimg = Mat::zeros(2*img.rows, 2*img.cols, CV_8UC3);

    //select a rec section in tile-image
    Mat roi(tileimg, Rect(0, 0, fltimg.cols, fltimg.rows));

    //copy small-image to tile-image
    fltimg.copyTo(roi);

    //repeat the process
    //2nd part
    myfilter(grayimg, fltimg, col2);

    roi= tileimg(Rect(fltimg.cols, 0, fltimg.cols, fltimg.rows));
    fltimg.copyTo(roi);
    //3rd part
    myfilter(grayimg, fltimg, col3);

    roi= tileimg(Rect(0, fltimg.rows, fltimg.cols, fltimg.rows));
    fltimg.copyTo(roi);
    //4th part
    myfilter(grayimg, fltimg, col4);

    roi= tileimg(Rect(fltimg.cols, fltimg.rows, fltimg.cols, fltimg.rows));
    fltimg.copyTo(roi);
    return tileimg;
}

/**
          filter the image with four given color
*/
void myfilter(Mat grayimg, Mat &fltimg, int col[4][3]){
    //For each pixel, reassign intensity value
    for (int i = 0; i < grayimg.rows; i++){
          for (int j = 0; j < grayimg.cols; j++){
                    int intensity = grayimg.at<uchar>(i,j);
                    if ( intensity >= 0 && intensity <= 64) {
                              //group 1
                              fltimg.at<Vec3b>(i,j)[0] = col[0][2];//B
                              fltimg.at<Vec3b>(i,j)[1] = col[0][1];//G
                              fltimg.at<Vec3b>(i,j)[2] = col[0][0];//R
                    }
                    else if (intensity >=65 && intensity <=127) {
                              //group 2
                              fltimg.at<Vec3b>(i,j)[0] = col[1][2];//B
                              fltimg.at<Vec3b>(i,j)[1] = col[1][1];//G
                              fltimg.at<Vec3b>(i,j)[2] = col[1][0];//R
                    }
                    else if (intensity >=128 && intensity <=192) {
                              //group 3
                              fltimg.at<Vec3b>(i,j)[0] = col[2][2];//B
                              fltimg.at<Vec3b>(i,j)[1] = col[2][1];//G
                              fltimg.at<Vec3b>(i,j)[2] = col[2][0];//R
                    }
                    else if (intensity >=193 && intensity <=255) {
                              //group 4
                              fltimg.at<Vec3b>(i,j)[0] = col[3][2];//B
                              fltimg.at<Vec3b>(i,j)[1] = col[3][1];//G
                              fltimg.at<Vec3b>(i,j)[2] = col[3][0];//R
                    }
          }
    }
}
