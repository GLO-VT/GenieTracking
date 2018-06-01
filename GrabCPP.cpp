// -----------------------------------------------------------------------------------------
// Sapera++ console grab example
// 
//    This program shows how to grab images from a camera into a buffer in the host
//    computer's memory, using Sapera++ Acquisition and Buffer objects, and a Transfer 
//    object to link them.  Also, a View object is used to display the buffer.
//
// -----------------------------------------------------------------------------------------

// Disable deprecated function warnings with Visual Studio 2005\

/// Modified for Use with OpenCV 
/// Comments with "///" indicate changes from original code 

#if defined(_MSC_VER) && _MSC_VER >= 1400
#pragma warning(disable: 4995)
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include "stdio.h"
#include "conio.h"
#include "math.h"
#include "sapclassbasic.h"
#include "ExampleUtils.h"
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>

using namespace cv;
using namespace std;


// Restore deprecated function warnings with Visual Studio 2005
#if defined(_MSC_VER) && _MSC_VER >= 1400
#pragma warning(default: 4995)
#endif

// Static Functions
static void XferCallback(SapXferCallbackInfo *pInfo);
static BOOL GetOptions(int argc, char *argv[], char *acqServerName, UINT32 *pAcqDeviceIndex, char *configFileName);
static BOOL GetOptionsFromCommandLine(int argc, char *argv[], char *acqServerName, UINT32 *pAcqDeviceIndex, char *configFileName);

/// Globals 
Mat Img;
Rect bounding_rect;
Point center;
vector<vector<Point>> contours; 
vector<Vec4i> hierarchy;

/// Address for UDP Socket
const char* srcIP = "127.0.0.1";
const char* destIP = "127.0.0.1";
int distx;
int disty;
int framecenx, frameceny,x,y;

int main(int argc, char* argv[])
{
   UINT32   acqDeviceNumber;
   char*    acqServerName = new char[CORSERVER_MAX_STRLEN];
   char*    configFilename = new char[MAX_PATH];

   printf("Sapera Console Grab Example (C++ version)\n");

   // Call GetOptions to determine which acquisition device to use and which config
   // file (CCF) should be loaded to configure it.
   // Note: if this were an MFC-enabled application, we could have replaced the lengthy GetOptions 
   // function with the CAcqConfigDlg dialog of the Sapera++ GUI Classes (see GrabMFC example)
   if (!GetOptions(argc, argv, acqServerName, &acqDeviceNumber, configFilename))
   {
      printf("\nPress any key to terminate\n");
      CorGetch();
      return 0;
   }

  

   

   SapAcquisition Acq;
   SapAcqDevice AcqDevice;
   SapBufferWithTrash Buffers;
   SapTransfer AcqToBuf = SapAcqToBuf(&Acq, &Buffers);
   SapTransfer AcqDeviceToBuf = SapAcqDeviceToBuf(&AcqDevice, &Buffers);
   SapTransfer* Xfer = NULL;
   SapView View;
   SapLocation loc(acqServerName, acqDeviceNumber);

   if (SapManager::GetResourceCount(acqServerName, SapManager::ResourceAcq) > 0)
   {
      Acq = SapAcquisition(loc, configFilename);
      Buffers = SapBufferWithTrash(2, &Acq);
      View = SapView(&Buffers, SapHwndAutomatic);
      AcqToBuf = SapAcqToBuf(&Acq, &Buffers, XferCallback, &View);
      Xfer = &AcqToBuf;

      // Create acquisition object
      if (!Acq.Create())
         goto FreeHandles;

   }

   else if (SapManager::GetResourceCount(acqServerName, SapManager::ResourceAcqDevice) > 0)
   {
      if (strcmp(configFilename, "NoFile") == 0)
         AcqDevice = SapAcqDevice(loc, FALSE);
      else
         AcqDevice = SapAcqDevice(loc, configFilename);

      Buffers = SapBufferWithTrash(2, &AcqDevice);
      View = SapView(&Buffers, SapHwndAutomatic);
      AcqDeviceToBuf = SapAcqDeviceToBuf(&AcqDevice, &Buffers, XferCallback, &View);
      Xfer = &AcqDeviceToBuf;

      // Create acquisition object
      if (!AcqDevice.Create())
         goto FreeHandles;
   }

   ///Initialize MAT Container for OpenCV to correct width and heigh, filled with zeros 
   Img = Mat::zeros(Buffers.GetHeight(), Buffers.GetWidth(), CV_8UC1);

   // Create buffer object
   if (!Buffers.Create())
      goto FreeHandles;
   

   // Create transfer object
   if (Xfer && !Xfer->Create())
      goto FreeHandles;

   // Create view object
   if (!View.Create())
      goto FreeHandles;
   
   // Start continous grab
  
   Xfer->Grab();
   
   ///Initialize MAT Container for OpenCV to correct width and heigh, filled with zeros  (need 2nd time to prevent being skipped by goto)
   Img = Mat::zeros(Buffers.GetHeight(), Buffers.GetWidth(), CV_8UC1);

   printf("Press any key to stop grab\n");
   CorGetch();

   // Stop grab
   Xfer->Freeze();
   if (!Xfer->Wait(5000))
      printf("Grab could not stop properly.\n");

FreeHandles:

   printf("Press any key to terminate\n");

   CorGetch();

   //unregister the acquisition callback
   Acq.UnregisterCallback();

   // Destroy view object
   if (!View.Destroy()) return FALSE;

   // Destroy transfer object
   if (Xfer && *Xfer && !Xfer->Destroy()) return FALSE;

   // Destroy buffer object
   if (!Buffers.Destroy()) return FALSE;

   // Destroy acquisition object
   if (!Acq.Destroy()) return FALSE;

   // Destroy acquisition object
   if (!AcqDevice.Destroy()) return FALSE;

   return 0;
}

static void XferCallback(SapXferCallbackInfo *pInfo)
{
   SapView *pView = (SapView *)pInfo->GetContext();
  
   int largest_area = 0;
   int largest_contour_index = 0;
   PUINT8 pData;
   char x_thos ;
   char x_huns ;
   char x_tens ;
   char x_ones ;
   			   ;
   char y_thos ;
   char y_huns ;
   char y_tens ;
   char y_ones ;
  
   // refresh view
   ///pView->Show();  no longer want to waste resources viewing with sapera window, using openCV window 
  
   SapBuffer *Buf = pView->GetBuffer(); ///retrieve current buffer
   Buf->GetAddress((void**)&pData); ///retrieve pointer to intensity matrix values from Buffer  
   for (int i = 0; i < Buf->GetHeight(); i++) ///loop through the size of the whole image and store intensity values in the Mat container Img
   {
	   for (int j = 0; j < Buf->GetWidth(); j++)
	   {
		   
		   Img.at<uchar>(i, j) = *pData;
		   pData++; ///increment through each memory address of pData 
	   }
   }
   Buf->ReleaseAddress((void**)&pData); ///release pointer to intensity matrix 
  
   /// GaussianBlur(Img, Img, Size(3, 3), 0, 0); 
   ///Filter the image (this operation lowers the number of contours, but is an expensive operation, usually faster to use only threshold)

   threshold(Img,Img, 30, 255, THRESH_BINARY); ///Threshold the gray
   findContours(Img, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_TC89_L1); /// Find the contours in the image
   
   for (int i = 0; i< contours.size(); i++) /// iterate through each contour. 
   {
	   double a = contourArea(contours[i], false);  ///  Find the area of contour

	   if (a>largest_area) 
	   {
		   largest_area = a;
		   largest_contour_index = i;                ///Store the index of largest contour
		   bounding_rect = boundingRect(contours[i]); /// Find the bounding rectangle for biggest contour   
		  // cout << i << "\n";
		  //cout << contours.size() << "\n";
	   }
   }
  
   rectangle(Img, bounding_rect, Scalar(120,120 ,120), 5, 2, 0); ///draw a rectangle around the disk of the sun 
   center = (bounding_rect.br() + bounding_rect.tl())*0.5; ///find center of the circle 
   circle(Img, center, 5, Scalar(120, 120, 120), 2); /// draw center on circle 
   
   x = center.x;
   y = center.y;


 //  frame_center.x = (Buf->GetWidth()) / 2;
 //  frame_center.y = (Buf->GetHeight()) / 2;  ///center of the frame 

   framecenx = (Buf->GetWidth()) / 2;
   frameceny = (Buf->GetHeight()) / 2;
   distx = x - framecenx;
   disty = y - frameceny;


   //cout << "Distance from center of Frame: " << (center.x - frame_center.x) << "," << (center.y - frame_center.y) << "\n";


   ///Data must be sent through UDP as Const Char* , so coordinate location parsed into chars 
   /// +48 accounts for the offset of ASCII values 
   
   if ((distx < 0)) ///conditions for if less than to send as const char * 
   {
	   x_thos = (char)((abs(distx) % 10) + 48); ///value for negative sign is ASCII
	   x_huns = (char)(((abs(distx) / 10) % 10) + 48);
	   x_tens = (char)(((abs(distx) / 100) % 10) + 48);
	   x_ones = 45;
   }
   else
   {
	  x_thos = (char)(((distx % 10) + 48));
	  x_huns = (char)((((distx) / 10) % 10) + 48);
	  x_tens = (char)((((distx) / 100) % 10) + 48);
	  x_ones = (char)((((distx) / 1000) % 10) + 48);

	  
   }
   if ((disty) < 0) ///identical to above but for y coordinate
   {
	   y_thos = (char)((abs(disty)%10) + 48);
	   y_huns = (char)(((abs(disty) / 10) % 10) + 48);
	   y_tens = (char)(((abs(disty) / 100) % 10) + 48);
	   y_ones = 45;
   }
   else
   {
	   y_thos = (char)(((disty % 10) + 48));
	   y_huns = (char)((((disty) / 10) % 10) + 48);
	   y_tens = (char)((((disty) / 100) % 10) + 48);
	   y_ones = (char)((((disty) / 1000) % 10) + 48);
   }
   
   
   /*
   x_thos = (char)(((x) % 10) + 48);
   x_huns = (char)((((x) / 10) % 10) + 48);
   x_tens = (char)((((x) / 100) % 10) + 48);
   x_ones = (char)((((x) / 1000) % 10) + 48);

   y_thos = (char)(((y) % 10) + 48);
   y_huns = (char)((((y) / 10) % 10) + 48);
   y_tens = (char)((((y) / 100) % 10) + 48);
   y_ones = (char)((((y) / 1000) % 10) + 48);
   */


   char comma = 44; ///ASCII value for comma 

   char pkt[18] = { x_ones,x_tens,x_huns,x_thos,comma,y_ones,y_tens,y_huns,y_thos }; ///Packet to be sent 

   ///UDP Configuartion 
   sockaddr_in dest;
   sockaddr_in local;
   WSAData data;
   WSAStartup(MAKEWORD(2, 2), &data);
   ULONG* srcAddr = new ULONG;
   ULONG* destAddr = new ULONG;

   local.sin_family = AF_INET;
   inet_pton(AF_INET, srcIP, srcAddr);
   local.sin_addr.s_addr = *srcAddr;
   local.sin_port = htons(0);

   dest.sin_family = AF_INET;
   inet_pton(AF_INET, destIP, destAddr);
   dest.sin_addr.s_addr = *destAddr;
   dest.sin_port = htons(8080);

   SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
   bind(s, (sockaddr *)&local, sizeof(local));

   sendto(s, pkt, strlen(pkt), 0, (sockaddr *)&dest, sizeof(dest));



   imshow("frame", Img);  /// ***USE THESE TO SHOW WITH OPENCV WINDOW
   waitKey(1);
   
   
   // refresh framerate
   static float lastframerate = 0.0f;
   SapTransfer* pXfer = pInfo->GetTransfer();
   if (pXfer->UpdateFrameRateStatistics())
   {
	   /// export the array of values here 

	  
      SapXferFrameRateInfo* pFrameRateInfo = pXfer->GetFrameRateStatistics();
      float framerate = 0.0f;

	  
      if (pFrameRateInfo->IsLiveFrameRateAvailable())
         framerate = pFrameRateInfo->GetLiveFrameRate();

      // check if frame rate is stalled
      if (pFrameRateInfo->IsLiveFrameRateStalled())
      {
         printf("Live frame rate is stalled.\n");
      }
      // update FPS only if the value changed by +/- 0.1
      else if ((framerate > 0.0f) && (abs(lastframerate - framerate) > 0.1f))
      {
         printf("Grabbing at %.1f frames/sec\n", framerate);
         lastframerate = framerate;
      }


   }

   delete srcAddr;
   delete destAddr;
}

static BOOL GetOptions(int argc, char *argv[], char *acqServerName, UINT32 *pAcqDeviceIndex, char *configFileName)
{
   // Check if arguments were passed
   if (argc > 1)
      return GetOptionsFromCommandLine(argc, argv, acqServerName, pAcqDeviceIndex, configFileName);
   else
      return GetOptionsFromQuestions(acqServerName, pAcqDeviceIndex, configFileName);
}

static BOOL GetOptionsFromCommandLine(int argc, char *argv[], char *acqServerName, UINT32 *pAcqDeviceIndex, char *configFileName)
{
   // Check the command line for user commands
   if ((strcmp(argv[1], "/?") == 0) || (strcmp(argv[1], "-?") == 0))
   {
      // print help
      printf("Usage:\n");
      printf("GrabCPP [<acquisition server name> <acquisition device index> <config filename>]\n");
      return FALSE;
   }

   // Check if enough arguments were passed
   if (argc < 4)
   {
      printf("Invalid command line!\n");
      return FALSE;
   }

   // Validate server name
   if (SapManager::GetServerIndex(argv[1]) < 0)
   {
      printf("Invalid acquisition server name!\n");
      return FALSE;
   }

   // Does the server support acquisition?
   int deviceCount = SapManager::GetResourceCount(argv[1], SapManager::ResourceAcq);
   int cameraCount = SapManager::GetResourceCount(argv[1], SapManager::ResourceAcqDevice);

   if (deviceCount + cameraCount == 0)
   {
      printf("This server does not support acquisition!\n");
      return FALSE;
   }

   // Validate device index
   if (atoi(argv[2]) < 0 || atoi(argv[2]) >= deviceCount + cameraCount)
   {
      printf("Invalid acquisition device index!\n");
      return FALSE;
   }

   ///if (cameraCount == 0)
   {
      // Verify that the specified config file exist
      OFSTRUCT of = { 0 };
      if (OpenFile(argv[3], &of, OF_EXIST) == HFILE_ERROR)
      {
         printf("The specified config file (%s) is invalid!\n", argv[3]);
         return FALSE;
      }
   }

   // Fill-in output variables
   CorStrncpy(acqServerName, argv[1], CORSERVER_MAX_STRLEN);
   *pAcqDeviceIndex = atoi(argv[2]);
   ///if (cameraCount == 0)
      CorStrncpy(configFileName, argv[3], MAX_PATH);

   return TRUE;
}

