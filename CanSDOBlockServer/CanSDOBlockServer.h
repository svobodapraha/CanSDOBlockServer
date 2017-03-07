#ifndef CanSDOBlockServer_H
#define CanSDOBlockServer_H

#pragma pack(push, 1)
typedef struct XMC_LMOCan
{
  uint32_t    can_identifier;
  uint8_t     can_data_length;
  uint8_t     can_data[8];
} XMC_LMOCan_t;


typedef union
{
    uint8_t SDO_Command;

    struct
    {
      unsigned  cs: 1;
      unsigned   s: 1;
      unsigned  cc: 1;
      unsigned   x: 2;
      unsigned ccs: 3;
    }InitBlockDownloadReq;

    struct
    {
      unsigned  ss: 2;
      unsigned  sc: 1;
      unsigned   x: 2;
      unsigned scs: 3;

    }InitBlockDownloadResp;

    struct
    {
      unsigned seqno:7;
      unsigned     c:1;
    }DownloadBlock;

    struct
    {
      unsigned   x: 5;
      unsigned  cs: 3;

    }Abort;

} SDO_Command_t;


typedef enum CommStatus
{
    comsrst_ServerInit,
    comsrst_WaitingForReq,
    comsrst_RespSent,
    comsrst_WaitForBlockData
} CommStatus_t;



#pragma pack(pop)

//TIME
#define KN_CLIENT_TIMEOUT_MS      2000
#define KN_SERVER_TIMEOUT_MS      2000


#define RSDO_Id   0x600U
#define TSDO_Id   0x580U
#define NODE_ID   1U
#define SDO_COMMAND_POS              0
#define SDO_EXP_DATA_POS             4
#define SDO_INDEX_POS                1
#define SDO_SUBINDEX_POS             3
#define SDO_BLKSIZE_POS              4
#define SDO_BLOCK_DATA_POS           1
#define SDO_BLOCK_DATA_SIZE          7

//#define KN_MAX_BLOCK_SIZE             74
//debug
#define KN_MAX_BLOCK_SIZE            6



#define SDO_COMMAND_POS 0
#define SDO_CCS_INIT_BLOCK_DOWN_REQ  6
#define SDO_SCS_INIT_BLOCK_DOWN_RESP 5
#define SDO_CS_ABORT                 4
#define SDO_RESERVED                 0
#define SDO_CRC_SUPPORT              1
#define SDO_DATA_SIZE_INDICATED      1
#define SDO_CLIENT_SUBCOMMAND_INIT   0
#define SDO_SERVER_SUBCOMMAND_INIT   0

//OD
#define OD_DOWNLOAD_PROGRAM_DATA     (0x1F50)
#define OD_PROGRAM_DATA_MAIN          1








#ifdef __cplusplus
 extern "C" {
 #endif

 int fnProcessCANMessage(XMC_LMOCan_t *ReceivedCanMsg);
 int fnInit(void);
 int fnSendCanMessage(XMC_LMOCan_t *CanMessageToSend);
 int fnRunStateMachine(XMC_LMOCan_t *ReceivedCanMsg, int iCommand);
 int fnServerTimer(int iInterval);


#ifdef __cplusplus
 }
 #endif

#endif
