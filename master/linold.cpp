#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <iostream>

#include <pigpio.h>

int port = 0xFFFF;
int gpioInit = 0;
int gpioResult = 0;
const unsigned int TX = 14;
const unsigned int BAUD = 19200;


int sendBreak();
int sendByte(unsigned char);

void handler(int s) {
  std::cerr << "caught something" << std::endl;
  if (port != 0xFFFF) {
    std::cerr << "port closed" << std::endl;
    serClose(port);
  }
  exit(1);
}




int main()
{

  gpioInit = gpioInitialise();

  if (gpioInit == PI_INIT_FAILED) {
    std::cerr << "cannot init gpio" << std::endl;
    exit(1);
  }

  std::cerr << "starting to send break" << std::endl;

  for(unsigned int i = 0; i < 1000; i++) {
    //std::cout << i << std::endl;
    /*if (sendBreak() == PI_INIT_FAILED) {
      std::cerr << "cannot send break" << std::endl;
      exit(1);
      }*/
    if (sendByte(0x55) < 0) {
      std::cerr << "cannot send uart" << std::endl;
      exit(1);
      }
   }
  exit(1);

  //char p[] = "/dev/serial0";
  char p[] = "/dev/ttyS0";

  port = serOpen(p, BAUD,0);

  //std::cout << port << std::cout;
  
  char buf[] = {0x55};

  //signal(SIGINT, handler);

  std::cerr << "starting" << "\n";
  
  for(unsigned int i = 0; i < 10000; i++) {
    serWrite(port, buf, 1);
    usleep(1000);
  }
  std::cerr << "ending" << "\n";

  serClose(port);

}


int sendByte(unsigned char byt) {
  char p[] = "/dev/ttyS0";
  port = serOpen(p, BAUD,0);
  if(port < 0) {
    std::cerr << "cannot open serial" << std::endl;
    return port;
  }
  char buf[] = {0};
  buf[0] = byt;
  //std::cerr << port << std::endl;
  serWrite(port, buf, 1);
  serClose(port);
  return 0;
}

int sendBreak() {

  gpioResult = gpioSetMode(TX, PI_OUTPUT);
  if (gpioResult != 0) {
    std::cerr << "gpio cannot be opened" << std::endl;
    return PI_INIT_FAILED;
  }
  for(unsigned int i = 0; i < 13; i++) {
    gpioWrite(TX, PI_OFF);
    usleep(int(1000000*9/BAUD));
  }
  gpioWrite(TX, PI_ON);
  usleep(1000000*9/BAUD);
  //gpioTerminate();
  return 0;

  
}
