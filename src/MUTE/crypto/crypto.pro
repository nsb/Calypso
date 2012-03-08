TEMPLATE=lib
TARGET=crypto
CONFIG += static
CONFIG += release
QT -= gui
QT -= core
DEFINES += NDEBUG
SOURCES =  algebra.cpp   emsa2.cpp     modes.cpp     randpool.cpp        \
	   algparam.cpp  eprecomp.cpp  mqueue.cpp    rdtables.cpp        \
	   asn.cpp       files.cpp     nbtheory.cpp  rijndael.cpp        \
	   basecode.cpp  filters.cpp   oaep.cpp      rng.cpp             \
	   cryptlib.cpp  fips140.cpp   pch.cpp       rsa.cpp             \
	   default.cpp   hex.cpp       pkcspad.cpp   sha.cpp             \
	   des.cpp       integer.cpp   pssr.cpp      simple.cpp          \
	   dessp.cpp     iterhash.cpp  pubkey.cpp    strciphr.cpp        \
	   dll.cpp       misc.cpp      queue.cpp     trdlocal.cpp \
	   gfpcrypt.cpp hmac.cpp dsa.cpp gf2n.cpp ecp.cpp ec2n.cpp osrng.cpp cpu.cpp hrtimer.cpp
