/*
 */

#ifndef __SHA1_H__
#define __SHA1_H__



/**
 * Computes a unencoded 20-byte digest from data.
 *
 * @param inData the data to hash.
 *   Must be destroyed by caller.
 * @param inDataLength the length of the data to hash.
 *   Must be destroyed by caller.
 *
 * @return the digest as a byte array of length 20.
 *   Must be destroyed by caller.
 */
unsigned char *computeRawSHA1Digest( unsigned char *inData, int inDataLength );



/**
 * Computes a unencoded 20-byte digest from an arbitrary string message.
 *
 * @param inString the message as a \0-terminated string.
 *   Must be destroyed by caller.
 *
 * @return the digest as a byte array of length 20.
 *   Must be destroyed by caller.
 */
unsigned char *computeRawSHA1Digest( char *inString );


    
/**
 * Computes a hex-encoded string digest from data.
 *
 * @param inData the data to hash.
 *   Must be destroyed by caller.
 * @param inDataLength the length of the data to hash.
 *   Must be destroyed by caller.
 *
 * @return the digest as a \0-terminated string.
 *   Must be destroyed by caller.
 */
char *computeSHA1Digest( unsigned char *inData, int inDataLength );


    
/**
 * Computes a hex-encoded string digest from an arbitrary string message.
 *
 * @param inString the message as a \0-terminated string.
 *   Must be destroyed by caller.
 *
 * @return the digest as a \0-terminated string.
 *   Must be destroyed by caller.
 */
char *computeSHA1Digest( char *inString );



#endif

