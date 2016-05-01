/**
 * @file arstream2_rtp.h
 * @brief Parrot Streaming Library - RTP implementation
 * @date 04/17/2015
 * @author aurelien.barre@parrot.com
 */

#ifndef _ARSTREAM2_RTP_H_
#define _ARSTREAM2_RTP_H_

#include <inttypes.h>
#define __USE_GNU
#include <sys/socket.h>
#undef __USE_GNU
#define ARSTREAM2_HAS_SENDMMSG //TODO


/*
 * Macros
 */

#define ARSTREAM2_RTP_IP_HEADER_SIZE 20
#define ARSTREAM2_RTP_UDP_HEADER_SIZE 8

#define ARSTREAM2_RTP_SENDER_SSRC 0x41525353
#define ARSTREAM2_RTP_SENDER_CNAME "ARStream2 RTP Sender"
#define ARSTREAM2_RTP_RECEIVER_SSRC 0x41525352
#define ARSTREAM2_RTP_RECEIVER_CNAME "ARStream2 RTP Receiver"


/*
 * Types
 */

/**
 * @brief RTP Header (see RFC3550)
 */
typedef struct {
    uint16_t flags;
    uint16_t seqNum;
    uint32_t timestamp;
    uint32_t ssrc;
} __attribute__ ((packed)) ARSTREAM2_RTP_Header_t;

#define ARSTREAM2_RTP_TOTAL_HEADERS_SIZE (sizeof(ARSTREAM2_RTP_Header_t) + ARSTREAM2_RTP_UDP_HEADER_SIZE + ARSTREAM2_RTP_IP_HEADER_SIZE)
#define ARSTREAM2_RTP_MAX_PAYLOAD_SIZE (0xFFFF - ARSTREAM2_RTP_TOTAL_HEADERS_SIZE)

/**
 * @brief RTP packet data
 */
typedef struct ARSTREAM2_RTP_Packet_s {
    uint64_t timeoutTimestamp;
    uint64_t ntpTimestamp;
    uint32_t rtpTimestamp;
    uint16_t seqNum;
    int markerBit;
    ARSTREAM2_RTP_Header_t header;
    uint8_t *headerExtension;
    unsigned int headerExtensionSize;
    uint8_t *payload;
    unsigned int payloadSize;
    struct iovec msgIov[3];
    size_t msgIovLength;
} ARSTREAM2_RTP_Packet_t;

/**
 * @brief RTP packet FIFO item
 */
typedef struct ARSTREAM2_RTP_PacketFifoItem_s {
    ARSTREAM2_RTP_Packet_t packet;

    struct ARSTREAM2_RTP_PacketFifoItem_s* prev;
    struct ARSTREAM2_RTP_PacketFifoItem_s* next;
} ARSTREAM2_RTP_PacketFifoItem_t;

/**
 * @brief RTP packet FIFO
 */
typedef struct ARSTREAM2_RTP_PacketFifo_s {
    int size;
    int count;
    ARSTREAM2_RTP_PacketFifoItem_t *head;
    ARSTREAM2_RTP_PacketFifoItem_t *tail;
    ARSTREAM2_RTP_PacketFifoItem_t *free;
    ARSTREAM2_RTP_PacketFifoItem_t *pool;
} ARSTREAM2_RTP_PacketFifo_t;

/**
 * @brief RTP sender context
 */
typedef struct ARSTREAM2_RTP_SenderContext_s {
    uint32_t senderSsrc;
    uint32_t rtpClockRate;
    uint32_t rtpTimestampOffset;
} ARSTREAM2_RTP_SenderContext_t;


/*
 * Functions
 */

int ARSTREAM2_RTP_FifoInit(ARSTREAM2_RTP_PacketFifo_t *fifo, int maxCount);

int ARSTREAM2_RTP_FifoFree(ARSTREAM2_RTP_PacketFifo_t *fifo);

ARSTREAM2_RTP_PacketFifoItem_t* ARSTREAM2_RTP_FifoPopFreeItem(ARSTREAM2_RTP_PacketFifo_t *fifo);

int ARSTREAM2_RTP_FifoPushFreeItem(ARSTREAM2_RTP_PacketFifo_t *fifo, ARSTREAM2_RTP_PacketFifoItem_t *item);

int ARSTREAM2_RTP_FifoEnqueueItem(ARSTREAM2_RTP_PacketFifo_t *fifo, ARSTREAM2_RTP_PacketFifoItem_t *item);

ARSTREAM2_RTP_PacketFifoItem_t* ARSTREAM2_RTP_FifoDequeueItem(ARSTREAM2_RTP_PacketFifo_t *fifo);

int ARSTREAM2_RTP_FifoEnqueuePacket(ARSTREAM2_RTP_PacketFifo_t *fifo, const ARSTREAM2_RTP_Packet_t *packet);

int ARSTREAM2_RTP_FifoEnqueuePackets(ARSTREAM2_RTP_PacketFifo_t *fifo, const ARSTREAM2_RTP_Packet_t *packets, int packetCount);

int ARSTREAM2_RTP_FifoDequeuePacket(ARSTREAM2_RTP_PacketFifo_t *fifo, ARSTREAM2_RTP_Packet_t *packet);

int ARSTREAM2_RTP_FifoDequeuePackets(ARSTREAM2_RTP_PacketFifo_t *fifo, ARSTREAM2_RTP_Packet_t *packets, int maxPacketCount, int *packetCount);

int ARSTREAM2_RTP_Sender_FifoFillMsgVec(ARSTREAM2_RTP_PacketFifo_t *fifo, struct mmsghdr *msgVec, unsigned int msgVecMaxCount);

int ARSTREAM2_RTP_Sender_FifoCleanFromMsgVec(ARSTREAM2_RTP_PacketFifo_t *fifo, struct mmsghdr *msgVec, unsigned int msgVecCount);

int ARSTREAM2_RTP_Sender_FifoCleanFromTimeout(ARSTREAM2_RTP_PacketFifo_t *fifo, uint64_t curTime);

int ARSTREAM2_RTP_Sender_GeneratePacket(ARSTREAM2_RTP_SenderContext_t *context, ARSTREAM2_RTP_Packet_t *packet,
                                        uint8_t *payload, unsigned int payloadSize,
                                        uint8_t *headerExtension, unsigned int headerExtensionSize,
                                        uint64_t ntpTimestamp, uint64_t timeoutTimestamp,
                                        uint16_t seqNum, int markerBit);

#endif /* _ARSTREAM2_RTP_H_ */
