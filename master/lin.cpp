#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <pigpio.h>

#define RX_PIN 15
#define TX_PIN 14
#define CS_PIN 23

int port = 0xFFFF;
int gpioInit = 0;
int gpioTX = -1;
int gpioRX = -1;
int gpioCS = -1;

uint16_t BAUD = 19200;
uint32_t INTERVAL = 1000000*9/BAUD;

int sendBreak();
int sendByte(unsigned char);
void close();
int init();
uint8_t calculateChecksum(uint8_t, uint8_t);
unsigned char pid(unsigned char);
int getBit(unsigned char);
void setBit(unsigned char, unsigned);

int main() {
    init();

    std::cout << std::hex << (int)pid(22) << std::endl;
    /*int val = PI_OFF;
    setBit(TX_PIN, val);

    std::cout << std::hex << val << (int)getBit(RX_PIN) << std::endl;
    */
    //gpioWrite(TX_PIN, PI_OFF);

    //while(1) {
    //  ;
    // }

    /*
    for(int i = 0; i < 10000; i++) {
      gpioWrite(TX_PIN, PI_OFF);
      int temp = gpioRead(RX_PIN);
      if (temp == PI_OFF) {
        std::cout << "1)" << i << " TX_PIN: " << PI_OFF << " RX_PIN: " << temp << std::endl;
        exit(1);
      }
      usleep(500);
      gpioWrite(TX_PIN, PI_ON);
      temp = gpioRead(RX_PIN);
      if (temp == PI_ON) {
        std::cout << "2)" << i << " TX_PIN: " << PI_ON << " RX_PIN: " << temp << std::endl;
        exit(1);
      }
      usleep(500);
      }*/

    for(int i = 0; i < 1000; i++) {
        //uint16_t message = 0xABCD;

        //sendBreak();
        gpioWrite(TX_PIN, PI_ON);
        usleep(1000000);
        //gpioWrite(TX_PIN, PI_OFF);
        //usleep(7*INTERVAL);

        sendBreak();
        sendByte(0x55);
        usleep(1000000);

        /*
        DO NOT REMOVE
        THIS IS THE WORKING LIN EXAMPLE MANUALY DECODED ON THE PDF OF THE GIT REPO
        sendBreak();
        sendByte(0x55);
        sendByte(pid(22));

        uint8_t lsb = message & 0xFF;
        uint8_t msb = (message >> 8) & 0xFF;
        uint8_t checksum = calculateChecksum(lsb, msb);

        sendByte(lsb);
        sendByte(msb);
        sendByte(checksum);
        std::cout << "lsb: " << std::hex << static_cast<int>(lsb) << " msb: " << std::hex << static_cast<int>(msb) << " checksum: " << checksum << std::endl;
        */
        /*gpioWrite(TX_PIN, PI_OFF);
        usleep(50*1000);
        gpioWrite(TX_PIN, PI_ON);
        usleep(50*1000);
        */
    }
    close();
    return 0;
}


int init() {
    gpioInit = gpioInitialise();

    gpioTX = gpioSetMode(TX_PIN, PI_OUTPUT);
    gpioRX = gpioSetMode(RX_PIN, PI_INPUT);
    gpioCS = gpioSetMode(CS_PIN, PI_OUTPUT);

    if (gpioTX != 0) {
        std::cerr << "gpio tx cannot be opened" << std::endl;
        exit(1);
    }
    if (gpioRX != 0) {
        std::cerr << "gpio rx cannot be opened" << std::endl;
        exit(1);
    }
    if (gpioInit == PI_INIT_FAILED) {
        std::cerr << "cannot init gpio" << std::endl;
        exit(1);
    }

    if (gpioCS != 0) {
        std::cerr << "cannot cs init gpio" << std::endl;
        exit(1);
    }

    gpioWrite(CS_PIN, PI_ON);
    return 0;
}

void close() {
    gpioWrite(CS_PIN, PI_OFF);
    gpioTerminate();
}

uint8_t calculateChecksum(uint8_t data1, uint8_t data2) {
    uint8_t checksum = 0;
    checksum += data1;
    checksum += data2;
    return checksum & 0xFF;
}

int sendByte(unsigned char byte) {
    gpioWrite(TX_PIN, PI_OFF);
    usleep(INTERVAL);
    /*
    int bit = gpioRead(RX_PIN);
    usleep(INTERVAL);
    if (bit != PI_OFF) {
      std::cerr << "writing and reading different:" << PI_OFF << bit << std::endl;
      exit(1);
      }*/

    unsigned char temp;

    for(int i = 0; i < 8; i++) {
        temp = (byte >> i) & 0x01;
        //std::cerr << temp << std::endl;
        if(temp == 0) {
            //std::cerr << "wert 0" << std::endl;
            gpioWrite(TX_PIN, PI_OFF);
        } else {
            //std::cerr << "wert 1" << std::endl;
            gpioWrite(TX_PIN, PI_ON);
        }
        usleep(INTERVAL);
    }

    gpioWrite(TX_PIN, PI_ON);
    usleep(INTERVAL);
    return 0;
}

int recessiveSleep(uint16_t sleepDuration) {
    for(uint16_t i = 0; i<sleepDuration; ++i) {
        gpioWrite(TX_PIN, PI_OFF);
        usleep(INTERVAL);
    }
    return 0;
}

int sendBreak() {
    for(uint8_t i = 0; i < 13; i++) {
        gpioWrite(TX_PIN, PI_OFF);
        usleep(INTERVAL);
        //int temp = gpioRead(RX_PIN);
        /*if(temp == PI_OFF) {
          std::cerr << "no match" << i << std::endl;
          exit(1);
          }*/
    }
    gpioWrite(TX_PIN, PI_ON);
    usleep(INTERVAL);
    return 0;
}


unsigned char pid(unsigned char pid) {
    unsigned char temp = pid & 0x3F;
    unsigned char p0 = ((pid & 0x01) + ((pid >> 1) & 0x01) + ((pid >> 2) & 0x01) + ((pid >> 4) & 0x01)) & 0x01;
    unsigned char p1 = (~(((pid>>1) & 0x01) + ((pid >> 3) & 0x01) + ((pid >> 4) & 0x01) + ((pid >> 5) & 0x01))) & 0x01;

    temp = (p0 << 6) | temp;
    temp = (p1 << 7) | temp;
    return temp;
}


int getBit(unsigned char bit) {
    int res = gpioRead(bit);
    std::cout << (int) res << std::endl;
    return res;
}

void setBit(unsigned char address, unsigned val) {
    gpioWrite(address, val);
    std::cout << address << (int) val << std::endl;
}
