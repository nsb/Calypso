/*
 * Modification History
 *
 * 2003-June-27   Jason Rohrer
 * Created.
 *
 * 2003-August-7   Jason Rohrer
 * Added a duplicate message detector.
 *
 * 2003-August-8   Jason Rohrer
 * Switched log to file output.
 * Added functions for getting and setting target connection count.
 *
 * 2003-August-11   Jason Rohrer
 * Added stopping log messages.
 *
 * 2003-August-12   Jason Rohrer
 * Changed to tell duplicate detector about our locally generated messages.
 *
 * 2003-August-13   Jason Rohrer
 * Added use of unique node name to prevent self and multiple connections.
 *
 * 2003-August-14   Jason Rohrer
 * Made unique name function publicly available.
 * Added a function for getting connected host list.
 *
 * 2003-August-24   Jason Rohrer
 * Added generation of node RSA keys.
 * Added functions for seeding the random number generator.
 *
 * 2003-August-25   Jason Rohrer
 * Added support for registering message handler functions.
 * Replaced local info hash with secure random string for unique names.
 *
 * 2003-October-9   Jason Rohrer
 * Added support for message limiters.
 *
 * 2003-October-12   Jason Rohrer
 * Switched to a floating point limit.
 * Added use of message ID tracker.
 *
 * 2003-November-2   Jason Rohrer
 * Switched inbound limit setting function to a floating point limit.
 *
 * 2003-November-24   Jason Rohrer
 * Added support for flags.
 *
 * 2003-December-5   Jason Rohrer
 * Added support for message utility.
 *
 * 2004-January-11   Jason Rohrer
 * Made include paths explicit to help certain compilers.
 *
 * 2004-February-4   Jason Rohrer
 * Added utility functions for modifying flag strings.
 *
 * 2004-February-20   Jason Rohrer
 * Added function for getting info about current connection attempt.
 *
 * 2004-March-1   Jason Rohrer
 * Added function for generating message IDs.
 *
 * 2004-March-9   Jason Rohrer
 * Added support for new FORWARD scheme.
 *
 * 2004-March-15   Jason Rohrer
 * Added forward hash code that was originally in ChannelReceivingThread.cpp.
 * Added code to ensure that forward hash seed is forwardable.
 *
 * 2004-March-19   Jason Rohrer
 * Added support for DROP tails.
 *
 * 2004-March-23   Jason Rohrer
 * Added a maximum connection count.
 *
 * 2004-December-12   Jason Rohrer
 * Added a setting for the log rollover time.
 *
 * 2004-December-24   Jason Rohrer
 * Added a function for getting the connection count.
 *
 * 2005-April-15   Jason Rohrer
 * Changed to use drop trees instead of drop chains.
 *
 * 2006-April-24   Jason Rohrer
 * Added function to generate virtual address mnemonics.
 *
 * 2006-June-2   Jason Rohrer
 * Added message signing support.
 *
 * 2006-June-3   Jason Rohrer
 * Added message counter wrap-around detection to prevent replay attacks.
 */



#include <stdio.h>

#include "applog.h"

#include "settings.h"
#include "MUTE/messageRouter.h"


#include "MUTE/ConnectionMaintainer.h"
#include "MUTE/OutboundChannelManager.h"
#include "MUTE/LocalAddressReceiver.h"
#include "MUTE/ServerThread.h"
#include "MUTE/MessageIDTracker.h"

#include "MUTE/common/CryptoUtils.h"


#include "minorGems/network/p2pParts/HostCatcher.h"
#include "MUTE/kblimiter.h"

#include "minorGems/network/HostAddress.h"
#include "minorGems/util/random/StdRandomSource.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/util/SettingsManager.h"
#include "minorGems/util/SimpleVector.h"
#include "minorGems/crypto/hashes/sha1.h"


// for toupper()
#include "ctype.h"

 extern class Settings *settings;


// static data elements
OutboundChannelManager *muteOutboundChannelManager = NULL;
LocalAddressReceiver *muteLocalAddressReceiver = NULL;
ConnectionMaintainer *muteConnectionMaintainer = NULL;
HostCatcher *muteHostCatcher = NULL;
MessageIDTracker *muteMessageIDTracker = NULL;
KbLimiter *muteOutboundMessagePerSecondLimiter = NULL;
KbLimiter *muteInboundMessagePerSecondLimiter = NULL;

char *muteNodeUniqueName = NULL;
unsigned int mutePort = 0;
ServerThread *muteServerThread = NULL;
StdRandomSource *muteRandomSource = NULL;

char *muteForwardHashSeed = NULL;
double muteContinueForwardProbability = 0.5;
int muteNumNeighborsToSendDropTailsTo = 0;
int muteDropTailTreeStartingTTL = 2;

char *muteMessageIDSigningPublicKey = NULL;
char *muteMessageIDSigningPrivateKey = NULL;


// used to generate mnemonics
QStringList muteDictionaryWords;






void muteSeedRandomGenerator( char *inSeedString ) {
    CryptoUtils::seedRandomGenerator( inSeedString );
    }



char *muteGetRandomGeneratorState() {
    return CryptoUtils::getRandomGeneratorState();
    }



void muteStart( unsigned int inPort ) {

    int logRolloverTime = settings->getLogRollOverInSecondsSetting();

//    AppLog::setLog( new FileLog( "MUTE.log",
//                                 (unsigned long)logRolloverTime ) );

//    AppLog::setLoggingLevel( AppLog::CRITICAL_ERROR_LEVEL );

    char *loggerName = "messageRouter";

    muteRandomSource = new StdRandomSource();

    muteNodeUniqueName = muteGetUniqueName();


    // read our setting for the probability of continuing to forward a message
    muteContinueForwardProbability = settings->getContinueForwardProbabilitySetting();

    // make sure our forward probability is positive
    if( muteContinueForwardProbability <= 0 ) {
        // default to 50% if not
        muteContinueForwardProbability = 0.5;
        }


    // we need to decide how many neighbors we will send DROP_CHAIN
    // messages on to.  We will use the following probability distribution,
    // where p(n) is the probability that we send DROP_CHAINs to n neighbors:
    //
    // p(0) = 3/4
    // p(i) = 1 / ( 2^(i+2) )
    //
    // Thus,
    // p(1) = 1/8
    // p(2) = 1/16
    // p(3) = 1/32
    // p(4) = 1/64
    //     ...
    //
    // We can achieve this distribution by first flipping an unfair coin
    // to decide whether we send to 0 or not (3/4 chance of sending to 0, and
    // 1/4 chance of sending to more).  If we decide to send to more than zero,
    // we can flip a series of fair coins until we flip "false" to decide
    // how many neighbors we send to.  Thus, for our series of fair flips, we
    // have:
    // false                => send to 1
    // true false           => send to 2
    // true true false      => send to 3
    // true true true false => send to 3
    // Note that the chance of choosing 2 is half the chance of choosing 1,
    // and the chance of choosing i+1 is half the chance of choosing i, for
    // all i >= 1, which is what we want.

    // a test program that demonstrates this technique can be found in
    // test/dropTreeProbabilityTest.cpp
    
    // with this formulation, our expected number of neighbors is 1/2
    double expectedDropRate = 0.5;

    
    // first, flip an unfair coin
    if( muteRandomSource->getRandomFloat() <= 0.75f ) {
        muteNumNeighborsToSendDropTailsTo = 0;
        }
    else {
        // flip a series of fair coins until we flip "false"

        // increment our neighbor count each time we flip true
        muteNumNeighborsToSendDropTailsTo = 1;

        while( muteRandomSource->getRandomFloat() <= 0.5f ) {
            muteNumNeighborsToSendDropTailsTo++;
            }
        }

    char *logMessage =
        autoSprintf( "Will send DROP_CHAIN messages on to %d neighbors.",
                     muteNumNeighborsToSendDropTailsTo );
    AppLog::info( loggerName, logMessage );
    delete [] logMessage;

    
    // pick a TTL that matches the expected length of our drop chain
    muteDropTailTreeStartingTTL = (int)( 1.0 / expectedDropRate );

    
    
    // keep picking hash seeds until we find one that we would
    // forward according to our forward probability
    muteForwardHashSeed = CryptoUtils::getRandomHexString( 20 );

    // re-hash our seed to see if it produces a new hash that would
    // be forwarded
    char *newHashSeed = muteComputeNewForwardHash( muteForwardHashSeed );

    // while new hash would not be forwarded
    while( newHashSeed == NULL ) {
        char *logMessage = autoSprintf( "Picked %s as forward hash seed, "
                                        "but it does not re-hash to a "
                                        "forwardable hash, so picking again.",
                                        muteForwardHashSeed );
        AppLog::info( loggerName, logMessage );
        delete [] logMessage;

        
        delete [] muteForwardHashSeed;
        muteForwardHashSeed = CryptoUtils::getRandomHexString( 20 );

        newHashSeed = muteComputeNewForwardHash( muteForwardHashSeed );
        }

    // we have found a new hash seed that would be forwarded

    // use this new hash seed as our hash seed
    delete [] muteForwardHashSeed;
    muteForwardHashSeed = newHashSeed;

    logMessage = autoSprintf( "Chose a forward hash seed: %s",
                              muteForwardHashSeed );

    AppLog::info( loggerName, logMessage );
    delete [] logMessage;
    
        
    muteOutboundChannelManager =
        new OutboundChannelManager( muteRandomSource );

    muteLocalAddressReceiver = new LocalAddressReceiver();

    muteHostCatcher = new HostCatcher( settings->getMaxHostList() );
    muteHostCatcher->loadHosts();

    muteMessageIDTracker = new MessageIDTracker( 100 );


    muteOutboundMessagePerSecondLimiter =
        new KbLimiter( settings->getOutboundKBLimitSetting() );

    
    muteInboundMessagePerSecondLimiter =
        new KbLimiter( settings->getInboundKBLimitSetting() );

    mutePort = inPort;
    
    muteConnectionMaintainer = new ConnectionMaintainer(
        mutePort,
        muteNodeUniqueName,
        muteLocalAddressReceiver,
        muteOutboundChannelManager,
        muteHostCatcher,
        muteMessageIDTracker,
        muteOutboundMessagePerSecondLimiter,
        muteInboundMessagePerSecondLimiter );
    
    muteServerThread = new ServerThread( mutePort, muteConnectionMaintainer );


    
    // read in the dictionary words

    muteDictionaryWords = settings->getDictionary();

    }



void muteStop() {

    settings->saveSettings();

    char *loggerName = "messageRouter";
    
    if( muteServerThread != NULL ) {
        AppLog::info( loggerName, "Destroying ServerThread." );
        delete muteServerThread;
        muteServerThread = NULL;
        }
    if( muteConnectionMaintainer != NULL ) {
        AppLog::info( loggerName, "Destroying Connectionmaintainer." );
        delete muteConnectionMaintainer;
        muteConnectionMaintainer = NULL;
        }
    if( muteHostCatcher != NULL ) {
        AppLog::info( loggerName, "Destroying HostCatcher." );
        delete muteHostCatcher;
        muteHostCatcher = NULL;
        }
    if( muteMessageIDTracker != NULL ) {
        AppLog::info( loggerName, "Destroying MessageIDTracker." );
        delete muteMessageIDTracker;
        muteMessageIDTracker = NULL;
        }
    if( muteOutboundMessagePerSecondLimiter != NULL ) {
        AppLog::info( loggerName,
                      "Destroying outbound KbLimiter." );
        delete muteOutboundMessagePerSecondLimiter;
        muteOutboundMessagePerSecondLimiter = NULL;
        }
    if( muteInboundMessagePerSecondLimiter != NULL ) {
        AppLog::info( loggerName,
                      "Destroying inbound KbLimiter." );
        delete muteInboundMessagePerSecondLimiter;
        muteInboundMessagePerSecondLimiter = NULL;
        }
    if( muteMessageIDSigningPublicKey != NULL ) {
        AppLog::info( loggerName, "Destroying MessageIDSigningPublicKey." );
        delete [] muteMessageIDSigningPublicKey;
        muteMessageIDSigningPublicKey = NULL;
        }
    if( muteMessageIDSigningPrivateKey != NULL ) {
        AppLog::info( loggerName, "Destroying MessageIDSigningPrivateKey." );
        delete [] muteMessageIDSigningPrivateKey;
        muteMessageIDSigningPrivateKey = NULL;        
        }
    if( muteLocalAddressReceiver != NULL ) {
        AppLog::info( loggerName, "Destroying LocalAddressReceiver." );
        delete muteLocalAddressReceiver;
        muteLocalAddressReceiver = NULL;
        }
    if( muteOutboundChannelManager != NULL ) {
        AppLog::info( loggerName, "Destroying OutboundChannelManager." );
        delete muteOutboundChannelManager;
        muteOutboundChannelManager = NULL;
        }
    if( muteNodeUniqueName != NULL ) {
        AppLog::info( loggerName, "Destroying node's unique name." );
        delete [] muteNodeUniqueName;
        muteNodeUniqueName = NULL;
        }
    if( muteForwardHashSeed != NULL ) {
        AppLog::info( loggerName, "Destroying node's forward hash seed." );
        delete [] muteForwardHashSeed;
        muteForwardHashSeed = NULL;
        }
    if( muteRandomSource != NULL ) {
        AppLog::info( loggerName, "Destroying RandomSource." );
        delete muteRandomSource;
        muteRandomSource = NULL;
        }

    muteDictionaryWords.clear();
    }



void muteAddHost( char *inAddress, unsigned int inPort ) {
    HostAddress *address = new HostAddress( stringDuplicate( inAddress ),
                                            inPort );
    muteHostCatcher->addHost( address );

    delete address;
    }



void muteSetTargetNumberOfConnections( int inTarget ) {
  if(muteConnectionMaintainer)
    muteConnectionMaintainer->setTargetConnectionCount( inTarget );
    }



int muteGetTargetNumberOfConnections() {
  if(muteConnectionMaintainer)
      return muteConnectionMaintainer->getTargetConnectionCount();
  else return 0;
    }



void muteSetMaxNumberOfConnections( int inMax ) {
  if(muteConnectionMaintainer)
    muteConnectionMaintainer->setMaxConnectionCount( inMax );
    }



int muteGetMaxNumberOfConnections() {
  if(muteConnectionMaintainer)
    return muteConnectionMaintainer->getMaxConnectionCount();
  else return 0;
    }



int muteGetConnectedHostList( char ***outHostAddresses,
                              int **outHostPorts,
                              int **outSentMessageCounts,
                              int **outQueuedMessageCounts,
                              int **outDroppedMessageCounts ) {

    SimpleVector<HostAddress *> *addressList =
        muteOutboundChannelManager->getConnectedHosts(
            outSentMessageCounts, outQueuedMessageCounts,
            outDroppedMessageCounts );

    int count = addressList->size();

    char **addresses = new char*[ count ];
    int *ports = new int[ count ];

    for( int i=0; i<count; i++ ) {
        HostAddress *host = *( addressList->getElement( i ) );

        addresses[i] = stringDuplicate( host->mAddressString );
        ports[i] = host->mPort;

        delete host;
        }

    *outHostAddresses = addresses;
    *outHostPorts = ports;

    delete addressList;
    
    return count;
    }



int muteGetConnectionCount() {
    return muteOutboundChannelManager->getConnectionCount();
    }



char muteGetCurrentConnectionAttempt( char **outHostAddress, int *outPort ) {
    return
        muteConnectionMaintainer->getCurrentConnectionAttempt( outHostAddress,
                                                               outPort );
    }

double muteGetOutboundMessagePerSecondLimit() {
    return muteOutboundMessagePerSecondLimiter->getLimit();
    }

double muteGetInboundMessagePerSecondLimit() {
    return muteInboundMessagePerSecondLimiter->getLimit();
    }



void muteSetMessageIDSigningKeys( char *inPublicKey, char *inPrivateKey ) {
    if( muteMessageIDSigningPublicKey != NULL ) {
        delete [] muteMessageIDSigningPublicKey;
        muteMessageIDSigningPublicKey = NULL;
        }
    if( muteMessageIDSigningPrivateKey != NULL ) {
        delete [] muteMessageIDSigningPrivateKey;
        muteMessageIDSigningPrivateKey = NULL;
        }

    if( inPublicKey != NULL ) {
        muteMessageIDSigningPublicKey = stringDuplicate( inPublicKey );
        }
    if( inPrivateKey != NULL ) {
        muteMessageIDSigningPrivateKey = stringDuplicate( inPrivateKey );
        }
    }



char muteHasCounterWrappedAround() {
    return muteMessageIDTracker->hasCounterRolledOver();
    }



void muteAddReceiveAddress( char *inAddress ) {
    muteLocalAddressReceiver->addReceiveAddress( inAddress );
    }



void muteRemoveReceiveAddress( char *inAddress ) {
    muteLocalAddressReceiver->removeReceiveAddress( inAddress );
    }



void muteSendMessage( char *inFromAddress,
                      char *inToAddress,
                      char *inMessage,
                      char *inFlags ) {

    // generate an ID for this outbound message
    char *messageID = muteGetFreshMessageID();

    //printf( "Registering outbound ID %s\n", messageID );
    // tell the ID tracker about our message so that we will drop
    // it if it gets routed back through us
    muteMessageIDTracker->checkIfIDFresh( messageID );
    
    char *flags;
    if( inFlags != NULL ) {
        flags = inFlags;
        }
    else {
        flags = "NONE";
        }

    
    if( strstr( flags, "FRESH_ROUTE" ) ) {
        // clear the routing information in the outbound direction
        muteOutboundChannelManager->
            clearRoutingInformation( inToAddress );

        // we don't need to clear routes for inFromAddress, since it is
        // our address and we shouldn't have any routing information for it
        }


    char *fullFlags;

    if( muteMessageIDSigningPublicKey != NULL &&
        muteMessageIDSigningPrivateKey != NULL ) {

        // add pubkey and signature to flags

        char *signature = CryptoUtils::rsaSign( muteMessageIDSigningPrivateKey,
                                                (unsigned char *)messageID,
                                                strlen( messageID ) );
        
        fullFlags = autoSprintf( "%s|PUBLIC_KEY_%s|SIGNED_ID_%s",
                                 flags,
                                 muteMessageIDSigningPublicKey,
                                 signature );

        delete [] signature;
        }
    else {
        fullFlags = stringDuplicate( flags );
        }
    
    
    // send the message
    // a fresh message has 0 utility so far, but the channel manager
    // will add chaff to protect our identity
    muteOutboundChannelManager->routeMessage( messageID,
                                              inFromAddress,
                                              inToAddress,
                                              fullFlags,
                                              0,  // utility
                                              inMessage );

    delete [] fullFlags;
    
    delete [] messageID;
    }



int muteAddGlobalMessageHandler(
    int (*inHandlerFunction)( char *, char *, char *, void * ),
    void *inExtraHandlerArgument ) {
    return muteLocalAddressReceiver->addGlobalMessageHandler(
        inHandlerFunction, inExtraHandlerArgument );
    }

int muteAddMessageHandler(
    int (*inHandlerFunction)( char *, char *, char *, void * ),
    void *inExtraHandlerArgument ) {
    return muteLocalAddressReceiver->addMessageHandler(
        inHandlerFunction, inExtraHandlerArgument );
    }



void muteRemoveGlobalMessageHandler( int inHandlerID ) {
    muteLocalAddressReceiver->removeGlobalMessageHandler( inHandlerID );
    }

void muteRemoveMessageHandler( int inHandlerID ) {
    muteLocalAddressReceiver->removeMessageHandler( inHandlerID );
    }



unsigned int muteGetWaitingMessageCount( char *inAddress ) {
    return muteLocalAddressReceiver->getWaitingMessageCount( inAddress );
    }



unsigned int muteGetReceivedMessages( char *inAddress,
                                      unsigned int inNumMessages,
                                      char ***outMessages,
                                      char ***outFromAddresses ) {
    return muteLocalAddressReceiver->getReceivedMessages(
        inAddress, inNumMessages, outMessages, outFromAddresses );
    }



char *muteGetUniqueName() {
    return CryptoUtils::getRandomHexString( 20 );
    }



char *muteGetMnemonic( char *inVirtualAddress ) {
    int numWords = 2;

    int numDictionaryWords = muteDictionaryWords.size();

    unsigned char *raw20Bytes = computeRawSHA1Digest( inVirtualAddress );

    
    char *mnemonic = stringDuplicate( "" );
    
    for( int i=0; i<numWords; i++ ) {

        // take the first four bytes of hash as a long

        unsigned long wordIndex =
            raw20Bytes[0] << 24 |
            raw20Bytes[1] << 16 |
            raw20Bytes[2] << 8 |
            raw20Bytes[3];

        // mod by num words in dictionary to force it in range
        
        wordIndex = wordIndex % numDictionaryWords;

        char *word = stringDuplicate(
            ( muteDictionaryWords.at( wordIndex ).toLocal8Bit().constData() ) );

        // upcase the first letter
        word[0] = toupper( word[0] );

        char *newMnemonic = autoSprintf( "%s%s", mnemonic, word );

        delete [] mnemonic;
        mnemonic = newMnemonic;

        delete [] word;

        // hash again
        unsigned char *nextHash =
            computeRawSHA1Digest( raw20Bytes, 20 );
        delete [] raw20Bytes;
        raw20Bytes = nextHash;
        }

    delete [] raw20Bytes;

    return mnemonic;
    }



char *muteGetFreshMessageID() {
    // first, generate a unique name
    char *uniqueName = muteGetUniqueName();

    unsigned int counter = muteMessageIDTracker->getFreshCounter();

    char *messageID = autoSprintf( "%s_%u", uniqueName, counter );
    delete [] uniqueName;
    
    return messageID;
    }



char *muteAddFlag( char *inFlags, char *inFlagToAdd ) {

    if( strcmp( inFlags, "NONE" ) == 0 ) {
        // new string only contains inFlagToAdd
        return stringDuplicate( inFlagToAdd );
        }
    else {
        // inFlags already contains flags
        
        // add inFlagToAdd to the end
        return autoSprintf( "%s|%s", inFlags, inFlagToAdd );
        }
    }



char *muteRemoveFlag( char *inFlags, char *inFlagToRemove ) {

    if( strstr( inFlags, inFlagToRemove ) == NULL ) {
        // inFlagToRemove does not exist in inFlags

        // no change to inFlags
        return stringDuplicate( inFlags );
        }
    else if( strcmp( inFlags, inFlagToRemove ) == 0 ) {
        // inFlags contains only inFlagToRemove

        // return the NONE flag string
        return stringDuplicate( "NONE" );
        }
    else {
        // inFlags contains multiple flags, including inFlagToRemove
        
        int numOldFlags;
        char **oldFlags = split( inFlags, "|", &numOldFlags );

        SimpleVector<char*> *newFlagVector = new SimpleVector<char*>();

        int i;
        for( i=0; i<numOldFlags; i++ ) {
            if( strcmp( oldFlags[i], inFlagToRemove ) != 0 ) {
                // not inFlagToRemove, add it to vector
                newFlagVector->push_back( oldFlags[i] );
                }
            else {
                // matches inFlagToRemove, delete it
                delete [] oldFlags[i];
                }
            }
        delete [] oldFlags;

        char **newFlags = newFlagVector->getElementArray();

        int numNewFlags = newFlagVector->size();
        delete newFlagVector;

        char *newFlagString = join( newFlags, numNewFlags, "|" );

        for( i=0; i<numNewFlags; i++ ) {
            delete [] newFlags[i];
            }
        delete [] newFlags;

        
        return newFlagString;
        }
    }



char *muteGetForwardHashSeed() {
    return stringDuplicate( muteForwardHashSeed );
    }



char *muteComputeNewForwardHash( char *inOldHash ) {

    // re-hash the hash to produce a new hex-encoded hash
    char *newHash =
        computeSHA1Digest( inOldHash );

    // compute the raw hash too so that we can easily extract the last byte
    unsigned char *newRawHash =
        computeRawSHA1Digest( inOldHash );

    // look at the last byte of the new hash
    // as a random number
    // (hash has length 20)
    unsigned char randomValue = newRawHash[ 19 ];


    delete [] newRawHash;
    
    
    // convert random value to a floating point
    // value in the range 0..1
    double randomFractionValue =
        (double)randomValue / 255.0;

                            
    if( randomFractionValue <=
        muteContinueForwardProbability ) {

        // keep forwarding, replacing the old hash with the new one
        return newHash;
        }
    else {
        // stop forwarding
        delete [] newHash;

        return NULL;
        }    
    }



char muteShouldDropTailChainMessages() {
    if( muteNumNeighborsToSendDropTailsTo == 0 ) {
        return true;
        }
    else {
        return false;
        }
    }



int muteGetNumNeighborsToSendDropTailsTo() {
    return muteNumNeighborsToSendDropTailsTo;
    }



int muteGetDropTailTreeStartingTTL() {
    return muteDropTailTreeStartingTTL;
    }








