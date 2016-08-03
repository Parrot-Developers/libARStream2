
package com.parrot.arsdk.arstream2;


import java.nio.ByteBuffer;

/**
 * This interface describes a listener for ARStream2Receiver events
 */
public interface ARStream2ReceiverListener
{
    /**
     * Called when sps/pps are received
     * 
     * Implementation init the decoder and return an array of codec input buffers that will be filled with received AU
     */
    ByteBuffer[] onSpsPpsReady(ByteBuffer sps, ByteBuffer pps);

    /**
     * Called when a new input buffer is require
     * 
     * Implementation must return the index of one of the available input buffer, or -1 if there is no available input buffer
     * The buffer is considered busy until
     */
    int getFreeBuffer();

    /**
     * Called when a buffer is ready to be sent to the decoder
     * 
     */
    void onBufferReady(int bufferIdx, long auExtRtpTimestamp, long auNtpTimestamp, long auTimestampLocal, ARSTREAM2_STREAM_RECEIVER_AU_SYNC_TYPE_ENUM auSyncType);
}

