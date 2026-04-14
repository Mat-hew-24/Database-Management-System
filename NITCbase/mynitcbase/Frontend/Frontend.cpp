#include "Frontend.h"
#include "../Buffer/StaticBuffer.h"
#include <cstring>
#include <iostream>

int Frontend::create_table(char relname[ATTR_SIZE], int no_attrs, char attributes[][ATTR_SIZE],
                           int type_attrs[])
{
  return Schema::createRel(relname, no_attrs, attributes, type_attrs);
}

int Frontend::drop_table(char relname[ATTR_SIZE])
{
  return Schema::deleteRel(relname);
}

int Frontend::open_table(char relname[ATTR_SIZE]) // ? %
{
  // ? Schema::openRel
  return Schema::openRel(relname);
}

int Frontend::close_table(char relname[ATTR_SIZE]) // ? %
{
  // Schema::closeRel
  return Schema::closeRel(relname);
}

int Frontend::alter_table_rename(char relname_from[ATTR_SIZE], char relname_to[ATTR_SIZE])
{
  // ? Schema::renameRel
  return Schema::renameRel(relname_from, relname_to);
}

int Frontend::alter_table_rename_column(char relname[ATTR_SIZE], char attrname_from[ATTR_SIZE],
                                        char attrname_to[ATTR_SIZE])
{
  // ? Schema::renameAttr
  return Schema::renameAttr(relname, attrname_from, attrname_to);
}

int Frontend::create_index(char relname[ATTR_SIZE], char attrname[ATTR_SIZE])
{
  // Schema::createIndex
  return Schema::createIndex(relname, attrname);
}

int Frontend::drop_index(char relname[ATTR_SIZE], char attrname[ATTR_SIZE])
{
  // Schema::dropIndex
  return Schema::dropIndex(relname, attrname);
}

int Frontend::insert_into_table_values(char relname[ATTR_SIZE], int attr_count, char attr_values[][ATTR_SIZE])
{
  // Algebra::insert
  return Algebra::insert(relname, attr_count, attr_values);
}

int Frontend::select_from_table(char relname_source[ATTR_SIZE], char relname_target[ATTR_SIZE])
{
  // Algebra::project
  return Algebra::project(relname_source, relname_target);
}

int Frontend::select_attrlist_from_table(char relname_source[ATTR_SIZE], char relname_target[ATTR_SIZE],
                                         int attr_count, char attr_list[][ATTR_SIZE])
{
  // Algebra::project
  return Algebra::project(relname_source, relname_target, attr_count, attr_list);
}

int Frontend::select_from_table_where(char relname_source[ATTR_SIZE], char relname_target[ATTR_SIZE],
                                      char attribute[ATTR_SIZE], int op, char value[ATTR_SIZE])
{
  // Algebra::select
  return Algebra::select(relname_source, relname_target, attribute, op, value);
}

int Frontend::select_attrlist_from_table_where(char relname_source[ATTR_SIZE], char relname_target[ATTR_SIZE],
                                               int attr_count, char attr_list[][ATTR_SIZE],
                                               char attribute[ATTR_SIZE], int op, char value[ATTR_SIZE])
{
  // Algebra::select + Algebra::project??
  int ret = Algebra::select(relname_source, TEMP, attribute, op, value);
  if (ret != SUCCESS)
    return ret;
  int relId = OpenRelTable::openRel(TEMP);
  if (relId < 0)
  {
    Schema::deleteRel(TEMP);
    return relId;
  }
  ret = Algebra::project(TEMP, relname_target, attr_count, attr_list);
  Schema::closeRel(TEMP);
  Schema::deleteRel(TEMP);
  return ret;
}

int Frontend::select_from_join_where(char relname_source_one[ATTR_SIZE], char relname_source_two[ATTR_SIZE],
                                     char relname_target[ATTR_SIZE],
                                     char join_attr_one[ATTR_SIZE], char join_attr_two[ATTR_SIZE])
{
  // Algebra::join
  return Algebra::join(relname_source_one, relname_source_two, relname_target, join_attr_one, join_attr_two);
}

int Frontend::select_attrlist_from_join_where(char relname_source_one[ATTR_SIZE], char relname_source_two[ATTR_SIZE],
                                              char relname_target[ATTR_SIZE],
                                              char join_attr_one[ATTR_SIZE], char join_attr_two[ATTR_SIZE],
                                              int attr_count, char attr_list[][ATTR_SIZE])
{
  // Algebra::join + project
  int ret = Algebra::join(relname_source_one, relname_source_two, TEMP, join_attr_one, join_attr_two);
  if (ret != SUCCESS)
    return ret;
  int relId = OpenRelTable::openRel(TEMP);
  if (relId < 0)
  {
    Schema::deleteRel(TEMP);
    return relId;
  }
  Algebra::project(TEMP, relname_target, attr_count, attr_list);
  OpenRelTable::closeRel(relId);
  Schema::deleteRel(TEMP);
  return SUCCESS;
}

int Frontend::custom_function(int argc, char argv[][ATTR_SIZE])
{
  if (argc == 3 && strcmp(argv[0], "MIN") == 0)
    return Frontend::select_min_attr_from_table(argv[1], argv[2]);
  else if (argc == 3 && strcmp(argv[0], "MAX") == 0)
    return Frontend::select_max_attr_from_table(argv[1], argv[2]);
  else if (argc == 3 && strcmp(argv[0], "AVG") == 0)
    return Frontend::select_avg_attr_from_table(argv[1], argv[2]);
  else if (argc == 5 && strcmp(argv[0], "DELETE") == 0)
  {
    char *relname = argv[1];
    char *attrname = argv[2];
    char *op_str = argv[3];
    char *value = argv[4];
    int op = -1;
    if (strcmp(op_str, "EQ") == 0)
      op = EQ;
    else if (strcmp(op_str, "LT") == 0)
      op = LT;
    else if (strcmp(op_str, "LE") == 0)
      op = LE;
    else if (strcmp(op_str, "GT") == 0)
      op = GT;
    else if (strcmp(op_str, "GE") == 0)
      op = GE;
    else if (strcmp(op_str, "NE") == 0)
      op = NE;
    else
      return E_INVALID;
    return Frontend::delete_from_table(relname, attrname, op, value);
  }
  else if (argc == 1 && strcmp(argv[0], "PRINT_BLOCKS") == 0)
    return Frontend::print_blocks();
  else if (argc == 1 && strcmp(argv[0], "PRINT_SLT") == 0)
    return Frontend::print_all_slotMap();
  return E_INVALID;
}

int Frontend::select_min_attr_from_table(char relname[ATTR_SIZE], char attrName[ATTR_SIZE])
{
  return Algebra::Aggregate(relname, attrName, "MIN");
}

int Frontend::select_max_attr_from_table(char relname[ATTR_SIZE], char attrName[ATTR_SIZE])
{
  return Algebra::Aggregate(relname, attrName, "MAX");
}

int Frontend::select_avg_attr_from_table(char relname[ATTR_SIZE], char attrName[ATTR_SIZE])
{
  return Algebra::Aggregate(relname, attrName, "AVG");
}

int Frontend::delete_from_table(char relname[ATTR_SIZE], char attrname[ATTR_SIZE], int op, char *value)
{
  return Algebra::deleterows(relname, attrname, op, value);
}

int Frontend::print_blocks()
{
  for (int i = 0; i < DISK_BLOCKS; i++)
  {
    if (i >= 0 && i <= 3)
      continue;
    int type = StaticBuffer::getStaticBlockType(i);
    if (type == UNUSED_BLK)
      continue;
    std::cout << "\nBlock " << i << " ";
    if (type == REC)
      std::cout << "[RECORD BLOCK]\n";
    else if (type == IND_INTERNAL)
      std::cout << "[INDEX INTERNAL]\n";
    else if (type == IND_LEAF)
      std::cout << "[INDEX LEAF]\n";
    BlockBuffer blk(i);
    HeadInfo head;
    blk.getHeader(&head);
    std::cout << "Entries: " << head.numEntries
              << " | Attrs: " << head.numAttrs
              << " | Slots: " << head.numSlots << "\n";
    if (type == REC)
    {
      RecBuffer recBuf(i);
      int slotCount = head.numSlots;
      int attrCount = head.numAttrs;
      unsigned char slotMap[slotCount];
      recBuf.getSlotMap(slotMap);

      for (int s = 0; s < slotCount; s++)
      {
        if (slotMap[s] == SLOT_UNOCCUPIED)
          continue;
        union Attribute rec[attrCount];
        recBuf.getRecord(rec, s);
        std::cout << "Slot " << s << ": ";

        for (int a = 0; a < attrCount; a++)
          std::cout << "[" << rec[a].sVal << " | " << rec[a].nVal << "] ";
        std::cout << "\n";
      }
    }
  }

  return SUCCESS;
}

int Frontend::print_all_slotMap()
{
  int bmap_values[DISK_BLOCKS];
  unsigned char buffer[BLOCK_SIZE];
  int blockCount = 0;

  for (int i = 0; i < 4; i++)
  {
    Disk::readBlock(buffer, i);
    for (int j = 0; j < BLOCK_SIZE; j++)
      bmap_values[blockCount++] = buffer[j];
  }
  for (int i = 0; i < DISK_BLOCKS; i++)
  {
    if (bmap_values[i] != UNUSED_BLK && StaticBuffer::getStaticBlockType(i) == REC)
    {
      RecBuffer buffer(i);
      struct HeadInfo head;
      buffer.getHeader(&head);
      unsigned char slotMap[head.numSlots];
      buffer.getSlotMap(slotMap);
      std::cout << "BLOCK : " << i << " | \n";
      for (int j = 0; j < head.numSlots; j++)
        std::cout << "  SLOT : " << j << " => " << (slotMap[j] == SLOT_OCCUPIED ? "OCCUPIED" : "UNOCCUPIED") << "\n";
    }
  }

  return SUCCESS;
}
