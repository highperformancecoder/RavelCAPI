/*
  Ravel C API. © Ravelation Pty Ltd 2020
*/

#include "ravelCAPI.h"
#include "ravelState.h"
#include "tensorOp.h"

using namespace std;

namespace ravel
{

  HandleState::HandleState(const CAPIRavelHandleState& state):
    x(state.x), y(state.y), collapsed(state.collapsed),
    displayFilterCaliper(state.displayFilterCaliper),
    reductionOp(toEnum<Op::ReductionOp>(state.reductionOp)),
    order(toEnum<HandleSort::Order>(state.order)),
    format(state.format)
  {
    if (state.description) description=state.description;
    if (state.customOrder)
      for (auto i=state.customOrder; *i; ++i)
        customOrder.push_back(*i);
    if (state.minLabel) minLabel=state.minLabel;
    if (state.maxLabel) maxLabel=state.maxLabel;
    if (state.sliceLabel) sliceLabel=state.sliceLabel;
  }

  RavelState::RavelState(const CAPIRavelState& state):  radius(state.radius)
  {
    if (state.handleStates)
      for (auto hs=state.handleStates; *hs; ++hs)
        handleStates.emplace_back(**hs);
    if (state.outputHandles)
      for (auto o=state.outputHandles; *o; ++o)
        outputHandles.emplace_back(*o);
  }

  HandleX::HandleX(const ravel::HandleState& state):
    m_description(state.description), m_customOrder(state.customOrder), m_format(state.format),
    m_minLabel(state.minLabel), m_maxLabel(state.maxLabel), m_sliceLabel(state.sliceLabel)
  {}

  StateX::StateX(const ravel::RavelState& state):
    m_handleStates(state.handleStates.begin(), state.handleStates.end()),
    m_outputHandles(state.outputHandles)
  {}


#if defined(CLASSDESC) || defined(ECOLAB_LIB)
  // sanity check that that the C enums are concordant with the C++ ones
  static_assert(sizeof(classdesc::enum_keysData<ravel::HandleSort::Order>::keysData)
                ==sizeof(classdesc::enum_keysData<RavelOrder>::keysData));
  static_assert(sizeof(classdesc::enum_keysData<ravel::HandleSort::OrderType>::keysData)
                ==sizeof(classdesc::enum_keysData<RavelOrderType>::keysData));
  static_assert(sizeof(classdesc::enum_keysData<Op::ReductionOp>::keysData)
                ==sizeof(classdesc::enum_keysData<RavelReductionOp>::keysData));
#endif

  using namespace civita;

  namespace
  {
    /// factory method for creating reduction operations
    TensorPtr createReductionOp(ravel::Op::ReductionOp op)
    {
      switch (op)
        {
        case ravel::Op::sum: return make_shared<Sum>();
        case ravel::Op::prod: return make_shared<Product>();
        case ravel::Op::av: return make_shared<civita::Average>();
        case ravel::Op::stddev: return make_shared<civita::StdDeviation>();
        case ravel::Op::min: return make_shared<Min>();
        case ravel::Op::max: return make_shared<Max>();
        default: throw runtime_error("Reduction "+to_string(op)+" not understood");
        }
    }
  } 

  vector<TensorPtr> createRavelChain(const RavelState& state, const TensorPtr& input)
  {
    set<string> outputHandles(state.outputHandles.begin(), state.outputHandles.end());
    vector<TensorPtr> chain{input};
    // TODO sorts and calipers
    for (auto& i: state.handleStates)
      {
        bool isOutput=outputHandles.count(i.description);
        if ((isOutput||i.collapsed) && // no point permuting the handle slices if it is simply sliced.
            (i.order!=ravel::HandleSort::none || i.displayFilterCaliper || !i.customOrder.empty()))
          {
            //apply sorting/calipers
            auto permuteAxis=make_shared<PermuteAxis>();
            permuteAxis->setArgument(chain.back(), {i.description, 0});
            auto& xv=chain.back()->hypercube().xvectors[permuteAxis->axis()];
            vector<size_t> perm;
            if (i.customOrder.empty())
              {
                for (size_t i=0; i<xv.size(); ++i)
                  perm.push_back(i);
                switch (i.order)
                  {
                  case ravel::HandleSort::staticForward:
                  case ravel::HandleSort::dynamicForward:
                    sort(perm.begin(), perm.end(),
                         [&](size_t i, size_t j) {return diff(xv[i],xv[j])<0;});
                    break;
                  case ravel::HandleSort::staticReverse:
                  case ravel::HandleSort::dynamicReverse:
                    sort(perm.begin(), perm.end(),
                         [&](size_t i, size_t j) {return diff(xv[i],xv[j])>0;});
                    break;
                  default:
                    assert(i.order==ravel::HandleSort::none);
                    break;
                  }
              }
            else
              {
                map<string, size_t> offsets;
                for (size_t i=0; i<xv.size(); ++i)
                  offsets[str(xv[i], xv.dimension.units)]=i;
                perm.clear();
                for (auto& j: i.customOrder)
                  if (offsets.count(j))
                    perm.push_back(offsets[j]);
              }
          
            if (i.displayFilterCaliper)
              {
                // remove any permutation items outside calipers
                if (!i.minLabel.empty())
                  for (auto j=perm.begin(); j!=perm.end(); ++j)
                    if (str(xv[*j],xv.dimension.units) == i.minLabel)
                      {
                        perm.erase(perm.begin(), j);
                        break;
                      }
                if (!i.maxLabel.empty())
                  for (auto j=perm.begin(); j!=perm.end(); ++j)
                    if (str(xv[*j],xv.dimension.units) == i.maxLabel)
                      {
                        perm.erase(j+1, perm.end());
                        break;
                      }
              }
            permuteAxis->setPermutation(std::move(perm));
            chain.push_back(permuteAxis);
          }
        if (!isOutput)
          {
            auto arg=chain.back();
            if (i.collapsed)
              {
                chain.emplace_back(createReductionOp(i.reductionOp));
                chain.back()->setArgument(arg, {i.description,0});
              }
            else
              {
                chain.emplace_back(new Slice);
                auto& xv=arg->hypercube().xvectors;
                auto axisIt=find_if(xv.begin(), xv.end(),
                                    [&](const XVector& j){return j.name==i.description;});
                if (axisIt==xv.end()) throw runtime_error("axis "+i.description+" not found");
                auto sliceIt=find_if(axisIt->begin(), axisIt->end(),
                                     [&](const civita::any& j){return str(j,axisIt->dimension.units)==i.sliceLabel;});
                // determine slice index
                size_t sliceIdx=0;
                if (sliceIt!=axisIt->end())
                  sliceIdx=sliceIt-axisIt->begin();
                chain.back()->setArgument(arg, {i.description, double(sliceIdx)});
              }
          }
      }      
    
    if (chain.back()->rank()>1)
      {
        auto finalPivot=make_shared<Pivot>();
        finalPivot->setArgument(chain.back(),{});
        finalPivot->setOrientation(state.outputHandles);
        chain.push_back(finalPivot);
      }
    return chain;
  }
}

using namespace ravel;
CAPIRavelHandleState::CAPIRavelHandleState(const ravel::HandleState& state):
  CAPIRavelHandleState(state.x,state.y,state.collapsed,
                       state.displayFilterCaliper,toEnum<enum RavelReductionOp>(state.reductionOp),
                       toEnum<enum RavelOrder>(state.order)) {}

CAPIRavelState::CAPIRavelState(const ravel::RavelState& state):
  CAPIRavelState(state.radius) {}


