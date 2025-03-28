/*
  Ravel C API. © Ravelation Pty Ltd 2020
*/

#include "ravelCAPITypes.h"
#include "ravelState.h"
#include "tensorOp.h"
#if defined(CLASSDESC) || defined(ECOLAB_LIB)
#include <classdesc_epilogue.h>
#endif

using namespace std;

namespace ravel
{

  template <class E> constexpr size_t enumSize()
  {
#if defined(CLASSDESC) || defined(ECOLAB_LIB)
    // use classdesc reflection
    return sizeof(classdesc::enum_keysData<E>::keysData)/
      sizeof(classdesc::enum_keysData<E>::keysData[0]);
#else
    return 0; // cannot determine this within standard C++
#endif
  }
  
  HandleState::HandleState(const CAPIRavelHandleState& state):
    x(state.x), y(state.y), collapsed(state.collapsed),
    displayFilterCaliper(state.displayFilterCaliper),
    reductionOp(toEnum<Op::ReductionOp>(state.reductionOp)),
    order(toEnum<HandleSort::Order>(state.order)),
    format(state.format), customOrderIsInverted(state.customOrderIsInverted)
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
    if (!input->hypercube().dimsAreDistinct())
      throw runtime_error("Axis names of input hypercube must be distinct");
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
                // custom ordering cannot have an empty customOrder -
                // but correct for previous misguided implementation
                // that confused staticForward with forward, etc.
               switch (i.order)
                  {
                  case ravel::HandleSort::forward:
                  case ravel::HandleSort::staticForward:
                  case ravel::HandleSort::dynamicForward:
                    sort(perm.begin(), perm.end(),
                         [&](size_t i, size_t j) {return diff(xv[i],xv[j])<0;});
                    break;
                  case ravel::HandleSort::reverse:
                  case ravel::HandleSort::staticReverse:
                  case ravel::HandleSort::dynamicReverse:
                    sort(perm.begin(), perm.end(),
                         [&](size_t i, size_t j) {return diff(xv[i],xv[j])>0;});
                    break;
                  default:
                    break;
                  }
              }
            else if (i.customOrderIsInverted)
              {
                set<string> customOrder(i.customOrder.begin(), i.customOrder.end());
                for (size_t i=0; i<xv.size(); ++i)
                  if (!customOrder.count(str(xv[i], xv.dimension.units)))
                    perm.push_back(i);
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
                       state.displayFilterCaliper,
                       toEnum<enum RavelReductionOp>(state.reductionOp),
                       toEnum<enum RavelOrder>(state.order),
                       state.customOrderIsInverted) {}

CAPIRavelState::CAPIRavelState(const ravel::RavelState& state):
  CAPIRavelState(state.radius) {}


void RavelDataSpec::setupPtrs()
{
  numCols=dimensionData.size();
  numAxes=m_dimCols.size();
  dimensionCols=m_dimCols.data();
  numData=m_dataCols.size();
  dataCols=m_dataCols.data();
#ifndef __EMSCRIPTEN__
  static_assert(enumSize<civita::Dimension>()==enumSize<CAPIRavelDimensionType>());
#endif
  for (auto& i: dimensionData)
    dimensionPtrs.emplace_back(toEnum<CAPIRavelDimensionType>(i.dimension.type),
        i.dimension.units.c_str(), i.name.c_str());
  dimensions=dimensionPtrs.data();
}
