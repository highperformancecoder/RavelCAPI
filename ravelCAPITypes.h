/*
  Ravel C API. © Ravelation Pty Ltd 2020
  Open source licensed under the MIT license.
*/

#ifndef RAVELCAPITYPES_H
#define RAVELCAPITYPES_H

#define RAVEL_CAPI_VERSION 10


#ifdef __cplusplus
#include "dimension.h"
namespace ravel
{
  struct HandleState;
  struct RavelState;
}
#define BOOL bool
#else
#define BOOL int
#endif

enum RavelReductionOp {ravel_sum, ravel_prod, ravel_av, ravel_stddev, ravel_min, ravel_max};

/// enum describing the sorting properties of handle
enum RavelOrder {ravel_none, ravel_forward, ravel_reverse, ravel_custom, ravel_static_forward, ravel_static_reverse, ravel_dynamic_forward, ravel_dynamic_reverse };

// interface, for use on CAPI
struct CAPIRavelHandleState
{
  double x,y; ///< handle tip coordinates (only angle important, not length)
  BOOL collapsed, displayFilterCaliper;
  BOOL customOrderIsInverted; ///< if true, then customOrder is slices not selected
  enum RavelReductionOp reductionOp;
  enum RavelOrder order;
  
  // To manage the memory pointed to by these pointers, best
  // practice is to extend this class, and ensure these pointers are
  // updated into internally managed memory. Lifetime of the
  // overall object needs to be clarified on the API. Beware object slicing.
  const char* description;
  const char* minLabel;
  const char* maxLabel;
  const char* sliceLabel;
  const char* format;
  const char** customOrder; ///< used if order==custom, null terminated

#ifdef __cplusplus
  explicit CAPIRavelHandleState(double x=0, double y=0, bool collapsed=false,
                       bool displayFilterCaliper=false, RavelReductionOp reductionOp=ravel_sum,
                                RavelOrder order=ravel_none,bool customOrderIsInverted=false):
    x(x), y(y), collapsed(collapsed), displayFilterCaliper(displayFilterCaliper),
    reductionOp(reductionOp), order(order),
    description(nullptr), minLabel(nullptr), maxLabel(nullptr), sliceLabel(nullptr),
    customOrder(nullptr), customOrderIsInverted(customOrderIsInverted) {}
  /// initialises just the simple data members
  CAPIRavelHandleState(const ravel::HandleState& state);
#endif
};

typedef struct CAPIRavelHandleState CAPIRavelHandleState;

struct CAPIRavelState
{
  double radius;
  const CAPIRavelHandleState** handleStates; ///< null terminated list of handle states
  const char** outputHandles; ///< null terminated list of output handles

#ifdef __cplusplus
  CAPIRavelState(double radius=100):
    radius(radius), handleStates(nullptr), outputHandles(nullptr) {}
  /// initialises just the simple data members
  CAPIRavelState(const ravel::RavelState& state);
#endif
};

typedef struct CAPIRavelState CAPIRavelState;

// types for CSV handling
enum CAPIRavelDuplicateKey {ravel_throwException, ravel_dupFirst, ravel_dupSum, ravel_dupProduct, ravel_dupMin, ravel_dupMax, ravel_dupAv};
enum CAPIRavelDimensionType {ravel_string,ravel_time,ravel_value};

struct CAPIRavelDimension
{
  enum CAPIRavelDimensionType type; ///< type of dimension
  const char* format;               ///< format string for ingesting value from strings
  const char* name;                 ///< name of dimension
#ifdef __cplusplus
  CAPIRavelDimension(CAPIRavelDimensionType type, const char* format, const char* name):
    type(type), format(format), name(name) {}
#endif
};
typedef struct CAPIRavelDimension CAPIRavelDimension;

/// describe the structure of a CSV file. 
struct CAPIRavelDataSpec
{
  char separator;       ///< field separator character
  char quote;           ///< quote character
  char escape;          ///< escape character, might be backslash, technically shouldn't be used for CSV
  char decSeparator;    ///< decimal "point", usually '.' or ','.
  /// number of columns this spec describes. Any additional columns
  /// are assumed to be data columns, and named by the header row cell.
  int numCols;       
  int dataRowOffset;    ///< start of the data section
  int headerRow;        ///< index of header row
  int mergeDelimiters; ///< if true, multiple separator characters are merged (eg space delimited files)
  int counter;         ///< count data items, not read their values
  int dontFail;        ///< do not throw an error on corrupt data, just ignore the data

  int numAxes;          ///< size of dimensionCols set
  int numData;          ///< size of  dataCols set
  int unusedInt;        ///< padding for long int alignment
  int* dimensionCols;   ///< set of columns that are dimensions, of size numAxes. Note dimensionCols ∩ dataCols = ∅
  int* dataCols;        ///< set of columns that are data, of size numData. Note dimensionCols ∩ dataCols = ∅
  CAPIRavelDimension* dimensions; ///< dimension vector of size numCols
 
#ifdef __cplusplus
  CAPIRavelDataSpec(): numCols(0), dataRowOffset(1), headerRow(0),
                       separator(','), quote('"'), escape('\0'),
                       decSeparator('.'),
                       mergeDelimiters(false), counter(false), dontFail(false),
                       numAxes(0), dimensionCols(nullptr),
                       numData(0), dataCols(nullptr),
                       dimensions(nullptr)
  {}
#endif
};

typedef struct CAPIRavelDataSpec CAPIRavelDataSpec;



#ifdef __cplusplus
#include <string>
#include <vector>
#if defined(CLASSDESC) || defined(ECOLAB_LIB)
#include <classdesc_access.h>
#else
#define CLASSDESC_ACCESS(x)
#endif

namespace ravel
{
  /// extra C++ fields for RavelHandleStateX
  class HandleX
  {
  public:
    HandleX() {}
    HandleX(const ravel::HandleState&);

  protected:
    CLASSDESC_ACCESS(HandleX);
    std::string m_description;
    std::vector<std::string> m_customOrder;
    std::string m_format;
    std::string m_minLabel, m_maxLabel, m_sliceLabel;
  };

  /// convenience class wrapping C++ RAII types and setting up pointers
  
  class RavelHandleStateX: public CAPIRavelHandleState, private HandleX
  {
  public:
    RavelHandleStateX() {}
    RavelHandleStateX(const ravel::HandleState& state):
      CAPIRavelHandleState(state), HandleX(state) {setupPointers();}
     RavelHandleStateX(const ravel::RavelHandleStateX& state):
      CAPIRavelHandleState(state), HandleX(state) {setupPointers();}
    RavelHandleStateX& operator=(const RavelHandleStateX& x) {
      CAPIRavelHandleState::operator=(x);
      HandleX::operator=(x);
      setupPointers();
      return *this;
    }
      
   
  private:
    CLASSDESC_ACCESS(RavelHandleStateX);
    std::vector<const char*> customOrderStrings;
    void setupPointers() {
      customOrderStrings.clear();
      for (auto& i: m_customOrder)
        customOrderStrings.push_back(i.c_str());
      // correct for a previous beta implementation that was misguided
      // custom ordering cannot have an empty customOrder
      if (m_customOrder.empty())
        switch (order)
          {
          case ravel_custom: order=ravel_none; break;
          case ravel_static_forward: case ravel_dynamic_forward: order=ravel_forward; break;
          case ravel_static_reverse: case ravel_dynamic_reverse: order=ravel_reverse; break;
          default: break;
          }
      customOrderStrings.push_back(nullptr);
      customOrder=customOrderStrings.data();
      format=m_format.c_str();
      minLabel=m_minLabel.c_str();
      maxLabel=m_maxLabel.c_str();
      sliceLabel=m_sliceLabel.c_str();
      description=m_description.c_str();
    }
  };

  class StateX
  {
  public:
    StateX() {}
    StateX(const ravel::RavelState& state);
  protected:
    CLASSDESC_ACCESS(StateX);
    std::vector<RavelHandleStateX> m_handleStates;
    std::vector<std::string> m_outputHandles;
  };
  
  /// convenience class wrapping C++ RAII types and setting up pointers
  class RavelStateX: public CAPIRavelState, private StateX
  {
  public:
    RavelStateX() {}
    RavelStateX(const ravel::RavelState& x): CAPIRavelState(x), StateX(x) {setupPointers();}
    RavelStateX(const RavelStateX& x): CAPIRavelState(x), StateX(x) {setupPointers();}
    RavelStateX& operator=(const RavelStateX& x) {
      CAPIRavelState::operator=(x);
      StateX::operator=(x);
      setupPointers();
      return *this;
    }
   
  private:
    CLASSDESC_ACCESS(RavelStateX);
    std::vector<const CAPIRavelHandleState*> handleStatePtrs;
    std::vector<const char*> outputHandlePtrs;
    void setupPointers() {
      handleStatePtrs.clear();
      for (auto& i: m_handleStates)
        handleStatePtrs.push_back(&i);
      handleStatePtrs.push_back(nullptr);
      handleStates=handleStatePtrs.data();
      outputHandlePtrs.clear();
      for (auto& i: m_outputHandles)
        outputHandlePtrs.push_back(i.c_str());
      outputHandlePtrs.push_back(nullptr);
      outputHandles=outputHandlePtrs.data();
    }
  };

  struct DataSpec;
  
  class RavelDataSpec: public CAPIRavelDataSpec
  {
  public:
    RavelDataSpec() {}
    RavelDataSpec(const DataSpec& x);
  private:
    CLASSDESC_ACCESS(RavelDataSpec);
    std::vector<int> m_dimCols;
    std::vector<int> m_dataCols;
    std::vector<CAPIRavelDimension> dimensionPtrs;
    std::vector<civita::NamedDimension> dimensionData;
    void setupPtrs();
  };
  
}

#if defined(CLASSDESC) || defined(ECOLAB_LIB)
#include "ravelCAPITypes.cd"
#endif

#endif // __cplusplus

#endif
