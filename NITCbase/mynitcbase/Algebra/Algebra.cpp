#include "Algebra.h"
#include <cstring>
#include <stdio.h>
#include <stdlib.h>

/* used to select all the records that satisfy a condition.
the arguments of the function are
- srcRel - the source relation we want to select from
- targetRel - the relation we want to select into. (ignore for now)
- attr - the attribute that the condition is checking
- op - the operator of the condition
- strVal - the value that we want to compare against (represented as a string)
*/

// ? $
// ? will return if a string can be parsed as a floating point number
bool isNumber(char *str)
{
  int len;
  float ignore;
  /*
    sscanf returns the number of elements read, so if there is no float matching
    the first %f, ret will be 0, else it'll be 1

    %n gets the number of characters read. this scanf sequence will read the
    first float ignoring all the whitespace before and after. and the number of
    characters read that far will be stored in len. if len == strlen(str), then
    the string only contains a float with/without whitespace. else, there's other
    characters.
  */
  int ret = sscanf(str, "%f %n", &ignore, &len);
  return ret == 1 && len == strlen(str);
}

// ! ........................................................................................................
int Algebra::select(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], char attr[ATTR_SIZE], int op, char strVal[ATTR_SIZE])
{
  int srcRelId = OpenRelTable::getRelId(srcRel); // we'll implement this later
  if (srcRelId == E_RELNOTOPEN)
    return E_RELNOTOPEN;
  AttrCatEntry attrCatEntry;
  // get the attribute catalog entry for attr, using AttrCacheTable::getAttrcatEntry()
  //    return E_ATTRNOTEXIST if it returns the error
  if (AttrCacheTable::getAttrCatEntry(srcRelId, attr, &attrCatEntry) != SUCCESS)
    return E_ATTRNOTEXIST;

  /*** Convert strVal (string) to an attribute of data type NUMBER or STRING ***/
  int type = attrCatEntry.attrType;
  Attribute attrVal;
  if (type == NUMBER)
  {
    if (!isNumber(strVal))
      return E_ATTRTYPEMISMATCH;
    attrVal.nVal = atof(strVal);
  }
  else
    strcpy(attrVal.sVal, strVal);

  /*** Selecting records from the source relation ***/

  // Before calling the search function, reset the search to start from the first hit
  // using RelCacheTable::resetSearchIndex()
  RelCacheTable::resetSearchIndex(srcRelId);
  RelCatEntry relCatEntry;
  // get relCatEntry using RelCacheTable::getRelCatEntry()
  if (RelCacheTable::getRelCatEntry(srcRelId, &relCatEntry) != SUCCESS)
    return E_RELNOTOPEN;
  /************************
  The following code prints the contents of a relation directly to the output
  console. Direct console output is not permitted by the actual the NITCbase
  specification and the output can only be inserted into a new relation. We will
  be modifying it in the later stages to match the specification.
  ************************/

  printf("|");
  for (int i = 0; i < relCatEntry.numAttrs; ++i)
  {
    AttrCatEntry attrCatEntry;
    if (AttrCacheTable::getAttrCatEntry(srcRelId, i, &attrCatEntry) != SUCCESS) // ? get attrCatEntry at offset i using AttrCacheTable::getAttrCatEntry()
      return E_ATTRNOTEXIST;

    printf(" %s |", attrCatEntry.attrName);
  }
  printf("\n");
  // ? fetch and print matching records
  while (true)
  {
    RecId recId = BlockAccess::linearSearch(srcRelId, attr, attrVal, op);
    if (recId.block != -1 && recId.slot != -1)
    {
      RecBuffer buffer(recId.block);
      Attribute record[relCatEntry.numAttrs];
      buffer.getRecord(record, recId.slot); // ? get the record at searchRes using BlockBuffer.getRecord
      // ? print the attribute values in the same format as above
      printf("|");
      for (int i = 0; i < relCatEntry.numAttrs; i++)
      {
        AttrCatEntry attrCatEntry;
        if (AttrCacheTable::getAttrCatEntry(srcRelId, i, &attrCatEntry) != SUCCESS) // ? get attrCatEntry at offset i using AttrCacheTable::getAttrCatEntry()
          return E_ATTRNOTEXIST;

        if (attrCatEntry.attrType == NUMBER)
          printf(" %g |", record[i].nVal);
        else
          printf(" %s |", record[i].sVal);
      }
      printf("\n");
    }
    else
      break; // (all records over)
  }

  return SUCCESS;
}
// ! ........................................................................................................
