
#include <cstdlib>
#include <memory>
#include <stdio.h>
#include <stdlib.h>

// #include "MyUdp.hpp"
#include "MyKcp.hpp"

#include <ikcp.h>

// #include "MyKcp.hpp"
// #include "MyUdp.hpp"

#include <map>
#include <thread>

int main() {

  // server

  MoChengKCP kcp;
  kcp.StartClient();

  kcp.Work();

  // client
  MoChengKCP kcp1;
  kcp1.StartClient();

  std::cout << "In output" << std::endl;
  std::string mes = "hello";
  kcp.KCPSend(mes + "1");
  kcp.KCPSend(mes + "2");

  kcp.KCPSend(mes + "3");
  kcp.KCPSend(mes + "4");
  kcp.KCPSend(mes + "5");
  kcp.KCPSend(mes + "6");

  kcp1.Work();
}