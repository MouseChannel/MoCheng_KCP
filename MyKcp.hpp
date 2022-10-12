#pragma once
// #include "MyUdp.hpp"
#include <arpa/inet.h>
#include <functional>
#include <ikcp.h>

#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <map>
#include <memory>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <ostream>
#include <sys/socket.h>
#include <sys/time.h>
#include <thread>

class MoChengKCP {
  struct sessionData {
    sockaddr_in addr;
    MoChengKCP *udp;
  };

private:
  int socketFd;
  sockaddr_in serverAddr;
  sockaddr_in endpoint;
  int len = sizeof(sockaddr);
  IKCPCB *testKcpClient;

  std::map<int, IKCPCB *> sessions;
  static int udp_output(const char *buf, int len, ikcpcb *kcp, void *user) {

    auto userData = (sessionData *)user;
    auto endpoint = userData->addr;

    userData->udp->UDP_Send(&endpoint, (void *)buf, len);
    std::cout << "In output" << std::endl;

    return 0;
  }

public:
  MoChengKCP() {
    socketFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socketFd < 0) {
      printf("socket init fail: %d\n", socketFd);
    }
    printf("socketFD  : %d\n", socketFd);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = 8888;
    // htons(1234);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    //  INADDR_ANY;

    auto err = ::bind(socketFd, (sockaddr *)&serverAddr, sizeof(serverAddr));
    if (err < 0) {
      printf("bind fail: %d\n", err);
    }

    printf("Init UDP done\n");
  }
  void SpawnNewKCPSession(int kcpID, void *user) {

    IKCPCB *kcpHandle = ikcp_create(kcpID, (void *)user);

    ikcp_nodelay(kcpHandle, 2, 10, 2, 1);
    ikcp_wndsize(kcpHandle, 128, 128);
    kcpHandle->rx_minrto = 10;
    kcpHandle->fastresend = 1;

    kcpHandle->output = udp_output;

    sessions.emplace(kcpID, kcpHandle);
  }
  void Work() {
    std::thread tt([this] { KCPUpdate(); });
    while (true) {

      char mes[2048];

      auto bufSize = recvfrom(socketFd, mes, 2048, 0, (sockaddr *)&endpoint,
                              (socklen_t *)&len);

      if (bufSize < 0) {
        printf(" err :   %d\n", errno);
        break;
      }

      std::cout << "udp received:  " << bufSize << "  |  " << mes << std::endl;
      //test
      if (!sessions[0]) {
        sessionData *user = new sessionData{endpoint, this};
        SpawnNewKCPSession(0, user);
      } else {

        auto rr = ikcp_input(sessions[0], mes, bufSize);
        std::cout << "kcp input:  " << rr << std::endl;
      }
    }
  }
  void KCPUpdate() {
    while (true) {

      for (auto &i : sessions) {

        auto kcp = i.second;

        kcp->updated = 1;
        ikcp_flush(kcp);

        char buf[1024];
        int size = ikcp_recv(kcp, buf, 1024);
        // std::cout << "kcp receive:  " << size << std::endl;
        if (size >= 0) {
          std::cout << "kcp receive:  " << buf << std::endl;
        }
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }
  inline IINT64 GetTime() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return ((IINT64)time.tv_sec) * 1000 + (time.tv_usec / 1000);
  }

  void UDP_Send(sockaddr_in *endpoint, void *buf, int len) {
    sendto(socketFd, buf, len, 0, (sockaddr *)endpoint, sizeof(sockaddr_in));

    std::cout << "udp Send done: " << ikcp_getconv(buf) << "  |  "
              << (char *)buf << endpoint->sin_port << std::endl;
  }

  void StartClient() {
    serverAddr.sin_port = 8888;
    sessionData *user = new sessionData{serverAddr, this};
    testKcpClient = ikcp_create(0, user);
    ikcp_nodelay(testKcpClient, 2, 10, 2, 1);
    ikcp_wndsize(testKcpClient, 128, 128);
    testKcpClient->rx_minrto = 10;
    testKcpClient->fastresend = 1;

    testKcpClient->output = udp_output;
    sessions.emplace(testKcpClient->conv, testKcpClient);
  }
  void KCPSend(std::string mes) {

    char buf[512];
    strcpy(buf, mes.c_str());

    ikcp_send(testKcpClient, buf, 512);
  }
};
