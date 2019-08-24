
#include <errno.h>
#include <fcntl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <sys/epoll.h>
#include <assert.h>

#define DISPLAY_STRING

static const int chunk_size = 32;
static const int flash_size = 1<<17;
static const int chunk_count = flash_size/chunk_size;

int set_interface_attribs(int fd, int speed) {
    struct termios tty;
    
    if (tcgetattr(fd, &tty) < 0) {
        printf("Error from tcgetattr: %s\n", strerror(errno));
        return -1;
    }
    
    
    cfsetospeed(&tty, (speed_t)speed);
    cfsetispeed(&tty, (speed_t)speed);
    
    tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;         /* 8-bit characters */
    tty.c_cflag &= ~PARENB;     /* no parity bit */
    tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    tty.c_cflag &= ~CRTSCTS;   
    
    /* setup for non-canonical mode */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;
    
    /* fetch bytes as they become available */
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 1;
    
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        printf("Error from tcsetattr: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

void set_mincount(int fd, int mcount)
{
    struct termios tty;
    
    if (tcgetattr(fd, &tty) < 0) {
        printf("Error tcgetattr: %s\n", strerror(errno));
        return;
    }
    
    tty.c_cc[VMIN] = mcount ? 1 : 0;
    tty.c_cc[VTIME] = 5;        /* half second timer */
    
    if (tcsetattr(fd, TCSANOW, &tty) < 0)
        printf("Error tcsetattr: %s\n", strerror(errno));
}




char read_buf[1024] = {};
int read_buf_len = 0;
int read_buf_line_len = 0;
int epfd;
int bytes_written = 0;
static const int bytes_to_write = 1<<17;
int serial_fd = -1;

int progress_count = 0;

FILE* source_file;

enum struct State {
    waiting_for_start,
    sending,
    waiting_for_crc,
} state = State::waiting_for_start; 


#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>

void send_chunk() {
    
    if (bytes_written>=bytes_to_write) {
        state = State::waiting_for_crc;
        printf("Waiting for CRC\n");
        
        {
            epoll_event epe = { .events=EPOLLIN };
            auto r1 = epoll_ctl(epfd, EPOLL_CTL_MOD, serial_fd, &epe);
            assert(!r1);
        }
        return;
    }
    
    char out_buf[32];
    auto read_len = fread(out_buf, 1, sizeof out_buf, source_file);
    if (read_len < sizeof out_buf  && read_len + bytes_written < bytes_to_write ) {
        assert(0);
    }
    
    auto write_len = write(serial_fd, out_buf, read_len);
    assert(write_len != -1);
    tcdrain(serial_fd);
    
    assert(write_len == 32);
    
    //printf("did write %ld\n", write_len);
    
    bytes_written += write_len;
    
    auto r2 = fseek(source_file, read_len-write_len, SEEK_CUR);
    assert(!r2);
}


void handle_line() {
    
    
#if 0
    printf("DEBUG GOT LINE: '");
    fwrite(read_buf, read_buf_line_len, 1, stdout);
    printf("'\n");
#endif
    
    
    
    switch (state) { default: assert(0);
        case State::waiting_for_start: {
            
            printf("DEBUG GOT LINE1: '");
            fwrite(read_buf, read_buf_line_len, 1, stdout);
            printf("'\n");
            
            auto got_start = strstr(read_buf, "START\n");
            if (got_start) {
                state = State::sending;
                printf("Sending\n");
                send_chunk();
#if 0
                {
                    epoll_event epe = { .events=EPOLLIN|EPOLLOUT };
                    auto r1 = epoll_ctl(epfd, EPOLL_CTL_MOD, serial_fd, &epe);
                    assert(!r1);
                }
#endif
            }
            
            
        } break;
        case State::sending: {
            
            auto got_tick = strncmp("PROGRESS_TICK\n", read_buf, read_buf_line_len) == 0;
            if (got_tick) {
                progress_count++;
                printf("got_tick %.2f\r", (float)progress_count/(float)chunk_count);
                send_chunk();
                
            } else {
                
                printf("DEBUG GOT LINE2: '");
                fwrite(read_buf, read_buf_line_len, 1, stdout);
                printf("'\n");
                
            }
        } break;
        case State::waiting_for_crc: {
            printf("DEBUG GOT LINE3: '");
            fwrite(read_buf, read_buf_line_len, 1, stdout);
            printf("'\n");
            if (strncmp("CHECKSUM\n", read_buf, 8) == 0) {
                exit(0);
            }
        } break;
    }
}




int main(int, char**argv) {
    
    argv++;
    source_file = fopen(*argv++, "r");
    assert(source_file);
    
    
    
    char *portname = "/dev/ttyUSB0";
    int wlen;
    
    printf("Opening  %s\n", portname);
    serial_fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
    printf("Opened\n");
    assert(serial_fd!=-1);
    set_interface_attribs(serial_fd, B2000000);
    
    tcflush(serial_fd, TCIFLUSH);
    
    //set_mincount(fd, 0);                /* set to pure timed read */
    
    /* simple output */
#if 0
    wlen = write(fd, "Hello!\n", 7);
    if (wlen != 7) {
        printf("Error from write: %d, %d\n", wlen, errno);
    }
#endif
    
    
    tcflush(serial_fd, TCIFLUSH); 
    
    int iFlags;
    // turn on DTR
    iFlags = TIOCM_DTR;
    ioctl(serial_fd, TIOCMBIS, &iFlags);
    
    // turn off DTR
    iFlags = TIOCM_DTR;
    ioctl(serial_fd, TIOCMBIC, &iFlags);
    
    
    tcdrain(serial_fd);    /* delay for output */
    
    
    
    
    epfd = epoll_create1(EPOLL_CLOEXEC);
    
    {
        epoll_event epe = { .events=EPOLLIN}; // |EPOLLOUT
        auto r1 = epoll_ctl(epfd, EPOLL_CTL_ADD, serial_fd, &epe);
        assert(!r1);
    }
    
    
    
    
    for (;;) {
        
        epoll_event epe;
        auto epr = epoll_wait(epfd, &epe, 1, 1000);
        assert(epr>-1);
        
        if (epr) {
            
            if (epe.events & EPOLLIN) {
                auto read_len = read(serial_fd, read_buf+read_buf_len,
                                     sizeof read_buf-read_buf_len-1);
                
                //printf("debug did read %ld\n", read_len);
                if (read_len<0) {
                    assert(0);
                } else if (read_len==0) {
                    assert(0);
                } else {
                    read_buf_len += read_len;
                    
                    while(auto nl = (char*)memchr(read_buf, '\n', read_buf_len)) {
                        read_buf_line_len = nl - read_buf;
                        
                        handle_line();
                        read_buf_len -= read_buf_line_len+1;
                        memmove(read_buf, nl+1, read_buf_len);
                    }
                }
            }
            
            //if (epe.events & EPOLLOUT) { handle_out(); }
        } else {
            //printf("timeout??\n");
        }
        if (epe.events & EPOLLERR) {
            assert(0);
        }
        if (epe.events & EPOLLHUP) {
            assert(0);
        }
        
        
        
    }
    
    
}

