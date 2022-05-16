#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <wait.h>
#include <dirent.h>
#include "cutil.h"
char *rtrim(char *str)
{
    if (str == NULL || *str == '\0')
    {
        return str;
    }

    int len = strlen(str);
    char *p = str + len - 1;
    while (p >= str && isspace(*p))
    {
        *p = '\0';
        --p;
    }

    return str;
}

char *ltrim(char *str)
{
    if (str == NULL || *str == '\0')
    {
        return str;
    }

    int len = 0;
    char *p = str;
    while (*p != '\0' && isspace(*p))
    {
        ++p;
        ++len;
    }

    memmove(str, p, strlen(str) - len + 1);

    return str;
}
char *trim(char *str)
{
    str = rtrim(str);
    str = ltrim(str);
    return str;
}
void rmAll(const char *path)
{
    char all[1000] = {0};
    //注意这一串不是给rm,是给终端调用rm,有空格必须用""不然终端会识别成2个词；通配符不能在""内
    sprintf(all, "rm \"%s\"/*", path);
    FTDEBUG("face-delete.log", "rm", "%s", all);
    system(all);
}
void rm(const char *path)
{
    char all[1000] = {0};
    sprintf(all, "rm \"%s\"", path);
    FTDEBUG("face-delete.log", "rm", "%s", all);
    system(all);
}