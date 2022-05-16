#ifndef CUTIL
#define CUTIL
char *trim(char *);
char *rtrim(char *);
char *ltrin(char *);
void rmAll(const char *path);
void rm(const char *path);
#define FDEBUG(F, X, ...)                   \
    do                                      \
    {                                       \
        \  
    FILE *p = fopen(F, "a");                \
        fprintf(p, "in line %d", __LINE__); \
        fprintf(p, X, ##__VA_ARGS__);       \
        fprintf(p, "\n");                   \
        fclose(p);                          \
    } while (0)
#define FTDEBUG(F, T, X, ...)                              \
    do                                                     \
    {                                                      \
        \  
    FILE *p = fopen(F, "a");                               \
        fprintf(p, "\n\n<%s>\n\nin line %d", T, __LINE__); \
                                                           \
        fprintf(p, X, ##__VA_ARGS__);                      \
        fprintf(p, "\n\n<\\%s>\n\n",\ 
        T);                                                \
        fclose(p);                                         \
    } while (0)
#ifdef DEBUG
#define DEBUG(X)                              \
    do                                        \
    {                                         \
                                              \
        printf("debug:%d,%s\n", __LINE__, X); \
    } while (0)
#else
#define DEBUG(X)
#define FDEBUG(F, X, ...)
#define FTDEBUG(F, T, X, ...)
#endif
#endif