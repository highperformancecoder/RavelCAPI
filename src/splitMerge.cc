#include "splitMerge.h"
#include "ravelError.h"
#ifdef CLASSDESC
#include <classdesc_epilogue.h>
#endif
#ifdef ECOLAB_LIB
#include <ecolab_epilogue.h>
#endif
#include <string>

using namespace std;

namespace ravel
{
  size_t SplitMerge::splitPoint() const
  {
    if (elem.empty())
      throw RavelError("No data area");

    vector<bool> mask(elem[0].size(), true);
    for (unsigned i=1; i<elem.size(); ++i)
      for (size_t j=0; j<elem[0].size(); ++j)
        mask[j] = mask[j] && elem[0].size()==elem[i].size() && 
          elem[0][j]==elem[i][j];
    
    // ignoring any constant leadin, find the split point
    size_t splitPt=0;
    for (size_t i=1; i+1<mask.size(); ++i)
      if (mask[i]&&!mask[i-1])
        {
          splitPt=i;
          break;
        }
    return splitPt;
  }

  void SplitMerge::splitRow(size_t idx)
  {
    // determine characters that remain constant in each element
    elem.clear();
    for (unsigned i=startCol(); i<numCols(); ++i)
      elem.push_back(get(idx, i));

    size_t splitPt=splitPoint();

    if (splitPt>0)
      {
        insertRows(idx,1);
        for (size_t i=0; i<elem.size(); ++i)
          {
            set(idx,i+startCol(),elem[i].substr(0,splitPt));
            set(idx+1,i+startCol(),elem[i].substr(splitPt));
          }
      }
  }
  
  void SplitMerge::splitCol(size_t idx)
  {
    // determine characters that remain constant in each element
    elem.clear();
    for (unsigned i=startRow(); i<numRows(); ++i)
      elem.push_back(get(i, idx));

    size_t splitPt=splitPoint();

    if (splitPt>0)
      {
        insertCols(idx,1);
        for (size_t i=0; i<elem.size(); ++i)
          {
            set(i+startRow(),idx,elem[i].substr(0,splitPt));
            set(i+startRow(),idx+1, elem[i].substr(splitPt));
          }
      }
  }
  


  void SplitMerge::mergeRows(size_t idx, size_t num)
  {
    // check if latter rows have constant leadin
    vector<bool> constLeadin(num-1,true);
    for (size_t i=idx+1; i<idx+num; ++i)
      for (size_t j=startCol()+1; j<numCols(); ++j)
        constLeadin[i-idx-1] = constLeadin[i-idx-1] && get(i,j)[0]==get(i,j-1)[0];

    vector<string> newRow(numCols()-startCol());
    for (unsigned i=idx; i<num; ++i)
      for (unsigned j=startCol(); j<numCols(); ++j)
        {
          if (i>idx && !constLeadin[i-idx-1])
            newRow[j-startCol()]+='.';
          newRow[j-startCol()]+=get(i,j);
        }
    
    for (unsigned i=startCol(); i<numCols(); ++i)
      set(idx, i,newRow[i-startCol()]);
    
    deleteRows(idx+1,num-1);
  }

  void SplitMerge::mergeCols(size_t idx, size_t num)
  {
   // check if latter cols have constant leadin
    vector<bool> constLeadin(num-1,true);
    for (size_t i=idx+1; i<idx+num; ++i)
      for (size_t j=startRow()+1; j<numRows(); ++j)
        constLeadin[i-idx-1] = constLeadin[i-idx-1] && get(j,i)[0]==get(j-1,i)[0];

    vector<string> newCol(numRows()-startRow());
    for (unsigned i=idx; i<num; ++i)
      for (unsigned j=startRow(); j<numRows(); ++j)
        {
          if (i>idx && !constLeadin[i-idx-1])
            newCol[j-startRow()]+='.';
          newCol[j-startRow()]+=get(j,i);
        }
    
    for (unsigned i=startRow(); i<numRows(); ++i)
      set(i,idx,newCol[i-startRow()]);
    
    deleteCols(idx+1,num-1);
  }



}
