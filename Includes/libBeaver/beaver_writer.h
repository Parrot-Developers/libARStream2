/**
 * @file beaver_writer.h
 * @brief Parrot Streaming Library - H.264 Writer
 * @date 08/04/2015
 * @author aurelien.barre@parrot.com
 */

#ifndef _BEAVER_WRITER_H_
#define _BEAVER_WRITER_H_

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

#include <inttypes.h>
#include <stdio.h>


/**
 * @brief Beaver Writer instance handle.
 */
typedef void* BEAVER_Writer_Handle;


/**
 * @brief Beaver Writer configuration for initialization.
 */
typedef struct
{
    int naluPrefix;                         /**< write a NAL unit start code before each NALU */

} BEAVER_Writer_Config_t;


/**
 * @brief Recovery point SEI syntax elements.
 */
typedef struct
{
    unsigned int recoveryFrameCnt;          /**< recovery_frame_cnt syntax element */
    unsigned int exactMatchFlag;            /**< exact_match_flag syntax element */
    unsigned int brokenLinkFlag;            /**< broken_link_flag syntax element */
    unsigned int changingSliceGroupIdc;     /**< changing_slice_group_idc syntax element */

} BEAVER_Writer_RecoveryPointSei_t;


/**
 * @brief Initialize a Beaver Writer instance.
 *
 * The library allocates the required resources. The user must call BEAVER_Writer_Free() to free the resources.
 *
 * @param writerHandle Pointer to the handle used in future calls to the library.
 * @param config The instance configuration.
 *
 * @return 0 if no error occurred.
 * @return -1 if an error occurred.
 */
int BEAVER_Writer_Init(BEAVER_Writer_Handle* writerHandle, BEAVER_Writer_Config_t* config);


/**
 * @brief Free a Beaver Writer instance.
 *
 * The library frees the allocated resources.
 *
 * @param writerHandle Instance handle.
 *
 * @return 0 if no error occurred.
 * @return -1 if an error occurred.
 */
int BEAVER_Writer_Free(BEAVER_Writer_Handle writerHandle);


/**
 * @brief Sets the Writer SPS and PPS context.
 *
 * The function imports SPS and PPS context from a Beaver Parser.
 *
 * @param[in] writerHandle Instance handle.
 * @param[in] spsContext SPS context to use (from a Beaver Parser)
 * @param[in] ppsContext PPS context to use (from a Beaver Parser)
 *
 * @return 0 if no error occurred.
 * @return -1 if an error occurred.
 */
int BEAVER_Writer_SetSpsPpsContext(BEAVER_Writer_Handle writerHandle, const void *spsContext, const void *ppsContext);


/**
 * @brief Write a SEI NAL unit.
 *
 * The function writes a Supplemental Enhancement Information NAL unit.
 * If recoveryPoint is not NULL, a recovery point SEI message is generated.
 * If userDataUnregisteredCount is not null, one or more user data unregistered messages are generated.
 *
 * @param[in] writerHandle Instance handle.
 * @param[in] recoveryPoint Recovery point syntax elements
 * @param[in] userDataUnregisteredCount User data count
 * @param[in] pbUserDataUnregistered User data input buffer array
 * @param[in] userDataUnregisteredSize User data input buffer size array
 * @param[in] pbOutputBuf Bitstream output buffer
 * @param[in] outputBufSize Bitstream output buffer size
 * @param[out] outputSize Bitstream output size
 *
 * @return 0 if no error occurred.
 * @return -1 if an error occurred.
 */
int BEAVER_Writer_WriteSeiNalu(BEAVER_Writer_Handle writerHandle, BEAVER_Writer_RecoveryPointSei_t *recoveryPoint, unsigned int userDataUnregisteredCount, const uint8_t *pbUserDataUnregistered[], unsigned int userDataUnregisteredSize[], uint8_t *pbOutputBuf, unsigned int outputBufSize, unsigned int *outputSize);


/**
 * @brief Write a gray I-slice NAL unit.
 *
 * The function writes an entirely gray I-slice NAL unit.
 *
 * @param[in] writerHandle Instance handle.
 * @param[in] firstMbInSlice Slice first macroblock index
 * @param[in] sliceMbCount Slice macroblock count
 * @param[in] sliceContext Optional slice context to use (from a Beaver Parser)
 * @param[in] pbOutputBuf Bitstream output buffer
 * @param[in] outputBufSize Bitstream output buffer size
 * @param[out] outputSize Bitstream output size
 *
 * @return 0 if no error occurred.
 * @return -1 if an error occurred.
 */
int BEAVER_Writer_WriteGrayISliceNalu(BEAVER_Writer_Handle writerHandle, unsigned int firstMbInSlice, unsigned int sliceMbCount, void *sliceContext, uint8_t *pbOutputBuf, unsigned int outputBufSize, unsigned int *outputSize);


/**
 * @brief Write a skipped P-slice NAL unit.
 *
 * The function writes an entirely skipped P-slice NAL unit.
 *
 * @param[in] writerHandle Instance handle.
 * @param[in] firstMbInSlice Slice first macroblock index
 * @param[in] sliceMbCount Slice macroblock count
 * @param[in] sliceContext Optional slice context to use (from a Beaver Parser)
 * @param[in] pbOutputBuf Bitstream output buffer
 * @param[in] outputBufSize Bitstream output buffer size
 * @param[out] outputSize Bitstream output size
 *
 * @return 0 if no error occurred.
 * @return -1 if an error occurred.
 */
int BEAVER_Writer_WriteSkippedPSliceNalu(BEAVER_Writer_Handle writerHandle, unsigned int firstMbInSlice, unsigned int sliceMbCount, void *sliceContext, uint8_t *pbOutputBuf, unsigned int outputBufSize, unsigned int *outputSize);


#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */

#endif /* #ifndef _BEAVER_WRITER_H_ */

