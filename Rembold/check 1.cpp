// Online C++ compiler to run C++ program online
#include <iostream>

const unsigned int bufsize = 130;


uint8_t capture[bufsize];



unsigned int converttickstonum(uint8_t num)
{
  if(num > 0 && num <= 4) {
    return 0;
  } else if(num > 4 && num <= 12) {
    return 1;
  } else if(num > 12 && num <= 20) {
    return 2;
  } else if(num > 20 && num <= 28) {
    return 3;
  } else if(num > 28 && num <= 38) {
    return 4;
  } else if(num > 38 && num <= 45) {
    return 5;
  } else if(num > 45 && num <= 55) {
    return 6;
  } else if(num > 55 && num <= 64) {
    return 7;
  } else if(num > 64 && num <= 72) {
    return 8;
  } else if(num > 72 && num <= 80) { // 75
    return 9;
  } else if(num > 80 && num <= 88) { // 84
    return 10;
  } else if(num > 88 && num <= 95) { // 92
    return 11;
  } else if(num > 95 && num <= 105) { // 101
    return 12;
  } else if(num > 105 && num <= 115) { // 109
    return 13;
  }
  
  return 0x0;
}

/*
uint8_t getpidbytefromheader(unsigned int ci)
{
  
  uint32_t temp = 0;
  unsigned int pos = 0;
  for(unsigned int i = 2; i < ci; i++) {
    unsigned int num = converttickstonum(capture[i]);
    for(unsigned int j = 0; j < num; j++) {
      if (i % 2 != 0) { //values[i] == 1
	temp |= 0x00000001 << pos;
      }
      pos++;
    }
  }

  temp &= 0x07F800;
  //  std::cout << "<->getpid" << std::endl;
  return (uint8_t)(temp >> 11);
  }*/


uint8_t getsyncbytefromheader(unsigned int ci)
{
  uint32_t temp = 0;
  unsigned int pos = 0;
  for(unsigned int i = 2; i < ci; i++) {
    unsigned int num = converttickstonum(capture[i]);
    for(unsigned int j = 0; j < num; j++) {
      if (i % 2 != 0) { //values[i] == 1
	temp |= 0x00000001 << pos;
      }
      pos++;
    }
  }

  temp &= 0x01FE;
  //std::cout << "<->getsync" << std::endl;
  return (uint8_t)(temp >> 1);
  }




uint8_t getpidbytefromheader(unsigned int ci)
{
  
  uint16_t templ = 0;
  uint16_t temph = 0;  
  unsigned int pos = 0;
  for(unsigned int i = 2; i < ci; i++) {
    unsigned int num = converttickstonum(capture[i]);
    for(unsigned int j = 0; j < num; j++) {
      if (i % 2 != 0) { //values[i] == 1
      if (pos >= 16) {
	    temph |= 0x0001 << (pos-16);
      }
      if (pos < 16) {
	    templ |= 0x0001 << pos;        
      }
      }
      pos++;
    }
  }

  templ &= 0xF800;
  temph &= 0x0007;
  templ >>= 11;
  temph <<= 5;
  
  //temph >> 11;
  //  std::cout << "<->getpid" << std::endl;
  return (uint8_t)(templ | temph);
}


uint8_t getbytefrommessage(unsigned int n, unsigned int ci)
{
  
  uint16_t temp0 = 0;
  uint16_t temp1 = 0;
  uint16_t temp2 = 0;
  /*uint16_t temp3 = 0;
  uint16_t temp4 = 0;
  uint16_t temp5 = 0;
  uint16_t temp6 = 0;
  uint16_t temp7 = 0;*/
  
  unsigned int pos = 0;
  for(unsigned int i = 2; i < ci; i++) {
    unsigned int num = converttickstonum(capture[i]);
    for(unsigned int j = 0; j < num; j++) {
      if (i % 2 != 0) {
	if (pos < 32 && pos >= 16) {
	  std::cout << std::dec << "i: " << i << " pos:" << pos << std::endl;  
	  std::cout << std::hex << (0x0001 << (pos-16)) << std::endl;
	  
	  temp0 |= (0x0001 << (pos-16));
	  std::cout << std::hex << temp0 << std::endl;
	  temp0 &= 0x1FE0;
	  std::cout << std::hex << temp0 << std::endl;
	  //temp0 >>= 5; 

	  
	}
	/*if (pos < 48 && pos >= 32) {
	  temp1 |= 0x0001 << (pos-32);
	}
	if (pos < 64 && pos >= 48) {
	  temp2 |= 0x0001 << (pos-48);
	}*/
	
      }
      pos++;
    }
  }

  if (n == 0) {
    std::cout << "result: "<< std::hex << temp0 << std::endl;
    temp0 &= 0x1FE0;
    
    return (uint8_t) (temp0 >> 5);
  }
  if (n == 1) {
    temp0 &= 0x8000;
    temp1 &= 0x003F;
    return (uint8_t)(temp0 >> 15 | temp1 << 1);
  }
  if (n == 2) {
    temp1 &= 0xFF00;
    return (uint8_t) temp1 >> 8;
  }
  
  return (uint8_t)0;
}


int main() {
    // Write C++ code here
    std::cout << "Try programiz.pro" << std::endl;
    
    for (unsigned int i = 0; i < bufsize; i++) {
        capture[i] = 0x0;
    }
    
    capture[0] = 108;
    capture[1] = 8;
    capture[2] = 8;
    capture[3] = 9;
    capture[4] = 8;
    capture[5] = 8;
    capture[6] = 9;
    capture[7] = 8;
    capture[8] = 8;
    capture[9] = 8;
    capture[10] = 9;
    capture[11] = 8;
    capture[12] = 9;
    capture[13] = 17;
    capture[14] = 17;
    capture[15] = 25;
    capture[16] = 9;
    capture[17] = 9;
    capture[18] = 8;
    capture[19] = 17;
    capture[20] = 17;
    capture[21] = 16;
    capture[22] = 17;
    capture[23] = 8;    

    
    
    
    int ci = 23;

    
    int temp1 = getsyncbytefromheader(ci);
    int temp2 = getpidbytefromheader(ci);
    int temp3 = getbytefrommessage(0, ci);
    
    std::cout << "temp1: " << std::hex << temp1 << std::endl; 
    std::cout << "temp2: "<< std::hex << temp2 << std::endl;
    std::cout << "temp3: "<< std::hex << temp3 << std::endl;


    return 0;
}



