#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/types.h>
#define __USE_POSIX
//#define __USE_BSD
#include <signal.h>
#include <setjmp.h>
#include <unordered_map>
#include <stack>
#include <string>
#include <atomic>
#include <nlohmann/json.hpp>
#include <iostream>
#include <iomanip>
#include <list>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <map>
#include <sys/select.h>
#include <memory>
#include <fstream>
#include <sys/sendfile.h>
#include <dirent.h>
#include "database.h"
#include "faceDetect.h"
#include "cutil.h"
#define portAgraph 6667
#define portAdata 6666
#define portBdata 7777
#define portBwarn 7778
#define portTick 6668
#define portBother 7779
#define MESSAE_LENGTH 1000
#define REQUEST_LENGTH 1000
#define ANUM 100
#define BNUM 3000
#define BCBEFORE "not verified yet"
#define BBBEFORE "not connect yet"
#define STORE_HOUR 3
#define GLEN_32 153666

#define ERROR_ACTION(X)                                   \
    if ((X) == -1)                                        \
    {                                                     \
        printf("error %s:%d", strerror(errno), __LINE__); \
        exit_database();                                  \
        exit(1);                                          \
    }
#define PHOTOS "photos"
#define FACES "faces"
#define VOICES "voices"
#define DATA_CONF "data.conf"
#define FACE_CONF "data.xml"
#define FACE_DEFAULT "/usr/local/share/opencv4/haarcascades/haarcascade_frontalface_alt2.xml"
#define DATA_DEFAULT "data.conf"
using namespace std;
using namespace nlohmann;
string user_dir;
int gepfd[4]; // for clean up
static int signum;
time_t clock_after;
using namespace std;
// for epoll_wait
struct epoll_event a_data_event[ANUM];
struct epoll_event a_graph_event[ANUM];
struct epoll_event a_video_event[ANUM];
struct epoll_event b_connect_event[BNUM];
struct epoll_event b_data_event[BNUM];
pthread_t Aticks[ANUM];
template <class T>
class WrapStack
{
private:
    pthread_mutex_t dlock;
    stack<T> vdata;

public:
    WrapStack()
    {
        DEBUG("wrapStack inited!");
        pthread_mutex_init(&dlock, NULL);
    }
    ~WrapStack()
    {
        pthread_mutex_destroy(&dlock);
    }
    auto push(int dat)
    {
        return vdata.push(dat);
    }
    auto pop(void)
    {
        return vdata.pop();
    }
    auto top(void)
    {
        return vdata.top();
    }
    void lock(void)
    {
        cout << "stackV lock" << endl;
        pthread_mutex_lock(&dlock);
    }
    void unlock(void)
    {
        cout << "stackV unlock" << endl;
        pthread_mutex_unlock(&dlock);
    }
    auto empty(void)
    {
        return vdata.empty();
    }
};
WrapStack<int> freeV;

struct Num
{
private:
    static int curA;
    static int curB;
    pthread_mutex_t lockA;
    pthread_mutex_t lockB;

public:
    void increaseA()
    {
        pthread_mutex_lock(&lockA);
        curA++;
        pthread_mutex_unlock(&lockA);
    }
    void decreaseA()
    {
        pthread_mutex_lock(&lockA);
        curA--;
        pthread_mutex_unlock(&lockA);
    }
    void increaseB()
    {
        pthread_mutex_lock(&lockB);
        curB++;
        pthread_mutex_unlock(&lockB);
    }
    void decreaseB()
    {
        pthread_mutex_lock(&lockB);
        curB--;
        pthread_mutex_unlock(&lockB);
    }
    Num()
    {
        curA = 0;
        curB = 0;
        pthread_mutex_init(&lockA, NULL);
        pthread_mutex_init(&lockB, NULL);
    }
    ~Num()
    {
        pthread_mutex_destroy(&lockA);
        pthread_mutex_destroy(&lockB);
    }
};
int Num::curA;
int Num::curB;
pthread_mutex_t BTestlock;
class BNodeInfo
{
public:
    static int id;
    int fd_data; // push,modified to pull
    int fd_other;
    int fd_warn;
    struct sockaddr_in clientData;
    string client_name;
    string board_name;
    string faces;
    BNodeInfo *pair;

    BNodeInfo()
    {
        pthread_mutex_lock(&BTestlock);
        id++;
        cout << "---B(id)---" + to_string(id) << endl;
        pthread_mutex_unlock(&BTestlock);
    }
    ~BNodeInfo()
    {
        pthread_mutex_lock(&BTestlock);
        id--;
        cout << "---~B(id)---()" + to_string(id) + " " + board_name << endl;
        pthread_mutex_unlock(&BTestlock);
    }
};
int BNodeInfo::id;
template <class V>
class WrapList
{
private:
    pthread_mutex_t dlock;
    list<V> data;

public:
    WrapList()
    {
        pthread_mutex_init(&dlock, NULL);
    }
    ~WrapList()
    {
        pthread_mutex_destroy(&dlock);
    }
    auto push_back(V dat)
    {
        return data.push_back(dat);
    }
    auto back()
    {
        return data.back();
    }
    auto begin()
    {
        return data.begin();
    }
    auto end()
    {
        return data.end();
    }
    auto pop_back()
    {
        return data.push_back();
    }
    auto remove(V dat)
    {
        return data.remove(dat);
    }
    auto erase(decltype(data.begin()) dat)
    {
        return data.erase(dat);
    }
    void lock(void)
    {
        cout << "nodesA locked" << endl;
        pthread_mutex_lock(&dlock);
    }
    void unlock(void)
    {
        cout << "nodesA unlocked" << endl;
        pthread_mutex_unlock(&dlock);
    }
};
class ANodeInfo
{
public:
    int fd_data;
    int fd_graph;
    int fd_tick;
    int vcode;
    struct sockaddr_in client_data, client_graph, client_tick;
    string message;
    string name;
    string position;
    string temp;
    string humi;
    string light;
    string smoke;
    string work_dir;
    string type;
    unsigned int wood_time = 0;
    // ANodeInfo *data_node;
    ANodeInfo *pair_node;
    WrapList<BNodeInfo *> connection;
    string face_conf, data_conf;
    string faces, photos, voices;
    double high_temp, high_humi, wrong_light, wrong_smoke;
    // unordered_map<string, pthread_mutex_t> face_locks;
    unique_ptr<pthread_t> threadVal = unique_ptr<pthread_t>(new pthread_t());
    // pthread_rwlock_t rw_lock;

    ANodeInfo()
    {
        // pthread_rwlock_init(&rw_lock, NULL);
    }
    ~ANodeInfo()
    {
        // pthread_rwlock_destroy(&rw_lock);
    }
    void writeVal()
    {
        // pthread_rwlock_wrlock(&rw_lock);
    }
    void readVal()
    {
        // pthread_rwlock_rdlock(&rw_lock);
    }
    void freeLock()
    {
        // pthread_rwlock_unlock(&rw_lock);
    }
};
bool operator==(const ANodeInfo &a, const ANodeInfo &b)
{
    return a.name == b.name;
}
template <class KEY, class VALUE>
class WrapMap
{
private:
    pthread_mutex_t dlock;
    unordered_map<KEY, VALUE> nodes;

public:
    WrapMap()
    {
        DEBUG("WrapMap inited!");
        pthread_mutex_init(&dlock, NULL);
    }
    ~WrapMap()
    {
        // cout << "debug:wrap hash_map destroyed" << endl;
        pthread_mutex_destroy(&dlock);
    }
    auto find(string key)
    {
        // cout << "debug:find" << endl;
        return nodes.find(key);
    }
    auto insert(pair<string, ANodeInfo *> &dat)
    {
        // cout << "debug:insert" << endl;
        return nodes.insert(dat);
    }
    auto emplace(string key, ANodeInfo *val)
    {
        // cout << "debug:emplace" << endl;
        // nodes.insert({key, val});
        return nodes.emplace(key, val);
    }
    auto erase(string key)
    {
        // cout << "debug:erase" << endl;
        // DEBUG(key.c_str());
        return nodes.erase(key);
    }
    auto begin()
    {
        // cout << "debug:begin" << endl;
        return nodes.begin();
    }
    auto end()
    {
        // cout << "debug:end" << endl;
        return nodes.end();
        // cout << "debug:after end" << endl;
    }
    auto count(string key)
    {
        // cout << "debug:count" << endl;
        // pthread_mutex_lock(&lock);
        return nodes.count(key);
        // pthread_mutex_unlock(&lock);
    }
    auto size()
    {
        // cout << "debug:size" << endl;
        return nodes.size();
    }
    void lock(void)
    {
        // cout << "debug:lock" << endl;
        pthread_mutex_lock(&dlock);
    }
    void unlock(void)
    {
        // cout << "debug:unlock" << endl;
        pthread_mutex_unlock(&dlock);
    }
};
string randomName(void)
{
    char tmp[20];
    time_t time_now = time(NULL);
    sprintf(tmp, "%ld", time_now);
    return tmp;
}
WrapMap<string, ANodeInfo *> nodesA;
// nodesA???????????????????????????????????????????????????????????????????????????????????????
struct tickInfo
{
    int fd_data;
    int fd_graph;
    int fd_tick;
    string type;
    string name;
    struct sockaddr_in client_tick;
    unique_ptr<pthread_t> tickThread = unique_ptr<pthread_t>(new pthread_t());
};
//?????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
unsigned int curA,
    curB;
Num numer;
char *ip_addr = "0.0.0.0";
int listenAdata, listenAgraph, listenAtick, listenBdata, listenBwarn, listenBother;
// #define exit ::exit
// #define free ::free
// #define memset ::memset
void clean_sock(void)
{
    //??????????????????exit????????????exit?????????????????????????????????????????????????????????????????????
    perror("clean check error");
    *(int *)(0xdeadc) = 1234;
    close(listenAdata);
    close(listenAgraph);
    close(listenAtick);
    close(listenBdata);
    close(listenBwarn);
    close(listenBother);
    DEBUG("in clean");
    for (int i = 0; i < 4; i++)
    {
        close(gepfd[i]);
    }
    DEBUG("before rm graph dir");
    // execlp("rm", "rm", "-rf", tmp.c_str(), NULL);
    // DEBUG("clean failed");
}
void *Adata(void *arg)
{
    printf("Adata:%d\n", syscall(__NR_gettid));
    struct epoll_event ev;
    string type;
    int epfd = (long)arg;
    int nfds;
    int n;
    int socket;
    int i;
    ANodeInfo *a_info;
    char message_box[MESSAE_LENGTH];
    while (1)
    {
        nfds = epoll_wait(epfd, a_data_event, ANUM, -1);
        if (nfds < 0)
        {
            if (errno == 4)
            {
                errno = 0;
                continue;
            }
            perror("epoll wait failed in Aread");
            exit_database();
            exit(1);
        }
        for (i = 0; i < nfds; i++)
        {
            errno = 0;
            if (a_data_event[i].events & EPOLLIN)
            {
                a_info = (ANodeInfo *)(a_data_event[i].data.ptr);
                if (a_info == NULL)
                {
                    printf("error in A read:NULL pointer\n");
                    exit(1);
                }
                unsigned int len;
                n = recv(a_info->fd_data, &len, sizeof(int), MSG_WAITALL);
                if (n == 0 | n < 0)
                {
                    if (n < 0 && errno != ECONNRESET)
                    {
                        FTDEBUG("Adata.log", "recv<0", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                        close(a_info->fd_data);
                        close(a_info->fd_graph);
                        close(a_info->fd_tick);
                        delete a_info;
                        exit_database();
                        exit(1);
                    }
                    else if (n == 0 | errno == ECONNRESET)
                    {
                        FTDEBUG("Adata.log", "recv==0|errno==ECONNRESET", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                        FTDEBUG("A.log", "data recv == 0", "(%d,%s)errno=%d,%s,n=%d", ntohs(a_info->client_data.sin_port), a_info->name.c_str(), errno, strerror(errno), n);
                        errno = 0;
                        char *p = (char *)malloc(20);
                        printf("%d:board[%s:%d] has disconnected:%s\n", __LINE__, inet_ntop(AF_INET, &a_info->client_data.sin_addr, p, 20), ntohs(a_info->client_data.sin_port), strerror(errno));
                        free(p);
                        if (epoll_ctl(epfd, EPOLL_CTL_DEL, a_info->fd_data, NULL) == -1)
                        {
                            FTDEBUG("Adata.log", "epoll_del", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                            exit_database();
                            exit(1);
                        }
                        nodesA.lock();
                        auto c = nodesA.find(a_info->name);
                        if (c != nodesA.end())
                        {
                            for (auto b = c->second->connection.begin(); b != c->second->connection.end(); b++)
                            {
                                json j;
                                j["type"] = "cmd";
                                j["content"] = "breset";
                                string a = j.dump();
                                int len = a.size();
                                len = htonl(len);
                                int m = send((*b)->fd_warn, &len, sizeof(len), 0);
                                if (m <= 0 && errno != EPIPE)
                                {
                                    FTDEBUG("Adata.log", "send failed", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                                    errno = 0;
                                    continue;
                                }
                                m = send((*b)->fd_warn, a.c_str(), a.size(), 0);
                                if (m <= 0 && errno != EPIPE)
                                {
                                    FTDEBUG("Adata.log", "send failed", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                                    errno = 0;
                                    continue;
                                }
                            }
                            nodesA.erase(a_info->name);
                        }

                        int code = a_info->vcode;
                        close(a_info->fd_data);
                        if (a_info->pair_node != NULL)
                        {
                            a_info->pair_node->pair_node = NULL;
                        }
                        delete a_info;
                        nodesA.unlock();
                        freeV.lock();
                        FDEBUG("vode.log", "vode=%d", code);
                        freeV.push(code);
                        freeV.unlock();
                        numer.decreaseA();
                        DEBUG("");
                        continue;
                    }
                }
                len = ntohl(len);
                n = recv(a_info->fd_data, message_box, len, MSG_WAITALL);

                if (n == 0 | n < 0)
                {
                    if (n < 0 && errno != ECONNRESET)
                    {
                        FTDEBUG("Adata.log", "recv<0", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                        errno = 0;
                        close(a_info->fd_data);
                        close(a_info->fd_graph);
                        close(a_info->fd_tick);
                        delete a_info;
                        exit_database();
                        exit(1);
                    }
                    else if (n == 0 | errno == ECONNRESET)
                    {
                        FTDEBUG("Adata.log", "recv==0|errno==ECONNRESET", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                        FTDEBUG("A.log", "data recv == 0", "(%d,%s)errno=%d,%s,n=%d", ntohs(a_info->client_data.sin_port), a_info->name.c_str(), errno, strerror(errno), n);
                        errno = 0;
                        char *p = (char *)malloc(20);
                        printf("%d:board %s:[%s:%d] has disconnected:%s\n", __LINE__, a_info->name.c_str(), inet_ntop(AF_INET, &a_info->client_data.sin_addr, p, 20), ntohs(a_info->client_data.sin_port), strerror(errno));
                        free(p);
                        if (epoll_ctl(epfd, EPOLL_CTL_DEL, a_info->fd_data, NULL) == -1)
                        {
                            FTDEBUG("Adata.log", "epoll del failed", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                            exit_database();
                            exit(1);
                        }
                        nodesA.lock();
                        auto c = nodesA.find(a_info->name);
                        if (c != nodesA.end())
                        {
                            for (auto b = c->second->connection.begin(); b != c->second->connection.end(); b++)
                            {
                                json j;
                                j["type"] = "cmd";
                                j["content"] = "breset";
                                string a = j.dump();
                                int len = a.size();
                                len = htonl(len);
                                int m = send((*b)->fd_data, &len, sizeof(len), 0);
                                if (m <= 0 && errno != EPIPE)
                                {
                                    FTDEBUG("Adata.log", "send failed", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                                    errno = 0;
                                    continue;
                                }
                                m = send((*b)->fd_data, a.c_str(), a.size(), 0);
                                if (m <= 0 && errno != EPIPE)
                                {
                                    FTDEBUG("Adata.log", "send failed", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                                    errno = 0;
                                    continue;
                                }
                            }
                            nodesA.erase(a_info->name);
                        }

                        int code = a_info->vcode;
                        close(a_info->fd_data);
                        if (a_info->pair_node != NULL)
                        {
                            a_info->pair_node->pair_node = NULL;
                        }
                        delete a_info;
                        nodesA.unlock();
                        freeV.lock();
                        FDEBUG("vode.log", "vode=%d", code);
                        freeV.push(code);
                        freeV.unlock();
                        numer.decreaseA();
                    }
                }
                else
                {
                    message_box[n] = 0;
                    FTDEBUG("Adata.log", "message", "(n==%d)%s", n, message_box);
                    if (message_box[0] == 0)
                    {
                        FTDEBUG("Adata.log", "message prefix", " (%d)%d %d %d %d %d %d %d %d %d %d %d %d", ntohs(a_info->client_data.sin_port), message_box[0], message_box[1], message_box[2],
                                message_box[3], message_box[4], message_box[5], message_box[6], message_box[7], message_box[8], message_box[9], message_box[10], message_box[11]);
                    }
                    a_info->message = message_box;
                    json data;
                    try
                    {
                        data = json::parse(message_box);
                    }
                    catch (exception &e)
                    {
                        FTDEBUG("Adata.log", "parse error", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                        exit(1);
                    }
                    type = data["type"];
                    if (type == "data")
                    {
                        unsigned int wood_time = a_info->wood_time;
                        if (wood_time == 0)
                        {
                            wood_time = a_info->wood_time = time(NULL);
                            printf("\n\n\njust for one time should it be\n\n\n");
                            save_board_data(message_box);
                        }
                        clock_after = time(NULL);
                        unsigned int sec = difftime(clock_after, wood_time);
                        if (sec >= 60 * 60 * STORE_HOUR)
                        {
                            printf("\n\n\nsecsecsec:%d\n\n\n", sec);
                            save_board_data(message_box);
                            a_info->wood_time = clock_after;
                        }
                        nodesA.lock();
                        a_info->name = data["name"];
                        a_info->position = data["position"];
                        a_info->humi = data["humi"];
                        a_info->temp = data["temp"];
                        a_info->light = data["light"];
                        a_info->smoke = data["smoke"];
                        nodesA.unlock();
                        double temp = stod(a_info->temp);
                        double humi = stod(a_info->humi);
                        double light = stod(a_info->light);
                        double smoke = stod(a_info->smoke);
                        if (temp > a_info->high_temp || humi > a_info->high_humi || light == a_info->wrong_light || smoke == a_info->wrong_smoke)
                        {
                            nodesA.lock();
                            auto p = nodesA.find(a_info->name);
                            if (p != nodesA.end())
                            {
                                json reply;
                                reply["type"] = "data";
                                reply["boardName"] = data["name"];
                                reply["temp"] = data["temp"];
                                reply["humi"] = data["humi"];
                                reply["position"] = data["position"];
                                reply["light"] = data["light"];
                                reply["smoke"] = data["smoke"];
                                char time_in[100];
                                time_t time_in_1 = time(NULL);
                                asctime_r(localtime(&time_in_1), time_in);
                                len = strlen(time_in);
                                time_in[len - 1] = '\0';
                                reply["time"] = time_in;
                                string reply_string = reply.dump();
                                for (auto m = p->second->connection.begin(); m != p->second->connection.end(); m++)
                                {
                                    int fd_tmp = (*m)->fd_warn;
                                    int len_tmp = reply_string.size();
                                    len_tmp = htonl(len_tmp);
                                    n = send(fd_tmp, &len_tmp, sizeof(len_tmp), 0);
                                    if (n <= 0 && (errno == EPIPE | errno == ECONNRESET))
                                    {
                                        FTDEBUG("Adata.log", "send<0", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                                        errno = 0;
                                        continue;
                                    }
                                    else if (n <= 0)
                                    {
                                        FTDEBUG("Adata.log", "send<0", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                                        errno = 0;
                                        continue;
                                    }
                                    n = send(fd_tmp, reply_string.c_str(), reply_string.size(), 0);
                                    if (n <= 0 && (errno == EPIPE | errno == ECONNRESET))
                                    {
                                        FTDEBUG("Adata.log", "send<0", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                                        errno = 0;
                                        continue;
                                    }
                                    else if (n <= 0)
                                    {
                                        FTDEBUG("Adata.log", "send<0", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                                        errno = 0;
                                        continue;
                                    }
                                }
                            }
                            nodesA.unlock();
                        }
                    }

                    else if (type == "cmd")
                    {
                        string name, content;
                        name = data["name"];
                        content = data["content"];
                        if (content == "logout")
                        {
                            char *p = (char *)malloc(20);
                            printf("%s:%d logout:%s\n", inet_ntop(AF_INET, &a_info->client_data.sin_addr, p, 20), ntohs(a_info->client_data.sin_port), strerror(errno));
                            free(p);
                            epoll_ctl(epfd, EPOLL_CTL_DEL, a_info->fd_data, NULL);
                            nodesA.lock();
                            auto c = nodesA.find(name);
                            if (c != nodesA.end())
                            {
                                for (auto b = c->second->connection.begin(); b != c->second->connection.end(); b++)
                                {
                                    json j;
                                    j["type"] = "cmd";
                                    j["content"] = "breset";
                                    string a = j.dump();
                                    int len = a.size();
                                    len = htonl(len);
                                    int m = send((*b)->fd_data, &len, sizeof(len), 0);
                                    if (m <= 0 && errno != EPIPE)
                                    {
                                        FTDEBUG("Adata.log", "send<0", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                                        errno = 0;
                                        continue;
                                    }
                                    m = send((*b)->fd_data, a.c_str(), a.size(), 0);
                                    if (m <= 0 && errno != EPIPE)
                                    {
                                        FTDEBUG("Adata.log", "send<0", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                                        errno = 0;
                                        continue;
                                    }
                                }
                                nodesA.erase(name);
                                int vcode = a_info->vcode;
                                freeV.lock();
                                FDEBUG("vode.log", "vode=%d", vcode);
                                freeV.push(vcode);
                                freeV.unlock();
                            }
                            close(a_info->fd_data);
                            a_info->pair_node->pair_node = NULL;
                            if (a_info->pair_node != NULL)
                            {
                                a_info->pair_node->pair_node = NULL;
                            }
                            delete a_info;
                            nodesA.unlock();
                            numer.decreaseA();
                        }
                    }
                }
            }
            else if (a_data_event[i].events & EPOLLERR)
            {
                // expectation never get here
                a_info = (ANodeInfo *)a_data_event[i].data.ptr;
                FTDEBUG("Adata.log", "epoll err", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                exit(1);
            }
            else if (a_data_event[i].events & EPOLLHUP)
            {
                // expectation never get here
                DEBUG("");
                a_info = (ANodeInfo *)a_data_event[i].data.ptr;
                char *p = (char *)malloc(20);
                FTDEBUG("Adata.log", "epoll hup", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                printf("line %d:board %s:[%s:%d] has disconnected:%s(net hup!!!)\n", __LINE__, a_info->name.c_str(), inet_ntop(AF_INET, &a_info->client_data.sin_addr, p, 20), ntohs(a_info->client_data.sin_port), strerror(errno));
                free(p);
                close(a_info->fd_data);
                nodesA.lock();
                if (a_info->pair_node != NULL)
                {
                    a_info->pair_node->pair_node = NULL;
                }
                nodesA.unlock();
                delete a_info;
            }
        }
    }
}
void *Agraph(void *arg)
{
    printf("Agraph:%d\n", syscall(__NR_gettid));
    struct epoll_event ev;
    int epfd = (long)arg;
    char *graph_buffer;
    int nfds;
    int len;
    int connfd;
    char time_buffer[50];
    ANodeInfo *info;
    int gfd;
    int n;
    while (1)
    {
        nfds = epoll_wait(epfd, a_graph_event, ANUM, -1);
        switch (nfds)
        {
        case -1:
            if (errno == 4)
            {
                errno = 0;
                continue;
            }
            perror("epoll wait failed in Agraph");
            exit_database();
            exit(1);
            break;
        default:
            for (int i = 0; i < nfds; i++)
            {
                errno = 0;
                if (a_graph_event[i].events & EPOLLIN)
                {
                    info = (ANodeInfo *)a_graph_event[i].data.ptr;
                    connfd = info->fd_graph;
                    n = recv(connfd, &len, sizeof(len), MSG_WAITALL);
                    if (n < 0 && errno != ECONNRESET)
                    {
                        FTDEBUG("Agraph.log", "recv len < 0", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                        perror("recv err;");
                        exit(1);
                    }
                    if (n == 0 | errno == ECONNRESET)
                    {
                        // TODO
                        //????????????????????????????????????????????????log??????????????????????????????
                        FTDEBUG("Agraph.log", "recv  len == 0|errno== ECONNRESET", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                        FTDEBUG("A.log", "graph recv == 0", "(%d,%s)errno=%d,%s,n=%d", ntohs(info->client_graph.sin_port), info->name.c_str(), errno, strerror(errno), n);
                        errno = 0;
                        if (epoll_ctl(epfd, EPOLL_CTL_DEL, connfd, NULL) == -1)
                        {
                            printf("A read epoll del failed %d:%s", __LINE__, strerror(errno));
                            exit_database();
                            exit(1);
                        }
                        close(connfd);
                        nodesA.lock();
                        if (info->pair_node != NULL)
                        {
                            info->pair_node->pair_node = NULL;
                        }
                        nodesA.unlock();
                        delete info;
                        continue;
                    }
                    len = ntohl(len);
                    FTDEBUG("Agraph.log", "time buffer len", "len=%d", len);
                    n = recv(connfd, time_buffer, len, MSG_WAITALL);
                    FTDEBUG("Agraph.log", "timer data", "%s", time_buffer);
                    if (n < 0 && errno != ECONNRESET)
                    {
                        FTDEBUG("Agraph.log", "recv len < 0", "errno=%d,%s,n=%d,len=%d", errno, strerror(errno), n, len);
                        perror("recv err;");
                        exit(1);
                    }
                    if (n == 0 | errno == ECONNRESET | n < len)
                    {
                        FTDEBUG("Agraph.log", "recv  len == 0|errno== ECONNRESET", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                        FTDEBUG("A.log", "graph recv == 0", "(%d,%s)errno=%d,%s,n=%d", ntohs(info->client_graph.sin_port), info->name.c_str(), errno, strerror(errno), n);
                        errno = 0;
                        if (epoll_ctl(epfd, EPOLL_CTL_DEL, connfd, NULL) == -1)
                        {
                            FTDEBUG("Agraph.log", "epoll del failed", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                            exit_database();
                            exit(1);
                        }
                        close(connfd);
                        nodesA.lock();
                        if (info->pair_node != NULL)
                        {
                            info->pair_node->pair_node = NULL;
                        }
                        nodesA.unlock();
                        delete info;
                        continue;
                    }
                    // get '\n' fucked
                    time_buffer[n - 1] = 0;
                    string photo_time = time_buffer;
                    string fileName = info->photos + "/" + time_buffer;
                    n = recv(connfd, &len, sizeof(len), MSG_WAITALL);
                    if (n < 0 && errno != ECONNRESET)
                    {
                        FTDEBUG("Agraph.log", "recv len < 0", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                        perror("recv err;");
                        exit(1);
                    }
                    if (n == 0 | errno == ECONNRESET)
                    {
                        FTDEBUG("Agraph.log", "recv  len == 0|errno== ECONNRESET", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                        FTDEBUG("A.log", "graph recv == 0", "(%d,%s)errno=%d,%s,n=%d", ntohs(info->client_graph.sin_port), info->name.c_str(), errno, strerror(errno), n);
                        errno = 0;
                        if (epoll_ctl(epfd, EPOLL_CTL_DEL, connfd, NULL) == -1)
                        {
                            FTDEBUG("Agraph.log", "epoll del failed", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                            exit_database();
                            exit(1);
                        }
                        close(connfd);
                        nodesA.lock();
                        if (info->pair_node != NULL)
                        {
                            info->pair_node->pair_node = NULL;
                        }
                        nodesA.unlock();
                        delete info;
                        continue;
                    }
                    len = ntohl(len);

                    gfd = open(fileName.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0777);
                    DEBUG("");
                    if (gfd < 0)
                    {
                        FTDEBUG("Agraph.log", "open tmpGfile failed", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                        perror("");
                        exit(1);
                    }
                    lseek(gfd, len - 1, SEEK_SET);
                    n = write(gfd, "\0", 1);
                    lseek(gfd, 0, SEEK_SET);
                    if (n < 0)
                    {
                        FTDEBUG("Agraph.log", "leek<0", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                        exit(1);
                    }
                    if ((graph_buffer = (char *)mmap(NULL, len, PROT_WRITE | PROT_READ, MAP_SHARED, gfd, 0)) == MAP_FAILED)
                    {
                        FTDEBUG("Agraph.log", "graph_buffer mmap failed", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                        perror("mhash_map failed in sendfile");
                        exit(1);
                    }
                    n = recv(connfd, graph_buffer, len, MSG_WAITALL);
                    if (n < 0 && errno != ECONNRESET)
                    {
                        FTDEBUG("Agraph.log", "recv len < 0", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                        perror("recv err;");
                        exit(1);
                    }
                    if (n == 0 | errno == ECONNRESET | n < len)
                    {
                        FTDEBUG("Agraph.log", "recv  len == 0/errno== ECONNRESET", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                        FTDEBUG("A.log", "graph recv == 0", "(%d,%s)errno=%d,%s,n=%d", ntohs(info->client_graph.sin_port), info->name.c_str(), errno, strerror(errno), n);
                        errno = 0;
                        if (epoll_ctl(epfd, EPOLL_CTL_DEL, connfd, NULL) == -1)
                        {
                            FTDEBUG("Agraph.log", "epoll del failed", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                            exit_database();
                            exit(1);
                        }
                        close(connfd);
                        nodesA.lock();
                        if (info->pair_node != NULL)
                        {
                            info->pair_node->pair_node = NULL;
                        }
                        nodesA.unlock();
                        delete info;
                        n = munmap(graph_buffer, len);
                        if (n < 0)
                        {
                            FTDEBUG("Agraph.log", "munmap failed", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                            exit(1);
                        }
                        continue;
                    }
                    else
                    {
                        close(gfd);
                        n = munmap(graph_buffer, len);
                        if (n < 0)
                        {
                            FTDEBUG("Agraph.log", "munmap failed", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                            exit(1);
                        }
                        int numFaces = faceDetect(info->face_conf, info->photos + "/" + photo_time, info->faces + "/" + photo_time + ".jpg");
                        if (numFaces == 0)
                        {
                            continue;
                        }
                        json j;
                        j["type"] = "face";
                        j["time"] = time_buffer;
                        string warning = j.dump();
                        len = warning.length();
                        nodesA.lock();
                        if (info->pair_node == NULL)
                        {
                            nodesA.unlock();
                            continue;
                        }
                        for (auto m = info->pair_node->connection.begin(); m != info->pair_node->connection.end(); m++)
                        {
                            int rlen = htonl(len);
                            n = send((*m)->fd_warn, &rlen, sizeof(rlen), 0);
                            if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
                            {

                                FTDEBUG("Agraph.log", "send failed", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                                errno = 0;
                                continue;
                            }
                            else if (n <= 0)
                            {

                                FTDEBUG("Agraph.log", "send failed", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                                errno = 0;
                                continue;
                            }
                            n = send((*m)->fd_warn, warning.c_str(), len, 0);
                            if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
                            {
                                FTDEBUG("Agraph.log", "send failed", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                                errno = 0;
                                continue;
                            }
                            else if (n <= 0)
                            {
                                FTDEBUG("Agraph.log", "send failed", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                                errno = 0;
                                continue;
                            }
                        }
                        nodesA.unlock();
                    }
                }
                else if (a_graph_event[i].events & EPOLLERR)
                {
                    // expectation never get here
                    info = (ANodeInfo *)a_graph_event[i].data.ptr;
                    FTDEBUG("Agraph.log", "epoll wait error", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                    perror("epoll wait error");
                    exit(1);
                }
                else if (a_graph_event[i].events & EPOLLHUP)
                {
                    // expectation never get here
                    info = (ANodeInfo *)a_graph_event[i].data.ptr;
                    char *p = (char *)malloc(20);
                    FTDEBUG("Agraph.log", "epoll hup", "errno=%d,%s,n=%d", errno, strerror(errno), n);
                    printf("line %d:board %s:[%s:%d] has disconnected:%s(net hup!!!)\n", __LINE__, info->name.c_str(), inet_ntop(AF_INET, &info->client_data.sin_addr, p, 20), ntohs(info->client_data.sin_port), strerror(errno));
                    free(p);
                    close(info->fd_graph);
                    nodesA.lock();
                    if (info->pair_node != NULL)
                    {
                        info->pair_node->pair_node = NULL;
                    }
                    nodesA.unlock();
                    delete info;
                }
            }
            break;
        }
    }
}
sigjmp_buf env;
void timeOut(int signo)
{
    cout << "fuck!!!" << endl;
    siglongjmp(env, 1);
}
void *stm32DataThread(void *args)
{
    ANodeInfo *data = (ANodeInfo *)args;
    int fd_data = data->fd_data, fd_graph = data->fd_graph;
    int len, rlen, n;
    char temp[18], humi[18], light[2], smoke[2];
    temp[17] = humi[17] = '\0';
    light[1] = smoke[1] = '\0';
    char message_box[MESSAE_LENGTH];
    char *graph_buffer;
    int gfd;
    string position;
    n = recv(fd_data, &rlen, sizeof(rlen), MSG_WAITALL);
    FTDEBUG("stm32.log", "para", "high_temp=%f,high_humi=%f", data->high_temp, data->high_humi);
    if (n <= 0)
    {
        FTDEBUG("stm32.log", "n<0", "stm32 recv position len n < 0:m=%d", n);
        goto clean_end;
    }
    len = ntohl(rlen);
    if (len <= 0)
    {
        FTDEBUG("stm32.log", "n<0", "stm32 recv position len n < 0:m=%d", n);
        goto clean_end;
    }
    n = recv(fd_data, message_box, len, MSG_WAITALL);
    if (n <= 0)
    {
        FTDEBUG("stm32.log", "n<0", "stm32 recv position n < 0:n=%d", n);
        goto clean_end;
    }
    position = message_box;
    data->position = message_box;
    while (1)
    {
        n = recv(fd_data, &rlen, sizeof(rlen), MSG_WAITALL);
        if (n <= 0)
        {
            FTDEBUG("stm32.log", "n<0", "stm32 recv data len n <= 0:n=%d", n);
            break;
        }
        len = ntohl(rlen);
        if (len != 36)
        {
            FTDEBUG("stm32.log", "len rectify", "stm32 recv data len!=36:len=%d", len);
            break;
        }
        n = recv(fd_data, temp, 17, MSG_WAITALL);
        if (n < 17)
        {
            FTDEBUG("stm32.log", "n<0", "stm32 recv data n < 17:n=%d", n);
            break;
        }
        n = recv(fd_data, humi, 17, MSG_WAITALL);
        if (n < 17)
        {
            FTDEBUG("stm32.log", "n<0", "stm32 recv data n < 17:n=%d", n);
            break;
        }
        n = recv(fd_data, light, 1, MSG_WAITALL);
        if (n < 1)
        {
            FTDEBUG("stm32.log", "n<0", "stm32 recv data n < 1:n=%d", n);
            break;
        }
        n = recv(fd_data, smoke, 1, MSG_WAITALL);
        if (n < 1)
        {
            FTDEBUG("stm32.log", "n<0", "stm32 recv data n < 1:n=%d", n);
            break;
        }
        trim(temp);
        trim(humi);
        FTDEBUG("stm32.log", "data", "recv temp = %s;humi = %s\n", temp, humi);
        // data->writeVal();
        nodesA.lock();
        data->temp = temp;
        data->humi = humi;
        data->light = light;
        data->smoke = smoke;
        nodesA.unlock();
        // data->freeLock();
        unsigned int wood_time = data->wood_time;

        if (wood_time == 0)
        {
            json j;
            j["name"] = data->name;
            j["position"] = data->position;
            j["temp"] = temp;
            j["humi"] = humi;
            j["light"] = light;
            j["smoke"] = smoke;
            wood_time = data->wood_time = time(NULL);
            printf("\n\n\njust for one time should it be\n\n\n");
            save_board_data(j.dump());
        }
        clock_after = time(NULL);
        unsigned int sec = difftime(clock_after, wood_time);
        double temp;
        double humi;
        double light;
        double smoke;
        int res;
        if (sec >= 60 * 60 * STORE_HOUR)
        {
            json j;
            j["name"] = data->name;
            j["position"] = data->position;
            j["temp"] = temp;
            j["humi"] = humi;
            j["light"] = light;
            j["smoke"] = smoke;
            printf("\n\n\nsecsecsec:%d\n\n\n", sec);
            save_board_data(j.dump());
            data->wood_time = clock_after;
        }
        try
        {
            temp = stod(data->temp);
            humi = stod(data->humi);
            light = stod(data->light);
            smoke = stod(data->smoke);
        }
        catch (exception &e)
        {
            cout << e.what() << endl;
            exit(1);
        }
        if (temp > data->high_temp || humi > data->high_humi || light == data->wrong_light || smoke == data->wrong_smoke)
        {
            json reply;
            reply["type"] = "data";
            reply["boardName"] = data->name;
            reply["temp"] = data->temp;
            reply["humi"] = data->humi;
            reply["light"] = data->light;
            reply["smoke"] = data->smoke;
            reply["position"] = data->position;
            char time_in[100];
            time_t time_in_1 = time(NULL);
            asctime_r(localtime(&time_in_1), time_in);
            len = strlen(time_in);
            time_in[len - 1] = '\0';
            reply["time"] = time_in;
            string reply_string = reply.dump();
            DEBUG("before send data to B");
            nodesA.lock();
            for (auto m = data->connection.begin(); m != data->connection.end(); m++)
            {
                int fd_tmp = (*m)->fd_warn;
                int len_tmp = reply_string.size();
                len_tmp = htonl(len_tmp);
                n = send(fd_tmp, &len_tmp, sizeof(len_tmp), 0);
                DEBUG("n=");
                printf("%d\n", n);
                if (n <= 0 && (errno == EPIPE | errno == ECONNRESET))
                {
                    errno = 0;
                    DEBUG("EPIPE");
                    // close((*m)->fd_data);
                    // p->second->connection.remove(*m);
                    continue;
                }
                else if (n <= 0)
                {
                    perror("send len to client failed");
                    // exit_database();
                    // exit(1);
                    continue;
                }
                n = send(fd_tmp, reply_string.c_str(), reply_string.size(), 0);
                if (n <= 0 && (errno == EPIPE | errno == ECONNRESET))
                {
                    errno = 0;
                    DEBUG("EPIPE");
                    // close((*m)->fd_data);
                    // p->second->connection.remove(*m);
                    continue;
                }
                else if (n <= 0)
                {
                    perror("send data to client failed");
                    // exit_database();
                    // exit(1);
                    continue;
                }
            }
            nodesA.unlock();
            DEBUG(reply_string.c_str());
        }
        n = recv(fd_graph, &rlen, sizeof(rlen), MSG_WAITALL);
        if (n <= 0)
        {
            FTDEBUG("stm32.log", "n<0", "stm32 recv graph len n <= 0:n=%d", n);
            break;
        }
        len = ntohl(rlen);
        if (len != GLEN_32)
        {
            FTDEBUG("stm32.log", "len rectify", "stm32 recv graph len != %d:len=%d", GLEN_32, len);
            break;
        }
        printf("glen=%d\n", len);
        time_t time_pic_in = time(NULL);
        char time_pic[20];
        asctime_r(localtime(&time_pic_in), time_pic);
        // get '\n' fucked,or there's bug for bmp
        time_pic[strlen(time_pic) - 1] = '\0';
        string fileName = data->photos + "/" + time_pic;
        gfd = open(fileName.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0777);
        DEBUG("");
        if (gfd < 0)
        {
            printf("%s\n", fileName.c_str());
            perror("");
            exit(1);
        }
        lseek(gfd, len - 1, SEEK_SET);
        n = write(gfd, "\0", 1);
        lseek(gfd, 0, SEEK_SET);
        ERROR_ACTION(n);
        if ((graph_buffer = (char *)mmap(NULL, len, PROT_WRITE | PROT_READ, MAP_SHARED, gfd, 0)) == MAP_FAILED)
        {
            // FAIL:INVALID ARGUMENT
            perror("mhash_map failed in sendfile");
            exit(1);
        }
        n = recv(data->fd_graph, graph_buffer, len, MSG_WAITALL);
        res = munmap(graph_buffer, len);
        if (res < 0)
        {
            FTDEBUG("stm32.log", "mmunmap failed", "errno=%d,%s", errno, strerror(errno));
            exit(1);
        }
        close(gfd);
        if (n <= 0 || n < len)
        {
            FTDEBUG("stm32.log", "len rectify", "stm32 recv graph<%d:n=%d", len, n);
            break;
        }
        string s = data->photos + "/" + time_pic;
        string t = data->faces + "/" + time_pic + ".jpg";
        // printf("filename=%s\n", s.c_str());
        // printf("conf data=%s\n", data->face_conf.c_str());
        // printf("dest=%s\n", t.c_str());
        int numFaces = faceDetect(data->face_conf, data->photos + "/" + time_pic, data->faces + "/" + time_pic + ".jpg");
        if (numFaces == 0)
        {
            continue;
        }
        json j;
        j["type"] = "face";
        j["time"] = time_pic;
        string warning = j.dump();
        len = warning.length();
        nodesA.lock();
        for (auto m = data->connection.begin(); m != data->connection.end(); m++)
        {
            int rlen = htonl(len);
            n = send((*m)->fd_warn, &rlen, sizeof(rlen), 0);
            if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
            {
                errno = 0;
                DEBUG("EPIPE");
                // close((*m)->fd_graph);
                // info->connection.remove(*m);
                continue;
            }
            else if (n <= 0)
            {
                perror("send glen to client failed");
                // exit_database();
                // exit(1);
                continue;
            }
            n = send((*m)->fd_warn, warning.c_str(), len, 0);
            if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
            {
                errno = 0;
                DEBUG("EPIPE");
                // close((*m)->fd_graph);
                // info->connection.remove(*m);
                continue;
            }
            else if (n <= 0)
            {
                perror("send gdata to client failed");
                // exit_database();
                // exit(1);
                continue;
            }
            DEBUG("send graph to client:");
            DEBUG((*m)->client_name.c_str());
        }
        nodesA.unlock();
    }
clean_end:
    nodesA.lock();
    // data->connection.lock();
    for (auto b = data->connection.begin(); b != data->connection.end(); b++)
    {
        DEBUG("in Adata sending");
        json j;
        j["type"] = "cmd";
        j["content"] = "breset";
        string a = j.dump();
        int len = a.size();
        len = htonl(len);
        int m = send((*b)->fd_warn, &len, sizeof(len), 0);
        if (m <= 0 && errno != EPIPE)
        {
            errno = 0;
            printf("send breset failed in %d:%s\n", __LINE__, strerror(errno));
            // exit_database();
            // exit(1);
            continue;
        }
        m = send((*b)->fd_warn, a.c_str(), a.size(), 0);
        if (m <= 0 && errno != EPIPE)
        {
            errno = 0;
            printf("send breset failed in %d:%s\n", __LINE__, strerror(errno));
            // exit_database();
            // exit(1);
            continue;
        }
        DEBUG(a.c_str());
    }
    // data->connection.unlock();
    nodesA.erase(data->name);
    nodesA.unlock();
    close(data->fd_data);
    close(data->fd_graph); //??????????????????fin????????????reset????????????close?????????fin????????????????????????????????????????????????????????????????????????????????????????????????
    //??????????????????????????????vcode??????????????????????????????????????????32???????????????vcode???
    // freeV.lock();
    // freeV.push(data->vcode);
    // freeV.unlock();
    delete data;
    numer.decreaseA();
}
void *TickTock(void *arg)
{
    // 32?????????????????????????????????????????????????????????????????????????????????????????????
    printf("ticktock:%d\n", syscall(__NR_gettid));
    char buffer[100];
    int timeout_int;
    tickInfo *a = (tickInfo *)arg;
    if (a->type == "raspi")
    {
        timeout_int = 5;
    }
    else if (a->type == "stm32")
    {
        timeout_int = 365;
    }
    while (1)
    {
        FTDEBUG("ticktock.log", "breath", "in working");
        cout << boolalpha;
        fd_set fds;
        int maxfd;
        FD_ZERO(&fds);
        FD_SET(a->fd_tick, &fds);
        maxfd = a->fd_tick + 1;
        timeval out = {timeout_int + 10, 0};
        int n = select(maxfd, &fds, NULL, NULL, &out);
        if (n < 0)
        {
            FDEBUG("ticktock.log", "select<0", "errno=%d,%s", errno, strerror(errno));
            exit(1);
        }
        if (n == 0)
        {
            FTDEBUG("A.log", "tick select == 0", "(%s)errno=%d,%s,n=%d", a->name.c_str(), errno, strerror(errno), n);
            close(a->fd_tick);
            shutdown((*a).fd_data, SHUT_RDWR);
            shutdown(a->fd_graph, SHUT_RDWR);
            delete a;
            return NULL;
        }
        else
        {
            n = recv(a->fd_tick, buffer, 100, 0);
            if (n == 0 | errno == ECONNRESET)
            {
                FTDEBUG("A.log", "tick recv == 0", "(%d,%s)errno=%d,%s,n=%d", ntohs(a->client_tick.sin_port), a->name.c_str(), errno, strerror(errno), n);
                errno = 0;
                close(a->fd_tick);
                shutdown(a->fd_data, SHUT_RDWR);
                shutdown(a->fd_graph, SHUT_RDWR);
                delete a;
                return NULL;
            }
            else if (n < 0 && errno != ECONNRESET)
            {
                FTDEBUG("ticktock.log", "tick recv <0", "n==%d,errno=%d", n, errno);
                exit(1);
            }
            else
            {
                FTDEBUG("ticktock.log", "data", "%s", buffer);
            }
        }
    }
}
void *Bdata(void *arg)
{
    printf("Bdata:%d\n", syscall(__NR_gettid));
    BNodeInfo *info;
    struct epoll_event ev;
    int epfd = (long)arg;
    int nfds;
    int n;
    int fd;
    int i;
    int len;
    char message_box[MESSAE_LENGTH];
    while (1)
    {
        DEBUG("Bdata working");
        // code below costs shit
        // nfds = epoll_wait(epfd, b_connect_event, BNUM, -1);
        nfds = epoll_wait(epfd, b_data_event, BNUM, -1);
        DEBUG("Bconn epoll shit");
        switch (nfds)
        {
        case -1:
            // debug interrupt
            if (errno == 4)
            {
                errno = 0;
                continue;
            }
            perror("epoll wait failed in Bread");
            printf("%d\n", errno);
            exit_database();
            exit(1);
        case 0:
            continue;
        default:
            for (i = 0; i < nfds; i++)
            {
                // info = (BNodeInfo *)b_connect_event[i].data.ptr;
                info = (BNodeInfo *)b_data_event[i].data.ptr;
                printf("in Bdata:%d,ptr:%x\n", __LINE__, info);
                if (b_data_event[i].events & EPOLLIN)
                {
                    fd = info->fd_data;
                    n = recv(fd, &len, sizeof(len), MSG_WAITALL);

                    if (n == 0 | errno == ECONNRESET)
                    {
                        errno = 0;
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                        close(fd);
                        delete info;
                        continue;
                    }
                    else if (n < 0)
                    {
                        printf("(line %d)n=%d ", __LINE__, n);
                        perror("???recv err)");
                        exit(1);
                    }
                    len = ntohl(len);
                    n = recv(fd, message_box, len, MSG_WAITALL);
                    puts(message_box);
                    ERROR_ACTION(n)
                    if (n == 0 | errno == ECONNRESET)
                    {
                        errno = 0;
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                        close(fd);
                        delete info;
                        continue;
                    }
                    json j;
                    nodesA.lock();
                    auto p = nodesA.find(info->board_name);
                    // nodesA.unlock();
                    if (p == nodesA.end())
                    {
                        DEBUG("did not find!");
                        // forget this is vicious
                        nodesA.unlock();
                        continue;
                    }

                    // p->second->readVal();
                    j["name"] = p->second->name;
                    j["position"] = p->second->position;
                    j["temp"] = p->second->temp;
                    j["humi"] = p->second->humi;
                    j["light"] = p->second->light;
                    j["smoke"] = p->second->smoke;
                    string tmp = p->second->name;
                    nodesA.unlock();
                    // p->second->freeLock();
                    string msg = j.dump();
                    len = msg.length();
                    len = htonl(len);
                    n = send(fd, &len, sizeof(len), 0);
                    if (errno == ECONNRESET | errno == EPIPE)
                    {
                        errno = 0;
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                        close(fd);
                        delete info;
                        continue;
                    }
                    else if (n < 0)
                    {
                        cout << "send data failed " << tmp << " in line " << __LINE__ << endl;
                        exit(1);
                    }
                    n = send(fd, msg.c_str(), msg.length(), 0);
                    if (errno == ECONNRESET | errno == EPIPE)
                    {
                        errno = 0;
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                        close(fd);
                        delete info;
                        continue;
                    }
                    else if (n < 0)
                    {
                        cout << "send data failed " << tmp << " in line " << __LINE__ << endl;
                        exit(1);
                    }
                }
                else if (b_data_event[i].events & EPOLLHUP)
                {
                    perror("epoll hup in B hup");
                    exit_database();
                    exit(1);
                }
                else if (b_data_event[i].events & EPOLLERR)
                {
                    perror("epoll err in B data");
                    exit_database();
                    exit(1);
                }
            }
        }
    }
}
void *Bconnect(void *arg)
{
    printf("Bconnect:%d\n", syscall(__NR_gettid));
    struct epoll_event ev;
    int epfd = (long)arg;
    int nfds;
    int n;
    char request[REQUEST_LENGTH];
    char message_box[MESSAE_LENGTH];
    BNodeInfo *info;
    int fd;
    int i;
    int len;
    int rlen;
    while (1)
    {
        DEBUG("Bconn working");
        nfds = epoll_wait(epfd, b_connect_event, BNUM, -1);
        DEBUG("Bconn epoll shit");
        switch (nfds)
        {
        case -1:
            // debug interrupt
            if (errno == 4)
            {
                errno = 0;
                continue;
            }
            perror("epoll wait failed in Bread");
            printf("%d\n", errno);
            exit_database();
            exit(1);
        case 0:
            continue;
        default:
            for (i = 0; i < nfds; i++)
            {
                if (b_connect_event[i].events & EPOLLIN)
                {
                    info = (BNodeInfo *)b_connect_event[i].data.ptr;
                    printf("in Bcon:%d,ptr:%x\n", __LINE__, info);
                    fd = info->fd_other;
                    n = recv(fd, &len, 4, MSG_WAITALL);
                    if (n == 0 | errno == ECONNRESET)
                    {
                        errno = 0;
                        char *p = (char *)malloc(20);
                        time_t now;
                        now = time(NULL);
                        printf("%s:socket error:connection with board [%s:%d]:%s\n", info->client_name.c_str(), inet_ntop(AF_INET, &info->clientData.sin_addr, p, 20), ntohs(info->clientData.sin_port), strerror(errno));
                        free(p);
                        ERROR_ACTION(epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev));
                        if (info->board_name != BBBEFORE)
                        {
                            nodesA.lock();
                            auto c = nodesA.find(info->board_name);
                            // nodesA.unlock();
                            if (c != nodesA.end())
                            {
                                c->second->connection.lock();
                                c->second->connection.remove(info);
                                c->second->connection.unlock();
                                // c->second->gdata_node->connection.remove(info);
                            }
                            nodesA.unlock();
                        }

                        //???????????????????????????????????????????????????????????????socket??????????????????????????????bad file descriptor
                        close(fd);
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                        close(info->fd_warn);
                        delete info;
                        numer.decreaseB();
                        continue;
                    }
                    else if (n < 0 && errno != ECONNRESET)
                    {
                        perror("recv failed in Bread");
                        exit_database();
                        exit(1);
                    }

                    else
                    {
                        len = ntohl(len);
                        n = recv(fd, request, len, MSG_WAITALL);
                        request[len] = '\0';
                        if (n == 0 | errno == ECONNRESET)
                        {
                            errno = 0;
                            char *p = (char *)malloc(20);
                            time_t now;
                            now = time(NULL);
                            printf("%s:socket error:connection with board [%s:%d]:%s\n", info->client_name.c_str(), inet_ntop(AF_INET, &info->clientData.sin_addr, p, 20), ntohs(info->clientData.sin_port), strerror(errno));
                            free(p);
                            ERROR_ACTION(epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev));
                            close(fd);

                            close(info->fd_warn);

                            if (info->board_name != BBBEFORE)
                            {
                                nodesA.lock();
                                auto c = nodesA.find(info->board_name);
                                // nodesA.unlock();
                                if (c != nodesA.end())
                                {
                                    c->second->connection.lock();
                                    c->second->connection.remove(info);
                                    c->second->connection.unlock();
                                    // c->second->gdata_node->connection.remove(info);
                                }
                                nodesA.unlock();
                            }
                            delete info;
                            epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                            numer.decreaseB();
                        }
                        else if (n < 0 && errno != ECONNRESET)
                        {
                            perror("recv failed in Bread");
                            exit_database();
                            exit(1);
                        }
                        else
                        {
                            json j = json::parse(request);
                            FDEBUG("bconnect.log", "<request>\n\n%s\n\n<\\request>", request);
                            string type = j["type"];
                            if (type == "connect")
                            {

                                string boardName = j["board name"];
                                string clientName = j["client name"];
                                info->board_name = boardName;
                                info->client_name = clientName;
                                info->pair->board_name = boardName;
                                info->pair->client_name = clientName;
                                nodesA.lock();
                                auto c = nodesA.find(boardName);

                                // nodesA.unlock();
                                if (c != nodesA.end())
                                {
                                    // nodesA unlock down after ask for c datax
                                    //  c->second->connection.lock();
                                    c->second->connection.push_back(info);

                                    // c->second->gdata_node->connection.push_back(info);
                                    // c->second->connection.unlock();
                                }
                                else
                                {
                                    nodesA.unlock();
                                    ERROR_ACTION(epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL));
                                    close(fd);
                                    continue;
                                }

                                info->faces = c->second->faces;
                                int vcode = c->second->vcode;
                                string high_temp = to_string(c->second->high_temp);
                                string high_humi = to_string(c->second->high_humi);
                                string wrong_light = to_string(c->second->wrong_light);
                                string wrong_smoke = to_string(c->second->wrong_smoke);
                                string board_type = c->second->type;
                                nodesA.unlock();
                                vcode = htonl(vcode);
                                n = send(fd, &vcode, 4, 0);
                                if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
                                {
                                    errno = 0;
                                }
                                else if (n <= 0)
                                {
                                    printf("send failed in %d:%s\n", __LINE__, strerror(errno));
                                    exit_database();
                                    exit(1);
                                }

                                int len = high_temp.length();
                                int rlen = htonl(len);
                                printf("high_temp=%s,len=%d\n", high_temp.c_str(), rlen);
                                n = send(fd, &rlen, sizeof(rlen), 0);
                                if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
                                {
                                    errno = 0;
                                }
                                else if (n <= 0)
                                {
                                    printf("send failed in %d:%s\n", __LINE__, strerror(errno));
                                    exit_database();
                                    exit(1);
                                }
                                n = send(fd, high_temp.c_str(), len, 0);
                                if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
                                {
                                    errno = 0;
                                }
                                else if (n <= 0)
                                {
                                    printf("send failed in %d:%s\n", __LINE__, strerror(errno));
                                    exit_database();
                                    exit(1);
                                }
                                len = high_humi.length();
                                rlen = htonl(len);
                                n = send(fd, &rlen, sizeof(rlen), 0);
                                //??????exception?????????????????????????????????????????????try-catch,????????????????????????
                                if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
                                {
                                    errno = 0;
                                }
                                else if (n <= 0)
                                {
                                    printf("send failed in %d:%s\n", __LINE__, strerror(errno));
                                    exit_database();
                                    exit(1);
                                }
                                n = send(fd, high_humi.c_str(), len, 0);
                                if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
                                {
                                    errno = 0;
                                }
                                else if (n <= 0)
                                {
                                    printf("send failed in %d:%s\n", __LINE__, strerror(errno));
                                    exit_database();
                                    exit(1);
                                }
                                len = wrong_light.length();
                                rlen = htonl(len);
                                n = send(fd, &rlen, sizeof(rlen), 0);
                                if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
                                {
                                    errno = 0;
                                }
                                else if (n <= 0)
                                {
                                    printf("send failed in %d:%s\n", __LINE__, strerror(errno));
                                    exit_database();
                                    exit(1);
                                }
                                n = send(fd, wrong_light.c_str(), len, 0);
                                if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
                                {
                                    errno = 0;
                                }
                                else if (n <= 0)
                                {
                                    printf("send failed in %d:%s\n", __LINE__, strerror(errno));
                                    exit_database();
                                    exit(1);
                                }
                                len = wrong_smoke.length();
                                rlen = htonl(len);
                                n = send(fd, &rlen, sizeof(rlen), 0);
                                if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
                                {
                                    errno = 0;
                                }
                                else if (n <= 0)
                                {
                                    printf("send failed in %d:%s\n", __LINE__, strerror(errno));
                                    exit_database();
                                    exit(1);
                                }
                                n = send(fd, wrong_smoke.c_str(), len, 0);
                                if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
                                {
                                    errno = 0;
                                }
                                else if (n <= 0)
                                {
                                    printf("send failed in %d:%s\n", __LINE__, strerror(errno));
                                    exit_database();
                                    exit(1);
                                }
                                len = board_type.length();
                                rlen = htonl(len);
                                n = send(fd, &rlen, sizeof(rlen), 0);
                                if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
                                {
                                    errno = 0;
                                }
                                else if (n <= 0)
                                {
                                    printf("send failed in %d:%s\n", __LINE__, strerror(errno));
                                    exit_database();
                                    exit(1);
                                }
                                n = send(fd, board_type.c_str(), len, 0);
                                if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
                                {
                                    errno = 0;
                                }
                                else if (n <= 0)
                                {
                                    printf("send failed in %d:%s\n", __LINE__, strerror(errno));
                                    exit_database();
                                    exit(1);
                                }
                            }
                            else if (type == "month data")
                            {
                                string name = j["board name"];
                                time_t time_now = time(NULL);
                                struct tm *now = localtime(&time_now);
                                string month_data = get_month_data(now->tm_mon + 1, name);
                                int len = month_data.size();
                                len = htonl(len);
                                n = send(fd, &len, sizeof(len), 0);
                                if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
                                {
                                    errno = 0;
                                }
                                else if (n <= 0)
                                {
                                    printf("send failed in %d:%s\n", __LINE__, strerror(errno));
                                    exit_database();
                                    exit(1);
                                }
                                n = send(fd, month_data.c_str(), month_data.size(), 0);
                                if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
                                {
                                    errno = 0;
                                }
                                else if (n <= 0)
                                {
                                    printf("send failed in %d:%s\n", __LINE__, strerror(errno));
                                    exit_database();
                                    exit(1);
                                }
                            }
                            else if (type == "delete month data")
                            {
                                string name = j["board name"];
                                time_t time_now = time(NULL);
                                struct tm *now = localtime(&time_now);
                                delete_data(now->tm_mon + 1, name);
                            }
                            else if (type == "face")
                            {
                                // nodesA??????????????????data???
                                string time = j["time"];
                                string fileName = info->faces + "/" + time + ".jpg";
                                FDEBUG("face-transfer.log", "file name:%s", fileName.c_str());
                                int fd_tmp = open(fileName.c_str(), O_RDONLY);
                                if (fd >= 0)
                                {
                                    len = lseek(fd_tmp, 0, SEEK_END);
                                    rlen = htonl(len);
                                    FDEBUG("face-transfer.log", "len=%d", len);
                                    n = send(fd, &rlen, sizeof(rlen), 0);
                                    if (n <= 0 && errno != EPIPE && errno != ECONNRESET)
                                    {
                                        FDEBUG("face-transfer.log", "send err:%s", fileName.c_str());
                                        exit(1);
                                    }
                                    lseek(fd_tmp, 0, SEEK_SET);
                                    n = sendfile(fd, fd_tmp, 0, len);
                                    if (n <= 0 && errno != EPIPE && errno != ECONNRESET)
                                    {
                                        FDEBUG("face-transfer.log", "send err:%s", fileName.c_str());
                                        exit(1);
                                    }
                                    close(fd_tmp);
                                }
                                else
                                {
                                    const char *note = "sys error:picture not found,please contact 649535675@qq.com";
                                    len = strlen(note);
                                    rlen = htonl(len);
                                    n = send(fd, &rlen, sizeof(rlen), 0);
                                    if (n <= 0 && errno != EPIPE && errno != ECONNRESET)
                                    {
                                        FDEBUG("face-transfer.log", "send err:%s", fileName.c_str());
                                        exit(1);
                                    }
                                    n = send(fd, note, len, 0);
                                    if (n <= 0 && errno != EPIPE && errno != ECONNRESET)
                                    {
                                        FDEBUG("face-transfer.log", "send err:%s", fileName.c_str());
                                        exit(1);
                                    }
                                }
                            }
                            else if (type == "all faces")
                            {
                                vector<string> files;
                                string a;
                                DIR *face_dir = opendir(info->faces.c_str());
                                while (1)
                                {
                                    struct dirent *face_d = readdir(face_dir);
                                    if (face_d == NULL)
                                        break;
                                    string tmp = face_d->d_name;
                                    if (tmp == "." || tmp == "..")
                                    {
                                        continue;
                                    }
                                    tmp = tmp.substr(0, tmp.find_last_of("."));
                                    files.push_back(tmp);
                                }
                                closedir(face_dir);
                                json j;
                                j["data"] = files;
                                j["num"] = files.size();
                                string faces_data = j.dump();
                                len = faces_data.length();
                                rlen = htonl(len);
                                n = send(fd, &rlen, sizeof(len), 0);
                                if (n <= 0 && errno != EPIPE && errno != EPIPE)
                                {
                                    DEBUG("send err");
                                    exit(1);
                                }
                                FDEBUG("face-all.log", "\n\n\n%s\n\n\n", faces_data.c_str());
                                n = send(fd, faces_data.c_str(), len, 0);
                                if (n <= 0 && errno != EPIPE && errno != EPIPE)
                                {
                                    DEBUG("send err");
                                    exit(1);
                                }
                            }
                            else if (type == "delete faces")
                            {
                                FTDEBUG("face-delete.log", "delete", "%s", "in delete all");
                                rmAll(info->faces.c_str());
                            }
                            else if (type == "delete face")
                            {
                                FTDEBUG("face-delete.log", "delete", "%s", "in delete");
                                string time = j["time"];
                                //??????????????????????????????system????????????????????????????????????????????????
                                string fileName = info->faces + "/" + time + ".jpg";
                                DEBUG("rm file name: " + fileName);
                                rm(fileName.c_str());
                            }
                        }
                    }
                }
                else if (b_connect_event[i].events & EPOLLHUP)
                {
                    perror("epoll hup in B connection");
                    exit_database();
                    exit(1);
                }
                else if (b_connect_event[i].events & EPOLLERR)
                {

                    perror("epoll err in B connection");
                    exit_database();
                    exit(1);
                }
            }
        }
    }
}
void *AThread(void *arg)
{
    printf("AThread:%d\n", syscall(__NR_gettid));
    struct sockaddr_in server_data, server_graph, client_data, client_graph;
    struct sockaddr_in server_tick, client_tick;
    struct epoll_event ev1, ev2;
    pthread_t adata, agraph;
    int connfdData, connfdGraph, connfdTick;
    int epfdData, epfdGraph;
    epfdData = epoll_create(ANUM);
    epfdGraph = epoll_create(ANUM);
    if (epfdData == -1 | epfdGraph == -1)
    {
        perror("AThread epfd create failed");
        exit(1);
    }
    gepfd[0] = epfdData;
    gepfd[1] = epfdGraph;
    ANodeInfo *a_info_1, *a_info_2;
    listenAgraph = socket(AF_INET, SOCK_STREAM, 0);
    listenAdata = socket(AF_INET, SOCK_STREAM, 0);
    listenAtick = socket(AF_INET, SOCK_STREAM, 0);
    if (listenAgraph == -1 | listenAdata == -1 | listenAtick == -1)
    {
        perror("AThread error create TCP socket");
        exit(1);
    }
    memset(&server_data, 0, sizeof(server_data));
    memset(&server_graph, 0, sizeof(server_graph));
    memset(&server_tick, 0, sizeof(server_tick));
    server_data.sin_family = AF_INET;
    server_data.sin_port = htons(portAdata);
    server_graph.sin_family = AF_INET;
    server_graph.sin_port = htons(portAgraph);
    server_tick.sin_family = AF_INET;
    server_tick.sin_port = htons(portTick);
    socklen_t client_data_addr_len = sizeof(client_data);
    socklen_t client_graph_addr_len = sizeof(client_graph);
    socklen_t client_tick_addr_len = sizeof(client_tick);
    if (inet_aton(ip_addr, &server_data.sin_addr) == 0)
    {
        perror("address transferring error");
        exit(1);
    }
    if (inet_aton(ip_addr, &server_graph.sin_addr) == 0)
    {
        perror("address transferring error");
        exit(1);
    }
    if (inet_aton(ip_addr, &server_tick.sin_addr) == 0)
    {
        perror("address transferring error");
        exit(1);
    }
    if (bind(listenAdata, (struct sockaddr *)&server_data, sizeof(server_data)) == -1)
    {
        perror("error while trying to bind on portAdata");
        exit(1);
    }
    if (bind(listenAgraph, (struct sockaddr *)&server_graph, sizeof(server_graph)) == -1)
    {
        perror("error while trying to bind on portAgraph");
        exit(1);
    }
    if (bind(listenAtick, (struct sockaddr *)&server_tick, sizeof(server_tick)) == -1)
    {
        perror("error while trying to bind on portAtick");
        exit(1);
    }
    if (listen(listenAdata, ANUM * 30) == -1)
    {
        printf("%d\n", listenAdata);
        perror("error while trying to listen to Adata");
        exit(1);
    }
    if (listen(listenAgraph, 1) == -1)
    {
        printf("%d\n", listenAgraph);
        perror("error while trying to listen to Agraph");
        exit(1);
    }
    if (listen(listenAtick, 1) == -1)
    {
        printf("%d\n", listenAgraph);
        perror("error while trying to listen to Agraph");
        exit(1);
    }
    ERROR_ACTION(pthread_create(&adata, NULL, Adata, (void *)epfdData))
    ERROR_ACTION(pthread_create(&agraph, NULL, Agraph, (void *)epfdGraph))
    // TODO voice thread
    int len_tmp;
    char message_buffer[200], type_buffer[200];
    char reply[200];
    int n;
    while (1)
    {
        DEBUG("ATH working");
        connfdData = accept(listenAdata, (struct sockaddr *)&client_data, &client_data_addr_len);
        if (connfdData < 0)
        {
            perror("error accepting from board(data)");
            continue;
            // exit(1);
        }

        fd_set accept_tout;
        int maxfd, n;
        timeval tout = {2, 200};
        timeval insurance = {10, 0};
        if (type_buffer == "stm32")
        {
            insurance.tv_sec = 50;
        }
        if (setsockopt(connfdData, SOL_SOCKET, SO_RCVTIMEO, &insurance, sizeof(insurance)) < 0)
        {
            FTDEBUG("AThread.log", "set data recv timeout failed", "errno=(%s,%d)", strerror(errno), errno);
            exit(1);
        }
        n = recv(connfdData, &len_tmp, sizeof(len_tmp), MSG_WAITALL);

        if (n == 0 | errno == ECONNRESET | errno == EAGAIN)
        {
            FTDEBUG("AThread.log", "AThread recv==0|errno == ECONNRESET", "errno=%d,%s", errno, strerror(errno));
            FTDEBUG("A.log", "AThread recv==0|errno == ECONNRESET", "errno=%d,%s", errno, strerror(errno));
            errno = 0;
            close(connfdData);
            continue;
        }
        else if (n < 0 && errno != ECONNRESET && errno != EAGAIN)
        {
            FTDEBUG("AThread.log", "AThread recv<0", "errno=%d,%s", errno, strerror(errno));
            exit(1);
        }
        len_tmp = ntohl(len_tmp);
        n = recv(connfdData, message_buffer, len_tmp, MSG_WAITALL);
        if (n < 0 && errno != ECONNRESET && errno != EAGAIN)
        {
            FTDEBUG("AThread.log", "AThread recv<0", "errno=%d,%s", errno, strerror(errno));
            exit(1);
        }
        if (n == 0 | errno == ECONNRESET | errno == EAGAIN)
        {
            FTDEBUG("AThread.log", "AThread recv==0|errno == ECONNRESET", "errno=%d,%s", errno, strerror(errno));
            FTDEBUG("A.log", "AThread recv==0|errno == ECONNRESET", "errno=%d,%s", errno, strerror(errno));
            errno = 0;
            close(connfdData);
            continue;
        }
        message_buffer[n] = 0;
        string s = message_buffer;
        // recv type
        FTDEBUG("AThread.log", "message_buffer", "%s", message_buffer);
        n = recv(connfdData, &len_tmp, sizeof(len_tmp), MSG_WAITALL);
        if (n == 0 | errno == ECONNRESET | errno == EAGAIN)
        {
            FTDEBUG("AThread.log", "AThread recv==0|errno == ECONNRESET", "errno=%d,%s", errno, strerror(errno));
            FTDEBUG("A.log", "(%s)AThread recv==0|errno == ECONNRESET", "errno=%d,%s", message_buffer, errno, strerror(errno));
            close(connfdData);
            continue;
        }
        else if (n < 0 && errno != ECONNRESET && errno != EAGAIN)
        {
            FTDEBUG("AThread.log", "AThread recv<0", "errno=%d,%s", message_buffer, errno, strerror(errno));
            exit(1);
        }
        len_tmp = ntohl(len_tmp);

        n = recv(connfdData, type_buffer, len_tmp, MSG_WAITALL);
        if (n < 0 && errno != ECONNRESET && errno != EAGAIN)
        {
            FTDEBUG("AThread.log", "AThread recv<0", "errno=%d,%s", message_buffer, errno, strerror(errno));
            exit(1);
        }
        if (n == 0 | errno == ECONNRESET | errno == EAGAIN)
        {
            FTDEBUG("AThread.log", "AThread recv==0|errno == ECONNRESET", "errno=%d,%s", errno, strerror(errno));
            FTDEBUG("A.log", "(%s)AThread recv==0|errno == ECONNRESET", "errno=%d,%s", message_buffer, errno, strerror(errno));
            errno = 0;
            DEBUG("let me know");
            close(connfdData);
            continue;
        }

        type_buffer[n] = 0;
        // puts(type_buffer);
        string t = type_buffer;
        int rcode, code = -1;
        if (t == "raspi")
        {
            nodesA.lock();
            n = nodesA.count(message_buffer);
            // printf("in 1432:%s,%d\n",message_buffer,n);
            nodesA.unlock();
            if (n != 0)
            {
                code = -1;
                strcpy(reply, "already has a board name ");
                strcat(reply, s.c_str());
            }

            else
            {
                freeV.lock();
                if (freeV.empty())
                {
                    DEBUG("");
                    code = -1;
                    strcpy(reply, "Video reaches maximum number of connections");
                }
                else
                {
                    DEBUG("");
                    code = freeV.top();
                    freeV.pop();

                    strcpy(reply, "conratulations,connection has been set");
                }
                freeV.unlock();
            }
            rcode = htonl(code);
            n = send(connfdData, &rcode, sizeof(rcode), 0);
            if (n <= 0)
            {
                if ((errno == EPIPE | errno == ECONNRESET))
                {
                    FTDEBUG("AThread.log", "AThread send<0", "errno=%d,%s", errno, strerror(errno));
                    FTDEBUG("A.log", "(%s)AThread send<0", "errno=%d,%s", message_buffer, errno, strerror(errno));
                    errno = 0;
                    close(connfdData);
                    if (code != -1)
                    {
                        freeV.lock();
                        freeV.push(code);
                        freeV.unlock();
                    }
                    continue;
                }
                else
                {
                    FTDEBUG("AThread.log", "AThread send<0", "errno=%d,%s", errno, strerror(errno));
                    exit_database();
                    exit(1);
                }
            }
            len_tmp = strlen(reply);
            len_tmp = htonl(len_tmp);
            n = send(connfdData, &len_tmp, sizeof(len_tmp), 0);
            if (n <= 0)
            {
                if ((errno == EPIPE | errno == ECONNRESET))
                {
                    FTDEBUG("AThread.log", "AThread send<0", "errno=%d,%s", errno, strerror(errno));
                    FTDEBUG("A.log", "AThread send<0", "(%s)errno=%d", message_buffer, errno);
                    errno = 0;
                    close(connfdData);
                    if (code != -1)
                    {
                        freeV.lock();
                        freeV.push(code);
                        freeV.unlock();
                    }
                    continue;
                }
                else
                {
                    FTDEBUG("AThread.log", "AThread send<0", "errno=%d,%s", errno, strerror(errno));
                    exit_database();
                    exit(1);
                }
            }
            n = send(connfdData, reply, strlen(reply), 0);
            if (n <= 0)
            {
                if ((errno == EPIPE | errno == ECONNRESET))
                {
                    FTDEBUG("AThread.log", "AThread send<0", "errno=%d,%s", errno, strerror(errno));
                    FTDEBUG("A.log", "AThread send<0", "(%s)errno=%d", message_buffer, errno);
                    errno = 0;
                    close(connfdData);
                    if (code != -1)
                    {
                        freeV.lock();
                        freeV.push(code);
                        freeV.unlock();
                    }
                    continue;
                }
                else
                {
                    FTDEBUG("AThread.log", "AThread send<0", "errno=%d,%s", errno, strerror(errno));
                    exit_database();
                    exit(1);
                }
            }
            if (code == -1)
            {
                n = recv(connfdData, message_buffer, 100, 0);
                FTDEBUG("AThread.log", "AThread code == -1", "(%s)n=%d", message_buffer, n);
                FTDEBUG("A.log", "AThread code == -1", "(%s)n=%d", message_buffer, n);
                close(connfdData);
                continue;
            }
            FTDEBUG("AThread.log", "accept port", "data=(%s,%d)", message_buffer, ntohs(client_data.sin_port));
        }
        else
        {
            code = -1;
            // TODO
            nodesA.lock();
            n = nodesA.count(message_buffer);
            // printf("in 1432:%s,%d\n",message_buffer,n);
            nodesA.unlock();
            if (n != 0)
            {
                FTDEBUG("A.log", "AThread code==-1(stm32)", "(%s)code=%d", message_buffer, code);
                FTDEBUG("AThread.log", "AThread code==-1(stm32)", "(%s)code=%d", message_buffer, code);
                close(connfdData);
                continue;
            }
        }
        // TO DO ??????????????????????????????????????????????????????
        insurance = {0, 0};
        if (setsockopt(connfdData, SOL_SOCKET, SO_RCVTIMEO, &insurance, sizeof(insurance)) < 0)
        {
            FTDEBUG("AThread.log", "set data recv timeout failed", "errno=(%s,%d)", strerror(errno), errno);
            exit(1);
        }
        FD_ZERO(&accept_tout);
        FD_SET(listenAgraph, &accept_tout);
        maxfd = listenAgraph + 1;
        if (type_buffer == "stm32")
        {
            tout.tv_sec = 50;
        }
        n = select(maxfd, &accept_tout, NULL, NULL, &tout);
        if (n == 0)
        {
            FTDEBUG("AThread.log", "AThread select graph==0", "errno=%d,%s", errno, strerror(errno));
            FTDEBUG("A.log", "AThread select graph==0", "errno=%d,%s", errno, strerror(errno));
            if (code != -1)
            {
                freeV.lock();
                freeV.push(code);
                freeV.unlock();
            }
            close(connfdData);
            continue;
        }
        else if (n < 0)
        {
            FTDEBUG("AThread.log", "AThread select graph<0", "errno=%d,%s", errno, strerror(errno));
            exit(1);
        }
        connfdGraph = accept(listenAgraph, (struct sockaddr *)&client_graph, &client_graph_addr_len);
        if (connfdGraph < 0)
        {
            FTDEBUG("AThread.log", "AThread accept graph<0", "errno=%d,%s", errno, strerror(errno));
            perror("error accepting from board(graph)");
            exit(1);
        }
        FTDEBUG("AThread.log", "accept port", "graph=(%s,%d)", message_buffer, ntohs(client_graph.sin_port));
        FD_ZERO(&accept_tout);
        FD_SET(listenAtick, &accept_tout);
        maxfd = listenAtick + 1;
        tout = {2, 200};
        if (type_buffer == "stm32")
        {
            tout.tv_sec = 50;
        }
        n = select(maxfd, &accept_tout, NULL, NULL, &tout);
        if (n < 0)
        {
            FTDEBUG("AThread.log", "AThread select tick<0", "errno=%d,%s", errno, strerror(errno));
            exit(1);
        }
        else if (n == 0)
        {
            FTDEBUG("AThread.log", "AThread select tick==0", "errno=%d,%s", errno, strerror(errno));
            FTDEBUG("A.log", "AThread select tick==0", "errno=%d,%s", errno, strerror(errno));
            if (code != -1)
            {
                freeV.lock();
                freeV.push(code);
                freeV.unlock();
            }
            close(connfdData);
            close(connfdGraph);
            continue;
        }
        connfdTick = accept(listenAtick, (struct sockaddr *)&client_tick, &client_tick_addr_len);
        if (connfdTick < 0)
        {
            FTDEBUG("AThread.log", "AThread accept tick<0", "errno=%d,%s", errno, strerror(errno));
            exit(1);
        }
        FTDEBUG("AThread.log", "accept port", "tick=(%s,%d)", message_buffer, ntohs(client_tick.sin_port));
        a_info_1 = new ANodeInfo;
        a_info_1->vcode = code;
        // cout << s << endl;
        //  a_info_1->name = s;
        DEBUG("");
        a_info_1->client_data = client_data;
        DEBUG("");
        a_info_1->client_graph = client_graph;
        DEBUG("");
        a_info_1->client_tick = client_tick;
        DEBUG("");
        a_info_1->fd_data = connfdData;
        DEBUG("");
        a_info_1->fd_graph = connfdGraph;
        DEBUG("");
        a_info_1->fd_tick = connfdTick;
        DEBUG("");
        a_info_1->wood_time = 0;
        DEBUG("");
        a_info_1->name = message_buffer;
        a_info_1->type = type_buffer;
        a_info_1->work_dir = user_dir + "/" + a_info_1->name;
        string face_conf = user_dir + "/" + s + "/" + FACE_CONF;
        string data_conf = user_dir + "/" + s + "/" + DATA_CONF;
        string face_dir = user_dir + "/" + s + FACES;
        string data_conf_from = user_dir + "/" + DATA_DEFAULT;
        if ((access(a_info_1->work_dir.c_str(), F_OK)))
        {
            mkdir(a_info_1->work_dir.c_str(), 0777);
        }
        a_info_1->data_conf = data_conf;
        if (access(data_conf.c_str(), F_OK))
        {
            symlink(data_conf_from.c_str(), data_conf.c_str());
        }
        ifstream inf(data_conf);
        inf.exceptions(ios::eofbit | ios::badbit | ios::failbit);
        inf >> a_info_1->high_temp >> a_info_1->high_humi >> a_info_1->wrong_light >> a_info_1->wrong_smoke;
        inf.close();
        a_info_1->face_conf = face_conf;
        a_info_1->faces = user_dir + "/" + s + "/" + FACES;
        a_info_1->photos = user_dir + "/" + s + "/" + PHOTOS;
        if (access(face_conf.c_str(), F_OK))
        {
            symlink(FACE_DEFAULT, face_conf.c_str());
        }
        if (access(a_info_1->faces.c_str(), F_OK))
        {
            mkdir(a_info_1->faces.c_str(), 0777);
        }
        if (access(a_info_1->photos.c_str(), F_OK))
        {
            mkdir(a_info_1->photos.c_str(), 0777);
        }
        if (t == "raspi")
        {
            a_info_2 = new ANodeInfo;
            a_info_1->pair_node = a_info_2;
            a_info_2->pair_node = a_info_1;
            a_info_2->vcode = code;
            a_info_2->type = type_buffer;
            a_info_2->name = message_buffer;
            a_info_2->client_data = client_data;
            a_info_2->client_graph = client_graph;
            a_info_2->client_tick = client_tick;
            a_info_2->fd_data = connfdData;
            a_info_2->fd_graph = connfdGraph;
            a_info_2->fd_tick = connfdTick;
            a_info_2->wood_time = 0;
            ev1.data.ptr = a_info_1;
            // ev1.events = EPOLLIN | EPOLLET | EPOLLERR | EPOLLHUP;
            ev1.events = EPOLLIN | EPOLLERR | EPOLLHUP;
            ev2.data.ptr = a_info_2;
            ev2.events = EPOLLIN | EPOLLERR | EPOLLHUP;
            a_info_2->work_dir = user_dir + "/" + a_info_2->name;
            a_info_2->face_conf = face_conf;
            a_info_2->faces = user_dir + "/" + s + "/" + FACES;
            a_info_2->photos = user_dir + "/" + s + "/" + PHOTOS;
            nodesA.lock();
            nodesA.emplace(message_buffer, a_info_1);
            nodesA.unlock();
            //???????????????close?????????epoll??????????????????
            ERROR_ACTION(epoll_ctl(epfdData, EPOLL_CTL_ADD, connfdData, &ev1))
            ERROR_ACTION(epoll_ctl(epfdGraph, EPOLL_CTL_ADD, connfdGraph, &ev2))

            DEBUG("before add element");
        }
        else if (t == "stm32")
        {
            nodesA.lock();
            nodesA.emplace(message_buffer, a_info_1);
            nodesA.unlock();
            pthread_create(a_info_1->threadVal.get(), NULL, stm32DataThread, a_info_1);
        }

        tickInfo *tickData = new tickInfo();
        tickData->fd_data = connfdData;
        tickData->fd_graph = connfdGraph;
        tickData->fd_tick = connfdTick;
        tickData->name = message_buffer;
        tickData->type = type_buffer;
        tickData->client_tick = client_tick;
        ERROR_ACTION(pthread_create(tickData->tickThread.get(), NULL, TickTock, tickData));
        DEBUG("after add element");
        DEBUG("before create dir");
        DEBUG("after create workdir");
        numer.increaseA();
    }
    DEBUG("............................ATH quiting..................................");
}

void *BThread(void *arg)
{
    printf("BThread:%d", syscall(__NR_gettid));
    BNodeInfo *info_1, *info_2;
    pthread_t bconnect, bdata;
    int epfdConnect, epfdData;
    int fdpro;
    epfdConnect = epoll_create(BNUM);
    epfdData = epoll_create(BNUM);
    ERROR_ACTION(epfdConnect)
    ERROR_ACTION(epfdData)
    gepfd[3] = epfdData;
    gepfd[4] = epfdConnect;
    struct sockaddr_in serverData, serverWarn, serverOther, clientData, clientWarn, clientOther;
    struct epoll_event ev1, ev2;
    int connfdData, connfdWarn, connfdOther;
    ERROR_ACTION(listenBdata = socket(AF_INET, SOCK_STREAM, 0))
    ERROR_ACTION(listenBwarn = socket(AF_INET, SOCK_STREAM, 0))
    ERROR_ACTION(listenBother = socket(AF_INET, SOCK_STREAM, 0))
    pthread_create(&bconnect, NULL, Bconnect, (void *)epfdConnect);
    pthread_create(&bdata, NULL, Bdata, (void *)epfdData);
    memset(&serverData, 0, sizeof(serverData));
    serverData.sin_family = AF_INET;
    serverData.sin_port = htons(portBdata);
    memset(&serverWarn, 0, sizeof(serverWarn));
    serverWarn.sin_family = AF_INET;
    serverWarn.sin_port = htons(portBwarn);
    memset(&serverOther, 0, sizeof(serverOther));
    serverOther.sin_family = AF_INET;
    serverOther.sin_port = htons(portBother);
    socklen_t client_addr_data_len = sizeof(clientData);
    socklen_t client_addr_warn_len = sizeof(clientWarn);
    socklen_t client_addr_other_len = sizeof(clientOther);
    if (inet_aton(ip_addr, &serverData.sin_addr) == 0)
    {
        perror("address transferring error");
        exit(1);
    }
    if (bind(listenBdata, (struct sockaddr *)&serverData, sizeof(serverData)) == -1)
    {
        perror("error while trying to bind on portBdata\n");
        exit(1);
    }
    if (listen(listenBdata, BNUM) == -1)
    {
        printf("%d\n", listenBdata);
        perror("error while trying to listen to Bdata\n");
        exit(1);
    }
    if (inet_aton(ip_addr, &serverWarn.sin_addr) == 0)
    {
        perror("address transferring error");
        exit(1);
    }
    if (bind(listenBwarn, (struct sockaddr *)&serverWarn, sizeof(serverWarn)) == -1)
    {
        perror("error while trying to bind on portA\n");
        exit(1);
    }
    if (listen(listenBwarn, 1) == -1)
    {
        printf("%d\n", listenBwarn);
        perror("error while trying to listen to B\n");
        exit(1);
    }
    if (inet_aton(ip_addr, &serverOther.sin_addr) == 0)
    {
        perror("address transferring error");
        exit(1);
    }
    if (bind(listenBother, (struct sockaddr *)&serverOther, sizeof(serverOther)) == -1)
    {
        perror("error while trying to bind on portA\n");
        exit(1);
    }
    if (listen(listenBother, 1) == -1)
    {
        printf("%d\n", listenBother);
        perror("error while trying to listen to B\n");
        exit(1);
    }
    while (1)
    {
        DEBUG("BTH working");
        //??????????????????????????????????????????????????????????????????????????????RST
        connfdData = accept(listenBdata, (struct sockaddr *)&clientData, &client_addr_data_len);
        if (connfdData < 0)
        {
            FTDEBUG("BThread.log", "accept1<0", "errno=%d,%s", errno, strerror(errno));
            perror("error accepting from android:");
            exit(1);
        }
        fd_set accept_tout;
        int maxfd, n;
        timeval tout = {2, 0};
        FD_ZERO(&accept_tout);
        FD_SET(listenBwarn, &accept_tout);
        maxfd = listenBwarn + 1;
        // recv name
        n = select(maxfd, &accept_tout, NULL, NULL, &tout);
        if (n == 0)
        {
            FTDEBUG("BThread.log", "select==0", "n=%d", n);
            close(connfdData);
            continue;
        }
        else if (n < 0)
        {
            FTDEBUG("BThread.log", "select<0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            exit(1);
            // close(connfdData);
            // continue;
        }
        connfdWarn = accept(listenBwarn, (struct sockaddr *)&clientWarn, &client_addr_warn_len);
        if (connfdWarn < 0)
        {
            FTDEBUG("BThread.log", "accept2<0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            exit(1);
        }
        FD_ZERO(&accept_tout);
        FD_SET(listenBother, &accept_tout);
        maxfd = listenBother + 1;
        tout = {2, 0};
        // recv name
        n = select(maxfd, &accept_tout, NULL, NULL, &tout);
        // ERROR_ACTION(n)
        if (n == 0)
        {
            FTDEBUG("BThread.log", "select==0", "n=%d", n);
            close(connfdData);
            close(connfdWarn);
            continue;
        }
        else if (n < 0)
        {
            FTDEBUG("BThread.log", "select<0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            exit(1);
            // close(connfdData);
            // close(connfdWarn);
            // continue;
        }
        connfdOther = accept(listenBother, (sockaddr *)&clientOther, &client_addr_other_len);
        if (connfdOther < 0)
        {
            FTDEBUG("BThread.log", "accept3<0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            exit(1);
        }

        json Bdata;
        nodesA.lock();
        int num = nodesA.size();
        Bdata["num"] = num;
        json m;
        json j = json::array();
        int i = 0;
        for (auto a = nodesA.begin(); a != nodesA.end(); a++)
        {
            m["name"] = a->second->name;
            cout << "name=" + a->second->name << endl;
            m["position"] = a->second->position;
            cout << "pos=" + a->second->position << endl;
            m["type"] = a->second->type;
            cout << "type=" + a->second->type << endl;
            j[i] = m; // here core dump,looking for reason
            i++;
        }
        nodesA.unlock();
        Bdata["nodes"] = j;
        string data_string = Bdata.dump();
        int len = data_string.length();
        len = htonl(len);
        n = send(connfdOther, &len, sizeof(len), 0);
        if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
        {
            FTDEBUG("BThread.log", "send<0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            errno = 0;
            close(connfdData);
            close(connfdWarn);
            close(connfdOther);
            continue;
        }
        else if (n <= 0)
        {
            FTDEBUG("BThread.log", "send<=0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            perror("faile in Bconn:");
            printf("errno=%d\n", errno);
            exit_database();
            exit(1);
        }
        n = send(connfdOther, data_string.c_str(), data_string.size(), 0);
        if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
        {
            FTDEBUG("BThread.log", "send<0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            errno = 0;
            close(connfdData);
            close(connfdWarn);
            close(connfdOther);
            // delete info_2;
            continue;
        }
        else if (n <= 0)
        {
            FTDEBUG("BThread.log", "send<=0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            perror("faile in Bconn:");
            printf("errno=%d\n", errno);
            exit_database();
            exit(1);
        }
        info_2 = new BNodeInfo;
        // printf("ptr info con = %x\n", __LINE__, info_2);
        ev2.events = EPOLLIN | EPOLLERR;
        ev2.data.ptr = info_2;
        info_2->clientData = clientData;
        info_2->fd_data = connfdData;
        info_2->fd_other = connfdOther;
        info_2->fd_warn = connfdWarn;
        info_2->client_name = BCBEFORE;
        info_2->board_name = BBBEFORE;

        info_1 = new BNodeInfo;
        // printf("ptr info data = %x\n", __LINE__, info_1);
        ev1.events = EPOLLIN | EPOLLERR;
        ev1.data.ptr = info_1;
        info_1->clientData = clientData;
        info_1->fd_data = connfdData;
        info_1->fd_other = connfdOther;
        info_1->fd_warn = connfdWarn;
        info_1->client_name = BCBEFORE;
        info_1->board_name = BBBEFORE;
        info_2->pair = info_1;
        info_1->pair = info_2;
        epoll_ctl(epfdConnect, EPOLL_CTL_ADD, connfdOther, &ev2);
        epoll_ctl(epfdData, EPOLL_CTL_ADD, connfdData, &ev1);
        numer.increaseB();
    }
}
pthread_mutex_t freelock;
int ack;
void sigPipeHandler(int signo)
{
    signum = signo;
    printf("[recv SIGPIPE!]\n");
}
int main(void)
{
    pthread_mutex_init(&BTestlock, NULL);
    printf("main:%d\n", syscall(__NR_gettid));
    for (int i = 0; i < ANUM; i++)
    {
        FTDEBUG("vode.log", "intilialize", "vode=%d", i);
        freeV.push(i);
    }
    struct passwd *cur_user = getpwuid(getuid());
    user_dir = cur_user->pw_dir;
    struct sigaction sigpipe, sigalarm;
    database_init();
    atexit(clean_sock);
    pthread_t pA, pB;
    pthread_t pC;
    pthread_create(&pA, NULL, AThread, NULL);
    pthread_create(&pB, NULL, BThread, NULL);

    pthread_join(pA, NULL);
    pthread_join(pB, NULL);
    while (1)
        ;
}
