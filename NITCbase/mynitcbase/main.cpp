#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
#include "cstring"
#include "iostream"

// -------- MEANS IT WAS ALREADY THERE (THE COMMENT)
void stage1qn1()
{
  std::cout << "\n--- BLOCK ALLOCATION MAP VALUES ---\n";
  unsigned char buffer[BLOCK_SIZE];
  int blockCount = 0;
  for (int blk = 0; blk < 4; blk++) // BAM = first 4 blocks
  {
    Disk::readBlock(buffer, blk);
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
      std::cout << (int)buffer[i] << " ";
      blockCount++;
      if (blockCount >= 500) // print first few only
        break;
    }
    if (blockCount >= 6000)
      break;
  }
}

int main(int argc, char *argv[])
{
  /*--------- Initialize the Run Copy of Disk ---------- */
  Disk disk_run;

  /*---------- StaticBuffer buffer;  --------*/
  unsigned char buffer[BLOCK_SIZE];
  Disk::readBlock(buffer, 7000); // read the block into buffer
  char message[] = "hello";
  memcpy(buffer + 20, message, 6); // buffer[20] = 'h' , buffer[21] = 'e' , => 1st param, pointer is 2nd paraam and 3rd param is its size => 'hello\0' is 6 bytes
  Disk::writeBlock(buffer, 7000);  // write the buffer into that block

  // PRINTING THE MESSAGE AFTER RETRIEVING FROM THE BLOCK WE JUST WROTE INTO
  unsigned char buffer2[BLOCK_SIZE];
  Disk::readBlock(buffer2, 7000);
  char message2[6];
  memcpy(message2, buffer2 + 20, 6);
  std::cout << message2;

  // BLOCK ALLOCATION MAP QN verification
  // stage1qn1();

  return 0;
  /*------------- OpenRelTable cache ------------ */;

  // return FrontendInterface::handleFrontend(argc, argv); //later we will come back here
}
