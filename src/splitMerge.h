/// Algorithm for splitting/merging axis rows/columns
#ifndef SPLITMERGE_H
#define SPLITMERGE_H
#include <string>
#include <vector>
#if defined(CLASSDESC) || defined(ECOLAB_LIB)
#include <classdesc_access.h>
#else
#define CLASSDESC_ACCESS(x)
#endif

namespace ravel
{
  enum ColRow { col, row };

  class SplitMerge
  {
    std::vector<std::string> elem;
    // ignoring any constant leadin, find the split point
    size_t splitPoint() const;
    CLASSDESC_ACCESS(SplitMerge);
  public:
    /// get the contents of cell located at (row, col)
    virtual std::string get(size_t row, size_t col) const=0;
    /// set the cell (row, col)
    virtual void set(size_t row, size_t col, const std::string& val)=0;
    /// start row of the data area
    virtual size_t startRow() const=0;
    /// start column of the data area
    virtual size_t startCol() const=0;
    /// number of rows
    virtual size_t numRows() const=0;
    /// number of columns
    virtual size_t numCols() const=0;
    /// insert \a n rows after row \a r
    virtual void insertRows(size_t r, size_t n)=0;
    /// insert \a n cols after \a c
    virtual void insertCols(size_t c, size_t n)=0;
    /// delete \a n rows after row \a r
    virtual void deleteRows(size_t r, size_t n)=0;
    /// delete \a n cols after \a c
    virtual void deleteCols(size_t c, size_t n)=0;

    virtual ~SplitMerge() {}

    /// split the \a idx row
    void splitRow(size_t idx);
    /// split the \a idx column
    void splitCol(size_t idx);
    /// merge rows between \a idx and \a idx+num
    void mergeRows(size_t idx, size_t num);
    /// merge columns between \a idx and \a idx+num
    void mergeCols(size_t idx, size_t num);
  };
}

#if defined(CLASSDESC) || defined(ECOLAB_LIB)
#include "splitMerge.cd"
#endif

#endif
