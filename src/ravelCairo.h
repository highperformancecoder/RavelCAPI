#ifndef RAVELCAIRO_H
#define RAVELCAIRO_H
#include "ravel.h"
#include "cda.h"

namespace ravel
{
  /// a class that will render the current ravel state to a graphics
  /// context G (=cairo_t* or HDC). The Ravel's origin and scale will
  /// be used as the coordinate system for drawing into the context
  template <class G>
  class RavelCairo: public Ravel
  {
    /// if this is nonnegative, then it specifies a handle that the
    /// mouse is hovering over, and to render the handle differently
    int toolTipHandle = -1;

    //classdesc::Exclude<G> g{ nullptr }; ///< weak reference to graphics context
    G g{ nullptr }; ///< weak reference to graphics context
    CLASSDESC_ACCESS(RavelCairo);
  public:
    void setG(G gg) {g=gg;}
    /// checks whether mouse is over a caliper label, otherwise calls Ravel::onMouseDown
    void onMouseDown(double xx, double yy);
    /// handles tooltip annotations on the ravel rendering. Returns
    /// true if state changes. (x,y) in window coordinates
    bool onMouseOver(double x, double y);
    /// returns handle id if mouse is over axis label. (x,y) in window
    /// coordinates
    int handleIfMouseOverAxisLabel(double x, double y) const;
    /// returns handle id if mouse is over reduction op label. (x,y) in window
    /// coordinates
    int handleIfMouseOverOpLabel(double x, double y) const;
    /// returns handle if mouse is over a displayed caliper label, -1 otherwise
    int handleIfMouseOverCaliperLabel(double x, double y) const;
    /// renders ravel image to graphics context g
    void render() const;
    /// returns reference to last handle mouse was over, and rendered
    /// with tooltips. Returns nullptr if no such handle is selected.
    Handle* selectedHandle() {
      return toolTipHandle>=0? &handles[toolTipHandle]: nullptr;
    }
    const Handle* selectedHandle() const {
      return toolTipHandle>=0? &handles[toolTipHandle]: nullptr;
    }
  };
}

#if defined(CLASSDESC) || defined(ECOLAB_LIB)
#include <xml_pack_base.h>
#include <xml_unpack_base.h>
#ifdef _CLASSDESC
#pragma omit xml_pack ravel::RavelCairo
#pragma omit xml_unpack ravel::RavelCairo
#pragma omit random_init ravel::RavelCairo
#endif
namespace classdesc_access
{
  template <class G> struct access_xml_pack<ravel::RavelCairo<G>>:
    public access_xml_pack<ravel::Ravel> {};
  template <class G> struct access_xml_unpack<ravel::RavelCairo<G>>:
    public access_xml_unpack<ravel::Ravel> {};
  template <class G> struct access_random_init<ravel::RavelCairo<G>>:
    public access_random_init<ravel::Ravel> {};
}
#include "ravelCairo.cd"
#endif
#endif
