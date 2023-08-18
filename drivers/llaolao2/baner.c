#include <fcntl.h>
#include <asm/types.h>
#include <linux/watchdog.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

int usage()
{
    printf("baner <devno> r/w/s/i <value>\n");
    return -1;
}
int
main(int argc, char * argv[])
{
    int n, fd;
    char name[20], buffer[100];
    if(argc < 3)
        return usage();

    sprintf(name, "/dev/huadeng%s", argv[1]);

    fd = open(name, O_RDWR);
    if(fd < 0) 
    {
        printf("open file %s failed with %d\n", name, errno);
        return -2;
    }
    if(argv[2][0] == 'r')
    {
        if((n = read(fd, buffer, sizeof(buffer)))>0)
        {
            printf("read %d bytes %s\n", n, buffer);
        }
        else
            printf("read file %s failed %d\n", name, errno);
    }
    else if(argv[2][0] == 'w')
    {
        if(argc < 4)
        {
            printf("missing what to write\n");
            goto tag_close;
        }

        if((n = write(fd, argv[3], strlen(argv[3])))>0)
        {
            printf("write %d bytes %s\n", n, argv[3]);
        }
        else
            printf("write file %s failed %d\n", name, errno);
    }

tag_close:
    close(fd);
    return 0;
}   

/*
    // Set the watchdog timeout to 20 seconds 
    //new_timeout = 20;
    //ioctl(fd, WDIOC_SETTIMEOUT, &new_timeout);
    while (1) {
    /* Graphics processing 
    // process_graphics();
    /* Pet the watchdog 
    ioctl(fd, WDIOC_KEEPALIVE, 0);
    /* Or instead do: write(wfd, "\0", 1);
    fsync(fd);
    }
*/

