/**
 * @file arstream2_rtp_h264.h
 * @brief Parrot Streaming Library - RTP H.264 payloading implementation
 * @date 04/25/2016
 * @author aurelien.barre@parrot.com
 */

#ifndef _ARSTREAM2_RTPH264_H_
#define _ARSTREAM2_RTPH264_H_

#include "arstream2_rtp.h"


/*
 * Macros
 */

#define ARSTREAM2_RTPH264_NALU_TYPE_STAPA 24
#define ARSTREAM2_RTPH264_NALU_TYPE_FUA 28


/*
 * Types
 */

/**
 * @brief NAL unit data
 */
typedef struct ARSTREAM2_RTPH264_Nalu_s {
    uint64_t timeoutTimestamp;
    uint64_t ntpTimestamp;
    int isLastInAu;
    int seqNumForcedDiscontinuity;
    uint8_t *metadata;
    unsigned int metadataSize;
    uint8_t *nalu;
    unsigned int naluSize;
    void *auUserPtr;
    void *naluUserPtr;
} ARSTREAM2_RTPH264_Nalu_t;

/**
 * @brief NAL unit FIFO item
 */
typedef struct ARSTREAM2_RTPH264_NaluFifoItem_s {
    ARSTREAM2_RTPH264_Nalu_t nalu;

    struct ARSTREAM2_RTPH264_NaluFifoItem_s* prev;
    struct ARSTREAM2_RTPH264_NaluFifoItem_s* next;
} ARSTREAM2_RTPH264_NaluFifoItem_t;

/**
 * @brief NAL unit FIFO
 */
typedef struct ARSTREAM2_RTPH264_NaluFifo_s {
    int size;
    int count;
    ARSTREAM2_RTPH264_NaluFifoItem_t *head;
    ARSTREAM2_RTPH264_NaluFifoItem_t *tail;
    ARSTREAM2_RTPH264_NaluFifoItem_t *free;
    ARSTREAM2_RTPH264_NaluFifoItem_t *pool;
} ARSTREAM2_RTPH264_NaluFifo_t;


/*
 * Functions
 */

int ARSTREAM2_RTPH264_FifoInit(ARSTREAM2_RTPH264_NaluFifo_t *fifo, int maxCount);

int ARSTREAM2_RTPH264_FifoFree(ARSTREAM2_RTPH264_NaluFifo_t *fifo);

ARSTREAM2_RTPH264_NaluFifoItem_t* ARSTREAM2_RTPH264_FifoPopFreeItem(ARSTREAM2_RTPH264_NaluFifo_t *fifo);

int ARSTREAM2_RTPH264_FifoPushFreeItem(ARSTREAM2_RTPH264_NaluFifo_t *fifo, ARSTREAM2_RTPH264_NaluFifoItem_t *item);

int ARSTREAM2_RTPH264_FifoEnqueueItem(ARSTREAM2_RTPH264_NaluFifo_t *fifo, ARSTREAM2_RTPH264_NaluFifoItem_t *item);

ARSTREAM2_RTPH264_NaluFifoItem_t* ARSTREAM2_RTPH264_FifoDequeueItem(ARSTREAM2_RTPH264_NaluFifo_t *fifo);

int ARSTREAM2_RTPH264_FifoEnqueueNalu(ARSTREAM2_RTPH264_NaluFifo_t *fifo, const ARSTREAM2_RTPH264_Nalu_t *nalu);

int ARSTREAM2_RTPH264_FifoEnqueueNalus(ARSTREAM2_RTPH264_NaluFifo_t *fifo, const ARSTREAM2_RTPH264_Nalu_t *nalus, int naluCount);

int ARSTREAM2_RTPH264_FifoDequeueNalu(ARSTREAM2_RTPH264_NaluFifo_t *fifo, ARSTREAM2_RTPH264_Nalu_t *nalu);

int ARSTREAM2_RTPH264_FifoDequeueNalus(ARSTREAM2_RTPH264_NaluFifo_t *fifo, ARSTREAM2_RTPH264_Nalu_t *nalus, int maxNaluCount, int *naluCount);

#endif /* _ARSTREAM2_RTPH264_H_ */
