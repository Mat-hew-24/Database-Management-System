#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
#include "cstring"
#include "iostream"

void stage1()
{
  unsigned char buffer[BLOCK_SIZE];
  char message[] = "hello";
  memcpy(buffer + 20, message, 6); // buffer[20] = 'h' , buffer[21] = 'e' , => 1st param, pointer is 2nd param is msg and 3rd param is its size => 'hello\0' is 6 bytes
  Disk::writeBlock(buffer, 7000);  // write the buffer into that block

  // PRINTING THE MESSAGE AFTER RETRIEVING FROM THE BLOCK WE JUST WROTE INTO
  unsigned char buffer2[BLOCK_SIZE];
  Disk::readBlock(buffer2, 7000);
  char message2[6];
  memcpy(message2, buffer2 + 20, 6);
  std::cout << message2 << "\n";
}

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

void stage2()
{

  // -------------------- create objects for the relation catalog and attribute catalog
  RecBuffer relCatBuffer(RELCAT_BLOCK);
  RecBuffer attrCatBuffer(ATTRCAT_BLOCK);

  HeadInfo relCatHeader;
  HeadInfo attrCatHeader;

  // ------------------------- load the headers of both the blocks into relCatHeader and attrCatHeader.
  // ------------------------- (we will implement these functions later)
  relCatBuffer.getHeader(&relCatHeader);
  attrCatBuffer.getHeader(&attrCatHeader);

  int no_of_relations = relCatHeader.numEntries;

  for (int i = 0; i < no_of_relations; i++)
  {
    Attribute relCatRecord[RELCAT_NO_ATTRS]; // ---------- will store the record from the relation catalog
    relCatBuffer.getRecord(relCatRecord, i);
    printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);

    int no_of_attr = attrCatHeader.numEntries;

    for (int j = 0; j < no_of_attr; j++)
    {

      Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
      attrCatBuffer.getRecord(attrCatRecord, j);

      if (strcmp(relCatRecord[RELCAT_REL_NAME_INDEX].sVal, attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal) == 0)
      {
        const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";
        printf("  %s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrType);
      }
    }
    printf("\n");
  }
}

void stage2qn1()
{

  // create objects for the relation catalog and attribute catalog
  RecBuffer relCatBuffer(RELCAT_BLOCK);
  HeadInfo relCatHeader;
  relCatBuffer.getHeader(&relCatHeader);

  // load the headers of both the blocks into relCatHeader and attrCatHeader.
  // (we will implement these functions later)

  int no_of_relations = relCatHeader.numEntries;

  for (int i = 0; i < no_of_relations; i++)
  {
    Attribute relCatRecord[RELCAT_NO_ATTRS]; // will store the record from the relation catalog
    relCatBuffer.getRecord(relCatRecord, i);
    printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);

    int attr_block = ATTRCAT_BLOCK;
    while (attr_block != -1)
    {
      RecBuffer attrCatBuffer(attr_block); // YOU DONT USE 'ATTRCAT_BLOCK' VAR COS YOU DONT WANNA REOPEN THE FIRST BLOCK AGAIN N AGAIN
      HeadInfo attrCatHeader;
      attrCatBuffer.getHeader(&attrCatHeader);
      int no_of_attr = attrCatHeader.numEntries;

      for (int j = 0; j < no_of_attr; j++)
      {

        Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
        attrCatBuffer.getRecord(attrCatRecord, j);

        if (strcmp(relCatRecord[RELCAT_REL_NAME_INDEX].sVal, attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal) == 0)
        {
          const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";
          printf("  %s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrType);
        }
      }
      attr_block = attrCatHeader.rblock;
    }
    printf("\n");
  }
}

void stage2qn2()
{
  int blk = ATTRCAT_BLOCK;

  RecBuffer buffer(blk);
  HeadInfo head;
  buffer.getHeader(&head);

  for (int i = 0; i < head.numEntries; i++) // header contains the metadata, but that metadata contains attribute named numEntries, which has the no. of records
  {
    Attribute rec[ATTRCAT_NO_ATTRS];
    buffer.getRecord(rec, i);

    if (strcmp(rec[ATTRCAT_REL_NAME_INDEX].sVal, "Students") == 0 &&
        strcmp(rec[ATTRCAT_ATTR_NAME_INDEX].sVal, "Class") == 0)
    {
      strcpy(rec[ATTRCAT_ATTR_NAME_INDEX].sVal, "Batch"); // you can only change attribute name
      buffer.setRecord(rec, i);

      printf("Updated attribute name Class -> Batch\n");
    }
  }
}

int main(int argc, char *argv[])
{
  /*--------- Initialize the Run Copy of Disk ---------- */
  Disk disk_run;

  /*---------- StaticBuffer buffer;  --------*/
  // stage1();

  // BLOCK ALLOCATION MAP QN verification
  // stage1qn1();

  // stage2();

  // PRINTING SCHEMA QN
  // stage2qn1();

  // SCHEMA UPDATION QN
  // stage2qn1();

  return 0;
  /*------------- OpenRelTable cache ------------ */;

  // return FrontendInterface::handleFrontend(argc, argv); //later we will come back here
}
