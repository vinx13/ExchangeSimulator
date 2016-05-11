#ifndef EXCHANGESIMULATOR_SERVER_WORKER_H
#define EXCHANGESIMULATOR_SERVER_WORKER_H


#include "MessageDispatcher.h"
#include "ConnectionFactory.h"

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>


struct BufferContext;

class Worker {
public:
    static const std::string TAG;

    static void workerMain(Worker *worker);

    static void onMasterCommand(evutil_socket_t fd, short what, void *arg);

    static void bevOnRead(bufferevent *bev, void *arg);

    static void bevOnWrite(bufferevent *bev, void *arg);

    static void bevOnError(bufferevent *bev, short what, void *arg);


    Worker(evutil_socket_t notify_conn_fd);

    ~Worker();

    void start();

    void stop();

    void putConnection(evutil_socket_t fd);

    BufferContext *bufferContextAlloc();

    void processInput(bufferevent *bev, const char *buf, const int len, BufferContext *context);

private:
    std::unique_ptr<std::thread> worker_thread_;
    std::vector<bufferevent *> conn_bevs_;
    std::queue<evutil_socket_t> new_conn_fds_;
    std::mutex conn_mutex_;
    evutil_socket_t master_cmd_fd_;
    event_base *event_base_;
    std::vector<BufferContext *> buffer_contexts_;
    ConnectionFactory::ConnectionPtr dbconn_;
    MessageDispatcher dispatcher_;
    event *event_master_cmd_;

    void registerConnection(int fd);

    void deregisterConnection(bufferevent *bev);
};


#endif //EXCHANGESIMULATOR_SERVER_WORKER_H
