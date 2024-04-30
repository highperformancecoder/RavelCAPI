/*
  Ravel C API. © Ravelation Pty Ltd 2020
*/

#ifndef RAVELSTATE_H
#define RAVELSTATE_H
#include <map>
#include <memory>
#include <string>
#include <vector>

struct CAPIRavelState;
struct CAPIRavelHandleState;

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
    std::vector<std::string> customOrder; 
    std::string minLabel, maxLabel, sliceLabel;
    HandleState() {}
    HandleState(const CAPIRavelHandleState& state);
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
  };

  /// creates a chain of tensor operations that represents a Ravel in
  /// state \a state, operating on \a arg
  std::vector<civita::TensorPtr> createRavelChain(const RavelState&, const civita::TensorPtr& arg);
}
#if defined(CLASSDESC) || defined(ECOLAB_LIB)
#include "ravelState.cd"
#endif

#endif
