#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "CanSDOBlockServer.h"

FILE  *FirmwareUpdateFile = NULL;
uint8_t u8NodeId = 0;
int iServerTimeOutTimer = 0;
CommStatus_t eCommStatus = comsrst_ServerInit;
uint16_t u16LastIndexProcessed = 0;
uint8_t  u8LastSubindexProcessed = 0;


int fnInit(void)
{
    u8NodeId = NODE_ID;
    eCommStatus = comsrst_WaitingForReq;
    FirmwareUpdateFile =  fopen("FirmwareUpdate.bin", "a+");
    if(FirmwareUpdateFile != NULL)
        return 0;
    else
        return -1;
}
int fnServerTimer(int iInterval)
{
    static int iTimeElapsed = 0;
    iTimeElapsed += iInterval;
    //fprintf( stderr, "Server elapsed: %u \n",   iTimeElapsed);
    if (iServerTimeOutTimer > iInterval) iServerTimeOutTimer -= iInterval; else iServerTimeOutTimer = 0;

    return (0);
}


#define RCV_DATA(byte)   (ReceivedCanMsg->can_data[byte])
#define SDO_COMMAND_RCV  (*((SDO_Command_t *)(&(RCV_DATA(SDO_COMMAND_POS)))))
#define SDO_INDEX_RCV    (*((uint16_t *)(&(ReceivedCanMsg->can_data[SDO_INDEX_POS]))))
#define SDO_SUBINDEX_RCV (*((uint8_t *) (&(ReceivedCanMsg->can_data[SDO_SUBINDEX_POS]))))
#define SDO_BLKSIZE_RCV  (*((uint8_t *) (&(ReceivedCanMsg->can_data[SDO_BLKSIZE_POS]))))

#define SDO_COMMAND_SND  (*((SDO_Command_t *)(&(SendCanMsg.can_data[SDO_COMMAND_POS]))))
#define SDO_EXP_DATA_SND (*((uint32_t *)(&(SendCanMsg.can_data[SDO_EXP_DATA_POS]))))
#define SDO_INDEX_SND    (*((uint16_t *)(&(SendCanMsg.can_data[SDO_INDEX_POS]))))
#define SDO_SUBINDEX_SND (*((uint8_t *) (&(SendCanMsg.can_data[SDO_SUBINDEX_POS]))))
#define SDO_BLKSIZE_SND  (*((uint8_t *) (&(SendCanMsg.can_data[SDO_BLKSIZE_POS]))))

#define CLEAR_MSG_SND    (memset(&SendCanMsg, 0, sizeof(XMC_LMOCan_t)))

int fnProcessCANMessage(XMC_LMOCan_t *ReceivedCanMsg)
{
    fprintf( stderr, "ID: 0x%.X \n", ReceivedCanMsg->can_identifier);
    fprintf( stderr, "len: %u \n",    ReceivedCanMsg->can_data_length);
    for (int i = 0; i < ReceivedCanMsg->can_data_length; ++i)
    {
      fprintf( stderr, "%.2X " , ReceivedCanMsg->can_data[i]);
    }
    fprintf( stderr, "\n\n");

    //Check ID
    if(ReceivedCanMsg->can_identifier != RSDO_Id + u8NodeId)
    {
        return -1;
    }



    fnRunStateMachine(ReceivedCanMsg, 0);




    if(FirmwareUpdateFile != NULL)
    {
      //fprintf( FirmwareUpdateFile, "%d", ReceivedCanMsg->can_data[0]);
      int iSizeToWrite = 0;
      iSizeToWrite = ReceivedCanMsg->can_data_length <= sizeof(ReceivedCanMsg->can_data)?ReceivedCanMsg->can_data_length:sizeof(ReceivedCanMsg->can_data);
      fwrite(ReceivedCanMsg->can_data, iSizeToWrite, 1, FirmwareUpdateFile);
      fflush (FirmwareUpdateFile);
    }

    return(0);
}

int fnRunStateMachine(XMC_LMOCan_t *ReceivedCanMsg, int iCommand)
{
    XMC_LMOCan_t SendCanMsg;
    iCommand = iCommand;

    //Test for ABORTMsg
    if(ReceivedCanMsg != NULL)
    {
        if(
            (SDO_COMMAND_RCV.Abort.cs == SDO_CS_ABORT) &&
            (SDO_COMMAND_RCV.Abort.x  == SDO_RESERVED)
          )
        {
            u16LastIndexProcessed   = 0;
            u8LastSubindexProcessed = 0;
            iServerTimeOutTimer     = 0;
            //TODO cancel writing
            eCommStatus = comsrst_WaitingForReq;
        }
    }

    //communication state machine
    switch (eCommStatus)
    {
        case comsrst_ServerInit:
            eCommStatus = comsrst_WaitingForReq;

        break;

        case comsrst_WaitingForReq:
            if (ReceivedCanMsg != NULL)
            {


                if(
                    (SDO_COMMAND_RCV.InitBlockDownloadReq.ccs == SDO_CCS_INIT_BLOCK_DOWN_REQ)&&
                    (SDO_COMMAND_RCV.InitBlockDownloadReq.cs  == SDO_CLIENT_SUBCOMMAND_INIT)
                  )
                {
                    //check correst OD Entry
                    if (
                         (SDO_INDEX_RCV     == OD_DOWNLOAD_PROGRAM_DATA) &&
                         (SDO_SUBINDEX_RCV  == OD_PROGRAM_DATA_MAIN)
                       )
                    {

                        //send response.
                        u16LastIndexProcessed = SDO_INDEX_RCV;
                        u8LastSubindexProcessed = SDO_SUBINDEX_RCV;
                        CLEAR_MSG_SND;
                        SDO_COMMAND_SND.InitBlockDownloadResp.scs = SDO_SCS_INIT_BLOCK_DOWN_RESP;
                        SDO_COMMAND_SND.InitBlockDownloadResp.x   = SDO_RESERVED;
                        SDO_COMMAND_SND.InitBlockDownloadResp.sc  = SDO_CRC_SUPPORT;
                        SDO_COMMAND_SND.InitBlockDownloadResp.ss  = SDO_SERVER_SUBCOMMAND_INIT;
                        SDO_INDEX_SND              = u16LastIndexProcessed;
                        SDO_SUBINDEX_SND           = u8LastSubindexProcessed;
                        SDO_BLKSIZE_SND            = KN_MAX_BLOCK_SIZE;
                        SendCanMsg.can_identifier = TSDO_Id + u8NodeId;
                        SendCanMsg.can_data_length = 8;
                        fnSendCanMessage(&SendCanMsg);

                        //Wait for Data
                        iServerTimeOutTimer =  KN_SERVER_TIMEOUT_MS;
                        eCommStatus = comsrst_WaitForBlockData;
                    }
                    else
                    {
                       //TODO Abort - not suported OD entry for download
                    }

                }
                else
                {
                    //TODO Abort - not Download request
                }

            }

        break;

        case comsrst_WaitForBlockData:
            if (ReceivedCanMsg != NULL)
            {

            }
            else
            {
            }

        case comsrst_RespSent:
            if (ReceivedCanMsg != NULL)
            {

            }
            else
            {
            }

        break;

        default:

        break;
    }

    return (0);

}

