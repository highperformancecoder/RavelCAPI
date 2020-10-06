/*
  Ravel C API. © Ravelation Pty Ltd 2020
*/

#ifndef RAVELSTATE_H
#define RAVELSTATE_H
#include <map>
#include <string>
#include <vector>

struct CAPIRavelState;
struct CAPIRavelHandleState;

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
    enum Order {none, forward, reverse, numForward, numReverse, timeForward, timeReverse, custom };
    enum OrderType {string, time, value};
  };

  // representing the state of the handles
  struct HandleState
  {
    std::string description;
    double x=ravelDefaultRadius, y=0; ///< handle tip coordinates (only angle important, not length)
    bool collapsed=false, displayFilterCaliper=false;
    Op::ReductionOp reductionOp=Op::sum;
    HandleSort::Order order=HandleSort::none;
    std::vector<std::string> customOrder; 
    std::string minLabel, maxLabel, sliceLabel;
    HandleState() {}
    HandleState(const CAPIRavelHandleState& state);
  };

  /// represents the full Ravel state
  struct RavelState
  {
    double radius=ravelDefaultRadius;
    /// sort 1D ravel by value. Ignored for any other rank.
    HandleSort::Order sortByValue=HandleSort::none;
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
  
}
#if defined(CLASSDESC) || defined(ECOLAB_LIB)
#include "ravelState.cd"
#endif

#endif
