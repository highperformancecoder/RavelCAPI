#ifndef FILTERCAIRO_H
#define FILTERCAIRO_H
#include "dataCube.h"

namespace ravel
{
  /// A class the draws and manages a filter control window
  /// Context G is either cairo_t* or an HDC
  struct MouseT
  {
    enum MouseTracking {none, lowerLimit, upperLimit};
  };

  template <class G>
  class FilterCairo: public DataCube, public MouseT
  {
    G g{ nullptr }; ///< weak reference to graphics context
    MouseTracking mouseTracking=none;
    CLASSDESC_ACCESS(FilterCairo);
    // clamp v to within [minVal(),maxVal()]
    void clamp(double& v);
  public:
    void setG(G gg) {g=gg;}
    double width=0, height=0; ///< window size to render into
    void render() const;
    void onMouse(double x, double y);
    void onMouseDown(double x, double y);
    void onMouseUp(double, double) {mouseTracking=none;}
  };

}

#if defined(CLASSDESC) || defined(ECOLAB_LIB)
#include "filterCairo.cd"
#ifdef _CLASSDESC
#pragma omit xml_pack ravel::FilterCairo
#pragma omit xml_unpack ravel::FilterCairo
#pragma omit random_init ravel::FilterCairo
#endif
namespace classdesc_access
{
  template <class G> struct access_xml_pack<ravel::FilterCairo<G>>:
    public access_xml_pack<ravel::DataCube> {};
  template <class G> struct access_xml_unpack<ravel::FilterCairo<G>>:
    public access_xml_unpack<ravel::DataCube> {};
  template <class G> struct access_random_init<ravel::FilterCairo<G>>:
    public access_random_init<ravel::DataCube> {};
}
#endif

#endif
