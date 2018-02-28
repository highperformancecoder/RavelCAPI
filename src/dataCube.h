#ifndef DATACUBE_H
#define DATACUBE_H

#include "cda.h"
#include "rawData.h"

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>

#include <math.h>

#include <boost/any.hpp>

namespace ravel
{
  class Ravel;

  /// Specification of the input data
  /**
     nRowAxes and nColAxes are specify the area containing axis labels

     commentRows and commentCols are a set of rows and columns
     excluded from the data set.

     if multiple column axes exist, and a single data item exists on a row axis,in those column axes, then that data item is taken to be the label of the row axis:

     A line of blank row axis entries is treated as a sequence of column axis names

     rowAxisName  | c1 | c2 | ...|
     coln1 | coln2|              |
     d1v1  | d2v2 | .
                       .

    Otherwise those labels are treated as column axis headers
     

    dim1 | dim2 | dimN | c1 | c2 | ...|
    -------------------------------------         
    d1v1 | d2v1 | dNv1 | .
    d1v2 | d2v2 | dNv2 |   .
    ...
    d1vm | d2vm | dNvm |      .
  */

  struct DataSpec
  {
    /// size of the header area in rows and columns (alternatively,
    /// start row/col of data area
    size_t nRowAxes=0, nColAxes=0;

    /// rows and columns that are comment lines to be ignored
    std::set<unsigned> commentRows, commentCols;
  };

  /// interface to some raw 2D array
  struct Tokeniser
  {
    /// extract next line of data
    virtual std::vector<boost::any> getLine()=0;
  };

  struct CSVFTokeniser: public Tokeniser
  {
    std::istream& input;
    char separator;
    CSVFTokeniser(std::istream& i, char s): input(i), separator(s) {}
    // return a row of items in the spreadsheet
    std::vector<boost::any> getLine() override; 
    CSVFTokeniser& operator=(const CSVFTokeniser&) = delete;
  };

  

  // simple data cube representation
  class DataCube
  {
  public:
    struct SortBy
    {
      enum Type {none,byValue};
      Type type;
      unsigned rowCol; //if byValue
      bool reverse; //reverse sorting order
      SortBy(Type type=none, bool reverse=false, const unsigned rowCol=0):
        type(type), rowCol(rowCol), reverse(reverse) {}
    };

  protected:
    RawData rawData;
    std::vector<std::vector<std::string> > m_dimLabels; // dxvy below
    std::vector<SortBy> m_sortBy;
    
  private:
    size_t xh, yh;  // x & y handle Id (from last populateArray)

    /// break point between axes that appeared as columns and those that
    /// appear as rows in the imported data
    size_t leastRowAxis;

    double m_minVal, m_maxVal;

    size_t m_maxRow=0, m_maxCol=0;

    CLASSDESC_ACCESS(DataCube);
    void hyperSliceAfterPartialReductions(RawData&,Ravel&,const RawData&) const;
  public:
    // dimension names starting with column titles, followed by the
    // row axes, which are not inferred from the input data
    std::vector<std::string> dimNames;  

    const std::vector<std::vector<std::string> >  dimLabels() const
    {return m_dimLabels;}

    void renameAxis(size_t axis, const std::string& newDescription, Ravel& ravel)
    {
      if (axis>=ravel.handles.size()) throw std::runtime_error("invalid axis");
      rawData.renameAxis(ravel.handles[axis].description, newDescription);
      ravel.handles[axis].description = newDescription;
    }
    
    /// max row and max col on which non blank data exists
    size_t maxRow() const {return m_maxRow;}
    size_t maxCol() const {return m_maxCol;}

    /// minimum & maximum data values
    double minVal() const {return m_minVal;}
    double maxVal() const {return m_maxVal;}

    /// limits used for filtering output
    double filterMin=-std::numeric_limits<double>::max();
    double filterMax=std::numeric_limits<double>::max();

    /// whether the filter is active or not
    bool filterOn=false;

    /// sort output along \a axis by values in row or column \a rowCol
    void sortBy(size_t axis, int rowCol, int dir) {
      if (axis<m_sortBy.size()) 
        m_sortBy[axis]=SortBy(SortBy::byValue,dir==-1,rowCol);
    }

    /// revert to original sorting order for \a axis
    void unsortAxis(size_t axis) {
      if (axis<m_sortBy.size()) m_sortBy[axis]=SortBy(SortBy::none);
    }
    

    /// read file into memory according to CSV specification.
    void loadFile(const std::string& fileName, char separator, const DataSpec& spec);

    void loadData(Tokeniser& tok, const DataSpec& spec);

    /// generate an initial data spec based on input tokeniser
    DataSpec initDataSpec(Tokeniser& tok);

    /// initialise a Ravel object based on data loaded here
    void initRavel(Ravel&) const;

    /// return the hypersliced data corresponding the Ravel settings
    RawData hyperSlice(Ravel& r) const
    {RawData sd; hyperSlice(sd,r); return sd;}
    void hyperSlice(RawData&,Ravel&) const;
    
    /// populate the data array managed by the subclass of this, using
    /// the setDataElement() method. The array is assumed to be cleared
    /// prior to this call, as setDataElement is not called on undefined
    /// or empty elements
    void populateArray(Ravel&);

    /// applies sorting and filtering transformations to the data
    /// output via setDataElement. xHandle and yHandle refer to the
    /// axes that row and col index into
    void filterDataElement(size_t col, size_t row, double v);

    /// sets data element \a col, \a row to value \a v
    virtual void setDataElement(size_t col, size_t row, double v)=0;

    /// returns a histogram of populated array values for evenly
    /// distributed bins, filled by previous call the populateArray
    std::vector<unsigned> histogram=std::vector<unsigned>(20);

    /// number of finite data elements in raw data
    size_t numFinite() const {
      size_t count=0;
      for (size_t i=0; i<rawData.size(); ++i) count+=std::isfinite(rawData[i]);
      return count;
    }

    size_t size() const {return rawData.size();}
    size_t rank() const {return rawData.rank();}
  };
}

#ifdef ECOLAB_LIB
#include <TCL_obj_stl.h>
#endif

#if defined(CLASSDESC) || defined(ECOLAB_LIB)

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif

#include "dataCube.cd"

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
#endif
#endif
