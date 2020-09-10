/*
  Ravel C API. © Ravelation Pty Ltd 2020
*/

#ifndef RAVELSTATE_H
#define RAVELSTATE_H
#include <map>
#include <string>
#include <vector>

namespace ravel
{
  static const double ravelDefaultRadius=100; ///< initial size of a Ravel widget

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

  struct CAPIHandleState;
  
  // representing the state of the handles
  struct HandleState
  {
    std::string description;
    double x,y; ///< handle tip coordinates (only angle important, not length)
    bool collapsed=false, displayFilterCaliper=false;
    Op::ReductionOp reductionOp=Op::sum;
    HandleSort::Order order=HandleSort::none;
    std::vector<std::string> customOrder; 
    std::string minLabel, maxLabel, sliceLabel;
    HandleState() {}
    HandleState(const CAPIHandleState& state);
  };

  // interface, for use on CAPI
  struct CAPIHandleState
  {
    double x,y; ///< handle tip coordinates (only angle important, not length)
    bool collapsed=false, displayFilterCaliper=false;
    Op::ReductionOp reductionOp=Op::sum;
    HandleSort::Order order=HandleSort::none;
    CAPIHandleState() {}
    /// initialises just the simple data members
    CAPIHandleState(const HandleState& state): x(state.x), y(state.y),
                                               collapsed(state.collapsed),
                                               displayFilterCaliper(state.displayFilterCaliper),
                                               reductionOp(state.reductionOp),
                                               order(state.order)
    {}
    
    // To manage the memory pointed to by these pointers, best
    // practice is to extend this class, and ensure these pointers are
    // updated into internally managed memory. Lifetime of the
    // overall object needs to be clarified on the API. Beware object slicing.
    const char* description=nullptr;
    const char* minLabel=nullptr;
    const char* maxLabel=nullptr;
    const char* sliceLabel=nullptr;
    const char** customOrder=nullptr; // used if order==custom, null terminated
  };

  class HandleStateX: public ravel::CAPIHandleState
  {
  public:
    HandleStateX() {}
    HandleStateX(HandleState&& state): CAPIHandleState(state),
      m_description(std::move(state.description)), m_customOrder(std::move(state.customOrder)),
      m_minLabel(std::move(state.minLabel)), m_maxLabel(std::move(state.maxLabel)),
      m_sliceLabel(std::move(state.sliceLabel))
    {setupPointers();}
    
    HandleStateX(const HandleState& state): CAPIHandleState(state),
      m_description(state.description), m_customOrder(state.customOrder),
      m_minLabel(state.minLabel), m_maxLabel(state.maxLabel), m_sliceLabel(state.sliceLabel)
    {setupPointers();}
    
  private:
    std::string m_description;
    std::vector<const char*> customOrderStrings;
    // note this member must appear after all members of
    // CAPIHandleState from the Ravel CAPI
    std::vector<std::string> m_customOrder; 
    std::string m_minLabel, m_maxLabel, m_sliceLabel;
    void setupPointers() {
      customOrderStrings.clear();
      for (auto& i: m_customOrder)
        customOrderStrings.push_back(i.c_str());
      customOrderStrings.push_back(nullptr);
      customOrder=customOrderStrings.data();
      minLabel=m_minLabel.c_str();
      maxLabel=m_maxLabel.c_str();
      sliceLabel=m_sliceLabel.c_str();
      description=m_description.c_str();
    }
  };
    

  inline HandleState::HandleState(const CAPIHandleState& state):
    x(state.x), y(state.y), collapsed(state.collapsed),
    displayFilterCaliper(state.displayFilterCaliper),
    reductionOp(state.reductionOp), order(state.order)
  {
    if (state.description) description=state.description;
    if (state.customOrder)
      for (auto i=state.customOrder; *i; ++i)
        customOrder.push_back(*i);
    if (state.minLabel) minLabel=state.minLabel;
    if (state.maxLabel) minLabel=state.maxLabel;
    if (state.sliceLabel) sliceLabel=state.sliceLabel;
  }

  struct CAPIRavelState;

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
  
  struct CAPIRavelState
  {
    double radius=ravelDefaultRadius;
    /// sort 1D ravel by value. Ignored for any other rank.
    HandleSort::Order sortByValue=HandleSort::none;
    CAPIRavelState() {}
    /// initialises just the simple data members
    CAPIRavelState(const RavelState& state): radius(state.radius), sortByValue(state.sortByValue) {}
    const CAPIHandleState** handleStates=nullptr; ///< null terminated list of handle states
    const char** outputHandles=nullptr; ///< null terminated list of output handles
  };

  class RavelStateX: public ravel::CAPIRavelState
  {
  public:
    RavelStateX() {}
//    RavelStateX(RavelState&& state): CAPIRavelState(state), 
//                                     m_handleStates(std::move(state.handleStates)),
//                                     m_outputHandles(std::move(state.outputHandles))
//    {setupPointers(); }
    
    RavelStateX(const RavelState& state): CAPIRavelState(state),
                              m_handleStates(state.handleStates.begin(), state.handleStates.end()),
                              m_outputHandles(state.outputHandles)
    {setupPointers();}

  private:
    std::vector<HandleStateX> m_handleStates;
    std::vector<const CAPIHandleState*> handleStatePtrs;
    std::vector<std::string> m_outputHandles;
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
  
  inline RavelState::RavelState(const CAPIRavelState& state):
    radius(state.radius), sortByValue(state.sortByValue)
  {
    if (state.handleStates)
      for (auto hs=state.handleStates; *hs; ++hs)
        handleStates.emplace_back(**hs);
    if (state.outputHandles)
      for (auto o=state.outputHandles; *o; ++o)
        outputHandles.emplace_back(*o);
  }

}
#if defined(CLASSDESC) || defined(ECOLAB_LIB)
#include "ravelState.cd"
#endif

#endif
