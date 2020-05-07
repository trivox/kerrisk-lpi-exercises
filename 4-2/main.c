/*
 * Michael Kerrisk: The Linux Programming Interface. Exercise 4-2
 *
 * Write a program like cp that, when used to copy a regular file that contains holes
 * (sequences of null bytes), also creates corresponding holes in the target file.
 *
 * Solution author: Alexander Tikhomirov <trivox@gmail.com> 
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>

#define BUFF_SZ 1024

static inline void usage( const char * fname )
{
    fprintf( stderr, "Usage: %s <file> <target>\n", fname );
}

static int handle_subblock( bool is_zeros, const char * start, const char * end,
        int count, int fd )
{
    int size = end - start;
    if( is_zeros ) {
        if( lseek( fd, size + count, SEEK_CUR ) < 0 ) {
            perror( "lseek" );
            return -1;
        }
    } else {
        if( write( fd, start, size ) < 0 ) {
            perror( "write" );
            return -2;
        }
    }
    return 0;
}

static int handle_block( int fd, char * buf, int size, int count )
{
    bool is_zeros = (count != 0); // if count non-zero we are inside zero-block
    char * start = buf   ;
    char * end  ; 
    int rc;
    for( end = start; end - buf < size; end++ ) {
        if( ( is_zeros && *end == 0 ) || ( !is_zeros && *end != 0 ) ) continue; 
        else {
            if( (rc = handle_subblock( is_zeros, start, end, count, fd )) != 0 ) { 
                fprintf( stderr, "Error (%d) handling block of file\n", rc );
                return rc;
            }
            start = end;
            count = 0;
            is_zeros = !(is_zeros); // revert state flag
        }
    }
    if( !is_zeros ) {
        // finilizing write of non-zero block
        if( write( fd, start, end-start ) < 0 ) {
            perror( "write (final)" );
            return -3;
        }
        return 0;
    }
    else return count + (end-start);
}

#define GOTO_RET(A) { ret = A; goto end; }

int main( int argc, char ** argv )
{
    int ret = 0;

    if( argc != 3 ) {
        usage( argv[0] );
        return -1;
    }

    const char * fi = argv[1] ;
    const char * fo = argv[2] ;

    int fdi = open( fi, O_RDONLY, 0);
    if( fdi < 0 ) {
        perror( "open in" );
        return -2;
    }

    int fdo = open( fo, O_WRONLY | O_CREAT | O_TRUNC,
                    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH );
    if( fdo < 0 ) {
        perror( "open out" );
        GOTO_RET(-3);
    }
    
    char buf[BUFF_SZ];
    int cur_zeros = 0;
    int read_rc = 0;

    while( (read_rc = read( fdi, buf, BUFF_SZ)) > 0 ) {
        cur_zeros = handle_block( fdo, buf, read_rc, cur_zeros );
        if( cur_zeros < 0 ) {
            fprintf( stderr, "handle_block returns %d\n", cur_zeros );
            GOTO_RET(-4);
        } 
    }

    if( read_rc < 0 ) {
        perror( "read" );
        GOTO_RET(-5);
    }

    // finish writing by putting intentional zero in the end (to reserve FS size)
    if( cur_zeros > 0 ) {
        char zero = 0;
        if( cur_zeros > 1 ) {
            if( lseek( fdo, cur_zeros-1, SEEK_CUR ) < 0 ) {
                perror( "lseek" );
                GOTO_RET(-6);
            }
        }
        if( write( fdo, &zero, 1 ) < 0 ) {
            perror( "write" );
            GOTO_RET(-7);
        }
    }

end:
    if( fdi > 0 ) close( fdi );
    if( fdo > 0 ) close( fdo );

    return ret;
}
