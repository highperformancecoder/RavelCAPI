/*
  Ravel C API. © Ravelation Pty Ltd 2020
*/

#include "ravelCAPI.h"
#include "ravelState.h"

using namespace ravel;

HandleState::HandleState(const CAPIRavelHandleState& state):
  x(state.x), y(state.y), collapsed(state.collapsed),
  displayFilterCaliper(state.displayFilterCaliper),
  reductionOp(toEnum<Op::ReductionOp>(state.reductionOp)), order(toEnum<HandleSort::Order>(state.order))
{
  if (state.description) description=state.description;
  if (state.customOrder)
    for (auto i=state.customOrder; *i; ++i)
      customOrder.push_back(*i);
  if (state.minLabel) minLabel=state.minLabel;
  if (state.maxLabel) minLabel=state.maxLabel;
  if (state.sliceLabel) sliceLabel=state.sliceLabel;
}

RavelState::RavelState(const CAPIRavelState& state):
  radius(state.radius), sortByValue(toEnum<HandleSort::Order>(state.sortByValue))
{
  if (state.handleStates)
    for (auto hs=state.handleStates; *hs; ++hs)
      handleStates.emplace_back(**hs);
  if (state.outputHandles)
    for (auto o=state.outputHandles; *o; ++o)
      outputHandles.emplace_back(*o);
}

RavelHandleStateX::RavelHandleStateX(ravel::HandleState&& state):
  CAPIRavelHandleState(state),
  m_description(std::move(state.description)), m_customOrder(std::move(state.customOrder)),
  m_minLabel(std::move(state.minLabel)), m_maxLabel(std::move(state.maxLabel)),
  m_sliceLabel(std::move(state.sliceLabel))
{setupPointers();}
    
RavelHandleStateX::RavelHandleStateX(const ravel::HandleState& state):
  CAPIRavelHandleState(state),
  m_description(state.description), m_customOrder(state.customOrder),
  m_minLabel(state.minLabel), m_maxLabel(state.maxLabel), m_sliceLabel(state.sliceLabel)
{setupPointers();}

RavelStateX::RavelStateX(ravel::RavelState&& state):
  CAPIRavelState(state),
  m_outputHandles(state.outputHandles)
  {
    for (auto& i: state.handleStates) m_handleStates.emplace_back(std::move(i));
    setupPointers();
  }

RavelStateX::RavelStateX(const ravel::RavelState& state):
  CAPIRavelState(state),
  m_handleStates(state.handleStates.begin(), state.handleStates.end()),
  m_outputHandles(state.outputHandles)
  {setupPointers();}

CAPIRavelHandleState::CAPIRavelHandleState(const ravel::HandleState& state):
  CAPIRavelHandleState(state.x,state.y,state.collapsed,
                       state.displayFilterCaliper,toEnum<enum RavelReductionOp>(state.reductionOp),
                       toEnum<enum RavelOrder>(state.order)) {}

CAPIRavelState::CAPIRavelState(const ravel::RavelState& state):
  CAPIRavelState(state.radius, toEnum<enum RavelOrder>(state.sortByValue)) {}


#if defined(CLASSDESC) || defined(ECOLAB_LIB)
// sanity check that that the C enums are concordant with the C++ ones
static_assert(sizeof(classdesc::enum_keysData<ravel::HandleSort::Order>::keysData)
              ==sizeof(classdesc::enum_keysData<RavelOrder>::keysData));
static_assert(sizeof(classdesc::enum_keysData<ravel::HandleSort::OrderType>::keysData)
              ==sizeof(classdesc::enum_keysData<RavelOrderType>::keysData));
static_assert(sizeof(classdesc::enum_keysData<Op::ReductionOp>::keysData)
              ==sizeof(classdesc::enum_keysData<RavelReductionOp>::keysData));
#endif
