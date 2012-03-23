//=============================================================================
// Copyright ?2010 NaturalPoint, Inc. All Rights Reserved.
// 
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall NaturalPoint, Inc. or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//=============================================================================


/*

SampleClient.cpp

This program connects to a NatNet server, receives a data stream, and writes that data stream
to an ascii file.  The purpose is to illustrate using the NatNetClient class.

Usage [optional]:

	SampleClient [ServerIP] [LocalIP] [OutputFilename]

	[ServerIP]			IP address of the server (e.g. 192.168.0.107) ( defaults to local machine)
	[OutputFilename]	Name of points file (pts) to write out.  defaults to Client-output.pts

*/

#include <stdio.h>
#include <tchar.h>
#include <conio.h>
#include <winsock2.h>

/*
#include <winsock2.h>   // must include before windows.h or ws2tcpip.h
#include <windows.h>
#include <Ws2tcpip.h>
#include <tchar.h>
#include <math.h>
#include "NATUtils.h"
#include <float.h>
*/

#include <math.h>
#include "NatNetTypes.h"
#include "NatNetClient.h"
#include "natutils.h"

#pragma warning( disable : 4996 )

void _WriteHeader(FILE* fp, sDataDescriptions* pBodyDefs);
void _WriteFrame(FILE* fp, sFrameOfMocapData* data);
void _WriteFooter(FILE* fp);
void __cdecl DataHandler(sFrameOfMocapData* data, void* pUserData);			// receives data from the server
void __cdecl MessageHandler(int msgType, char* msg);		// receives NatNet error mesages
void resetClient();
int CreateClient(int iConnectionType);
void Matrix_FromQuat(HMatrix &M,double x, double y, double z, double w, int order);   //EulerAngles is the same type as quat

unsigned int MyServersDataPort = 3130;
unsigned int MyServersCommandPort = 3131;

NatNetClient* theClient;
FILE* fp;

char szMyIPAddress[128] = "";
char szServerIPAddress[128] = "";

int _tmain(int argc, _TCHAR* argv[])
{
    int iResult;
    int iConnectionType = ConnectionType_Multicast;
    //int iConnectionType = ConnectionType_Unicast;
    
    // parse command line args
    if(argc>1)
    {
        strcpy(szServerIPAddress, argv[1]);	// specified on command line
        printf("Connecting to server at %s...\n", szServerIPAddress);
    }
    else
    {
        strcpy(szServerIPAddress, "");		// not specified - assume server is local machine
        printf("Connecting to server at LocalMachine\n");
    }
    if(argc>2)
    {
        strcpy(szMyIPAddress, argv[2]);	// specified on command line
        printf("Connecting from %s...\n", szMyIPAddress);
    }
    else
    {
        strcpy(szMyIPAddress, "");		// not specified - assume server is local machine
        printf("Connecting from LocalMachine...\n");
    }

    // Create NatNet Client
    iResult = CreateClient(iConnectionType);
    if(iResult != ErrorCode_OK)
    {
        printf("Error initializing client.  See log for details.  Exiting");
        return 1;
    }
    else
    {
        printf("Client initialized and ready.\n");
    }


	// send/receive test request
	printf("[SampleClient] Sending Test Request\n");
	void* response;
	int nBytes;
	iResult = theClient->SendMessageAndWait("TestRequest", &response, &nBytes);
	if (iResult == ErrorCode_OK)
	{
		printf("[SampleClient] Received: %s", (char*)response);
	}

	// Retrieve Data Descriptions from server
	printf("\n\n[SampleClient] Requesting Data Descriptions...");
	sDataDescriptions* pDataDefs = NULL;
	int nBodies = theClient->GetDataDescriptions(&pDataDefs);
	if(!pDataDefs)
	{
		printf("[SampleClient] Unable to retrieve Data Descriptions.");
		//return 1;
	}
	else
	{
        printf("[SampleClient] Received %d Data Descriptions:\n", pDataDefs->nDataDescriptions );
        for(int i=0; i < pDataDefs->nDataDescriptions; i++)
        {
            printf("Data Description # %d (type=%d)\n", i, pDataDefs->arrDataDescriptions[i].type);
            if(pDataDefs->arrDataDescriptions[i].type == Descriptor_MarkerSet)
            {
                // MarkerSet
                sMarkerSetDescription* pMS = pDataDefs->arrDataDescriptions[i].Data.MarkerSetDescription;
                //printf("MarkerSet Name : %s\n", pMS->szName);
                //for(int i=0; i < pMS->nMarkers; i++)
                   // printf("%s\n", pMS->szMarkerNames[i]);

            }
            else if(pDataDefs->arrDataDescriptions[i].type == Descriptor_RigidBody)
            {
                // RigidBody
                sRigidBodyDescription* pRB = pDataDefs->arrDataDescriptions[i].Data.RigidBodyDescription;
                //printf("RigidBody Name : %s\n", pRB->szName);
                //printf("RigidBody ID : %d\n", pRB->ID);
                //printf("RigidBody Parent ID : %d\n", pRB->parentID);
                //printf("Parent Offset : %3.2f,%3.2f,%3.2f\n", pRB->offsetx, pRB->offsety, pRB->offsetz);
            }
            else if(pDataDefs->arrDataDescriptions[i].type == Descriptor_Skeleton)
            {
                // Skeleton
                sSkeletonDescription* pSK = pDataDefs->arrDataDescriptions[i].Data.SkeletonDescription;
                //printf("Skeleton Name : %s\n", pSK->szName);
                //printf("Skeleton ID : %d\n", pSK->skeletonID);
                //printf("RigidBody (Bone) Count : %d\n", pSK->nRigidBodies);
                for(int j=0; j < pSK->nRigidBodies; j++)
                {
					// dan edit
                    sRigidBodyDescription* pRB = &pSK->RigidBodies[j];
                    //printf("  RigidBody Name : %s\n", pRB->szName);
                    //printf("  RigidBody ID : %d\n", pRB->ID);
                   // printf("  RigidBody Parent ID : %d\n", pRB->parentID);
                    //printf("  Parent Offset : %3.2f,%3.2f,%3.2f\n", pRB->offsetx, pRB->offsety, pRB->offsetz);
                }
            }
            else
            {
                printf("Unknown data type.");
                // Unknown
            }
        }      
	}

	
	// Create data file for writing received stream into
	char szFile[MAX_PATH];
	char szFolder[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, szFolder);
	if(argc > 3)
		sprintf(szFile, "%s\\%s", szFolder, argv[3]);
	else
		sprintf(szFile, "%s\\rpyLocal.pts",szFolder);
	fp = fopen(szFile, "w");
	if(!fp)
	{
		printf("error opening output file %s.  Exiting.", szFile);
		exit(1);
	}
	if(pDataDefs)
		_WriteHeader(fp, pDataDefs);

	// Ready to receive marker stream!
	printf("\nClient is connected to server and listening for data...\n");
	int c;
	bool bExit = false;
	while(c =_getch())
	{
		switch(c)
		{
			case 'q':
				bExit = true;		
				break;	
			case 'r':
				resetClient();
				break;	
            case 'p':
                sServerDescription ServerDescription;
                memset(&ServerDescription, 0, sizeof(ServerDescription));
                theClient->GetServerDescription(&ServerDescription);
                if(!ServerDescription.HostPresent)
                {
                    printf("Unable to connect to server. Host not present. Exiting.");
                    return 1;
                }
                break;	
            case 'f':
                {
                    sFrameOfMocapData* pData = theClient->GetLastFrameOfData();
                    printf("Most Recent Frame: %d", pData->iFrame);
                }
                break;	
            case 'm':	                        // change to multicast
                iResult = CreateClient(ConnectionType_Multicast);
                if(iResult == ErrorCode_OK)
                    printf("Client connection type changed to Multicast.\n\n");
                else
                    printf("Error changing client connection type to Multicast.\n\n");
                break;
            case 'u':	                        // change to unicast
                iResult = CreateClient(ConnectionType_Unicast);
                if(iResult == ErrorCode_OK)
                    printf("Client connection type changed to Unicast.\n\n");
                else
                    printf("Error changing client connection type to Unicast.\n\n");
                break;


			default:
				break;
		}
		if(bExit)
			break;
	}

	// Done - clean up.
	theClient->Uninitialize();
	_WriteFooter(fp);
	fclose(fp);

	return ErrorCode_OK;
}

// Establish a NatNet Client connection
int CreateClient(int iConnectionType)
{
    // release previous server
    if(theClient)
    {
        theClient->Uninitialize();
        delete theClient;
    }

    // create NatNet client
    theClient = new NatNetClient(iConnectionType);

    // [optional] use old multicast group
    //theClient->SetMulticastAddress("224.0.0.1");

    // print version info
    unsigned char ver[4];
    theClient->NatNetVersion(ver);
    printf("NatNet Sample Client (NatNet ver. %d.%d.%d.%d)\n", ver[0], ver[1], ver[2], ver[3]);

    // Set callback handlers
    theClient->SetMessageCallback(MessageHandler);
    theClient->SetVerbosityLevel(Verbosity_Debug);
    theClient->SetDataCallback( DataHandler, theClient );	// this function will receive data from the server

    // Init Client and connect to NatNet server
    // to use NatNet default port assigments
    int retCode = theClient->Initialize(szMyIPAddress, szServerIPAddress);
    // to use a different port for commands and/or data:
    //int retCode = theClient->Initialize(szMyIPAddress, szServerIPAddress, MyServersCommandPort, MyServersDataPort);
    if (retCode != ErrorCode_OK)
    {
        printf("Unable to connect to server.  Error code: %d. Exiting", retCode);
        return ErrorCode_Internal;
    }
    else
    {
        // print server info
        sServerDescription ServerDescription;
        memset(&ServerDescription, 0, sizeof(ServerDescription));
        theClient->GetServerDescription(&ServerDescription);
        if(!ServerDescription.HostPresent)
        {
            printf("Unable to connect to server. Host not present. Exiting.");
            return 1;
        }
        printf("[SampleClient] Server application info:\n");
        printf("Application: %s (ver. %d.%d.%d.%d)\n", ServerDescription.szHostApp, ServerDescription.HostAppVersion[0],
            ServerDescription.HostAppVersion[1],ServerDescription.HostAppVersion[2],ServerDescription.HostAppVersion[3]);
        printf("NatNet Version: %d.%d.%d.%d\n", ServerDescription.NatNetVersion[0], ServerDescription.NatNetVersion[1],
            ServerDescription.NatNetVersion[2], ServerDescription.NatNetVersion[3]);
        printf("Client IP:%s\n", szMyIPAddress);
        printf("Server IP:%s\n", szServerIPAddress);
        printf("Server Name:%s\n\n", ServerDescription.szHostComputerName);
    }

    return ErrorCode_OK;

}

// DataHandler receives data from the server
void __cdecl DataHandler(sFrameOfMocapData* data, void* pUserData)
{
	NatNetClient* pClient = (NatNetClient*) pUserData;

	printf("Received frame %d\n", data->iFrame);
	if(fp)
		_WriteFrame(fp,data);
	int i=0;

    // same system latency test
    float fThisTick = (float)GetTickCount();
    float fDiff = fThisTick - data->fLatency;
    double dDuration = fDiff;
    //printf("Latency (same system) (msecs): %3.2lf\n", dDuration);


	// Other Markers
	//printf("Other Markers [Count=%d]\n", data->nOtherMarkers);
	for(i=0; i < data->nOtherMarkers; i++)
	{
		//printf("Other Marker %d : %3.2f\t%3.2f\t%3.2f\n",
			//i,
			//data->OtherMarkers[i][0],
			//data->OtherMarkers[i][1],
			//data->OtherMarkers[i][2]);
	}

	// Rigid Bodies
	//printf("Rigid Bodies [Count=%d]\n", data->nRigidBodies);
	for(i=0; i < data->nRigidBodies; i++)
	{
		//printf("Rigid Body [ID=%d  Error=%3.2f]\n", data->RigidBodies[i].ID, data->RigidBodies[i].MeanError);
		//printf("\tx\ty\tz\tqx\tqy\tqz\tqw\n");
		//printf("\t%3.2f\t%3.2f\t%3.2f\t%3.2f\t%3.2f\t%3.2f\t%3.2f\n",
			//data->RigidBodies[i].x,
			//data->RigidBodies[i].y,
			//data->RigidBodies[i].z,
			//data->RigidBodies[i].qx,
			//data->RigidBodies[i].qy,
			//data->RigidBodies[i].qz,
			//data->RigidBodies[i].qw);

		//printf("\tRigid body markers [Count=%d]\n", data->RigidBodies[i].nMarkers);
		//for(int iMarker=0; iMarker < data->RigidBodies[i].nMarkers; iMarker++)
		//{
            //printf("\t\t");
            //if(data->RigidBodies[i].MarkerIDs)
                //printf("MarkerID:%d", data->RigidBodies[i].MarkerIDs[iMarker]);
            //if(data->RigidBodies[i].MarkerSizes)
                //printf("\tMarkerSize:%3.2f", data->RigidBodies[i].MarkerSizes[iMarker]);
            //if(data->RigidBodies[i].Markers)
                //printf("\tMarkerPos:%3.2f,%3.2f,%3.2f\n" ,
                    //data->RigidBodies[i].Markers[iMarker][0],
                    //data->RigidBodies[i].Markers[iMarker][1],
                    //data->RigidBodies[i].Markers[iMarker][2]);
      //  }
	}

    // skeletons
    //printf("Skeletons [Count=%d]\n", data->nSkeletons);
    for(i=0; i < data->nSkeletons; i++)
    {
        sSkeletonData skData = data->Skeletons[i];
        //printf("Skeleton [ID=%d  Bone count=%d]\n", skData.skeletonID, skData.nRigidBodies);
        for(int j=0; j< skData.nRigidBodies; j++)
        {
            sRigidBodyData rbData = skData.RigidBodyData[j];
            //printf("Bone %d\t%3.2f\t%3.2f\t%3.2f\t%3.2f\t%3.2f\t%3.2f\t%3.2f\n",
                    //rbData.ID, rbData.x, rbData.y, rbData.z, rbData.qx, rbData.qy, rbData.qz, rbData.qw );
    
            //printf("\tRigid body markers [Count=%d]\n", rbData.nMarkers);
           // for(int iMarker=0; iMarker < data->RigidBodies[i].nMarkers; iMarker++)
            //{
                //printf("\t\t");
               // if(rbData.MarkerIDs)
                    //printf("MarkerID:%d", rbData.MarkerIDs[iMarker]);
                //if(rbData.MarkerSizes)
                    //printf("\tMarkerSize:%3.2f", rbData.MarkerSizes[iMarker]);
                //if(rbData.Markers)
                    //printf("\tMarkerPos:%3.2f,%3.2f,%3.2f\n" ,
                    //data->RigidBodies[i].Markers[iMarker][0],
                    //data->RigidBodies[i].Markers[iMarker][1],
                    //data->RigidBodies[i].Markers[iMarker][2]);
            //}
        }
    }
}

// MessageHandler receives NatNet error/debug messages
void __cdecl MessageHandler(int msgType, char* msg)
{
	//printf("\n%s\n", msg);
}

/* File writing routines */
void _WriteHeader(FILE* fp, sDataDescriptions* pBodyDefs)
{
	fprintf(fp,"LuarmR LuarmP LuarmY X Y Z LuarmL ");                 
	fprintf(fp,"LelbowR LelbowP LelbowY X Y Z LelbowL ");
	fprintf(fp,"RuarmR RuarmP RuarmY X Y Z  RuarmL ");
	fprintf(fp,"RelbowR RelbowP RelbowY X Y Z RelbowL ");
	fprintf(fp,"LThighR LThighP LThighY X Y Z LThighL ");
	fprintf(fp,"RThighR RThighP RThighY X Y Z RThighL ");
	fprintf(fp,"LShinR LShinP LShinY X Y Z LShinL ");
	fprintf(fp,"RShinR RShinP RShinY X Y Z RShinL ");
	fprintf(fp,"LfootR LfootP LfootY X Y Z LfootL ");
	fprintf(fp,"RfootR RfootP RfootY X Y Z RfootL ");
	fprintf(fp,"HipR HipP HipY X Y Z HipL ");
	fprintf(fp,"LShoulderR  LShoulderP LShoulderY X Y Z LShoulderL ");
	fprintf(fp,"RShoulderR  RShoulderP RShoulderY X Y Z RShoulderL ");
	fprintf(fp,"LhandR  LhandP  LhandY X Y Z LhandL ");
	fprintf(fp,"RhandR  RhandP  RhandY X Y Z RhandL ");
	fprintf(fp,"HeadR  HeadP  HeadY X Y Z HeadL "); 
	fprintf(fp,"ChestR ChestP ChestY X Y Z ChestL "); 
	fprintf(fp,"UChestR UChestP UChestY X Y Z UChestL ");
	fprintf(fp,"LfooteR LfooteP LfooteY X Y Z LfooteL ");
	fprintf(fp,"RfooteR RfooteP RfooteY X Y Z RfooteL ");
	fprintf(fp,"NeckR NeckP NeckY X Y Z NeckL");
	fprintf(fp,"Frame ");
	fprintf(fp,"\n");

}
/*
void Matrix_FromQuat(HMatrix &M,double x, double y, double z, double w, int order)   //EulerAngles is the same type as quat
{
    //HMatrix M;
    double Nq = x*x+y*y+z*z+w*w;
    double s = (Nq > 0.0) ? (2.0 / Nq) : 0.0;
    double xs = x*s,	  ys = y*s,	 zs = z*s;
    double wx = w*xs,	  wy = w*ys,	 wz = w*zs;
    double xx = x*xs,	  xy = x*ys,	 xz = x*zs;
    double yy = y*ys,	  yz = y*zs,	 zz = z*zs;
    M[X][X] = 1.0 - (yy + zz); M[X][Y] = xy - wz; M[X][Z] = xz + wy;
    M[Y][X] = xy + wz; M[Y][Y] = 1.0 - (xx + zz); M[Y][Z] = yz - wx;
    M[Z][X] = xz - wy; M[Z][Y] = yz + wx; M[Z][Z] = 1.0 - (xx + yy);
    M[W][X]=M[W][Y]=M[W][Z]=M[X][W]=M[Y][W]=M[Z][W]=0.0; M[W][W]=1.0;
    //return (M);
}
*/
void _WriteFrame(FILE* fp, sFrameOfMocapData* data)
{
    float Luarm_r = 0.0,  Luarm_p = 0.0, Luarm_y = 0.0, Luarm_X = 0.0, Luarm_Y, Luarm_Z, Luarm_l = 0.0;
	float Lelbow_r = 0.0, Lelbow_p = 0.0, Lelbow_y = 0.0,Lelbow_X = 0.0, Lelbow_Y, Lelbow_Z, Lelbow_l = 0.0;
	float Ruarm_r = 0.0,  Ruarm_p = 0.0, Ruarm_y = 0.0, Ruarm_X = 0.0, Ruarm_Y, Ruarm_Z,Ruarm_l = 0.0;
	float Relbow_r = 0.0, Relbow_p = 0.0, Relbow_y = 0.0,Relbow_X = 0.0, Relbow_Y, Relbow_Z, Relbow_l = 0.0;
	float Lthigh_r = 0.0, Lthigh_p = 0.0, Lthigh_y = 0.0,Lthigh_X = 0.0, Lthigh_Y, Lthigh_Z, Lthigh_l = 0.0;
	float Rthigh_r = 0.0, Rthigh_p = 0.0, Rthigh_y = 0.0,Rthigh_X = 0.0, Rthigh_Y, Rthigh_Z, Rthigh_l = 0.0;
	float Lshin_r = 0.0,  Lshin_p = 0.0, Lshin_y = 0.0,Lshin_X = 0.0, Lshin_Y, Lshin_Z, Lshin_l = 0.0;
	float Rshin_r = 0.0,  Rshin_p = 0.0, Rshin_y = 0.0,Rshin_X = 0.0, Rshin_Y, Rshin_Z, Rshin_l = 0.0;
	float Lfoot_r = 0.0,  Lfoot_p = 0.0, Lfoot_y = 0.0,Lfoot_X = 0.0, Lfoot_Y, Lfoot_Z, Lfoot_l = 0.0;
	float Rfoot_r = 0.0,  Rfoot_p = 0.0, Rfoot_y = 0.0,Rfoot_X = 0.0, Rfoot_Y, Rfoot_Z, Rfoot_l = 0.0;
	float UHip_r = 0.0,   UHip_p = 0.0,  UHip_y = 0.0,UHip_X = 0.0, UHip_Y = 0.0, UHip_Z = 0.0, UHip_l = 0.0;
	float Lshoulder_r = 0.0, Lshoulder_p = 0.0, Lshoulder_y = 0.0,Lshoulder_X = 0.0, Lshoulder_Y = 0.0, Lshoulder_Z = 0.0,Lshoulder_l = 0.0;
	float Rshoulder_r = 0.0, Rshoulder_p = 0.0, Rshoulder_y = 0.0,Rshoulder_X = 0.0, Rshoulder_Y = 0.0, Rshoulder_Z = 0.0,Rshoulder_l = 0.0;
	float Lhand_r = 0.0,  Lhand_p = 0.0, Lhand_y = 0.0,Lhand_X = 0.0,  Lhand_Y = 0.0, Lhand_Z = 0.0,Lhand_l = 0.0;
	float Rhand_r = 0.0,  Rhand_p = 0.0, Rhand_y = 0.0,Rhand_X = 0.0,  Rhand_Y = 0.0, Rhand_Z = 0.0,Rhand_l = 0.0;
    float Head_r = 0.0,   Head_p = 0.0,  Head_y = 0.0,Head_X = 0.0,   Head_Y = 0.0,  Head_Z = 0.0,Head_l = 0.0;
	float Chest_r = 0.0,  Chest_p = 0.0, Chest_y = 0.0,Chest_X = 0.0,  Chest_Y = 0.0, Chest_Z = 0.0,Chest_l = 0.0;
	float Uchest_r = 0.0, Uchest_p = 0.0,Uchest_y = 0.0,Uchest_X = 0.0, Uchest_Y = 0.0,Uchest_Z = 0.0,Uchest_l = 0.0;
	float Lfoote_r = 0.0, Lfoote_p = 0.0,Lfoote_y = 0.0,Lfoote_X = 0.0, Lfoote_Y = 0.0,Lfoote_Z = 0.0,Lfoote_l = 0.0;
	float Rfoote_r = 0.0, Rfoote_p = 0.0,Rfoote_y = 0.0,Rfoote_X = 0.0, Rfoote_Y = 0.0,Rfoote_Z = 0.0,Rfoote_l = 0.0;
	float Neck_r = 0.0, Neck_p, Neck_y, Neck_X, Neck_Y, Neck_Z, Neck_l;

	int i=0; 
	for(i=0; i < data->nSkeletons; i++)
    {
        sSkeletonData skData = data->Skeletons[i];
		
		if( data->iFrame >= 2229 && data->iFrame <=  2430 )
		{ 
			//fprintf(fp, "Skeleton [ID=%d  Bone count=%d]\n", skData.skeletonID, skData.nRigidBodies);
		    for(int j=0; j< skData.nRigidBodies; j++)
			{
				sRigidBodyData rbData = skData.RigidBodyData[j];
				
				/*
				if(  rbData.ID == 65544 ) { char Name[] = "Lshoulder"; Luarm_x = rbData.x; 	Luarm_y = rbData.y; Luarm_z = rbData.z;} //Luarm_yaw_2 = -yaw3, Luarm_pitch_2 = -pitch3, Luarm_roll_2 = -roll3;         Luarm_rm[0][0] = 1.0 - 2.0*rbData.qy*rbData.qy - 2.0*rbData.qz*rbData.qz; Luarm_rm[0][1] = 2.0*rbData.qx*rbData.qy - 2.0*rbData.qz*rbData.qw; Luarm_rm[0][2] = 2.0*rbData.qx*rbData.qz + 2.0*rbData.qy*rbData.qw; Luarm_rm[1][0] = 2.0*rbData.qx*rbData.qy + 2.0*rbData.qz*rbData.qw; Luarm_rm[1][1] = 1.0 - 2.0*rbData.qx*rbData.qx - 2.0*rbData.qz*rbData.qz; Luarm_rm[1][2] = 2.0*rbData.qy*rbData.qz - 2.0*rbData.qx*rbData.qw; Luarm_rm[2][0] = 2.0*rbData.qx*rbData.qz - 2.0*rbData.qy*rbData.qw; Luarm_rm[2][1] = 2.0*rbData.qy*rbData.qz + 2.0*rbData.qx*rbData.qw; Luarm_rm[2][2] = 1.0 - 2.0*rbData.qx*rbData.qx - 2.0*rbData.qy*rbData.qy;}
				if(  rbData.ID == 65545 ) { char Name[] = "LElbow";    Lelbow_x = rbData.x, Lelbow_y = rbData.y, Lelbow_z = rbData.z;}// fprintf(fp," %3.2f \n",Lelbow_yaw_1);} //Lelbow_yaw_2 = -yaw3, Lelbow_pitch_2 = -pitch3, Lelbow_roll_2 = -roll3;         Lelbow_rm[0][0] = 1.0 - 2.0*rbData.qy*rbData.qy - 2.0*rbData.qz*rbData.qz; Lelbow_rm[0][1] = 2.0*rbData.qx*rbData.qy - 2.0*rbData.qz*rbData.qw; Lelbow_rm[0][2] = 2.0*rbData.qx*rbData.qz + 2.0*rbData.qy*rbData.qw; Lelbow_rm[1][0] = 2.0*rbData.qx*rbData.qy + 2.0*rbData.qz*rbData.qw; Lelbow_rm[1][1] = 1.0 - 2.0*rbData.qx*rbData.qx - 2.0*rbData.qz*rbData.qz; Lelbow_rm[1][2] = 2.0*rbData.qy*rbData.qz - 2.0*rbData.qx*rbData.qw; Lelbow_rm[2][0] = 2.0*rbData.qx*rbData.qz - 2.0*rbData.qy*rbData.qw; Lelbow_rm[2][1] = 2.0*rbData.qy*rbData.qz + 2.0*rbData.qx*rbData.qw; Lelbow_rm[2][2] = 1.0 - 2.0*rbData.qx*rbData.qx - 2.0*rbData.qy*rbData.qy;}	
				if(  rbData.ID == 65549 ) { char Name[] = "Rshoulder"; Ruarm_x = rbData.x, Ruarm_y = rbData.y, Ruarm_z = rbData.z;} //, Ruarm_yaw_2 = -yaw3, Ruarm_pitch_2 = -pitch3, Ruarm_roll_2 = -roll3;         Ruarm_rm[0][0] = 1.0 - 2.0*rbData.qy*rbData.qy - 2.0*rbData.qz*rbData.qz; Ruarm_rm[0][1] = 2.0*rbData.qx*rbData.qy - 2.0*rbData.qz*rbData.qw; Ruarm_rm[0][2] = 2.0*rbData.qx*rbData.qz + 2.0*rbData.qy*rbData.qw; Ruarm_rm[1][0] = 2.0*rbData.qx*rbData.qy + 2.0*rbData.qz*rbData.qw; Ruarm_rm[1][1] = 1.0 - 2.0*rbData.qx*rbData.qx - 2.0*rbData.qz*rbData.qz; Ruarm_rm[1][2] = 2.0*rbData.qy*rbData.qz - 2.0*rbData.qx*rbData.qw; Ruarm_rm[2][0] = 2.0*rbData.qx*rbData.qz - 2.0*rbData.qy*rbData.qw; Ruarm_rm[2][1] = 2.0*rbData.qy*rbData.qz + 2.0*rbData.qx*rbData.qw; Ruarm_rm[2][2] = 1.0 - 2.0*rbData.qx*rbData.qx - 2.0*rbData.qy*rbData.qy;}
				if(  rbData.ID == 65550 ) { char Name[] = "RElbow";    Relbow_x = rbData.x, Relbow_y = rbData.y, Relbow_z = rbData.z;}//, Relbow_yaw_2 = -yaw3, Relbow_pitch_2 = -pitch3, Relbow_roll_2 = -roll3;         Relbow_rm[0][0] = 1.0 - 2.0*rbData.qy*rbData.qy - 2.0*rbData.qz*rbData.qz; Relbow_rm[0][1] = 2.0*rbData.qx*rbData.qy - 2.0*rbData.qz*rbData.qw; Relbow_rm[0][2] = 2.0*rbData.qx*rbData.qz + 2.0*rbData.qy*rbData.qw; Relbow_rm[1][0] = 2.0*rbData.qx*rbData.qy + 2.0*rbData.qz*rbData.qw; Relbow_rm[1][1] = 1.0 - 2.0*rbData.qx*rbData.qx - 2.0*rbData.qz*rbData.qz; Relbow_rm[1][2] = 2.0*rbData.qy*rbData.qz - 2.0*rbData.qx*rbData.qw; Relbow_rm[2][0] = 2.0*rbData.qx*rbData.qz - 2.0*rbData.qy*rbData.qw; Relbow_rm[2][1] = 2.0*rbData.qy*rbData.qz + 2.0*rbData.qx*rbData.qw; Relbow_rm[2][2] = 1.0 - 2.0*rbData.qx*rbData.qx - 2.0*rbData.qy*rbData.qy;}
				
				if(  rbData.ID == 65553 ) { char Name[] = "LThigh";    Lthigh_x = rbData.x, Lthigh_y = rbData.y, Lthigh_z = rbData.z;}				
				if(  rbData.ID == 65557 ) { char Name[] = "RThigh";    Rthigh_x = rbData.x, Rthigh_y = rbData.y, Rthigh_z = rbData.z;}//, Rthigh_yaw_2 = -yaw3, Rthigh_pitch_2 = -pitch3, Rthigh_roll_2 = -roll3;   Rthigh_rm[0][0] = 1.0 - 2.0*rbData.qy*rbData.qy - 2.0*rbData.qz*rbData.qz; Rthigh_rm[0][1] = 2.0*rbData.qx*rbData.qy - 2.0*rbData.qz*rbData.qw; Rthigh_rm[0][2] = 2.0*rbData.qx*rbData.qz + 2.0*rbData.qy*rbData.qw; Rthigh_rm[1][0] = 2.0*rbData.qx*rbData.qy + 2.0*rbData.qz*rbData.qw; Rthigh_rm[1][1] = 1.0 - 2.0*rbData.qx*rbData.qx - 2.0*rbData.qz*rbData.qz; Rthigh_rm[1][2] = 2.0*rbData.qy*rbData.qz - 2.0*rbData.qx*rbData.qw; Rthigh_rm[2][0] = 2.0*rbData.qx*rbData.qz - 2.0*rbData.qy*rbData.qw; Rthigh_rm[2][1] = 2.0*rbData.qy*rbData.qz + 2.0*rbData.qx*rbData.qw; Rthigh_rm[2][2] = 1.0 - 2.0*rbData.qx*rbData.qx - 2.0*rbData.qy*rbData.qy;}
				if(  rbData.ID == 65554 ) { char Name[] = "LShin";     Lshin_x = rbData.x, Lshin_y = rbData.y, Lshin_z = rbData.z;}
				if(  rbData.ID == 65558 ) { char Name[] = "RShin";     Rshin_x = rbData.x, Rshin_y = rbData.y, Rshin_z = rbData.z;}//, Rshin_yaw_2 = -yaw3, Rshin_pitch_2 = -pitch3, Rshin_roll_2 = -roll3;         Rshin_rm[0][0] = 1.0 - 2.0*rbData.qy*rbData.qy - 2.0*rbData.qz*rbData.qz; Rshin_rm[0][1] = 2.0*rbData.qx*rbData.qy - 2.0*rbData.qz*rbData.qw; Rshin_rm[0][2] = 2.0*rbData.qx*rbData.qz + 2.0*rbData.qy*rbData.qw; Rshin_rm[1][0] = 2.0*rbData.qx*rbData.qy + 2.0*rbData.qz*rbData.qw; Rshin_rm[1][1] = 1.0 - 2.0*rbData.qx*rbData.qx - 2.0*rbData.qz*rbData.qz; Rshin_rm[1][2] = 2.0*rbData.qy*rbData.qz - 2.0*rbData.qx*rbData.qw; Rshin_rm[2][0] = 2.0*rbData.qx*rbData.qz - 2.0*rbData.qy*rbData.qw; Rshin_rm[2][1] = 2.0*rbData.qy*rbData.qz + 2.0*rbData.qx*rbData.qw; Rshin_rm[2][2] = 1.0 - 2.0*rbData.qx*rbData.qx - 2.0*rbData.qy*rbData.qy;}
				
				if(  rbData.ID == 65555 ) { char Name[] = "LFoot";     Lfoot_x = rbData.x, Lfoot_y = rbData.y, Lfoot_z = rbData.z;}//, Lfoot_yaw_2 = -yaw3, Lfoot_pitch_2 = -pitch3, Lfoot_roll_2 = -roll3;         Lfoot_rm[0][0] = 1.0 - 2.0*rbData.qy*rbData.qy - 2.0*rbData.qz*rbData.qz; Lfoot_rm[0][1] = 2.0*rbData.qx*rbData.qy - 2.0*rbData.qz*rbData.qw; Lfoot_rm[0][2] = 2.0*rbData.qx*rbData.qz + 2.0*rbData.qy*rbData.qw; Lfoot_rm[1][0] = 2.0*rbData.qx*rbData.qy + 2.0*rbData.qz*rbData.qw; Lfoot_rm[1][1] = 1.0 - 2.0*rbData.qx*rbData.qx - 2.0*rbData.qz*rbData.qz; Lfoot_rm[1][2] = 2.0*rbData.qy*rbData.qz - 2.0*rbData.qx*rbData.qw; Lfoot_rm[2][0] = 2.0*rbData.qx*rbData.qz - 2.0*rbData.qy*rbData.qw; Lfoot_rm[2][1] = 2.0*rbData.qy*rbData.qz + 2.0*rbData.qx*rbData.qw; Lfoot_rm[2][2] = 1.0 - 2.0*rbData.qx*rbData.qx - 2.0*rbData.qy*rbData.qy;}
				if(  rbData.ID == 65559 ) { char Name[] = "RFoot";     Rfoot_x = rbData.x, Rfoot_y = rbData.y, Rfoot_z = rbData.z;}//, Rfoot_yaw_2 = -yaw3, Rfoot_pitch_2 = -pitch3, Rfoot_roll_2 = -roll3;         Rfoot_rm[0][0] = 1.0 - 2.0*rbData.qy*rbData.qy - 2.0*rbData.qz*rbData.qz; Rfoot_rm[0][1] = 2.0*rbData.qx*rbData.qy - 2.0*rbData.qz*rbData.qw; Rfoot_rm[0][2] = 2.0*rbData.qx*rbData.qz + 2.0*rbData.qy*rbData.qw; Rfoot_rm[1][0] = 2.0*rbData.qx*rbData.qy + 2.0*rbData.qz*rbData.qw; Rfoot_rm[1][1] = 1.0 - 2.0*rbData.qx*rbData.qx - 2.0*rbData.qz*rbData.qz; Rfoot_rm[1][2] = 2.0*rbData.qy*rbData.qz - 2.0*rbData.qx*rbData.qw; Rfoot_rm[2][0] = 2.0*rbData.qx*rbData.qz - 2.0*rbData.qy*rbData.qw; Rfoot_rm[2][1] = 2.0*rbData.qy*rbData.qz + 2.0*rbData.qx*rbData.qw; Rfoot_rm[2][2] = 1.0 - 2.0*rbData.qx*rbData.qx - 2.0*rbData.qy*rbData.qy;}
				*/
				if(  rbData.ID == 65537 ) // Upper Hip (UHip)
				{
					Quat q;
					q.x = rbData.qx;
					q.y = rbData.qy;
					q.z = rbData.qz;
					q.w = rbData.qw;
					EulerAngles deg = Eul_FromQuat(q, EulOrdYZXr);	// anything the upper yaw dows 
					UHip_r = deg.x;
					UHip_p = deg.y;
					UHip_y = deg.z;
					UHip_l = sqrt(rbData.x*rbData.x + rbData.y*rbData.y + rbData.z*rbData.z);
					UHip_X = rbData.x;
					UHip_Y = rbData.y;
					UHip_Z = rbData.z;
					
				}

				if( rbData.ID == 65540 ) // Neck
				{
					Quat q;
					q.x = rbData.qx;
					q.y = rbData.qy;
					q.z = rbData.qz;
					q.w = rbData.qw;
					//EulerAngles deg = Eul_FromQuat(q, EulOrdYZXr);	// anything the upper yaw dows 
					EulerAngles deg = Eul_FromQuat(q, EulOrdYZXr);	// anything the upper yaw dows 
					Neck_r = deg.x;
					Neck_p = deg.y;
					Neck_y = deg.z;
					Neck_l = sqrt(rbData.x*rbData.x + rbData.y*rbData.y + rbData.z*rbData.z);
					Neck_X = rbData.x;
					Neck_Y = rbData.y;
					Neck_Z = rbData.z;

				}

				

				if(  rbData.ID == 65543 ) // Upper left shoulder
				{
					Quat q;
					q.x = rbData.qx;
					q.y = rbData.qy;
					q.z = rbData.qz;
					q.w = rbData.qw;
					//EulerAngles deg = Eul_FromQuat(q, EulOrdYZXr);	// anything the upper yaw dows 
					EulerAngles deg = Eul_FromQuat(q, EulOrdYZXr);	// anything the upper yaw dows 
					Lshoulder_r = deg.x;
					Lshoulder_p = deg.y;
					Lshoulder_y = deg.z;
					Lshoulder_l = sqrt(rbData.x*rbData.x + rbData.y*rbData.y + rbData.z*rbData.z);
					Lshoulder_X = rbData.x;
					Lshoulder_Y = rbData.y;
					Lshoulder_Z = rbData.z;
				}
				//if(  rbData.ID == 65548 ) { char Name[] = "URshoulder"; Rshoulder_x = rbData.x, Rshoulder_y = rbData.y, Rshoulder_z = rbData.z;} //Rshoulder_rm[0][0] = 1.0 - 2.0*rbData.qy*rbData.qy - 2.0*rbData.qz*rbData.qz; Rshoulder_rm[0][1] = 2.0*rbData.qx*rbData.qy - 2.0*rbData.qz*rbData.qw; Rshoulder_rm[0][2] = 2.0*rbData.qx*rbData.qz + 2.0*rbData.qy*rbData.qw; Rshoulder_rm[1][0] = 2.0*rbData.qx*rbData.qy + 2.0*rbData.qz*rbData.qw; Rshoulder_rm[1][1] = 1.0 - 2.0*rbData.qx*rbData.qx - 2.0*rbData.qz*rbData.qz; Rshoulder_rm[1][2] = 2.0*rbData.qy*rbData.qz - 2.0*rbData.qx*rbData.qw; Rshoulder_rm[2][0] = 2.0*rbData.qx*rbData.qz - 2.0*rbData.qy*rbData.qw; Rshoulder_rm[2][1] = 2.0*rbData.qy*rbData.qz + 2.0*rbData.qx*rbData.qw; Rshoulder_rm[2][2] = 1.0 - 2.0*rbData.qx*rbData.qx - 2.0*rbData.qy*rbData.qy;}
				
				//if(  rbData.ID == 65546 ) { char Name[] = "LHand";     Lhand_x = rbData.x, Lhand_y = rbData.y, Lhand_z = rbData.z;}
				//if(  rbData.ID == 65551 ) { char Name[] = "RHand";     Rhand_x = rbData.x, Rhand_y = rbData.y, Rhand_z = rbData.z;} 
				
				//if(  rbData.ID == 65541 ) { char Name[] = "Head";      Head_x = rbData.x, Head_y = rbData.y, Head_z = rbData.z;}        //Head_rbData.x_2 = -rbData.x3, Head_pitch_2 = -pitch3, Head_roll_2 = -roll3;               Head_rm[0][0] = 1.0 - 2.0*rbData.qy*rbData.qy - 2.0*rbData.qz*rbData.qz; Head_rm[0][1] = 2.0*rbData.qx*rbData.qy - 2.0*rbData.qz*rbData.qw; Head_rm[0][2] = 2.0*rbData.qx*rbData.qz + 2.0*rbData.qy*rbData.qw; Head_rm[1][0] = 2.0*rbData.qx*rbData.qy + 2.0*rbData.qz*rbData.qw; Head_rm[1][1] = 1.0 - 2.0*rbData.qx*rbData.qx - 2.0*rbData.qz*rbData.qz; Head_rm[1][2] = 2.0*rbData.qy*rbData.qz - 2.0*rbData.qx*rbData.qw; Head_rm[2][0] = 2.0*rbData.qx*rbData.qz - 2.0*rbData.qy*rbData.qw; Head_rm[2][1] = 2.0*rbData.qy*rbData.qz + 2.0*rbData.qx*rbData.qw; Head_rm[2][2] = 1.0 - 2.0*rbData.qx*rbData.qx - 2.0*rbData.qy*rbData.qy;}
				if(  rbData.ID == 65538 ) // Chest (WST)
				{
					Quat q;
					q.x = rbData.qx;
					q.y = rbData.qy;
					q.z = rbData.qz;
					q.w = rbData.qw;
					EulerAngles deg = Eul_FromQuat(q, EulOrdYZXr);	// anything the upper yaw dows 
					Chest_r = deg.x;
					Chest_p = deg.y;
					Chest_y = deg.z;
					Chest_l = sqrt(rbData.x*rbData.x + rbData.y*rbData.y + rbData.z*rbData.z);
					Chest_X = rbData.x;
					Chest_Y = rbData.y;
					Chest_Z = rbData.z;

					
				}
				if(  rbData.ID == 65539 ) // Upper Chest UChest
				{
					Quat q;
					q.x = rbData.qx;
					q.y = rbData.qy;
					q.z = rbData.qz;
					q.w = rbData.qw;
					EulerAngles deg = Eul_FromQuat(q, EulOrdYZXr);	// anything the upper yaw dows 
					Uchest_r = deg.x;
					Uchest_p = deg.y;
					Uchest_y = deg.z;
					Uchest_l = sqrt(rbData.x*rbData.x + rbData.y*rbData.y + rbData.z*rbData.z);
					Uchest_X = rbData.x;
					Uchest_Y = rbData.y;
					Uchest_Z = rbData.z;
				}

				//if(  rbData.ID == 131052) { char Name[] = "LFootend";   Lfoote_x = rbData.x, Lfoote_y = rbData.y, Lfoote_z =rbData.z;}
				//if(  rbData.ID == 131048) { char Name[] = "RFootend";   Rfoote_x = rbData.x, Rfoote_y = rbData.y, Rfoote_z =rbData.z;}
				

			}				/*fprintf(fp, "%3.2f\t  %3.2f\t %3.2f\t %3.2f\t  %3.2f\t  %3.2f\t  %3.2f\t %3.2f\t %3.2f\t %3.2f\t  %3.2f\t  %3.2f\t   %3.2f\t %3.2f\t %3.2f\t %3.2f\t  %3.2f\t %3.2f   %d        \n",
						     Luarm_x, Luarm_y,Luarm_z,Lelbow_x,Lelbow_y,Lelbow_z,Ruarm_x,Ruarm_y,Ruarm_z,Relbow_x,Relbow_y,Relbow_z, Hip_x,  Hip_y,  Hip_z,  Trunk_x, Trunk_y,Trunk_z,data->iFrame);
				*/
			
				fprintf(fp,"%4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t   ",Luarm_r, Luarm_p, Luarm_y,Luarm_X, Luarm_Y, Luarm_Z, Luarm_l );        //1:7            
				fprintf(fp,"%4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t   ",Lelbow_r, Lelbow_p, Lelbow_y,Lelbow_X, Lelbow_Y, Lelbow_Z, Lelbow_l );	//14
				fprintf(fp,"%4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t   ",Ruarm_r, Ruarm_p, Ruarm_y,Ruarm_X, Ruarm_Y, Ruarm_Z, Ruarm_l );		//21
				fprintf(fp,"%4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t   ",Relbow_r, Relbow_p, Relbow_y,Relbow_X, Relbow_Y, Relbow_Z, Relbow_l );	//28
				fprintf(fp,"%4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t   ",Lthigh_r, Lthigh_p, Lthigh_y,Lthigh_X, Lthigh_Y, Lthigh_Z, Lthigh_l );	//35
				fprintf(fp,"%4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t   ",Rthigh_r, Rthigh_p, Rthigh_y,Rthigh_X, Rthigh_Y, Rthigh_Z, Rthigh_l );	//42
				fprintf(fp,"%4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t   ",Lshin_r, Lshin_p, Lshin_y,Lshin_X, Lshin_Y, Lshin_Z, Lshin_l );		//49
				fprintf(fp,"%4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t   ",Rshin_r, Rshin_p, Rshin_y,Rshin_X, Rshin_Y, Rshin_Z, Rshin_l );		//56
				fprintf(fp,"%4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t   ",Lfoot_r, Lfoot_p, Lfoot_y,Lfoot_X, Lfoot_Y, Lfoot_Z, Lfoot_l );		//63
				fprintf(fp,"%4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t   ",Rfoot_r, Rfoot_p, Rfoot_y,Rfoot_X, Rfoot_Y, Rfoot_Z, Rfoot_l );		//70
				fprintf(fp,"%4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t   ",UHip_r, UHip_p, UHip_y,UHip_X, UHip_Y, UHip_Z, UHip_l );			//71:77
				fprintf(fp,"%4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t   ",Lshoulder_r,  Lshoulder_p, Lshoulder_y,Lshoulder_X,  Lshoulder_Y, Lshoulder_Z, Lshoulder_l );	//84
				fprintf(fp,"%4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t   ",Rshoulder_r, Rshoulder_p,  Rshoulder_y,Rshoulder_X, Rshoulder_Y,  Rshoulder_Z, Rshoulder_l );	//91
				fprintf(fp,"%4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t   ",Lhand_r,  Lhand_p,  Lhand_y,Lhand_X,  Lhand_Y,  Lhand_Z, Lhand_l );		//98
				fprintf(fp,"%4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t   ",Rhand_r,  Rhand_p,  Rhand_y,Rhand_X,  Rhand_Y,  Rhand_Z, Rhand_l );		//105
				fprintf(fp,"%4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t   ",Head_r,  Head_p,  Head_y,Head_X,  Head_Y,  Head_Z, Head_l );			//112
				fprintf(fp,"%4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t   ",Chest_r, Chest_p, Chest_y,Chest_X, Chest_Y, Chest_Z, Chest_l );		//113:119
				fprintf(fp,"%4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t   ",Uchest_r, Uchest_p, Uchest_y,Uchest_X, Uchest_Y, Uchest_Z, Uchest_l );	//120:126
				fprintf(fp,"%4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t   ",Lfoote_r, Lfoote_p, Lfoote_y,Lfoote_X, Lfoote_Y, Lfoote_Z, Lfoote_l );	//134
				fprintf(fp,"%4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t   ",Rfoote_r, Rfoote_p, Rfoote_y,Rfoote_X, Rfoote_Y, Rfoote_Z, Rfoote_l );	//141
				fprintf(fp,"%4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t %4.9f\t   ",Neck_r, Neck_p, Neck_y,Neck_X, Neck_Y, Neck_Z, Neck_l );	//142:148
				fprintf(fp, "%d",    data->iFrame);
				fprintf(fp, "\n");
				fflush(fp);

		}
	}
}

void _WriteFooter(FILE* fp)
{
	//fprintf(fp, "</Data>\n\n");
	//fprintf(fp, "</MarkerSet>\n");
}

void resetClient()
{
	int iSuccess;

	printf("\n\nre-setting Client\n\n.");

	iSuccess = theClient->Uninitialize();
	if(iSuccess != 0)
		printf("error un-initting Client\n");

	iSuccess = theClient->Initialize(szMyIPAddress, szServerIPAddress);
	if(iSuccess != 0)
		printf("error re-initting Client\n");


}

