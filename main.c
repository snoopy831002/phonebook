#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include IMPL

#define DICT_FILE "./dictionary/words.txt"
#define TABLE_SIZE 1024

static double diff_in_second(struct timespec t1, struct timespec t2)
{
    struct timespec diff;
    if (t2.tv_nsec-t1.tv_nsec < 0) {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
    } else {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
    }
    return (diff.tv_sec + diff.tv_nsec / 1000000000.0);
}

int main(int argc, char *argv[])
{
    FILE *fp;
    int i = 0;
    char line[MAX_LAST_NAME_SIZE];
    struct timespec start, end;
    double cpu_time1, cpu_time2;

    /* check file opening */
    fp = fopen(DICT_FILE, "r");
    if (fp == NULL) {
        printf("cannot open the file\n");
        return -1;
    }

    /* build the entry */
#if defined(HASH)
    entry pHead[TABLE_SIZE], *e[TABLE_SIZE];
    printf("size of entry : %lu bytes\n", sizeof(entry));
    for(i = 0; i<TABLE_SIZE; i++) {
        e[i] = &pHead[i];
        e[i]->pNext = NULL;
    }
    i = 0;
    /* build the entry*/
#if defined(__GNUC__)
    __builtin___clear_cache((char *) pHead, (char *) pHead +TABLE_SIZE* sizeof(entry));
#endif
    clock_gettime(CLOCK_REALTIME, &start);
    while (fgets(line, sizeof(line), fp)) {
        while (line[i] != '\0')
            i++;
        line[i - 1] = '\0';
        i = 0;
        unsigned int hash=djb2Hash(line)%TABLE_SIZE;
        e[hash]=append(line,e[hash]);
    }
    clock_gettime(CLOCK_REALTIME, &end);
    //calcutate append() cpu time
    cpu_time1 = diff_in_second(start, end);
    /*close file as soon as possible*/
    fclose(fp);
    for(i=0; i<TABLE_SIZE; i++) {
        e[i]=&pHead[i];
    }


    /* the givn last name to find */
    char input[MAX_LAST_NAME_SIZE]="zyxel";
    unsigned int hash = djb2Hash(input)%TABLE_SIZE;
    e[hash]=&pHead[hash];
    assert(findName(input,e[hash])&&
           "Did you inplement findName in " IMPL "?");
    assert(0 == strcmp(findName(input, e[hash])->lastName, "zyxel"));
#if defined(__GNUC__)
    __builtin___clear_cache((char *) pHead, (char *) pHead + TABLE_SIZE*sizeof(entry));
#endif
    /* compute the execution time */
    clock_gettime(CLOCK_REALTIME, &start);
    findName(input, e[hash]);
    clock_gettime(CLOCK_REALTIME, &end);
    //calculate append() cpu time
    cpu_time2=diff_in_second(start,end);
    FILE *output;
    output=fopen("hash.txt","a");
#if defined(OPT)
    output=fopen("opt.txt","a");
#else
    output=fopen("orig.txt","a");
#endif
    fprintf(output, "append() findName() %lf %lf\n", cpu_time1, cpu_time2);
    fclose(output);
    printf("execution time of append() : %lf sec\n", cpu_time1);
    printf("execution time of findName() : %lf sec\n", cpu_time2);
    for(i=0; i<TABLE_SIZE; i++) {
        if(pHead[i].pNext) free(pHead[i].pNext);
    }



#else
    /* build the entry */
    entry *pHead, *e;
    pHead = (entry *) malloc(sizeof(entry));
    printf("size of entry : %lu bytes\n", sizeof(entry));
    e = pHead;
    e->pNext = NULL;
#if defined(__GNUC__)
    __builtin___clear_cache((char *) pHead, (char *) pHead + sizeof(entry));
#endif
    clock_gettime(CLOCK_REALTIME, &start);
    while (fgets(line, sizeof(line), fp)) {
        while (line[i] != '\0')
            i++;
        line[i - 1] = '\0';
        i = 0;
        e = append(line, e);
    }
    clock_gettime(CLOCK_REALTIME, &end);
    cpu_time1 = diff_in_second(start, end);
    /* close file as soon as possible */
    fclose(fp);
    e = pHead;
    /* the givn last name to find */
    char input[MAX_LAST_NAME_SIZE] = "zyxel";
    e = pHead;
    assert(findName(input, e) &&
           "Did you implement findName() in " IMPL "?");
    assert(0 == strcmp(findName(input, e)->lastName, "zyxel"));
#if defined(__GNUC__)
    __builtin___clear_cache((char *) pHead, (char *) pHead + sizeof(entry));
#endif
    /* compute the execution time */
    clock_gettime(CLOCK_REALTIME, &start);
    findName(input, e);
    clock_gettime(CLOCK_REALTIME, &end);
    cpu_time2 = diff_in_second(start, end);
    FILE *output;
#if defined(OPT)
    output = fopen("opt.txt", "a");
#else
    output = fopen("orig.txt", "a");
#endif
    fprintf(output, "append() findName() %lf %lf\n", cpu_time1, cpu_time2);
    fclose(output);
    printf("execution time of append() : %lf sec\n", cpu_time1);
    printf("execution time of findName() : %lf sec\n", cpu_time2);
    if (pHead->pNext) free(pHead->pNext);
    free(pHead);
#endif
    return 0;
}
