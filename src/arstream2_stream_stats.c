/**
 * @file arstream2_stream_stats.c
 * @brief Parrot Streaming Library - Stream Stats
 * @date 10/04/2016
 * @author aurelien.barre@parrot.com
 */

#include <stdio.h>
#include <stdlib.h>

#include <libARSAL/ARSAL_Print.h>

#include "arstream2_stream_stats_internal.h"


#define ARSTREAM2_STREAM_STATS_TAG "ARSTREAM2_StreamStats"

#define ARSTREAM2_STREAM_STATS_VIDEO_STATS_OUTPUT_PATH "videostats"
#define ARSTREAM2_STREAM_STATS_VIDEO_STATS_OUTPUT_FILENAME "videostats"
#define ARSTREAM2_STREAM_STATS_VIDEO_STATS_OUTPUT_FILEEXT "dat"

#define ARSTREAM2_STREAM_STATS_VIDEO_STATS_OUTPUT_INTERVAL (1000000)

#define ARSTREAM2_STREAM_STATS_RTP_STATS_OUTPUT_PATH "rtpstats"
#define ARSTREAM2_STREAM_STATS_RTP_STATS_OUTPUT_FILENAME "rtpstats"
#define ARSTREAM2_STREAM_STATS_RTP_STATS_OUTPUT_FILEEXT "dat"

#define ARSTREAM2_STREAM_STATS_RTP_STATS_OUTPUT_INTERVAL (1000000)

#define ARSTREAM2_STREAM_STATS_RTP_LOSS_OUTPUT_PATH "rtploss"
#define ARSTREAM2_STREAM_STATS_RTP_LOSS_OUTPUT_FILENAME "rtploss"
#define ARSTREAM2_STREAM_STATS_RTP_LOSS_OUTPUT_FILEEXT "dat"


void ARSTREAM2_StreamStats_VideoStatsFileOpen(ARSTREAM2_StreamStats_VideoStatsContext_t *context, const char *debugPath, const char *friendlyName,
                                              const char *dateAndTime, uint32_t mbStatusZoneCount, uint32_t mbStatusClassCount)
{
    char szOutputFileName[500];
    szOutputFileName[0] = '\0';

    if ((!context) || (!dateAndTime))
    {
        return;
    }

    if ((debugPath) && (strlen(debugPath)))
    {
        snprintf(szOutputFileName, sizeof(szOutputFileName), "%s/%s/%s_%s.%s", debugPath,
                 ARSTREAM2_STREAM_STATS_VIDEO_STATS_OUTPUT_PATH,
                 ARSTREAM2_STREAM_STATS_VIDEO_STATS_OUTPUT_FILENAME,
                 dateAndTime,
                 ARSTREAM2_STREAM_STATS_VIDEO_STATS_OUTPUT_FILEEXT);
    }

    if (strlen(szOutputFileName))
    {
        context->outputFile = fopen(szOutputFileName, "w");
        if (!context->outputFile)
        {
            ARSAL_PRINT(ARSAL_PRINT_INFO, ARSTREAM2_STREAM_STATS_TAG, "Unable to open video stats output file '%s'", szOutputFileName);
        }
        else
        {
            ARSAL_PRINT(ARSAL_PRINT_INFO, ARSTREAM2_STREAM_STATS_TAG, "Opened video stats output file '%s'", szOutputFileName);
        }
    }

    if (context->outputFile)
    {
        char szTitle[200];
        int titleLen = 0;
        szTitle[0] = '\0';
        if ((friendlyName) && (strlen(friendlyName)))
        {
            titleLen += snprintf(szTitle + titleLen, 200 - titleLen, "%s ", friendlyName);
        }
        titleLen += snprintf(szTitle + titleLen, 200 - titleLen, "%s", dateAndTime);
        ARSAL_PRINT(ARSAL_PRINT_INFO, ARSTREAM2_STREAM_STATS_TAG, "Video stats output file title: '%s'", szTitle);
        fprintf(context->outputFile, "# %s\n", szTitle);
        fprintf(context->outputFile, "timestamp rssi totalFrameCount outputFrameCount erroredOutputFrameCount discardedFrameCount missedFrameCount");
        fprintf(context->outputFile, " timestampDeltaIntegral timestampDeltaIntegralSq timingErrorIntegral timingErrorIntegralSq estimatedLatencyIntegral estimatedLatencyIntegralSq erroredSecondCount");
        uint32_t i, j;
        for (i = 0; i < mbStatusZoneCount; i++)
        {
            fprintf(context->outputFile, " erroredSecondCountByZone[%d]", i);
        }
        for (j = 0; j < mbStatusClassCount; j++)
        {
            for (i = 0; i < mbStatusZoneCount; i++)
            {
                fprintf(context->outputFile, " macroblockStatus[%d][%d]", j, i);
            }
        }
        fprintf(context->outputFile, "\n");
        context->fileOutputTimestamp = 0;
    }
}


void ARSTREAM2_StreamStats_VideoStatsFileClose(ARSTREAM2_StreamStats_VideoStatsContext_t *context)
{
    if (context->outputFile)
    {
        fclose(context->outputFile);
        context->outputFile = NULL;
    }
}


void ARSTREAM2_StreamStats_VideoStatsFileWrite(ARSTREAM2_StreamStats_VideoStatsContext_t *context, const ARSTREAM2_H264_VideoStats_t *videoStats)
{
    if ((!context) || (!videoStats))
    {
        return;
    }

    if (!context->outputFile)
    {
        return;
    }

    if (context->fileOutputTimestamp == 0)
    {
        /* init */
        context->fileOutputTimestamp = videoStats->timestamp;
    }
    if (videoStats->timestamp >= context->fileOutputTimestamp + ARSTREAM2_STREAM_STATS_VIDEO_STATS_OUTPUT_INTERVAL)
    {
        if (context->outputFile)
        {
            fprintf(context->outputFile, "%llu %i %lu %lu %lu %lu %lu", (long long unsigned int)videoStats->timestamp, videoStats->rssi,
                    (long unsigned int)videoStats->totalFrameCount, (long unsigned int)videoStats->outputFrameCount,
                    (long unsigned int)videoStats->erroredOutputFrameCount, (long unsigned int)videoStats->discardedFrameCount,
                    (long unsigned int)videoStats->missedFrameCount);
            fprintf(context->outputFile, " %llu %llu %llu %llu %llu %llu %lu",
                    (long long unsigned int)videoStats->timestampDeltaIntegral, (long long unsigned int)videoStats->timestampDeltaIntegralSq,
                    (long long unsigned int)videoStats->timingErrorIntegral,(long long unsigned int)videoStats->timingErrorIntegralSq,
                    (long long unsigned int)videoStats->estimatedLatencyIntegral, (long long unsigned int)videoStats->estimatedLatencyIntegralSq,
                    (long unsigned int)videoStats->erroredSecondCount);
            uint32_t i, j;
            for (i = 0; i < videoStats->mbStatusZoneCount; i++)
            {
                fprintf(context->outputFile, " %lu", (long unsigned int)videoStats->erroredSecondCountByZone[i]);
            }
            for (j = 0; j < videoStats->mbStatusClassCount; j++)
            {
                for (i = 0; i < videoStats->mbStatusZoneCount; i++)
                {
                    fprintf(context->outputFile, " %lu", (long unsigned int)videoStats->macroblockStatus[j][i]);
                }
            }
            fprintf(context->outputFile, "\n");
        }
        context->fileOutputTimestamp = videoStats->timestamp;
    }
}


void ARSTREAM2_StreamStats_RtpStatsFileOpen(ARSTREAM2_StreamStats_RtpStatsContext_t *context, const char *debugPath,
                                            const char *friendlyName, const char *dateAndTime)
{
    char szOutputFileName[500];
    szOutputFileName[0] = '\0';

    if ((!context) || (!dateAndTime))
    {
        return;
    }

    if ((debugPath) && (strlen(debugPath)))
    {
        snprintf(szOutputFileName, sizeof(szOutputFileName), "%s/%s/%s_%s.%s", debugPath,
                 ARSTREAM2_STREAM_STATS_RTP_STATS_OUTPUT_PATH,
                 ARSTREAM2_STREAM_STATS_RTP_STATS_OUTPUT_FILENAME,
                 dateAndTime,
                 ARSTREAM2_STREAM_STATS_RTP_STATS_OUTPUT_FILEEXT);
    }

    if (strlen(szOutputFileName))
    {
        context->outputFile = fopen(szOutputFileName, "w");
        if (!context->outputFile)
        {
            ARSAL_PRINT(ARSAL_PRINT_INFO, ARSTREAM2_STREAM_STATS_TAG, "Unable to open RTP stats output file '%s'", szOutputFileName);
        }
        else
        {
            ARSAL_PRINT(ARSAL_PRINT_INFO, ARSTREAM2_STREAM_STATS_TAG, "Opened RTP stats output file '%s'", szOutputFileName);
        }
    }

    if (context->outputFile)
    {
        char szTitle[200];
        int titleLen = 0;
        szTitle[0] = '\0';
        if ((friendlyName) && (strlen(friendlyName)))
        {
            titleLen += snprintf(szTitle + titleLen, 200 - titleLen, "%s ", friendlyName);
        }
        titleLen += snprintf(szTitle + titleLen, 200 - titleLen, "%s", dateAndTime);
        ARSAL_PRINT(ARSAL_PRINT_INFO, ARSTREAM2_STREAM_STATS_TAG, "RTP stats output file title: '%s'", szTitle);
        fprintf(context->outputFile, "# %s\n", szTitle);
        fprintf(context->outputFile, "timestamp rssi senderPacketCount senderByteCount");
        fprintf(context->outputFile, " senderReportTimestamp senderReportLastInterval senderReportIntervalPacketCount senderReportIntervalByteCount");
        fprintf(context->outputFile, " receiverReportTimestamp receiverReportRoundTripDelay receiverReportInterarrivalJitter receiverReportReceiverLostCount receiverReportReceiverFractionLost receiverReportReceiverExtHighestSeqNum");
        fprintf(context->outputFile, " djbMetricsReportTimestamp djbMetricsReportDjbNominal djbMetricsReportDjbMax djbMetricsReportDjbHighWatermark djbMetricsReportDjbLowWatermark");
        fprintf(context->outputFile, " peerClockDelta clockDeltaRoundTripDelay clockDeltaPeer2meDelay clockDeltaMe2peerDelay");
        fprintf(context->outputFile, "\n");
        context->fileOutputTimestamp = 0;
    }
}


void ARSTREAM2_StreamStats_RtpStatsFileClose(ARSTREAM2_StreamStats_RtpStatsContext_t *context)
{
    if (context->outputFile)
    {
        fclose(context->outputFile);
        context->outputFile = NULL;
    }
}


void ARSTREAM2_StreamStats_RtpStatsFileWrite(ARSTREAM2_StreamStats_RtpStatsContext_t *context, const ARSTREAM2_RTP_RtpStats_t *rtpStats)
{
    if ((!context) || (!rtpStats))
    {
        return;
    }

    if ((context->outputFile) && (rtpStats->receiverReport.timestamp != 0))
    {
        if (context->fileOutputTimestamp == 0)
        {
            /* init */
            context->fileOutputTimestamp = rtpStats->timestamp;
        }
        if (rtpStats->timestamp >= context->fileOutputTimestamp + ARSTREAM2_STREAM_STATS_RTP_STATS_OUTPUT_INTERVAL)
        {
            fprintf(context->outputFile, "%llu %i %lu %llu", (long long unsigned int)rtpStats->timestamp, rtpStats->rssi,
                    (long unsigned int)rtpStats->senderPacketCount, (long long unsigned int)rtpStats->senderByteCount);
            if (rtpStats->senderReport.timestamp)
            {
                fprintf(context->outputFile, " %llu %lu %lu %lu", (long long unsigned int)rtpStats->senderReport.timestamp,
                        (long unsigned int)rtpStats->senderReport.lastInterval, (long unsigned int)rtpStats->senderReport.intervalPacketCount,
                        (long unsigned int)rtpStats->senderReport.intervalByteCount);
            }
            else
            {
                fprintf(context->outputFile, " %llu %lu %lu %lu", (long long unsigned int)0,
                        (long unsigned int)0, (long unsigned int)0,
                        (long unsigned int)0);
            }
            if (rtpStats->receiverReport.timestamp != 0)
            {
                fprintf(context->outputFile, " %llu %lu %lu %lu %lu %lu", (long long unsigned int)rtpStats->receiverReport.timestamp,
                        (long unsigned int)rtpStats->receiverReport.roundTripDelay, (long unsigned int)rtpStats->receiverReport.interarrivalJitter,
                        (long unsigned int)rtpStats->receiverReport.receiverLostCount, (long unsigned int)rtpStats->receiverReport.receiverFractionLost,
                        (long unsigned int)rtpStats->receiverReport.receiverExtHighestSeqNum);
            }
            else
            {
                fprintf(context->outputFile, " %llu %i %lu %lu %lu %lu %lu", (long long unsigned int)0, 0,
                        (long unsigned int)0, (long unsigned int)0,
                        (long unsigned int)0, (long unsigned int)0,
                        (long unsigned int)0);
            }
            if (rtpStats->djbMetricsReport.timestamp)
            {
                fprintf(context->outputFile, " %llu %lu %lu %lu %lu", (long long unsigned int)rtpStats->djbMetricsReport.timestamp,
                        (long unsigned int)rtpStats->djbMetricsReport.djbNominal, (long unsigned int)rtpStats->djbMetricsReport.djbMax,
                        (long unsigned int)rtpStats->djbMetricsReport.djbHighWatermark, (long unsigned int)rtpStats->djbMetricsReport.djbLowWatermark);
            }
            else
            {
                fprintf(context->outputFile, " %llu %lu %lu %lu %lu", (long long unsigned int)0,
                        (long unsigned int)0, (long unsigned int)0,
                        (long unsigned int)0, (long unsigned int)0);
            }
            fprintf(context->outputFile, " %lld %lu %lu %lu",
                    (long long int)rtpStats->clockDelta.peerClockDelta, (long unsigned int)rtpStats->clockDelta.roundTripDelay,
                    (long unsigned int)rtpStats->clockDelta.peer2meDelay, (long unsigned int)rtpStats->clockDelta.me2peerDelay);
            fprintf(context->outputFile, "\n");
            context->fileOutputTimestamp = rtpStats->timestamp;
        }
    }
}


void ARSTREAM2_StreamStats_RtpLossFileOpen(ARSTREAM2_StreamStats_RtpLossContext_t *context, const char *debugPath,
                                           const char *friendlyName, const char *dateAndTime)
{
    char szOutputFileName[500];
    szOutputFileName[0] = '\0';

    if ((!context) || (!dateAndTime))
    {
        return;
    }

    if ((debugPath) && (strlen(debugPath)))
    {
        snprintf(szOutputFileName, sizeof(szOutputFileName), "%s/%s/%s_%s.%s", debugPath,
                 ARSTREAM2_STREAM_STATS_RTP_LOSS_OUTPUT_PATH,
                 ARSTREAM2_STREAM_STATS_RTP_LOSS_OUTPUT_FILENAME,
                 dateAndTime,
                 ARSTREAM2_STREAM_STATS_RTP_LOSS_OUTPUT_FILEEXT);
    }

    if (strlen(szOutputFileName))
    {
        context->outputFile = fopen(szOutputFileName, "w");
        if (!context->outputFile)
        {
            ARSAL_PRINT(ARSAL_PRINT_INFO, ARSTREAM2_STREAM_STATS_TAG, "Unable to open RTP loss output file '%s'", szOutputFileName);
        }
        else
        {
            ARSAL_PRINT(ARSAL_PRINT_INFO, ARSTREAM2_STREAM_STATS_TAG, "Opened RTP loss output file '%s'", szOutputFileName);
        }
    }

    if (context->outputFile)
    {
        char szTitle[200];
        int titleLen = 0;
        szTitle[0] = '\0';
        if ((friendlyName) && (strlen(friendlyName)))
        {
            titleLen += snprintf(szTitle + titleLen, 200 - titleLen, "%s ", friendlyName);
        }
        titleLen += snprintf(szTitle + titleLen, 200 - titleLen, "%s", dateAndTime);
        ARSAL_PRINT(ARSAL_PRINT_INFO, ARSTREAM2_STREAM_STATS_TAG, "RTP loss output file title: '%s'", szTitle);
        fprintf(context->outputFile, "# %s\n", szTitle);
        fprintf(context->outputFile, "timestamp startSeqNum endSeqNum receivedFlag");
        fprintf(context->outputFile, "\n");
    }
}


void ARSTREAM2_StreamStats_RtpLossFileClose(ARSTREAM2_StreamStats_RtpLossContext_t *context)
{
    if (context->outputFile)
    {
        fclose(context->outputFile);
        context->outputFile = NULL;
    }
}


void ARSTREAM2_StreamStats_RtpLossFileWrite(ARSTREAM2_StreamStats_RtpLossContext_t *context, const ARSTREAM2_RTP_RtpStats_t *rtpStats)
{
    if ((!context) || (!rtpStats))
    {
        return;
    }

    if ((context->outputFile) && (rtpStats->lossReport.timestamp != 0))
    {
        fprintf(context->outputFile, "%llu %u %u ", (long long unsigned int)rtpStats->lossReport.timestamp,
                (rtpStats->lossReport.startSeqNum & 0xFFFF), (rtpStats->lossReport.endSeqNum & 0xFFFF));
        int i, j, k, packetCount = (int)rtpStats->lossReport.endSeqNum - (int)rtpStats->lossReport.startSeqNum + 1;
        if (packetCount <= 0) packetCount += (1 << 16);
        int wordCount = (packetCount >> 5) + ((packetCount & 0x1F) ? 1 : 0);
        for (i = 0, k = 0; i < wordCount; i++)
        {
            if (k == packetCount)
            {
                break;
            }
            for (j = 0; j < 32; j++, k++)
            {
                if (k == packetCount)
                {
                    break;
                }
                fprintf(context->outputFile, "%c", ((rtpStats->lossReport.receivedFlag[i] >> (31 - j)) & 1) ? '1' : '0');
            }
        }
        fprintf(context->outputFile, "\n");
    }
}
