# -*- cmake -*-

include(00-Common)
include(libgpg-error)
include(libgcrypt)
include(Linking)

if (WINDOWS)

project(gcrypt)

    include_directories(
	../gpg.vs/inc.vs
	../gpg.vs/libgcrypt-1.2.2.vs/custom
	../libgpg-error/libgpg-error-1.0/src
	libgcrypt-1.2.2/src
	libgcrypt-1.2.2/mpi
	libgcrypt-1.2.2/mpi/generic
	)

set(gcrypt_SOURCE_FILES
    libgcrypt-1.2.2/src/ath.c
    libgcrypt-1.2.2/src/global.c
    libgcrypt-1.2.2/src/misc.c
    libgcrypt-1.2.2/src/missing-string.c
    libgcrypt-1.2.2/src/module.c
    libgcrypt-1.2.2/src/secmem.c
    libgcrypt-1.2.2/src/sexp.c
    libgcrypt-1.2.2/src/stdmem.c

    libgcrypt-1.2.2/cipher/ac.c
    libgcrypt-1.2.2/cipher/arcfour.c
    libgcrypt-1.2.2/cipher/blowfish.c
    libgcrypt-1.2.2/cipher/cast5.c
    libgcrypt-1.2.2/cipher/cipher.c
    libgcrypt-1.2.2/cipher/crc.c
    libgcrypt-1.2.2/cipher/des.c
    libgcrypt-1.2.2/cipher/dsa.c
    libgcrypt-1.2.2/cipher/elgamal.c
    libgcrypt-1.2.2/cipher/md.c
    libgcrypt-1.2.2/cipher/md4.c
    libgcrypt-1.2.2/cipher/md5.c
    libgcrypt-1.2.2/cipher/primegen.c
    libgcrypt-1.2.2/cipher/pubkey.c
    libgcrypt-1.2.2/cipher/random.c
    libgcrypt-1.2.2/cipher/rfc2268.c
    libgcrypt-1.2.2/cipher/rijndael.c
    libgcrypt-1.2.2/cipher/rmd160.c
#    libgcrypt-1.2.2/cipher/rndegd.c
#    libgcrypt-1.2.2/cipher/rndlinux.c
#    libgcrypt-1.2.2/cipher/rndunix.c
    libgcrypt-1.2.2/cipher/rndw32.c
    libgcrypt-1.2.2/cipher/rsa.c
    libgcrypt-1.2.2/cipher/serpent.c
    libgcrypt-1.2.2/cipher/sha1.c
    libgcrypt-1.2.2/cipher/sha256.c
    libgcrypt-1.2.2/cipher/sha512.c
    libgcrypt-1.2.2/cipher/tiger.c
    libgcrypt-1.2.2/cipher/twofish.c

    libgcrypt-1.2.2/mpi/mpi-add.c
    libgcrypt-1.2.2/mpi/mpi-bit.c
    libgcrypt-1.2.2/mpi/mpi-cmp.c
    libgcrypt-1.2.2/mpi/mpi-div.c
    libgcrypt-1.2.2/mpi/mpi-gcd.c
    libgcrypt-1.2.2/mpi/mpi-inline.c
    libgcrypt-1.2.2/mpi/mpi-inv.c
    libgcrypt-1.2.2/mpi/mpi-mpow.c
    libgcrypt-1.2.2/mpi/mpi-mul.c
    libgcrypt-1.2.2/mpi/mpi-pow.c
    libgcrypt-1.2.2/mpi/mpi-scan.c
    libgcrypt-1.2.2/mpi/mpicoder.c
    libgcrypt-1.2.2/mpi/mpih-div.c
    libgcrypt-1.2.2/mpi/mpih-mul.c
    libgcrypt-1.2.2/mpi/mpiutil.c

    libgcrypt-1.2.2/mpi/generic/mpih-add1.c
    libgcrypt-1.2.2/mpi/generic/mpih-lshift.c
    libgcrypt-1.2.2/mpi/generic/mpih-mul1.c
    libgcrypt-1.2.2/mpi/generic/mpih-mul2.c
    libgcrypt-1.2.2/mpi/generic/mpih-mul3.c
    libgcrypt-1.2.2/mpi/generic/mpih-rshift.c
    libgcrypt-1.2.2/mpi/generic/mpih-sub1.c
    libgcrypt-1.2.2/mpi/generic/udiv-w-sdiv.c
    )

set(gcrypt_HEADER_FILES
    CMakeLists.txt

    libgcrypt-1.2.2/src/ath.h
    libgcrypt-1.2.2/src/cipher.h
    libgcrypt-1.2.2/src/g10lib.h
    libgcrypt-1.2.2/src/gcrypt-module.h
    libgcrypt-1.2.2/src/gcrypt.h
    libgcrypt-1.2.2/src/mpi.h
    libgcrypt-1.2.2/src/secmem.h
    libgcrypt-1.2.2/src/stdmem.h
    libgcrypt-1.2.2/src/types.h

    libgcrypt-1.2.2/cipher/bithelp.h
    libgcrypt-1.2.2/cipher/random.h
    libgcrypt-1.2.2/cipher/rand-internal.h
    libgcrypt-1.2.2/cipher/rmd.h

    libgcrypt-1.2.2/mpi/longlong.h
    libgcrypt-1.2.2/mpi/mpi-inline.h
    libgcrypt-1.2.2/mpi/mpi-internal.h

    libgcrypt-1.2.2/mpi/generic/mpi-asm-defs.h
    )

set_source_files_properties(${gcrypt_HEADER_FILES}
                            PROPERTIES HEADER_FILE_ONLY TRUE)

list(APPEND gcrypt_SOURCE_FILES ${gcrypt_HEADER_FILES})

add_library (gcrypt ${gcrypt_SOURCE_FILES})
target_link_libraries(
    gcrypt
    )

else (WINDOWS)

include(ExternalProject)

# Gotta fake a DOWNLOAD_COMMAND to work around a bug in cmake before 2.8.3.
externalproject_add(gcrypt
    DEPENDS gpg-error
    DOWNLOAD_COMMAND ""
    PREFIX ${CMAKE_BINARY_DIR}/libgcrypt
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/libgcrypt/libgcrypt-1.2.2
    CONFIGURE_COMMAND <SOURCE_DIR>/configure -prefix=<INSTALL_DIR> --with-gpg-error-prefix=${CMAKE_BINARY_DIR}/libgpg-error
    INSTALL_COMMAND make install
    )

endif (WINDOWS)


