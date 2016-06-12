#include "ravelTCL.h"
#include "dataCubeTCL.h"
#include "TCL_obj_stl.h"
#include "ravelCairo.h"

// this shouldn't be needed, but make gets confused without it!
#include "splitMerge.h"

#include <cairo_base.h>
#include <ecolab_epilogue.h>

#include <fstream>
typedef ravel::Ravel Ravel;
typedef ravel::DataCubeTCL DataCube;

using namespace ecolab;
using ravel::RavelCairo;
using namespace std;

TCLTYPE(Ravel);
TCLTYPE(DataCube);

namespace
{
  // sets ecolab_library if local directory 
  int setEcolabLib()
  {
    std::ifstream f("./library/init.tcl");
    if (f)
      {
        tclvar TCL_obj_lib("ecolab_library","./library");
      }
    return 0;
  }
  int dum=setEcolabLib();

  struct RavelItem: public ecolab::cairo::ImageItem
  {
    /// accessor to the ravel from TCL
    char* ravelName;
  };

  // cairo item for rendering ravels on canvases
  struct RavelCairoImage: public cairo::CairoImage
  {
    static Tk_ConfigSpec configSpecs[];
    ravel::RavelCairo<cairo_t*> thisRavel;
    ravel::RavelCairo<cairo_t*>* ravel=&thisRavel; // weak pointer to above or some other Ravel
    void draw() {
      if (cairoSurface)
        {
          cairoSurface->clear();
          ravel->setG(cairoSurface->cairo());
          ravel->render();
        }
    }
  };

  static Tk_CustomOption tagsOption = {
    (Tk_OptionParseProc *) Tk_CanvasTagsParseProc,
    Tk_CanvasTagsPrintProc, (ClientData) NULL
  };

#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
  Tk_ConfigSpec RavelCairoImage::configSpecs[]=
    {
      {TK_CONFIG_STRING, "-ravelName", NULL, NULL,
       NULL, Tk_Offset(RavelItem, ravelName), 0},
      {TK_CONFIG_CUSTOM, "-tags", NULL, NULL,
       NULL, 0, TK_CONFIG_NULL_OK, &tagsOption},
      {TK_CONFIG_END}
    };

    int ravelCreatProc(Tcl_Interp *interp, Tk_Canvas canvas, 
                       Tk_Item *itemPtr, int objc,Tcl_Obj *CONST objv[])
    {
      RavelItem* ravelItem=(RavelItem*)(itemPtr);
      ravelItem->ravelName=NULL;
      int r=cairo::createImage<RavelCairoImage>
        (interp,canvas,itemPtr,objc,objv);
      member_entry<RavelCairo<cairo_t*>>* me;
      if (r==TCL_OK && ravelItem->ravelName)
        if (RavelCairoImage* ravelIm=dynamic_cast<RavelCairoImage*>(ravelItem->cairoItem))
          {
            if (TCL_obj_properties().count(ravelItem->ravelName) &&
                (me=dynamic_cast<member_entry<RavelCairo<cairo_t*>>*>
                 (TCL_obj_properties()[ravelItem->ravelName].get()))
                && me->memberptr)
              ravelIm->ravel=me->memberptr;
            else
              {
                // register the ravel object with TCL
                ravelIm->ravel=&ravelIm->thisRavel;
                TCL_obj(null_TCL_obj,ravelItem->ravelName, ravelIm->thisRavel);
              }
            if (ravelIm->cairoSurface)
              ravelIm->ravel->setG(ravelIm->cairoSurface->cairo());
          }
      return r;
  }

  void ravelDeleteProc(Tk_Canvas canvas, Tk_Item *itemPtr, Display *display)
  {
    RavelItem* ravelItem=(RavelItem*)(itemPtr);
    TCL_obj_deregister(ravelItem->ravelName);
    cairo::TkImageCode::DeleteImage(canvas,itemPtr,display);
  }

  // overrride cairoItem's configureProc to process the extra config options
  int configureProc(Tcl_Interp *interp,Tk_Canvas canvas,Tk_Item *itemPtr,
                    int objc,Tcl_Obj *CONST objv[],int flags)
  {
    return cairo::TkImageCode::configureCairoItem
      (interp,canvas,itemPtr,objc,objv,flags, RavelCairoImage::configSpecs);
  }

  // register the RavelCairo item as a canvas item
    int registerItem()
    {
      static Tk_ItemType ravelItemType = cairo::cairoItemType();
      ravelItemType.name="ravel";
      ravelItemType.itemSize=sizeof(RavelItem);
      ravelItemType.createProc=ravelCreatProc;
      ravelItemType.configProc=configureProc;
      ravelItemType.configSpecs=RavelCairoImage::configSpecs;
      ravelItemType.deleteProc=ravelDeleteProc;      
      Tk_CreateItemType(&ravelItemType);
      return 0;
    }

  int dum1=registerItem();
}
