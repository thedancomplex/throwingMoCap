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

#include "NatNetTypes.h"
#include "NatNetClient.h"

#pragma warning( disable : 4996 )

void _WriteHeader(FILE* fp, sDataDescriptions* pBodyDefs);
void _WriteFrame(FILE* fp, sFrameOfMocapData* data);
void _WriteFooter(FILE* fp);
void __cdecl DataHandler(sFrameOfMocapData* data, void* pUserData);			// receives data from the server
void __cdecl MessageHandler(int msgType, char* msg);		// receives NatNet error mesages
void resetClient();
int CreateClient(int iConnectionType);

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
                    sRigidBodyDescription* pRB = &pSK->RigidBodies[j];
                   // printf("  RigidBody Name : %s\n", pRB->szName);
                    //printf("  RigidBody ID : %d\n", pRB->ID);
                    //printf("  RigidBody Parent ID : %d\n", pRB->parentID);
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
		sprintf(szFile, "%s\\xyzGlobal.pts",szFolder);
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
    fprintf(fp,"Luarm_x   Luarm_y   Luarm_z   Lelbow_x   Lelbow_y   Lelbow_z   Ruarm_x   Ruarm_y   Ruarm_z   Relbow_x   Relbow_y   Relbow_z   LThigh_x   LThigh_y  LThigh_z   RThigh_x   RThigh_y  RThigh_z LShin_x LShin_y LShin_z RShin_x RShin_y RShin_z  Lfoot_x Lfoot_y Lfoot_z Rfoot_x Rfoot_y Rfoot_z Chest_x   Chest_y  Chest_z");
	fprintf(fp,"Hip_x     Hip_y     Hip_z     LShoulder_x  LShoulder_y LShoulder_z RShoulder_x  RShoulder_y RShoulder_z  Lhand_x  Lhand_y  Lhand_z  Rhand_x  Rhand_y  Rhand_z  Head_x  Head_y  Head_z  Chest_x Chest_y Chest_z  UChest_x UChest_y UChest_z Neck_x Neck_y Neck_z Frame");
	fprintf(fp,"\n");

}

void _WriteFrame(FILE* fp, sFrameOfMocapData* data)
{
    float Luarm_x,  Luarm_y, Luarm_z;
	float Lelbow_x, Lelbow_y, Lelbow_z;
	float Ruarm_x,  Ruarm_y, Ruarm_z;
	float Relbow_x, Relbow_y, Relbow_z;
	float Lthigh_x, Lthigh_y, Lthigh_z;
	float Rthigh_x, Rthigh_y, Rthigh_z;
	float Lshin_x,  Lshin_y, Lshin_z;
	float Rshin_x,  Rshin_y, Rshin_z;
	float Lfoot_x,  Lfoot_y, Lfoot_z;
	float Rfoot_x,  Rfoot_y, Rfoot_z;
	float UHip_x,   UHip_y,  UHip_z;
	float Lshoulder_x, Lshoulder_y, Lshoulder_z;
	float Rshoulder_x, Rshoulder_y, Rshoulder_z;
	float Lhand_x,  Lhand_y, Lhand_z;
	float Rhand_x,  Rhand_y, Rhand_z;
    float Head_x,   Head_y,  Head_z;
	float Chest_x,  Chest_y, Chest_z;
	float UChest_x, UChest_y,UChest_z;
	float Lfoote_x, Lfoote_y,Lfoote_z;
	float Rfoote_x, Rfoote_y,Rfoote_z;
	float Neck_x,   Neck_y,  Neck_z;
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
				
				if(  rbData.ID == 65537 ) { char Name[] = "UHip";       UHip_x = rbData.x, UHip_y = rbData.y, UHip_z = rbData.z;} //, Hip_yaw_2 = -yaw3, Hip_pitch_2 = -pitch3, Hip_roll_2 = -roll3;                     Hip_rm[0][0] = 1.0 - 2.0*rbData.qy*rbData.qy - 2.0*rbData.qz*rbData.qz; Hip_rm[0][1] = 2.0*rbData.qx*rbData.qy - 2.0*rbData.qz*rbData.qw; Hip_rm[0][2] = 2.0*rbData.qx*rbData.qz + 2.0*rbData.qy*rbData.qw; Hip_rm[1][0] = 2.0*rbData.qx*rbData.qy + 2.0*rbData.qz*rbData.qw; Hip_rm[1][1] = 1.0 - 2.0*rbData.qx*rbData.qx - 2.0*rbData.qz*rbData.qz; Hip_rm[1][2] = 2.0*rbData.qy*rbData.qz - 2.0*rbData.qx*rbData.qw; Hip_rm[2][0] = 2.0*rbData.qx*rbData.qz - 2.0*rbData.qy*rbData.qw; Hip_rm[2][1] = 2.0*rbData.qy*rbData.qz + 2.0*rbData.qx*rbData.qw; Hip_rm[2][2] = 1.0 - 2.0*rbData.qx*rbData.qx - 2.0*rbData.qy*rbData.qy;}

				if(  rbData.ID == 65543 ) { char Name[] = "ULshoulder"; Lshoulder_x = rbData.x, Lshoulder_y = rbData.y, Lshoulder_z = rbData.z;}  //Lshoulder_rm[0][0] = 1.0 - 2.0*rbData.qy*rbData.qy - 2.0*rbData.qz*rbData.qz; Lshoulder_rm[0][1] = 2.0*rbData.qx*rbData.qy - 2.0*rbData.qz*rbData.qw; Lshoulder_rm[0][2] = 2.0*rbData.qx*rbData.qz + 2.0*rbData.qy*rbData.qw; Lshoulder_rm[1][0] = 2.0*rbData.qx*rbData.qy + 2.0*rbData.qz*rbData.qw; Lshoulder_rm[1][1] = 1.0 - 2.0*rbData.qx*rbData.qx - 2.0*rbData.qz*rbData.qz; Lshoulder_rm[1][2] = 2.0*rbData.qy*rbData.qz - 2.0*rbData.qx*rbData.qw; Lshoulder_rm[2][0] = 2.0*rbData.qx*rbData.qz - 2.0*rbData.qy*rbData.qw; Lshoulder_rm[2][1] = 2.0*rbData.qy*rbData.qz + 2.0*rbData.qx*rbData.qw; Lshoulder_rm[2][2] = 1.0 - 2.0*rbData.qx*rbData.qx - 2.0*rbData.qy*rbData.qy;}
				if(  rbData.ID == 65548 ) { char Name[] = "ULshoulder"; Rshoulder_x = rbData.x, Rshoulder_y = rbData.y, Rshoulder_z = rbData.z;} //Rshoulder_rm[0][0] = 1.0 - 2.0*rbData.qy*rbData.qy - 2.0*rbData.qz*rbData.qz; Rshoulder_rm[0][1] = 2.0*rbData.qx*rbData.qy - 2.0*rbData.qz*rbData.qw; Rshoulder_rm[0][2] = 2.0*rbData.qx*rbData.qz + 2.0*rbData.qy*rbData.qw; Rshoulder_rm[1][0] = 2.0*rbData.qx*rbData.qy + 2.0*rbData.qz*rbData.qw; Rshoulder_rm[1][1] = 1.0 - 2.0*rbData.qx*rbData.qx - 2.0*rbData.qz*rbData.qz; Rshoulder_rm[1][2] = 2.0*rbData.qy*rbData.qz - 2.0*rbData.qx*rbData.qw; Rshoulder_rm[2][0] = 2.0*rbData.qx*rbData.qz - 2.0*rbData.qy*rbData.qw; Rshoulder_rm[2][1] = 2.0*rbData.qy*rbData.qz + 2.0*rbData.qx*rbData.qw; Rshoulder_rm[2][2] = 1.0 - 2.0*rbData.qx*rbData.qx - 2.0*rbData.qy*rbData.qy;}
				
				if(  rbData.ID == 65546 ) { char Name[] = "LHand";     Lhand_x = rbData.x, Lhand_y = rbData.y, Lhand_z = rbData.z;}
				if(  rbData.ID == 65551 ) { char Name[] = "RHand";     Rhand_x = rbData.x, Rhand_y = rbData.y, Rhand_z = rbData.z;} 
				
				if(  rbData.ID == 65541 ) { char Name[] = "Head";      Head_x = rbData.x, Head_y = rbData.y, Head_z = rbData.z;}        //Head_rbData.x_2 = -rbData.x3, Head_pitch_2 = -pitch3, Head_roll_2 = -roll3;               Head_rm[0][0] = 1.0 - 2.0*rbData.qy*rbData.qy - 2.0*rbData.qz*rbData.qz; Head_rm[0][1] = 2.0*rbData.qx*rbData.qy - 2.0*rbData.qz*rbData.qw; Head_rm[0][2] = 2.0*rbData.qx*rbData.qz + 2.0*rbData.qy*rbData.qw; Head_rm[1][0] = 2.0*rbData.qx*rbData.qy + 2.0*rbData.qz*rbData.qw; Head_rm[1][1] = 1.0 - 2.0*rbData.qx*rbData.qx - 2.0*rbData.qz*rbData.qz; Head_rm[1][2] = 2.0*rbData.qy*rbData.qz - 2.0*rbData.qx*rbData.qw; Head_rm[2][0] = 2.0*rbData.qx*rbData.qz - 2.0*rbData.qy*rbData.qw; Head_rm[2][1] = 2.0*rbData.qy*rbData.qz + 2.0*rbData.qx*rbData.qw; Head_rm[2][2] = 1.0 - 2.0*rbData.qx*rbData.qx - 2.0*rbData.qy*rbData.qy;}
				if(  rbData.ID == 65538 ) { char Name[] = "Chest";     Chest_x = rbData.x, Chest_y = rbData.y, Chest_z = rbData.z;}     //if(  rbData.ID == 65553 ) { char Name[] = "LThigh";    Lthigh_x = rbData.x, Lthigh_y = rbData.y, Lthigh_z = rbData.z;}//, Lthigh_yaw_2 = -yaw3, Lthigh_pitch_2 = -pitch3, Lthigh_roll_2 = -roll3;   Lthigh_rm[0][0] = 1.0 - 2.0*rbData.qy*rbData.qy - 2.0*rbData.qz*rbData.qz; Lthigh_rm[0][1] = 2.0*rbData.qx*rbData.qy - 2.0*rbData.qz*rbData.qw; Lthigh_rm[0][2] = 2.0*rbData.qx*rbData.qz + 2.0*rbData.qy*rbData.qw; Lthigh_rm[1][0] = 2.0*rbData.qx*rbData.qy + 2.0*rbData.qz*rbData.qw; Lthigh_rm[1][1] = 1.0 - 2.0*rbData.qx*rbData.qx - 2.0*rbData.qz*rbData.qz; Lthigh_rm[1][2] = 2.0*rbData.qy*rbData.qz - 2.0*rbData.qx*rbData.qw; Lthigh_rm[2][0] = 2.0*rbData.qx*rbData.qz - 2.0*rbData.qy*rbData.qw; Lthigh_rm[2][1] = 2.0*rbData.qy*rbData.qz + 2.0*rbData.qx*rbData.qw; Lthigh_rm[2][2] = 1.0 - 2.0*rbData.qx*rbData.qx - 2.0*rbData.qy*rbData.qy;}
				if(  rbData.ID == 65539 ) { char Name[] = "UChest";    UChest_x = rbData.x, UChest_y = rbData.y, UChest_z = rbData.z;} //Chest_yaw_2 = -rbData.x3, Chest_pitch_2 = -pitch3, Chest_roll_2 = -roll3;         Chest_rm[0][0] = 1.0 - 2.0*rbData.qy*rbData.qy - 2.0*rbData.qz*rbData.qz; Chest_rm[0][1] = 2.0*rbData.qx*rbData.qy - 2.0*rbData.qz*rbData.qw; Chest_rm[0][2] = 2.0*rbData.qx*rbData.qz + 2.0*rbData.qy*rbData.qw; Chest_rm[1][0] = 2.0*rbData.qx*rbData.qy + 2.0*rbData.qz*rbData.qw; Chest_rm[1][1] = 1.0 - 2.0*rbData.qx*rbData.qx - 2.0*rbData.qz*rbData.qz; Chest_rm[1][2] = 2.0*rbData.qy*rbData.qz - 2.0*rbData.qx*rbData.qw; Chest_rm[2][0] = 2.0*rbData.qx*rbData.qz - 2.0*rbData.qy*rbData.qw; Chest_rm[2][1] = 2.0*rbData.qy*rbData.qz + 2.0*rbData.qx*rbData.qw; Chest_rm[2][2] = 1.0 - 2.0*rbData.qx*rbData.qx - 2.0*rbData.qy*rbData.qy;}

				if(  rbData.ID == 131052) { char Name[] = "LFootend";   Lfoote_x = rbData.x, Lfoote_y = rbData.y, Lfoote_z =rbData.z;}
				if(  rbData.ID == 131048) { char Name[] = "RFootend";   Rfoote_x = rbData.x, Rfoote_y = rbData.y, Rfoote_z =rbData.z;}
		        if(  rbData.ID == 65540)  { char Name[] = "Neck";      Neck_x = rbData.x, Neck_y = rbData.y, Neck_z =rbData.z;}

			}				/*fprintf(fp, "%3.2f\t  %3.2f\t %3.2f\t %3.2f\t  %3.2f\t  %3.2f\t  %3.2f\t %3.2f\t %3.2f\t %3.2f\t  %3.2f\t  %3.2f\t   %3.2f\t %3.2f\t %3.2f\t %3.2f\t  %3.2f\t %3.2f   %d        \n",
						     Luarm_x, Luarm_y,Luarm_z,Lelbow_x,Lelbow_y,Lelbow_z,Ruarm_x,Ruarm_y,Ruarm_z,Relbow_x,Relbow_y,Relbow_z, Hip_x,  Hip_y,  Hip_z,  Trunk_x, Trunk_y,Trunk_z,data->iFrame);
				*/
			fprintf(fp,"%5.9f\t",Luarm_x);
			fprintf(fp,"%5.9f\t",Luarm_y);
			fprintf(fp,"%5.9f\t",Luarm_z);

			fprintf(fp,"%5.9f\t",Lelbow_x);
			fprintf(fp,"%5.9f\t",Lelbow_y);
			fprintf(fp,"%5.9f\t",Lelbow_z);
			
			fprintf(fp,"%5.9f\t",Ruarm_x);
			fprintf(fp,"%5.9f\t",Ruarm_y);
			fprintf(fp,"%5.9f\t",Ruarm_z);
			
			fprintf(fp,"%5.9f\t",Relbow_x);
			fprintf(fp,"%5.9f\t",Relbow_y);
			fprintf(fp,"%5.9f\t",Relbow_z);//4 sets. left and right, Upper arm and elbow

			fprintf(fp,"%5.9f\t",Lthigh_x);
			fprintf(fp,"%5.9f\t",Lthigh_y);
			fprintf(fp,"%5.9f\t",Lthigh_z);

			fprintf(fp,"%5.9f\t",Rthigh_x);
			fprintf(fp,"%5.9f\t",Rthigh_y);
			fprintf(fp,"%5.9f\t",Rthigh_z);

			
			fprintf(fp,"%5.9f\t",Lshin_x);
			fprintf(fp,"%5.9f\t",Lshin_y);
			fprintf(fp,"%5.9f\t",Lshin_z);

			fprintf(fp,"%5.9f\t",Rshin_x);
			fprintf(fp,"%5.9f\t",Rshin_y);
			fprintf(fp,"%5.9f\t",Rshin_z); //4 sets, left and right, Thigh and Shin
						
			fprintf(fp,"%5.9f\t",Lfoot_x);
			fprintf(fp,"%5.9f\t",Lfoot_y);
			fprintf(fp,"%5.9f\t",Lfoot_z);

			fprintf(fp,"%5.9f\t",Rfoot_x);
			fprintf(fp,"%5.9f\t",Rfoot_y);
			fprintf(fp,"%5.9f\t",Rfoot_z);
						
			fprintf(fp,"%5.9f\t",UHip_x);
			fprintf(fp,"%5.9f\t",UHip_y);
			fprintf(fp,"%5.9f\t",UHip_z);


			fprintf(fp,"%5.9f\t",Lshoulder_x);
			fprintf(fp,"%5.9f\t",Lshoulder_y);
			fprintf(fp,"%5.9f\t",Lshoulder_z);

			fprintf(fp,"%5.9f\t",Rshoulder_x);
			fprintf(fp,"%5.9f\t",Rshoulder_y);
			fprintf(fp,"%5.9f\t",Rshoulder_z);

			fprintf(fp,"%5.9f\t",Lhand_x);
			fprintf(fp,"%5.9f\t",Lhand_y);
			fprintf(fp,"%5.9f\t",Lhand_z);

			fprintf(fp,"%5.9f\t",Rhand_x);
			fprintf(fp,"%5.9f\t",Rhand_y);
			fprintf(fp,"%5.9f\t",Rhand_z);//4 sets

			fprintf(fp,"%5.9f\t",Head_x);
			fprintf(fp,"%5.9f\t",Head_y);
			fprintf(fp,"%5.9f\t",Head_z);

			fprintf(fp,"%5.9f\t",Chest_x);
			fprintf(fp,"%5.9f\t",Chest_y);
			fprintf(fp,"%5.9f\t",Chest_z);


			fprintf(fp,"%5.9f\t",UChest_x);
			fprintf(fp,"%5.9f\t",UChest_y);
			fprintf(fp,"%5.9f\t",UChest_z);

			fprintf(fp,"%5.9f\t",Lfoote_x);
			fprintf(fp,"%5.9f\t",Lfoote_y);
			fprintf(fp,"%5.9f\t",Lfoote_z);

			fprintf(fp,"%5.9f\t",Rfoote_x);
			fprintf(fp,"%5.9f\t",Rfoote_y);
			fprintf(fp,"%5.9f\t",Rfoote_z);
			fprintf(fp,"%5.9f\t",Neck_x);
			fprintf(fp,"%5.9f\t",Neck_y);
			fprintf(fp,"%5.9f\t",Neck_z);

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

