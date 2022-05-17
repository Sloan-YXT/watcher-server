
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
    DEBUG("Adata start");
    while (1)
    {
        DEBUG("Adata working");
        nfds = epoll_wait(epfd, a_data_event, ANUM, -1);
        DEBUG("Adata epoll shit");
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
        DEBUG("Adata epoll wait success");
        for (i = 0; i < nfds; i++)
        {
            if (a_data_event[i].events & EPOLLIN)
            {
                a_info = (ANodeInfo *)(a_data_event[i].data.ptr);
                if (a_info == NULL)
                {
                    printf("error in A read:NULL pointer\n");
                    exit(1);
                }
                unsigned int len;
                DEBUG("before recv len");
                n = recv(a_info->fd_data, &len, sizeof(int), MSG_WAITALL);
                if (n == 0 | n < 0)
                {
                    if (n < 0 && errno != ECONNRESET)
                    {
                        perror("recv failed in A");
                        close(a_info->fd_data);
                        close(a_info->fd_graph);
                        close(a_info->fd_tick);
                        delete a_info;
                        exit_database();
                        exit(1);
                    }
                    else if (n == 0 | errno == ECONNRESET)
                    {
                        errno = 0;
                        char *p = (char *)malloc(20);
                        printf("%d:board[%s:%d] has disconnected:%s\n", __LINE__, inet_ntop(AF_INET, &a_info->client_data.sin_addr, p, 20), ntohs(a_info->client_data.sin_port), strerror(errno));
                        free(p);
                        if (epoll_ctl(epfd, EPOLL_CTL_DEL, a_info->fd_data, NULL) == -1)
                        {
                            printf("A read epoll del failed %d:%s", __LINE__, strerror(errno));
                            exit_database();
                            exit(1);
                        }
                        numer.decreaseA();
                        cout << a_info->name << endl;
                        nodesA.lock();
                        auto c = nodesA.find(a_info->name);
                        // nodesA.unlock();
                        if (c != nodesA.end())
                        {
                            // c->second->connection.lock();
                            for (auto b = c->second->connection.begin(); b != c->second->connection.end(); b++)
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
                                    continue;
                                }
                                DEBUG(a.c_str());
                            }
                            DEBUG("");
                            nodesA.erase(a_info->name);
                        }
                        int code = a_info->vcode;
                        close(a_info->fd_data);
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
                        errno = 0;
                        perror("recv failed in A");
                        close(a_info->fd_data);
                        close(a_info->fd_graph);
                        close(a_info->fd_tick);
                        delete a_info;
                        exit_database();
                        exit(1);
                    }
                    else if (n == 0 | errno == ECONNRESET)
                    {
                        errno = 0;
                        char *p = (char *)malloc(20);
                        printf("%d:board %s:[%s:%d] has disconnected:%s\n", __LINE__, a_info->name.c_str(), inet_ntop(AF_INET, &a_info->client_data.sin_addr, p, 20), ntohs(a_info->client_data.sin_port), strerror(errno));
                        free(p);
                        if (epoll_ctl(epfd, EPOLL_CTL_DEL, a_info->fd_data, NULL) == -1)
                        {
                            printf("A read epoll del failed %d:%s", __LINE__, strerror(errno));
                            exit_database();
                            exit(1);
                        }
                        numer.decreaseA();
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
                                    errno = 0;
                                    printf("send breset failed in %d:%s\n", __LINE__, strerror(errno));
                                    continue;
                                }
                                m = send((*b)->fd_data, a.c_str(), a.size(), 0);
                                if (m <= 0 && errno != EPIPE)
                                {
                                    errno = 0;
                                    printf("send breset failed in %d:%s\n", __LINE__, strerror(errno));
                                    continue;
                                }
                            }
                            nodesA.erase(a_info->name);
                        }
                        int code = a_info->vcode;
                        close(a_info->fd_data);
                        delete a_info;
                        nodesA.unlock();
                        DEBUG("after erase");
                        freeV.lock();
                        FDEBUG("vode.log", "vode=%d", code);
                        freeV.push(code);
                        freeV.unlock();
                        numer.decreaseA();
                    }
                }
                else
                {
                    DEBUG("recv message");
                    message_box[n] = 0;
                    DEBUG(message_box);
                    a_info->message = message_box;
                    puts(message_box);
                    json data;
                    try
                    {
                        data = json::parse(message_box);
                    }
                    catch (exception &e)
                    {
                        cout << "parse error in " + __LINE__ << endl;
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
                        DEBUG("");
                        // a_info->writeVal();
                        nodesA.lock();
                        a_info->name = data["name"];
                        a_info->position = data["position"];
                        a_info->humi = data["humi"];
                        a_info->temp = data["temp"];
                        a_info->light = data["light"];
                        a_info->smoke = data["smoke"];
                        nodesA.unlock();
                        // a_info->freeLock();
                        DEBUG("");

                        double temp = stod(a_info->temp);
                        double humi = stod(a_info->humi);
                        double light = stod(a_info->light);
                        double smoke = stod(a_info->smoke);
                        if (temp > a_info->high_temp || humi > a_info->high_humi || light == a_info->wrong_light || smoke == a_info->wrong_smoke)
                        {
                            nodesA.lock();
                            auto p = nodesA.find(a_info->name);
                            // nodesA.unlock();
                            if (p != nodesA.end())
                            {
                                json reply;
                                DEBUG("");
                                reply["type"] = "data";
                                DEBUG("");
                                reply["boardName"] = data["name"];
                                reply["temp"] = data["temp"];
                                DEBUG("");
                                reply["humi"] = data["humi"];
                                DEBUG("");
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
                                DEBUG("before send data to B");
                                // p->second->connection.lock();
                                for (auto m = p->second->connection.begin(); m != p->second->connection.end(); m++)
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
                                        continue;
                                    }
                                    else if (n <= 0)
                                    {
                                        perror("send data to client failed");
                                        continue;
                                    }
                                    DEBUG(reply_string.c_str());
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
                            epoll_ctl(epfd, EPOLL_CTL_DEL, a_info->fd_data, &ev);
                            nodesA.lock();
                            auto c = nodesA.find(name);
                            // nodesA.unlock();
                            if (c != nodesA.end())
                            {
                                // c->second->connection.lock();
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
                                        errno = 0;
                                        printf("send breset failed in %d:%s\n", __LINE__, strerror(errno));
                                        continue;
                                    }
                                    m = send((*b)->fd_data, a.c_str(), a.size(), 0);
                                    if (m <= 0 && errno != EPIPE)
                                    {
                                        errno = 0;
                                        printf("send breset failed in %d:%s\n", __LINE__, strerror(errno));
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
                            delete a_info;
                            numer.decreaseA();
                        }
                    }
                }
            }
            else if (a_data_event[i].events & EPOLLERR)
            {
                // expectation never get here
                a_info = (ANodeInfo *)a_data_event[i].data.ptr;
                printf("in %d:fd %d", __LINE__, a_info->fd_data);
                perror("epoll wait error");
                exit(1);
            }
            else if (a_data_event[i].events & EPOLLHUP)
            {
                // expectation never get here
                DEBUG("");
                a_info = (ANodeInfo *)a_data_event[i].data.ptr;
                char *p = (char *)malloc(20);
                cout << "debug:epoll hup" << endl;
                printf("line %d:board %s:[%s:%d] has disconnected:%s(net hup!!!)\n", __LINE__, a_info->name.c_str(), inet_ntop(AF_INET, &a_info->client_data.sin_addr, p, 20), ntohs(a_info->client_data.sin_port), strerror(errno));
                free(p);
                close(a_info->fd_data);
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
                if (a_graph_event[i].events & EPOLLIN)
                {
                    info = (ANodeInfo *)a_graph_event[i].data.ptr;
                    cout << "debug:" << info->photos << endl;
                    connfd = info->fd_graph;
                    DEBUG("in Agraph");
                    n = recv(connfd, &len, sizeof(len), MSG_WAITALL);
                    if (n < 0 && errno != ECONNRESET)
                    {
                        perror("recv err;");
                        exit(1);
                    }
                    if (n == 0 | errno == ECONNRESET)
                    {
                        errno = 0;
                        // TODO
                        //这里存在一直断板子连接的问题，看log应该是这里，不明原因
                        printf("in line%d,n=%d,errno=%d\n", __LINE__, n, errno);
                        if (epoll_ctl(epfd, EPOLL_CTL_DEL, connfd, NULL) == -1)
                        {
                            printf("A read epoll del failed %d:%s", __LINE__, strerror(errno));
                            exit_database();
                            exit(1);
                        }
                        close(connfd);
                        delete info;
                        DEBUG("after delete");
                        continue;
                    }
                    DEBUG("in Agraph");
                    len = ntohl(len);
                    n = recv(connfd, time_buffer, len, MSG_WAITALL);
                    if (n < 0 && errno != ECONNRESET)
                    {
                        perror("recv err;");
                        exit(1);
                    }
                    if (n == 0 | errno == ECONNRESET)
                    {
                        errno = 0;
                        if (epoll_ctl(epfd, EPOLL_CTL_DEL, connfd, NULL) == -1)
                        {
                            printf("A read epoll del failed %d:%s", __LINE__, strerror(errno));
                            exit_database();
                            exit(1);
                        }
                        close(connfd);
                        delete info;
                        DEBUG("after delete");
                        continue;
                    }
                    // get '\n' fucked
                    time_buffer[n - 1] = 0;
                    string photo_time = time_buffer;
                    string fileName = info->photos + "/" + time_buffer;
                    n = recv(connfd, &len, sizeof(len), MSG_WAITALL);
                    if (n < 0 && errno != ECONNRESET)
                    {
                        perror("recv err;");
                        exit(1);
                    }
                    if (n == 0 | errno == ECONNRESET)
                    {
                        errno = 0;
                        if (epoll_ctl(epfd, EPOLL_CTL_DEL, connfd, NULL) == -1)
                        {
                            printf("A read epoll del failed %d:%s", __LINE__, strerror(errno));
                            exit_database();
                            exit(1);
                        }
                        close(connfd);
                        delete info;
                        DEBUG("after delete");
                        continue;
                    }
                    len = ntohl(len);

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
                    n = recv(connfd, graph_buffer, len, MSG_WAITALL);
                    if (n < 0 && errno != ECONNRESET)
                    {
                        perror("recv err;");
                        exit(1);
                    }
                    if (n == 0 | errno == ECONNRESET)
                    {
                        errno = 0;
                        DEBUG("");
                        if (epoll_ctl(epfd, EPOLL_CTL_DEL, connfd, NULL) == -1)
                        {
                            printf("A read epoll del failed %d:%s", __LINE__, strerror(errno));
                            exit_database();
                            exit(1);
                        }
                        close(connfd);
                        delete info;
                        ERROR_ACTION(munmap(graph_buffer, len));
                        continue;
                    }
                    else
                    {
                        close(gfd);
                        ERROR_ACTION(munmap(graph_buffer, len));
                        DEBUG("");
                        //多线程图像访问不需要加锁，除非mmap
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
                        DEBUG("");
                        nodesA.lock();
                        for (auto m = info->pair_node->connection.begin(); m != info->pair_node->connection.end(); m++)
                        {
                            int rlen = htonl(len);
                            n = send((*m)->fd_warn, &rlen, sizeof(rlen), 0);
                            if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
                            {
                                errno = 0;
                                DEBUG("EPIPE");
                                fprintf(stderr, "face detect dump1\n");
                                continue;
                            }
                            else if (n <= 0)
                            {
                                perror("send glen to client failed");
                                continue;
                            }
                            printf("debug:graph len=%d\n", len);
                            n = send((*m)->fd_warn, warning.c_str(), len, 0);
                            if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
                            {
                                errno = 0;
                                DEBUG("EPIPE");
                                continue;
                            }
                            else if (n <= 0)
                            {
                                perror("send gdata to client failed");
                                continue;
                            }
                            DEBUG("send graph to client:");
                            DEBUG((*m)->client_name.c_str());
                        }
                        nodesA.unlock();
                    }
                }
                else if (a_graph_event[i].events & EPOLLERR)
                {
                    info = (ANodeInfo *)a_graph_event[i].data.ptr;
                    printf("in %d:fd %d", __LINE__, info->fd_data);
                    perror("epoll wait error");
                    exit(1);
                }
                else if (a_graph_event[i].events & EPOLLHUP)
                {
                    info = (ANodeInfo *)a_graph_event[i].data.ptr;
                    char *p = (char *)malloc(20);
                    cout << "debug:epoll hup(g)" << endl;
                    printf("line %d:board %s:[%s:%d] has disconnected:%s(net hup!!!)\n", __LINE__, info->name.c_str(), inet_ntop(AF_INET, &info->client_data.sin_addr, p, 20), ntohs(info->client_data.sin_port), strerror(errno));
                    free(p);
                    close(info->fd_graph);
                    delete info;
                }
            }
            break;
        }
        DEBUG("");
    }
    DEBUG("");
}