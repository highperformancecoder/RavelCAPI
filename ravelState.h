/*
  Ravel C API. © Ravelation Pty Ltd 2020
*/

#ifndef RAVELSTATE_H
#define RAVELSTATE_H
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <dimension.h>
#include "CSVTools.h"

struct CAPIRavelState;
struct CAPIRavelHandleState;
struct CAPIRavelDataSpec;

namespace civita
{
  class ITensor;
  using TensorPtr=std::shared_ptr<ITensor>;
}

namespace ravel
{
  static const double ravelDefaultRadius=100; ///< initial size of a Ravel widget

  /// utility for converting between compatible enums
  template <class T, class U> inline T toEnum(U x) {return T(int(x));}

  struct Op
  {
    enum ReductionOp {sum, prod, av, stddev, min, max};
  };

  /// enum describing the sorting properties of handle
  struct HandleSort
  {
    // num* and time* deprecated here
    
    // static* and dynamic* is the same as custom as far as Ravel is
    // concerned, but may be used by clients to implement dynamic
    // sorting. All custom ordering enumerators must be declared after custom.
    enum Order {none, forward, reverse, custom, staticForward, staticReverse, dynamicForward, dynamicReverse };
  };

  // representing the state of the handles
  struct HandleState
  {
    std::string description;
    double x=ravelDefaultRadius, y=0; ///< handle tip coordinates (only angle important, not length)
    bool collapsed=false, displayFilterCaliper=false;
    Op::ReductionOp reductionOp=Op::sum;
    HandleSort::Order order=HandleSort::none;
    std::string format; // for deprecated time* sort orders
    /// if true, then customOrder is slices not selected. Deprecated,
    /// supported just for old rvl files
    bool customOrderIsInverted=false; 
    std::vector<std::string> customOrder, customOrderComplement; 
    std::string minLabel, maxLabel, sliceLabel;
    HandleState() {}
    HandleState(const CAPIRavelHandleState& state);
#if defined(__cplusplus) && __cplusplus >= 202002L
#ifndef __APPLE__
    auto operator<=>(const HandleState&) const = default;
#endif
    bool operator==(const HandleState&) const = default;
#endif
  };

  /// represents the full Ravel state
  struct RavelState
  {
    double radius=ravelDefaultRadius;
    std::vector<HandleState> handleStates;
    std::vector<std::string> outputHandles;
    RavelState() {}
    RavelState(const CAPIRavelState& );
      
    bool empty() const {return handleStates.empty();}
    void clear() {
      handleStates.clear();
      outputHandles.clear();
    }
#if defined(__cplusplus) && __cplusplus >= 202002L
#ifndef __APPLE__
    auto operator<=>(const RavelState&) const = default;
#endif
    bool operator==(const RavelState&) const = default;
#endif
  };

  struct DuplicateKeyAction
  {
    enum Type {throwException, first, sum, product, min, max, av};
  };
  
  struct DataSpec: public CSVSpec
  {
    int dataRowOffset=1;    ///< start of the data section
    int headerRow=0;        ///< index of header row
    bool mergeDelimiters=false; ///< if true, multiple separator characters are merged (eg space delimited files)
    bool counter=false;         ///< count data items, not read their values
    bool dontFail=false;        ///< do not throw an error on corrupt data, just ignore the data

    std::set<unsigned> dimensionCols;   ///< set of columns that are dimensions, of size numAxes. Note dimensionCols ∩ dataCols = ∅
    std::set<unsigned> dataCols;        ///< set of columns that are data, of size numData. Note dimensionCols ∩ dataCols = ∅
    std::vector<civita::NamedDimension> dimensions; ///< dimension vector of size numCols
    DataSpec() {}
    DataSpec(const CAPIRavelDataSpec&);
  };
  
  /// creates a chain of tensor operations that represents a Ravel in
  /// state \a state, operating on \a arg
  std::vector<civita::TensorPtr> createRavelChain(const RavelState&, const civita::TensorPtr& arg);
}
#if defined(CLASSDESC) || defined(ECOLAB_LIB)
#include "ravelState.cd"
#endif

#endif
