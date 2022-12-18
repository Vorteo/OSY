#include "functions.h"

void print_ls(struct stat statbuf, char* filename)
{
    struct tm dm;

    printf("%ld \t", statbuf.st_ino);
    printf("%07o \t",statbuf.st_mode);
    printf("%ld \t",statbuf.st_nlink);
    printf("%d \t",statbuf.st_uid);
    printf("%d \t",statbuf.st_gid);
    printf("%ld \t",statbuf.st_size);

    dm = *(gmtime(&statbuf.st_mtime));
	printf("%d-%d-%d %d:%d:%d\t",dm.tm_mday,dm.tm_mon + 1,dm.tm_year + 1900,dm.tm_hour,dm.tm_min,dm.tm_sec);
    printf("%s \t", filename);

}