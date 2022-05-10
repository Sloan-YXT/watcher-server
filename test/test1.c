#include <stdio.h>
#include <unistd.h>
int main(void)
{
     //execlp("ln","ln","-s","/usr/local/share/opencv4/haarcascades/haarcascade_frontalface_alt2.xml"," data.conf",NULL);
     symlink("/usr/local/share/opencv4/haarcascades/haarcascade_frontalface_alt2.xml","data.conf");
     //perror("exec failed");
}