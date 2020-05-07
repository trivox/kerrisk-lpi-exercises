/*
 * Michael Kerrisk: The Linux Programming Interface. Exercise 4-1
 *
 * The tee command reads its standard input until end-of-file, writing a copy of the
 * input to standard output and to the file named in its command-line argument. Implement
 * tee using I/O system calls. By default, tee overwrites any existing file with the
 * given name. Implement the -a command-line option (tee -a file), which causes tee to
 * append text to the end of a file if it already exist.
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
    fprintf( stderr, "Usage: %s [-a] <filename>\n", fname );
}

static void handle_write_error( int rc, const char * fname )
{
    if( rc < 0 ) {
        perror( fname );
    } else {
        fprintf(stderr, "Could not write more then %d bytes to %s\n", rc, fname );
    }
}

int main( int argc, char ** argv )
{
    if( argc != 2 && argc != 3 ) {
        usage( argv[0] );
        return -1;
    }

    int trunc = O_TRUNC; // truncate by default
    int opt;

    while( (opt = getopt(argc, argv, "a")) != -1 ) {
        switch( opt ) {
            case 'a': trunc = O_APPEND ; break     ; // no truncation
            default : usage( argv[0] ) ; return -2 ;
        }
    } 

    if( optind >= argc ) {
        usage( argv[0] );
        return -3;
    }

    const char * fname = argv[optind] ;

    int fd = open( fname, O_WRONLY | O_CREAT | trunc,
                   S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH );
    if( fd < 0 ) {
        perror( "open" );
        return -4;
    }
    
    char buf[BUFF_SZ];
    int read_rc = 0;
    int write_rc = 0;
    while( (read_rc = read(STDIN_FILENO, buf, BUFF_SZ)) > 0 ) {
        if( (write_rc = write(STDOUT_FILENO, buf, read_rc)) != read_rc ) {
            handle_write_error( write_rc, "stdout" );
            close( fd );
            return -5;
        }
        if( (write_rc = write(fd, buf, read_rc)) != read_rc ) {
            handle_write_error( write_rc, fname );
            close( fd );
            return -6;
        }
    }

    if( read_rc < 0 ) {
        perror( "read" );
        close( fd );
        return -7;
    }

    if( close(fd) < 0 ) {
        perror( "close" );
        return -8;
    }

    return 0;
}
