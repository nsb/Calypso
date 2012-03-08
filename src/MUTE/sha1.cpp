/*
 * Modification History
 *
 * 2002-May-25   Jason Rohrer
 * Changed to use minorGems endian.h
 * Added a function for hashing an entire string to a hex digest.
 * Added a function for getting a raw digest.
 * Fixed a deletion bug.
 *
 * 2003-August-24   Jason Rohrer
 * Switched to use minorGems hex encoding.
 *
 * 2003-September-15   Jason Rohrer
 * Added support for hashing raw (non-string) data.
 *
 * 2003-September-21   Jason Rohrer
 * Fixed bug that was causing overwrite of input data.
 *
 * 2004-January-13   Jason Rohrer
 * Fixed system includes.
 */



/*
 * sha1.c
 *
 * Originally witten by Steve Reid <steve@edmweb.com>
 * 
 * Modified by Aaron D. Gifford <agifford@infowest.com>
 *
 * NO COPYRIGHT - THIS IS 100% IN THE PUBLIC DOMAIN
 *
 * The original unmodified version is available at:
 *    ftp://ftp.funet.fi/pub/crypt/hash/sha/sha1.c
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR(S) OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "MUTE/sha1.h"
#include <string.h>
#include <stdio.h>
#include <QCryptographicHash>

// for hex encoding
//#include "minorGems/formats/encodingUtils.h"



unsigned char *computeRawSHA1Digest( unsigned char *inData,
                                     int inDataLength ) {

    QCryptographicHash hash (QCryptographicHash::Sha1);
    hash.addData((const char *)inData,inDataLength);
    unsigned char *buffer = new unsigned char[ hash.result().size()+1 ];
    memcpy(buffer,hash.result().data(),hash.result().size()+1);
    return buffer;

    }



unsigned char *computeRawSHA1Digest( char *inString ) {
    QCryptographicHash hash (QCryptographicHash::Sha1);
    hash.addData(inString,strlen(inString));
    unsigned char *buffer = new unsigned char[ hash.result().size()+1 ];
    memcpy(buffer,hash.result().data(),hash.result().size()+1);
    return buffer;
    }



char *computeSHA1Digest( char *inString ) {
    QCryptographicHash hash (QCryptographicHash::Sha1);
    hash.addData(inString,strlen(inString));
    QByteArray hex=hash.result().toHex().toUpper();
    char *buffer = new char[ hex.size()+1 ];
    memcpy(buffer,hex.data(),hex.size()+1);
    return buffer;
    }



char *computeSHA1Digest( unsigned char *inData, int inDataLength ) {
    QCryptographicHash hash (QCryptographicHash::Sha1);
    hash.addData((const char *)inData,inDataLength);
    QByteArray hex=hash.result().toHex().toUpper();
    char *buffer = new char[ hex.size()+1 ];
    memcpy(buffer,hex.data(),hex.size()+1);
    return buffer;
    }



