#ifndef DATACUBETCL_H
#define DATACUBETCL_H
#include "ravelTCL.h"
#include "ravelCairo.h"
#include "dataCube.h"
#include "filterCairo.h"
#include <cairo_base.h>
#include <TCL_obj_base.h>

#include <cairo/cairo.h>
#include <fstream>

namespace ravel
{
  struct DataCubeTCL: public FilterCairo<cairo_t*>, public DataSpec
  {
    RavelCairo<cairo_t*> ravel;
    char separator;
    void loadFile(const std::string& fileName) {
      DataCube::loadFile(fileName, separator, *this);
      initRavel(ravel);
    }

    void initSpec(const char* fileName) {
      std::ifstream input(fileName);
      CSVFTokeniser tok(input, separator);
      DataSpec::operator=(initDataSpec(tok));
    }
    
    classdesc::Exclude<classdesc::shared_ptr<ecolab::cairo::Surface> > fs;
    void setFilterImage(const std::string& image)
    {
      fs.reset(new ecolab::cairo::TkPhotoSurface
              (Tk_FindPhoto(ecolab::interp(), image.c_str())));
      setG(fs->cairo());
      this->render();
      fs->blit();
    }

    classdesc::Exclude<classdesc::shared_ptr<ecolab::cairo::Surface> > rs;
    void setRavelImage(const std::string& image)
    {
      rs.reset(new ecolab::cairo::TkPhotoSurface
              (Tk_FindPhoto(ecolab::interp(), image.c_str())));
      cairo_surface_set_device_offset(rs->surface(), 
                                      0.5*rs->width(), 0.5*rs->height());
      ravel.setG(rs->cairo());
      ravel.render();
      rs->blit();
    }
   
    void resizeHistogram(unsigned numBins)
    {histogram.resize(numBins);}


    void render() {
      if (fs.get()) {fs->clear(); FilterCairo<cairo_t*>::render(); fs->blit();}
      if (rs.get()) {rs->clear(); ravel.render(); rs->blit();}
    }

    void onMouse(double x, double y) {FilterCairo<cairo_t*>::onMouse(x,y); render();}
    void onMouseDown(double x, double y) {FilterCairo<cairo_t*>::onMouseDown(x,y); render();}

    std::string array;
    // extension to tclvar allowing 2D integer indexing
    struct TclArray: public ecolab::tclvar
    {
      TclArray(const std::string& s): tclvar(s.c_str()) {}
      tclvar operator()(size_t i, size_t j) {
        std::ostringstream index;
        index << i<<","<<j;
        return operator[](index.str().c_str());
      }
    };
        
    // populates a TCL array variable according to current slice and
    // rollup parameters
    // TODO handle ranks other than 2.
    void populateArray(ecolab::TCL_args array) {
      if (ravel.handles.size()<2) return;
      array >> this->array;
      ecolab::tclcmd() << "array unset"<<this->array<<"\n";
      DataCube::populateArray(ravel);
      // add row & column labels
      TclArray tclArray(this->array);
      int xh=0, yh=1;
      if (ravel.handleIds.size()>0) xh=ravel.handleIds[0];
      if (ravel.handleIds.size()>1) yh=ravel.handleIds[1];
      if (xh==yh) {xh=0; yh=1;}
      Handle xHandle=ravel.handles[xh];
      Handle yHandle=ravel.handles[yh];
      for (size_t i=0; i<xHandle.sliceLabels.size(); ++i)
        // first index of tclArray is row index which is the yHandle index
        tclArray(0,i+1)=xHandle.sliceLabels[i].c_str();
      for (size_t i=0; i<yHandle.sliceLabels.size(); ++i)
        tclArray(i+1,0)=yHandle.sliceLabels[i].c_str();
          
    }

    virtual void setDataElement(size_t i, size_t j, double v) override {
      TclArray tclArray(array);
      // first index of tclArray is row index which is the yHandle index
      tclArray(j+1,i+1)=v;
    }

  };
}
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif

#include "dataCubeTCL.cd"

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#endif
