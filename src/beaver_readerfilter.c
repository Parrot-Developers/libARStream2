/**
 * @file beaver_readerfilter.c
 * @brief H.264 Elementary Stream Reader and Filter
 * @date 08/04/2015
 * @author aurelien.barre@parrot.com
 */

#include <stdio.h>
#include <stdlib.h>

#include <libARSAL/ARSAL_Print.h>

#include <libBeaver/beaver_readerfilter.h>


#define BEAVER_READERFILTER_TAG "BEAVER_ReaderFilter"


typedef struct BEAVER_ReaderFilter_s
{
    BEAVER_Filter_Handle filter;
    ARSTREAM_Reader2_t *reader;

} BEAVER_ReaderFilter_t;



int BEAVER_ReaderFilter_Init(BEAVER_ReaderFilter_Handle *readerFilterHandle, BEAVER_ReaderFilter_Config_t *config)
{
    int ret = 0;
    eARSTREAM_ERROR err = ARSTREAM_OK;
    BEAVER_ReaderFilter_t *readerFilter = NULL;

    if (!readerFilterHandle)
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, BEAVER_READERFILTER_TAG, "Invalid pointer for handle");
        return -1;
    }
    if (!config)
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, BEAVER_READERFILTER_TAG, "Invalid pointer for config");
        return -1;
    }

    readerFilter = (BEAVER_ReaderFilter_t*)malloc(sizeof(*readerFilter));
    if (!readerFilter)
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, BEAVER_READERFILTER_TAG, "Allocation failed (size %ld)", sizeof(*readerFilter));
        ret = -1;
    }

    if (ret == 0)
    {
        memset(readerFilter, 0, sizeof(*readerFilter));

        BEAVER_Filter_Config_t filterConfig;
        memset(&filterConfig, 0, sizeof(filterConfig));
        filterConfig.auFifoSize = config->auFifoSize;
        filterConfig.waitForSync = config->waitForSync;
        filterConfig.outputIncompleteAu = config->outputIncompleteAu;
        filterConfig.filterOutSpsPps = config->filterOutSpsPps;
        filterConfig.filterOutSei = config->filterOutSei;
        filterConfig.replaceStartCodesWithNaluSize = config->replaceStartCodesWithNaluSize;
        filterConfig.generateSkippedPSlices = config->generateSkippedPSlices;
        filterConfig.generateFirstGrayIFrame = config->generateFirstGrayIFrame;

        ret = BEAVER_Filter_Init(&readerFilter->filter, &filterConfig);
        if (ret != 0)
        {
            ARSAL_PRINT(ARSAL_PRINT_ERROR, BEAVER_READERFILTER_TAG, "Error while creating beaver_filter: %d", ret);
            ret = -1;
        }
    }

    if (ret == 0)
    {
        ARSTREAM_Reader2_Config_t readerConfig;
        memset(&readerConfig, 0, sizeof(readerConfig));

        readerConfig.serverAddr = config->serverAddr;
        readerConfig.mcastAddr = config->mcastAddr;
        readerConfig.mcastIfaceAddr = config->mcastIfaceAddr;
        readerConfig.serverStreamPort = config->serverStreamPort;
        readerConfig.serverControlPort = config->serverControlPort;
        readerConfig.clientStreamPort = config->clientStreamPort;
        readerConfig.clientControlPort = config->clientControlPort;
        readerConfig.naluCallback = BEAVER_Filter_ArstreamReader2NaluCallback;
        readerConfig.naluCallbackUserPtr = (void*)readerFilter->filter;
        readerConfig.maxPacketSize = config->maxPacketSize;
        readerConfig.maxBitrate = config->maxBitrate;
        readerConfig.maxLatencyMs = config->maxLatencyMs;
        readerConfig.maxNetworkLatencyMs = config->maxNetworkLatencyMs;
        readerConfig.insertStartCodes = 1;

        readerFilter->reader = ARSTREAM_Reader2_New(&readerConfig, &err);
        if (err != ARSTREAM_OK)
        {
            ARSAL_PRINT(ARSAL_PRINT_ERROR, BEAVER_READERFILTER_TAG, "Error while creating reader : %s", ARSTREAM_Error_ToString(err));
            ret = -1;
        }
    }

    if (ret == 0)
    {
        *readerFilterHandle = (BEAVER_ReaderFilter_Handle*)readerFilter;
    }
    else
    {
        if (readerFilter)
        {
            if (readerFilter->reader) ARSTREAM_Reader2_Delete(&(readerFilter->reader));
            if (readerFilter->filter) BEAVER_Filter_Free(&(readerFilter->filter));
            free(readerFilter);
        }
        *readerFilterHandle = NULL;
    }

    return ret;
}


int BEAVER_ReaderFilter_Free(BEAVER_ReaderFilter_Handle *readerFilterHandle)
{
    BEAVER_ReaderFilter_t* readerFilter;
    int ret = 0;
    eARSTREAM_ERROR err;

    if ((!readerFilterHandle) || (!*readerFilterHandle))
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, BEAVER_READERFILTER_TAG, "Invalid pointer for handle");
        return -1;
    }

    readerFilter = (BEAVER_ReaderFilter_t*)*readerFilterHandle;

    err = ARSTREAM_Reader2_Delete(&readerFilter->reader);
    if (err != ARSTREAM_OK)
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, BEAVER_READERFILTER_TAG, "Unable to delete reader: %s", ARSTREAM_Error_ToString(err));
        ret = -1;
    }

    if (ret == 0)
    {
        ret = BEAVER_Filter_Free(&readerFilter->filter);
        if (ret != 0)
        {
            ARSAL_PRINT(ARSAL_PRINT_ERROR, BEAVER_READERFILTER_TAG, "Unable to delete beaver_filter: %d", ret);
            ret = -1;
        }
    }

    return ret;
}


void* BEAVER_ReaderFilter_RunFilterThread(void *readerFilterHandle)
{
    BEAVER_ReaderFilter_t* readerFilter = (BEAVER_ReaderFilter_t*)readerFilterHandle;

    if (!readerFilterHandle)
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, BEAVER_READERFILTER_TAG, "Invalid handle");
        return NULL;
    }

    return BEAVER_Filter_RunFilterThread((void*)readerFilter->filter);
}


void* BEAVER_ReaderFilter_RunStreamThread(void *readerFilterHandle)
{
    BEAVER_ReaderFilter_t* readerFilter = (BEAVER_ReaderFilter_t*)readerFilterHandle;

    if (!readerFilterHandle)
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, BEAVER_READERFILTER_TAG, "Invalid handle");
        return NULL;
    }

    return ARSTREAM_Reader2_RunStreamThread((void*)readerFilter->reader);
}


void* BEAVER_ReaderFilter_RunControlThread(void *readerFilterHandle)
{
    BEAVER_ReaderFilter_t* readerFilter = (BEAVER_ReaderFilter_t*)readerFilterHandle;

    if (!readerFilterHandle)
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, BEAVER_READERFILTER_TAG, "Invalid handle");
        return NULL;
    }

    return ARSTREAM_Reader2_RunControlThread((void*)readerFilter->reader);
}


int BEAVER_ReaderFilter_StartFilter(BEAVER_ReaderFilter_Handle readerFilterHandle, BEAVER_Filter_SpsPpsCallback_t spsPpsCallback, void* spsPpsCallbackUserPtr,
                                    BEAVER_Filter_GetAuBufferCallback_t getAuBufferCallback, void* getAuBufferCallbackUserPtr,
                                    BEAVER_Filter_AuReadyCallback_t auReadyCallback, void* auReadyCallbackUserPtr)
{
    BEAVER_ReaderFilter_t* readerFilter = (BEAVER_ReaderFilter_t*)readerFilterHandle;
    int ret = 0;

    if (!readerFilterHandle)
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, BEAVER_READERFILTER_TAG, "Invalid handle");
        return -1;
    }

    ret = BEAVER_Filter_Start(readerFilter->filter, spsPpsCallback, spsPpsCallbackUserPtr,
                              getAuBufferCallback, getAuBufferCallbackUserPtr,
                              auReadyCallback, auReadyCallbackUserPtr);
    if (ret != 0)
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, BEAVER_READERFILTER_TAG, "Unable to pause beaver_filter: %d", ret);
        return ret;
    }

    return ret;
}


int BEAVER_ReaderFilter_PauseFilter(BEAVER_ReaderFilter_Handle readerFilterHandle)
{
    BEAVER_ReaderFilter_t* readerFilter = (BEAVER_ReaderFilter_t*)readerFilterHandle;
    int ret = 0;

    if (!readerFilterHandle)
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, BEAVER_READERFILTER_TAG, "Invalid handle");
        return -1;
    }

    ret = BEAVER_Filter_Pause(readerFilter->filter);
    if (ret != 0)
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, BEAVER_READERFILTER_TAG, "Unable to pause beaver_filter: %d", ret);
        return ret;
    }

    ARSTREAM_Reader2_InvalidateNaluBuffer(readerFilter->reader);

    return ret;
}


int BEAVER_ReaderFilter_Stop(BEAVER_ReaderFilter_Handle readerFilterHandle)
{
    BEAVER_ReaderFilter_t* readerFilter = (BEAVER_ReaderFilter_t*)readerFilterHandle;
    int ret = 0;

    if (!readerFilterHandle)
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, BEAVER_READERFILTER_TAG, "Invalid handle");
        return -1;
    }

    ARSTREAM_Reader2_StopReader(readerFilter->reader);

    ret = BEAVER_Filter_Stop(readerFilter->filter);
    if (ret != 0)
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, BEAVER_READERFILTER_TAG, "Unable to stop beaver_filter: %d", ret);
        return ret;
    }

    return ret;
}


int BEAVER_ReaderFilter_GetSpsPps(BEAVER_ReaderFilter_Handle readerFilterHandle, uint8_t *spsBuffer, int *spsSize, uint8_t *ppsBuffer, int *ppsSize)
{
    BEAVER_ReaderFilter_t* readerFilter = (BEAVER_ReaderFilter_t*)readerFilterHandle;

    if (!readerFilterHandle)
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, BEAVER_READERFILTER_TAG, "Invalid handle");
        return -1;
    }

    return BEAVER_Filter_GetSpsPps(readerFilter->filter, spsBuffer, spsSize, ppsBuffer, ppsSize);
}


int BEAVER_ReaderFilter_InitResender(BEAVER_ReaderFilter_Handle readerFilterHandle, BEAVER_ReaderFilter_ResenderHandle *resenderHandle, BEAVER_ReaderFilter_ResenderConfig_t *config)
{
    BEAVER_ReaderFilter_t* readerFilter = (BEAVER_ReaderFilter_t*)readerFilterHandle;
    eARSTREAM_ERROR err = ARSTREAM_OK;
    ARSTREAM_Reader2_Resender_t* retResender = NULL;
    int ret = 0;

    if (!readerFilterHandle)
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, BEAVER_READERFILTER_TAG, "Invalid handle");
        return -1;
    }
    if (!resenderHandle)
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, BEAVER_READERFILTER_TAG, "Invalid pointer for resender");
        return -1;
    }
    if (!config)
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, BEAVER_READERFILTER_TAG, "Invalid pointer for config");
        return -1;
    }

    ARSTREAM_Reader2_Resender_Config_t resenderConfig;
    memset(&resenderConfig, 0, sizeof(resenderConfig));

    resenderConfig.clientAddr = config->clientAddr;
    resenderConfig.mcastAddr = config->mcastAddr;
    resenderConfig.mcastIfaceAddr = config->mcastIfaceAddr;
    resenderConfig.serverStreamPort = config->serverStreamPort;
    resenderConfig.serverControlPort = config->serverControlPort;
    resenderConfig.clientStreamPort = config->clientStreamPort;
    resenderConfig.clientControlPort = config->clientControlPort;
    resenderConfig.maxPacketSize = config->maxPacketSize;
    resenderConfig.targetPacketSize = config->targetPacketSize;
    resenderConfig.maxLatencyMs = config->maxLatencyMs;
    resenderConfig.maxNetworkLatencyMs = config->maxNetworkLatencyMs;

    retResender = ARSTREAM_Reader2_Resender_New(readerFilter->reader, &resenderConfig, &err);
    if (err != ARSTREAM_OK)
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, BEAVER_READERFILTER_TAG, "Error while creating resender : %s", ARSTREAM_Error_ToString(err));
        ret = -1;
    }
    else
    {
        *resenderHandle = (BEAVER_ReaderFilter_ResenderHandle)retResender;
    }

    return ret;
}


int BEAVER_ReaderFilter_FreeResender(BEAVER_ReaderFilter_ResenderHandle *resenderHandle)
{
    int ret = 0;
    eARSTREAM_ERROR err = ARSTREAM_OK;

    err = ARSTREAM_Reader2_Resender_Delete((ARSTREAM_Reader2_Resender_t**)resenderHandle);
    if (err != ARSTREAM_OK)
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, BEAVER_READERFILTER_TAG, "Error while deleting resender : %s", ARSTREAM_Error_ToString(err));
        ret = -1;
    }

    return ret;
}


void* BEAVER_ReaderFilter_RunResenderStreamThread(void *resenderHandle)
{
    return ARSTREAM_Reader2_Resender_RunStreamThread(resenderHandle);
}


void* BEAVER_ReaderFilter_RunResenderControlThread(void *resenderHandle)
{
    return ARSTREAM_Reader2_Resender_RunControlThread(resenderHandle);
}


int BEAVER_ReaderFilter_StopResender(BEAVER_ReaderFilter_ResenderHandle resenderHandle)
{
    ARSTREAM_Reader2_Resender_Stop((ARSTREAM_Reader2_Resender_t*)resenderHandle);

    return 0;
}

