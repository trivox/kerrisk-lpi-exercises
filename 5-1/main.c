/*
 * Michael Kerrisk: The Linux Programming Interface. Exercise 5-1
 *
 * Modify the programm in Listing 5-3 (accessing the large file) to use standard file I/O
 * system calls open() and lseek() and the off_t data type. Compile the programm with the
 * _FILE_OFFSET_BITS macro set to 64, and test it to show that a large file can be
 * successfully created.
 *
 * Solution author: Alexander Tikhomirov <trivox@gmail.com> 
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define BUFF_SZ 1024

static inline void usage( const char * fname )
{
    fprintf( stderr, "Usage: %s <file> <target>\n", fname );
}

#define GOTO_END(A) { ret = A; goto end; } 

int main( int argc, char ** argv )
{
    int ret = 0;
    off_t off;

    if( argc != 3 ) {
        usage( argv[0] );
        return -1;
    }

    long long tmp = atoll( argv[2] );
    if( tmp < 0 ) {
        fprintf( stderr, "Negative offset %lld not allowed\n", tmp );
        return -2;
    }

    // possible narrowing (imp. dependent)
    off = (off_t) tmp;
    if( (long long) off != tmp ) {
        fprintf( stderr, "Offset is too large (%lld) for this platform\n", tmp);
        return -3;
    }

    int fd = open( argv[1], O_RDWR | O_CREAT,
                   S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH );
    if( fd < 0 ) {
        perror( "open" );
        return -4;
    }

    if( lseek( fd, off, SEEK_SET ) < 0 ) {
        perror( "lseek" );
        GOTO_END( -5 );
    }

    if( write( fd, "test", 4 ) < 0 ) {
        perror( "write" );
        GOTO_END( -6 );
    }

end:
    if( fd >= 0 ) close( fd );
    return ret;
}
