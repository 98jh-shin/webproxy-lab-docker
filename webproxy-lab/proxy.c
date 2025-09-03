// ReSharper disable CppParameterMayBeConst
#include <stdio.h>

#include "csapp.h"

void doit(int fd);
int parse_uri(char *uri, char *hostname, char *path, char *port);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg);
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

void doit(int fd) {
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char hostname[MAXLINE], path[MAXLINE], port[MAXLINE];
    rio_t rio;
    int clientfd;
    int serverfd;

    Rio_readinitb(&rio, fd);
    Rio_readlineb(&rio, buf, MAXLINE);

    sscanf(buf, "%s %s %s", method, uri, version);

    // if (strcasecmp(method, "GET") && strcasecmp(method, "HEAD")) {
    //     clienterror(fd, method, "501", "Not implemented",
    //                 "Tiny does not implement this method");
    //     return;
    // }

    // hostname, path, port 추출
    parse_uri(uri, hostname, path, port);

    serverfd = Open_clientfd(hostname, port);
    if (serverfd < 0) {
        printf("Connection failed to %s:%s", hostname, port);
        return;
    }

    // tiny로 전송할 HTTP 요청 생성
    char request_header[MAXLINE];
    sprintf(request_header, "GET %s HTTP/1.1\r\n", path);
    Rio_writen(serverfd, request_header, strlen(request_header));

    // 필수 헤더 전송
    sprintf(request_header, "Host: %s\r\n", hostname);
    Rio_writen(serverfd, request_header, strlen(request_header));
    Rio_writen(serverfd, (void *)user_agent_hdr, strlen(user_agent_hdr));
    Rio_writen(serverfd, "Connection: close\r\n",
               strlen("Connection: close\r\n"));
    Rio_writen(serverfd, "Proxy-Connection: close\r\n",
               strlen("Proxy-Connection: close\r\n"));

    // 클라이언트로부터 받은 나머지 헤더들을 서버로 전달
    while (Rio_readlineb(&rio, buf, MAXLINE) > 0) {
        // 헤더의 끝을 만나면 중단
        if (strcmp(buf, "\r\n") == 0) {
            break;
        }
        // 프록시가 직접 설정한 헤더는 다시 보내지 않음
        if (strncasecmp(buf, "Host:", 5) != 0 &&
            strncasecmp(buf, "User-Agent:", 11) != 0 &&
            strncasecmp(buf, "Connection:", 11) != 0 &&
            strncasecmp(buf, "Proxy-Connection:", 17) != 0) {
            Rio_writen(serverfd, buf, strlen(buf));
        }
    }
    // 헤더의 끝을 알리는 빈 줄 전송
    Rio_writen(serverfd, "\r\n", 2);

    /* 5. 서버로부터 받은 응답을 클라이언트에게 그대로 전달 */
    rio_t server_rio;
    Rio_readinitb(&server_rio, serverfd);
    size_t n;
    while ((n = Rio_readnb(&server_rio, buf, MAXLINE)) > 0) {
        Rio_writen(fd, buf, n);
    }
    /* 6. 서버와의 연결 종료 */
    Close(serverfd);
}

int parse_uri(char *uri, char *hostname, char *path, char *port) {
    char *host_ptr = strstr(uri, "://");
    if (host_ptr == NULL) {
        // http://로 시작하지 않으면 호스트 이름이 바로 시작한다고 가정
        host_ptr = uri;
    } else {
        host_ptr += 3;  // "://" 3칸 전진
    }

    char *port_ptr = strchr(host_ptr, ':');
    char *path_ptr = strchr(host_ptr, '/');

    if (path_ptr == NULL) {
        strcpy(path, "/");
    } else {
        strcpy(path, path_ptr);
    }

    // :가 없는 경우 기본 포트, :가 있는데 path가 없으면 :뒤에가 포트, :가
    // 있는데 path가 있고 혹시 path_ptr이 앞에 있으면 :뒤에는 포트가 아님.
    if (port_ptr != NULL && (path_ptr == NULL || port_ptr < path_ptr)) {
        strcpy(port, port_ptr + 1);

        *port_ptr = '\0';

        char *path_in_port = strstr(port, "/");

        if (path_in_port != NULL) {
            *path_in_port = '\0';
        }
    } else {
        strcpy(port, "80");
    }

    if (path_ptr != NULL) {
        *path_ptr = '\0';
    }

    strcpy(hostname, host_ptr);

    return 0;
}

int main(int argc, char **argv) {
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    listenfd = Open_listenfd(argv[1]);

    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

        Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port,
                    MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        doit(connfd);
        close(connfd);
    }

    return 0;
}