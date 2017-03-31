#ifndef MENU_H
#define MENU_H
#include <string>
#include <vector>
#include <memory>
#include <cmath>

namespace ravel
{
  struct MenuBase
  {
    std::string label;
    mutable double x, y, w, h; ///< bounding box - (x,y) of centre, width & height
    bool inLabel(double xx, double yy) const ///< return true if (x,y) is within label
    {return std::abs(xx-x)<0.5*w && std::abs(yy-y)<0.5*h;}

    /// render to graphics context \a g
    template <class G>  void render(G& g) const;

    /// respond to mouse movement event. Returns true if event handled
    virtual bool onMouseOver(double x, double y)=0;
    /// respond to mouse click event. Returns true if event handled
    virtual bool onMouseDown(double x, double y)=0;
    virtual ~MenuBase() {}
  };

  
  class Menu: public MenuBase
  {
  public:
    std::vector<std::unique_ptr<MenuBase>> items;
    bool posted=false; ///< whether menu is posted (opened)
    Menu() {}
    Menu(double x, double y) {MenuBase::x=x; MenuBase::y=y;}
    /// render to graphics context \a g
    template <class G> void render(G& g) const;
    bool onMouseOver(double x, double y) override;
    bool onMouseDown(double x, double y) override;
  };

  template <class F>
  struct Command: public MenuBase
  {
    F f;
    bool onMouseOver(double x, double y) override {}
    // TODO 
    bool onMouseDown(double x, double y) override {f();}
  };


  template <class G>  void Menu::render(G& g) const
  {
    if (posted)
      {
        double y1=y;
        for (auto& i: items)
          {
            i->x=x;
            i->y=y1;
            i->render(g);
            y1+=i->h;
          }
      }
    else
      MenuBase::render(g);
  }

  template <class G>  void MenuBase::render(G& g) const
  {
    g.setTextExtents(label);
    w=g.textWidth();
    h=g.textHeight();
    g.moveTo(x,y+h);
    g.showText(label);
    g.moveTo(x,y);
    g.relLineTo(w,0);
    g.relLineTo(0,h);
    g.relLineTo(-w,0);
    g.relLineTo(0,-h);
    g.stroke();
  }


}

#if defined(CLASSDESC) || defined(ECOLAB_LIB)
#include "menu.cd"
#endif
#endif
