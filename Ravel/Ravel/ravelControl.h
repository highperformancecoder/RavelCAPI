#ifndef RAVELCONTROL_H
#define RAVELCONTROL_H

//#define EXCEL12

#include "xlExtras.h"
#include "ravelCairo.h"
#include "filterCairo.h"
#include "ravelError.h"
#include <CommCtrl.h>
#include <map>
#include <set>
#include <boost/locale.hpp>
using boost::locale::conv::utf_to_utf;
using namespace std;
using namespace xll;
using namespace ravel;
using namespace xlExtras;

struct XLTokeniser : public Tokeniser
{
  unsigned short row=0;
  OPERX& data; // region of XL cells
  XLTokeniser(OPERX& data) : data(data) {}
  XLTokeniser& operator=(const XLTokeniser&) = delete;
  vector<boost::any> getLine() override;
};

struct XLDataCube : public FilterCairo<HDC>
{
  string sourceSheet, destSheet;
  unsigned short nsRows, nsCols; ///< number of rows and cols in source data
  DataSpec spec; // describes axes names and labels
  void setDataElement(size_t i, size_t j, double v) override;
  void loadData();
  void initSpec();
  void colourAxes();
  classdesc::Exclude<OPERX> destData; //stage for data for efficiency reasons
};

// data to be persisted
struct RavelCtlData: public RavelCairo<HDC>, public XLDataCube
{
  string title;
  enum ChartType {none=0, line=4, column=52, bar=58};
  ChartType chart=none;
  string serialise() const;
  void deserialise(const string&);
};

// RAII wrapper around a menu
class Menu
{
  HMENU menu;
  CLASSDESC_ACCESS(Menu);
public:
  Menu(): menu(CreateMenu()) {
    MENUINFO minfo={sizeof(MENUINFO),MIM_STYLE};
    GetMenuInfo(menu,&minfo);
    minfo.dwStyle|=MNS_NOTIFYBYPOS;
    SetMenuInfo(menu,&minfo);
  }
  ~Menu() {DestroyMenu(menu);}
  operator HMENU() const {return menu;}
  template <class E>
  void insertMenuItems(std::initializer_list<std::pair<E,std::string>> x)
  {
    for (auto i=x.begin(); i!=x.end(); ++i)
      AppendMenu(menu,MF_STRING, unsigned(i->first),utf_to_utf<TCHAR>(i->second).c_str());
  }
};

struct RavelCtl : public RavelCtlData
{
  bool updateExcel=false; // flag controlling callback to Excel
  bool dirty = false; // flag indicating whether source document has been edited
  const double filterWidgetWidth = 0.15; ///< width of filter widget control as a fraction of window width
  HWND hwnd{ 0 };
  int handle; // used for passing selected handle information to dialog boxes

  Menu menubar, reductionMenu, filterMenu, sortMenu, xsortMenu, ysortMenu, 
    chartMenu, helpMenu;

  TRACKMOUSEEVENT eventTrack{ sizeof(TRACKMOUSEEVENT),TME_LEAVE,0,HOVER_DEFAULT };
  void setup();
  void render();
  void renderFilter(HWND hwnd); ///< render filter widget into hwnd 
  void redraw(HWND hwnd); // seems to be needed on Windows 64 bit
  void populateSheet();
  void addChart() const;
  // creates a RavelCtl with its window, and stores the reference inside the Windows user data field for use by the WindowProc and cleanup
  static RavelCtl& create();
  void getSpreadsheetSize();
  void notifyExcel(bool force=false); 
  void restoreCheckpoint();
  void checkpoint();
  static string checkpointName() {return "ravelCheckpoint";}

  // sort by values given in the selected row or column
  bool sortRequested=false;
  HandleSort::Order sortDir;
  void checkSorting();

  static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  // handle reduction operation selections
  static LRESULT CALLBACK reductionOpCtlProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  // handle filter control events
  static LRESULT CALLBACK filterWidgetProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  // handle output chart selections
  static LRESULT CALLBACK chartSelectorProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  // handle dialog box for renaming axes
  static INT_PTR CALLBACK nameDlgProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  ~RavelCtl();
};


// weak ptr maps from source/dest sheet IDs to the ravelctl controlling them
extern map<IDSHEET, RavelCtl*> sourceRavels, destRavels;
// lists sheets scheduled for deletion, and whether they're dirty
struct SheetInfo
{
  IDSHEET other;
  bool dirty;
};
extern map<IDSHEET, SheetInfo> sheetsToDelete;


#include "ravelControl.cd"
#endif
