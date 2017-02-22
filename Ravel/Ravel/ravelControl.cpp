//#define EXCEL12

#include "xlExtras.h"
#include "ravelControl.h"
#include "ravelVersion.h"
#undef IDOK
#undef IDCANCEL
#include "resource.h"
#include <CommCtrl.h>
#include <Shlwapi.h>
#include <shellapi.h>
#include <gdiplus.h>
#include <map>
#include <sstream>
// generation of assignment operators
#pragma warning(disable:4512)
#include <algorithm>
using namespace std;
using namespace xll;
using namespace ravel;
using namespace Gdiplus;
using classdesc::xml_pack_t;
using classdesc::xml_unpack_t;

#include <classdesc_epilogue.h>

namespace
{
#ifdef _WIN64
  HMODULE xllModule=GetModuleHandle(_T("Ravel_x64.xll"));
#else
  HMODULE xllModule=GetModuleHandle(_T("Ravel_Win32.xll"));
#endif
}

vector<boost::any> XLTokeniser::getLine()
{
  vector<boost::any> r;
  if (row<data.rows())
    for (unsigned short col = 0; col < data.columns(); ++col)
      r.push_back(toAny(data(row, col)));
  row++;
  return r;
}

void XLDataCube::setDataElement(size_t i, size_t j, double v)
{
  destData((unsigned short)j, (unsigned short)i) = v;
}

void XLDataCube::loadData()
{
 OPERX data = CallXL(xlCoerce, SSOPERX(sheetId(sourceSheet),0, 0, (OPERX::xrw)nsRows, (OPERX::xcol)nsCols));
  XLTokeniser xltokeniser(data);
  DataCube::loadData(xltokeniser, spec);
}

void XLDataCube::initSpec()
{
  OPERX data = CallXL(xlCoerce, SSOPERX(sheetId(sourceSheet),0, 0, (OPERX::xrw)nsRows, (OPERX::xcol)nsCols));
  XLTokeniser xltokeniser(data);
  spec=initDataSpec(xltokeniser);

  // try increasing the  numbers of axes columns to see if we can successfully load
  bool notLoaded = true;
  for (int i = 0; notLoaded && i < 5; ++i)
    try
      {
        loadData();
        notLoaded = false;
      }
    catch (...) 
      {
        spec.nColAxes++;
      }
  if (notLoaded)
    throw RavelError("Unable to figure out the axes. Please specify by context clicking on the columns");
}
  
void XLDataCube::colourAxes()
{
  if (nsRows>0 && spec.nColAxes>0)
    CallXL(xlcSelect, OPERX(0, 0, (OPERX::xrw)nsRows, (OPERX::xcol)spec.nColAxes));
  // set slected foreground colour blue
  // using the older Format.Font command, as Font.Properties does not work as advertised
  // we have to specify a font size here - so use 12 points.
  CallXL(xlcFormatFont, OPERX(), 12, true/*bold*/, OPERX(), OPERX(), OPERX(), 5);
  if (spec.nRowAxes>0 && nsCols>0)
    CallXL(xlcSelect, OPERX(0, 0, (OPERX::xrw)spec.nRowAxes, (OPERX::xcol)nsCols));
  CallXL(xlcFormatFont, OPERX(), 12, true/*bold*/, OPERX(), OPERX(), OPERX(),
         5/*blue*/);
  if (nsRows>0 && nsCols>0)
    CallXL(xlcSelect, OPERX((OPERX::xrw)spec.nRowAxes, (OPERX::xcol)spec.nColAxes, (OPERX::xrw)nsRows, (OPERX::xcol)nsCols));
  CallXL(xlcFormatFont, OPERX(), 12, OPERX(), OPERX(), OPERX(), OPERX(), 1/*black*/);

  for (size_t c: spec.commentCols)
    {
      if (nsRows>0)
        CallXL(xlcSelect, OPERX(0,unsigned(c),nsRows,1));
      CallXL(xlcFormatFont, OPERX(), 12, OPERX(), OPERX(), OPERX(), OPERX(),3/*red*/);
    }
  for (size_t r : spec.commentRows)
    {
      if (nsCols>0)
        CallXL(xlcSelect, OPERX(unsigned(r), 0, 1, nsCols));
      CallXL(xlcFormatFont, OPERX(), 12, OPERX(), OPERX(), OPERX(), OPERX(),3/*red*/);
    }
}

// weak ptr maps from source/dest sheet IDs to the ravelctl controlling them
map<IDSHEET, RavelCtl*> sourceRavels, destRavels;
map<IDSHEET,SheetInfo> sheetsToDelete;

namespace
{
  const TCHAR* menuLabels[] = { _T("sum"), _T("prod"), _T("av"), _T("stddev"), _T("min"), _T("max") };
  const unsigned numOps = sizeof(menuLabels) / sizeof(menuLabels[0]);

  // mapping of chart selection menu items to charts
  RavelCtl::ChartType charts[]={RavelCtl::none,RavelCtl::line,RavelCtl::column,RavelCtl::bar};
  const unsigned numCharts=sizeof(charts)/sizeof(charts[0]);
  //  const TCHAR* chartNames[]={_T("None"),_T("Line"),_T("Column"),_T("Bar")};

  void toggle(bool& x) {x=!x;}
 
  struct FilterMenuItems
  {
    enum Items {values, xAxis, yAxis};
  };

  

  // returns menu position of sort items. These start at 2, to allow x
  // and y sort menus to come first
  HandleSort::Order sortMenuMap[]={HandleSort::none, HandleSort::none, HandleSort::forward, HandleSort::reverse, HandleSort::none};

  // used to convert enums labels to menu item ids. Is incremented so
  // label 0 is avoided, as 0 has special meaning within menus
  template <class E>
  unsigned incr(E x) {return unsigned(x)+1;}

  std::initializer_list<std::pair<unsigned,std::string>>
    reductionMenuItems =
    {
      {Op::sum, "sum"},
      {Op::prod, "product"},
      {Op::av, "average"},
      {Op::stddev, "std deviation"},
      {Op::min, "minimum"},
      {Op::max, "maximum"}
    },
    filterMenuItems =
    {
      {FilterMenuItems::values,"Filter values"},
      {FilterMenuItems::xAxis,"Filter x axis"},
      {FilterMenuItems::yAxis,"Filter y axis"}
    },
    axisSortMenuItems =
    {
      {SortedVector::none,"Original Order"},
      {SortedVector::forward, "Sort by slice label"},
      {SortedVector::reverse, "Reverse sort by slice label"},
      {SortedVector::numForward, "Numerically sort by slice label"},
      {SortedVector::numReverse, "Reverse numerically sort by slice label"}
    },
    sortMenuItems =
      {
        {SortedVector::forward,"Sort by selected row/column"},
        {SortedVector::reverse, "Reverse sort by selected row/column" },
        {SortedVector::none, "Original sort order" }
      },
    chartMenuItems =
    {
      {RavelCtl::ChartType::none, "none"},
      {RavelCtl::ChartType::line, "line"},
      {RavelCtl::ChartType::column, "column"},
      {RavelCtl::ChartType::bar,"bar"}
    },
    helpMenuItems =
    {
      {0, "About Ravel"},
      {1, "User Guide"}
    };
    
  void radioCheckMenu(HMENU menu, unsigned item)
  {CheckMenuRadioItem(menu,0,GetMenuItemCount(menu),item,MF_BYPOSITION);}

  // records child window handles of toplevel
  struct TopLvlWndData
  {
    HWND redOpSelector, ravelWnd, filterWidget, chartSelector;
  };

  // handle messages for the toplevel ravel panel
  LRESULT CALLBACK topLvlWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) 
  {
    TopLvlWndData* wndData = (TopLvlWndData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    RavelCtl* r=(RavelCtl*)GetWindowLongPtr(wndData->ravelWnd,GWLP_USERDATA);
    switch (msg)  {
    case WM_SIZE:
      {
        unsigned filterWidth=r->filterOn?r->filterWidgetWidth*LOWORD(lparam):0;
        unsigned ravelWidth=LOWORD(lparam) - filterWidth;
        MoveWindow(wndData->ravelWnd, 0, 0, ravelWidth, HIWORD(lparam), TRUE);
        MoveWindow(wndData->filterWidget, LOWORD(lparam)-filterWidth, 10, 
                   filterWidth, HIWORD(lparam)-20, TRUE);
      }
      return 0;
    case WM_PAINT:
      {
        // clear background
        HBRUSH bkGrnd = CreateSolidBrush(RGB(255, 255, 255));
        PAINTSTRUCT ps;
        HDC dc=BeginPaint(hwnd,&ps);
        FillRect(dc, &ps.rcPaint, bkGrnd);
        DeleteObject(bkGrnd);
        EndPaint(hwnd,&ps);
        return 0;
      }
    case WM_MENUCOMMAND:
      {
        if ((HMENU)lparam==r->reductionMenu)
          {
            Op::ReductionOp op=Op::ReductionOp(wparam);
            for (Handle& h: r->handles)
              h.reductionOp = op;
            r->nextRedOp=op;
            radioCheckMenu(r->reductionMenu,wparam);
            r->redraw(r->hwnd);
            r->notifyExcel(true);
          }
        else if ((HMENU)lparam==r->filterMenu)
          {
            switch (FilterMenuItems::Items(wparam))
              {
              case FilterMenuItems::values:
                toggle(r->filterOn);

                {
                  // resize top level window bigger or smaller to allow for filterWidget
                  RECT rect;
                  GetWindowRect(hwnd, &rect);
				  int fwOffset = (r->filterOn ? 1 : -1)*r->filterWidgetWidth*(rect.right - rect.left);
                  MoveWindow(hwnd, rect.left, rect.top, rect.right-rect.left+fwOffset, 
                             rect.bottom-rect.top, TRUE);
                  ShowWindow(wndData->filterWidget, r->filterOn?SW_SHOW:SW_HIDE);
                }

                r->redraw(r->hwnd);
                r->notifyExcel(r->DataCube::filterMax<r->maxVal()||
                               r->DataCube::filterMin>r->minVal());
                break;
              case FilterMenuItems::xAxis:
                toggle(r->handles[r->handleIds[0]].displayFilterCaliper);
                r->redraw(r->hwnd);
                break;
              case FilterMenuItems::yAxis:
                toggle(r->handles[r->handleIds[1]].displayFilterCaliper);
                r->redraw(r->hwnd);
                break;
              }
          }
        else if ((HMENU)lparam==r->xsortMenu)
          {
            r->handles[r->handleIds[0]].sliceLabels.order
              (SortedVector::Order(wparam));
            r->unsortAxis(r->handleIds[0]);
            r->redraw(r->hwnd);
            r->notifyExcel(true);
            radioCheckMenu(r->xsortMenu,wparam);
          }
        else if ((HMENU)lparam==r->ysortMenu)
          {
            r->handles[r->handleIds[1]].sliceLabels.order
              (SortedVector::Order(wparam));
           r->unsortAxis(r->handleIds[1]);
           r->redraw(r->hwnd);
            r->notifyExcel(true);
            radioCheckMenu(r->ysortMenu,wparam);
          }
        else if ((HMENU)lparam==r->sortMenu)
          {
            // save request for sorting for next getRavelResult
            r->sortRequested=true;
            r->sortDir=sortMenuMap[wparam];
            radioCheckMenu(r->sortMenu,wparam);
            r->notifyExcel(true);
          }
       else if ((HMENU)lparam==r->chartMenu)
          {
            r->chart=charts[wparam];
            r->notifyExcel(true);
            radioCheckMenu(r->chartMenu,wparam);
         }
        else if ((HMENU)lparam==r->helpMenu)
          {
            switch (wparam)
              {
              case 0: // about Ravel
                MessageBox(hwnd,
                           utf_to_utf<TCHAR>("Ravel version " RAVEL_VERSION ". ©High Performance Coders").c_str(),
                           _T("About Ravel"),MB_OK);
                break;
              case 1: // open user guide in web browser
                {
                  const int fnameSz=1000;
                  TCHAR fname[fnameSz];
                  GetModuleFileName(xllModule,fname,fnameSz);
                  PathRemoveFileSpec(fname);
                  ShellExecute(NULL,_T("open"),(fname+basic_string<TCHAR>(_T("/ravelDoc/ravelDoc.html"))).c_str(),NULL,NULL,1);
                }
                break;
              }
          }
     }
      return 0;
    case WM_CLOSE:
      SendMessage(wndData->ravelWnd,WM_CLOSE,0,0);
      delete wndData;
      return DefWindowProc(hwnd, msg, wparam, lparam);
    default:
      return DefWindowProc(hwnd, msg, wparam, lparam);
    }
  }
}

string RavelCtlData::serialise() const
{
  ostringstream s;
  xml_pack_t x(s);
  xml_pack(x,"ravel",*this);
  return s.str();
}

void RavelCtlData::deserialise(const string& s)
{
  xml_unpack_t x; 
  istringstream is(s);
  x.parse(is);
  xml_unpack(x,"ravel",*this);
}

void RavelCtl::setup() 
{
  if (!hwnd)
    {
	  RECT targetClientArea = { 0, 0, 0, 0 };
	  targetClientArea.bottom = int(2 * radius() / 0.9);
	  targetClientArea.right = /*opSelectorWidth + chartSelectorWidth + filterWidgetWidth +*/ targetClientArea.bottom;
	  DWORD tlStyle = WS_POPUP | WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | BS_FLAT;
	  AdjustWindowRect(&targetClientArea, tlStyle, true/*menu*/);
	  // window handles are only 32 bits for interoperability with 32 bit windows.
#pragma warning(push)
#pragma warning(disable:4312)
      HWND excel = (HWND)Excel<XLOPER12>(xlGetHwnd).val.w;
#pragma warning(pop)
      HWND toplevel = CreateWindow(_T("Button"), utf_to_utf<TCHAR>(title).c_str(), tlStyle, 100, 100, 
                                   abs(targetClientArea.right-targetClientArea.left), 
                                   abs(targetClientArea.top-targetClientArea.bottom), excel, 0, 0, 0);
	  if (toplevel)
        {

          // reduction menu
          AppendMenu(menubar,MF_POPUP|MF_STRING,(UINT_PTR)(HMENU)reductionMenu,_T("Reduction"));

		  reductionMenu.insertMenuItems(reductionMenuItems);
		  radioCheckMenu(reductionMenu,0);

          AppendMenu(menubar,MF_POPUP|MF_STRING,(UINT_PTR)(HMENU)filterMenu,
                     _T("Filter"));
		  filterMenu.insertMenuItems(filterMenuItems);

          AppendMenu(menubar,MF_POPUP|MF_STRING,(UINT_PTR)(HMENU)sortMenu,
                     _T("Sort"));
		  AppendMenu(sortMenu,MF_POPUP|MF_STRING,(UINT_PTR)(HMENU)xsortMenu,
                     _T("x axis"));
          xsortMenu.insertMenuItems(axisSortMenuItems);
		  radioCheckMenu(xsortMenu,0);
		  AppendMenu(sortMenu,MF_POPUP|MF_STRING,(UINT_PTR)(HMENU)ysortMenu,
                     _T("y axis"));
		  ysortMenu.insertMenuItems(axisSortMenuItems);
		  radioCheckMenu(ysortMenu,0);

		  // sort by value
          sortMenu.insertMenuItems(sortMenuItems);

          AppendMenu(menubar,MF_POPUP|MF_STRING,(UINT_PTR)(HMENU)chartMenu,
                     _T("Chart"));
		  chartMenu.insertMenuItems(chartMenuItems);
		  radioCheckMenu(chartMenu,0);

          // can't quite place this flush right, but should be last menu
          helpMenu.insertMenuItems(helpMenuItems);
		  AppendMenu(menubar, MF_POPUP | MF_STRING, (UINT_PTR)(HMENU)helpMenu,
                     _T("Help"));
		  
		  SetMenu(toplevel, menubar);

          RECT rect;
          GetClientRect(toplevel, &rect);

          InitCommonControls();

          // create Ravel window
          hwnd= CreateWindow(_T("Button"), _T(""), WS_CHILD | SS_ETCHEDFRAME | WS_VISIBLE, 0, 0, rect.right - rect.left, rect.bottom - rect.top, toplevel, 0, 0, 0);
		  SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
		  SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)windowProc);

          // create filter widget window
          HWND filter=CreateWindow(_T("Button"), _T(""), WS_CHILD | SS_ETCHEDFRAME, rect.right-filterWidgetWidth, 10, filterWidgetWidth, rect.bottom - rect.top-20, toplevel, 0, 0, 0);
		  SetWindowLongPtr(filter, GWLP_USERDATA, (LONG_PTR)this);
		  SetWindowLongPtr(filter, GWLP_WNDPROC, (LONG_PTR)filterWidgetProc);
		  FilterCairo<HDC>::width=filterWidgetWidth;
		  FilterCairo<HDC>::height=rect.bottom - rect.top-20;

		  GetClientRect(hwnd, &rect);
		  x = 0.5*(rect.right - rect.left);
          y = 0.5*(rect.bottom - rect.top);
		  rescale(0.9*min(x, y));

		  SetWindowLongPtr(toplevel, GWLP_USERDATA, (LONG_PTR)new TopLvlWndData{ 0/*opSelector*/, hwnd,filter,0/*chartSelector*/ });
          SetWindowLongPtr(toplevel, GWLP_WNDPROC, (LONG_PTR)topLvlWndProc);
	  }
    }
}

namespace
{
  template <class XCairo>
  struct SetupDC
  {
    PAINTSTRUCT ps;
    HWND hwnd;
    HDC dc;
    HFONT font;
    ULONG_PTR gdiToken;
    
    SetupDC(XCairo&g, HWND hwnd): hwnd(hwnd)
    {
      GdiplusStartupInput gdiplusStartupInput;
      GdiplusStartup(&gdiToken,&gdiplusStartupInput,nullptr);
      dc=BeginPaint(hwnd, &ps);
      // set a suitable font
      HFONT font = CreateFont
        (12, 0, 0, 0, FW_NORMAL, FALSE/*italic*/, FALSE/*underline*/, 
         FALSE/*strikethrough*/, DEFAULT_CHARSET/* how to specify unicode?*/, 
         OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
         DEFAULT_PITCH | FF_SWISS, NULL);
      SelectObject(dc, font);
      g.setG(dc);
    }
    void clearBackground()
    {
      // clear background
      HBRUSH bkGrnd = CreateSolidBrush(RGB(255, 255, 255));
      FillRect(dc, &ps.rcPaint, bkGrnd);
      DeleteObject(bkGrnd);
    }
    ~SetupDC() 
    {
      DeleteObject(font);
      EndPaint(hwnd, &ps);
      GdiplusShutdown(gdiToken);
    }
  };

}

void RavelCtl::render() 
{
  SetupDC<RavelCairo<HDC>> sdc(*this,hwnd);
  sdc.clearBackground();
  // translate coordinate space so that (0,0) is at centre of Ravel
  OffsetViewportOrgEx(sdc.dc, (int)x, (int)y, NULL);
  RavelCairo::render();
}

void RavelCtl::renderFilter(HWND hwnd) 
{
  SetupDC<FilterCairo<HDC>> sdc(*this,hwnd);
  sdc.clearBackground();
  FilterCairo::render();
}

void RavelCtl::redraw(HWND hwnd) // seems to be needed on Windows 64 bit
{
  RECT rect;
  GetClientRect(hwnd, &rect);
  InvalidateRect(hwnd, &rect, FALSE);
}
	
void RavelCtl::populateSheet()
{
  if (handles.size()<2)
    throw RavelError("No data to ravel!");
  SaveRestoreSelection ss;
  CallXL(xlcWorkbookActivate, workbookName()+destSheet);
    destData = OPERX((int)handles[handleIds[1]].sliceLabels.size(), (int)handles[handleIds[0]].sliceLabels.size());
  populateArray(*this);
  CallXL(xlSet, SSOPERX(sheetId(destSheet), 2,1,(unsigned short)destData.rows(),(unsigned short)destData.columns()),destData);
  // fill in the slice labels
  if (handles[handleIds[0]].collapsed())
    CallXL(xlSet, SSOPERX(sheetId(destSheet), 0, 1), handles[handleIds[0]].reductionDescription());
  else
    {
      auto& xlabels = handles[handleIds[0]].sliceLabels;
      for (unsigned short i = 0, i1=1; i < xlabels.size(); ++i)
	if (handles[handleIds[0]].mask.count(i))
	  CallXL(xlSet, SSOPERX(sheetId(destSheet), 1, i1++), xlabels[i]);
    }
  if (handles[handleIds[1]].collapsed())
    CallXL(xlSet, SSOPERX(sheetId(destSheet), 2, 0), handles[handleIds[1]].reductionDescription());
  else
    {
      auto& ylabels = handles[handleIds[1]].sliceLabels;
      for (unsigned short i = 0, i1=2; i < ylabels.size(); ++i)
	if (handles[handleIds[1]].mask.count(i))
	  CallXL(xlSet, SSOPERX(sheetId(destSheet), i1++, 0), ylabels[i]);
    }
  // make first row of output sheet a descriptive title
  CallXL(xlSet, SSOPERX(sheetId(destSheet), 0, 5), description());
  CallXL(xlcSelect, OPERX(0, 0, 1, int(handles[handleIds[0]].sliceLabels.size())));
  // formatText dosn't bahve according to doc.
//  CallXL(xlcFormatText, OPERX(2), OPERX(1), OPERX(0), OPERX(false), OPERX(true), OPERX(false),OPERX(false),OPERX(false)); // centred
  // set top and bottom borders to a medium line
  CallXL(xlcBorder, 0, 0, 0, 2, 2);

  checkpoint();
}

// creates a RavelCtl with its window, and stores the reference inside the Windows user data field for use by the WindowProc and cleanup
RavelCtl& RavelCtl::create()
{
  IDSHEET id=CallXL(xlSheetId).val.mref.idSheet;
  if (sourceRavels.count(id))
    return *sourceRavels[id];
  else
    {
      string sheetName;
      string workbookNm=workbookName(sheetName);
     
      RavelCtl* r=new RavelCtl;
      r->getSpreadsheetSize();

      // check if a hidden "ravelCheckpoint" exists in the current workbook
      try {
        // throws if the named workbook doesn't exist
	sheetId(checkpointName());
	r->restoreCheckpoint();

        // workbook may have changed since last saved
        

        // check source and destination sheets exist
        try
          {
            sheetId(r->sourceSheet);
	    sheetId(r->destSheet);
	  }
        catch (std::runtime_error)
          {
	    throw RavelError("Ravel checkpoint found, but source/destination sheets missing");
          }
	CallXL(xlcWorkbookActivate, workbookNm + r->sourceSheet);
	CallXL(xlcWorkbookSelect, workbookNm + r->sourceSheet);
	r->loadData();
      }
      catch (RavelError) { throw; }
      catch (std::runtime_error)
        {
          // make a copy of the source workbook, to work from, as ravel
          // needs to colorise the text this temporary workbook will be
          // deleted when te ravel is closed, unless the user has edited
          // the data
	  CallXL(xlcWorkbookCopy, sheetName);
	  CallXL(xlfWindowTitle, "Ravel: " + sheetName);

          workbookNm=workbookName(r->sourceSheet);
	  // create the destination worksheet
          CallXL(xlcWorkbookInsert, 1);
	  // note Excel places a 31 character limit on sheet names
          r->destSheet=("Ravel - "+sheetName).substr(0,31);
	  CallXL(xlcWorkbookName, CallXL(xlSheetNm, CallXL(xlSheetId)),
                 workbookNm+r->destSheet);

          // create the hidden checkpoint sheet
          CallXL(xlcWorkbookInsert, 1);
	  CallXL(xlcWorkbookName, CallXL(xlSheetNm, CallXL(xlSheetId)),
                 workbookNm+checkpointName());
	  CallXL(xlcWorkbookHide,workbookNm+checkpointName(),true/* very hidden*/);
	  r->initSpec();
	  r->initRavel(*r);
	  r->rescale(180);
	  r->updateExcel=true;
	}
      sourceRavels[sheetId(workbookNm+r->sourceSheet)] = 
        destRavels[sheetId(workbookNm+r->destSheet)] = r;
      CallXL(xlcOnEntry, workbookNm+r->sourceSheet, "markRavelDirty");
      CallXL(xlcWorkbookActivate, workbookNm+r->sourceSheet);
      CallXL(xlcWorkbookSelect, workbookNm+r->sourceSheet);
      r->setup();

      return *r; //ownership is handled in WindowProc
    }
}

void RavelCtl::getSpreadsheetSize()
{
 // extract number of rows and columns of the region containing data
  SaveRestoreSelection ss;
  CallXL(xlcSelectLastCell);
  nsRows = (unsigned short)CallXL(xlfGetCell, 2).val.num;
  nsCols = (unsigned short)CallXL(xlfGetCell, 3).val.num;
}

void  RavelCtl::notifyExcel(bool force) 
{
  if (force || updateExcel)
    {
      // F15 sent to excel is bound to getRavelResult macro - hacky!
	  // window handles are only 32 bits for interoperability with 32 bit windows.
#pragma warning(push)
#pragma warning(disable:4312)
		PostMessage((HWND)Excel<XLOPER12>(xlGetHwnd).val.w, WM_KEYDOWN, VK_F15, 1);
#pragma warning(pop)
		updateExcel = false;
    }
}

static string readCheckpoint()
{
  IDSHEET checkpointSheet=sheetId(workbookName()+RavelCtl::checkpointName());
  // because of the idiotic 255 character limit on cell data, the
  // checkpoint is spread over multiple cells
  double numCells=CallXL(xlCoerce,SSOPERX(checkpointSheet,0,0));
  try
  {
	  OPERX savedRavelData = CallXL(xlCoerce, SSOPERX(checkpointSheet, 1, 0, numCells, 1));
	  string savedRavel;
	  for (unsigned i = 0; i < numCells; ++i)
		  savedRavel += str<char>(savedRavelData(i, 0));
	  return savedRavel;
  }
  catch (...)
  {
	  return string();
  }
}

void RavelCtl::restoreCheckpoint()
{
  deserialise(readCheckpoint());
}

void RavelCtl::checkpoint()
{
  string savedRavel=serialise();
  if (savedRavel.empty()) return;
  // if no change, don't try to update
  if (savedRavel==readCheckpoint()) return;

  int numCells=int(savedRavel.length()-1)/255 + 1;
  OPERX dest(numCells+1,1);
  dest(0,0)=numCells;
  for (int i=0; i<numCells; i++)
    dest(i+1,0)=RavelOPERX(savedRavel.substr(i*255,255));
  CallXL(xlSet, SSOPERX(sheetId(workbookName() + checkpointName()), 0, 0, numCells + 1, 1), dest);
}

void RavelCtl::checkSorting()
{
  if (sortRequested)
    {
      sortRequested=false;
      if (sortDir == HandleSort::none)
        {
          handles[handleIds[0]].sliceLabels.order(SortedVector::none);
          unsortAxis(handleIds[0]);
          handles[handleIds[1]].sliceLabels.order(SortedVector::none);
          unsortAxis(handleIds[1]);
        }
      else
        {
          // figure out row or column based on selection
          OPERX sel = CallXL(xlfSelection);
          unsigned rw = 0, cl = 0, nrows = 1, ncols = 1;
          if (sel.xltype == xltypeSRef)
            {
              rw = sel.val.sref.ref.rwFirst;
              cl = sel.val.sref.ref.colFirst;
              nrows = sel.val.sref.ref.rwLast - sel.val.sref.ref.rwFirst + 1;
              ncols = sel.val.sref.ref.colLast - sel.val.sref.ref.colFirst + 1;
            }

          if (nrows >= nsRows)
            {
              if (cl == 0)
                {
                  handles[handleIds[1]].sliceLabels.order(sortDir);
                  unsortAxis(handleIds[1]);
                }
             else if (cl > 0)
                {
                  handles[handleIds[1]].sliceLabels.order(HandleSort::none);
                  sortBy(handleIds[1], cl - 1, sortDir == HandleSort::forward ? 1 : -1);
                }
            }
          else if (ncols >= nsCols)
            {
              if (rw == 1)
                {
                  handles[handleIds[0]].sliceLabels.order(sortDir);
                  unsortAxis(handleIds[0]);
                }
              else if (rw > 1)
                {
                  handles[handleIds[0]].sliceLabels.order(HandleSort::none);
				  sortBy(handleIds[0], rw - 2, sortDir == HandleSort::forward ? 1 : -1);
                }
            }
          else
            XLL_ERROR("Neither a row or column has been selected");
        }
      redraw(hwnd);
    }
}

namespace
{
  struct EnableHovers : public tagTRACKMOUSEEVENT
  {
    EnableHovers(HWND hwnd) {
      cbSize = sizeof(*this);
      dwFlags = TME_HOVER;
      hwndTrack = hwnd;
      dwHoverTime = HOVER_DEFAULT;
    }
  };

  struct LastError: public std::exception
  {
      const char* what() const {
      static char msg[256];
      FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,NULL,GetLastError(),0,msg,256,NULL);
      return msg;
    }
  };
}

// used to refresh Excel after multiple arrow keypresses
static void CALLBACK refreshExcel(HWND hwnd, UINT, UINT_PTR, DWORD)
{
  ((RavelCtl*)GetWindowLongPtr(hwnd, GWLP_USERDATA))->notifyExcel();
}

// returns opposite virtual arrow key
static WPARAM opposite(WPARAM x)
{
  switch (x)
    {
    case VK_LEFT: return VK_RIGHT;
    case VK_RIGHT: return VK_LEFT;
    case VK_UP: return VK_DOWN;
    case VK_DOWN: return VK_UP;
    default: return x;
    }
}

namespace
{
  struct PopupMenu
  {
    HMENU h;
    operator HMENU() const {return h;}
    PopupMenu() {h=CreatePopupMenu();}
    ~PopupMenu() {DestroyMenu(h);}
  };
}


LRESULT CALLBACK RavelCtl::windowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) 
  try {
    RavelCtl* r = (RavelCtl*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (!r) return DefWindowProc(hwnd, msg, wparam, lparam);
	RavelCairo<HDC>& rc = static_cast<RavelCairo<HDC>&>(*r);
    switch (msg)  {
    case WM_LBUTTONDOWN:
      {
		SetupDC<RavelCairo<HDC>> sdc(*r, hwnd);
        POINTS& p = (POINTS&)lparam;
        SetCapture(hwnd);
        rc.onMouseDown(p.x, p.y);
        return 0;
      }
    case WM_LBUTTONDBLCLK:
       {
        POINTS& p = (POINTS&)lparam;
        int handle=r->handleIfMouseOver(p.x-r->x,p.y-r->y);
        if (handle>=0)
          {
            r->handles[handle].toggleCollapsed();
            r->redraw(hwnd);
            r->notifyExcel(true);
          }            
        return 0;
      }
    case WM_RBUTTONDOWN:
      {
        SetupDC<RavelCairo<HDC>> sdc(*r,hwnd);
        POINTS& p = (POINTS&)lparam;
        int handle=r->handleIfMouseOverOpLabel(p.x, p.y);
        if (handle>=0)
          {
            PopupMenu menu;//=CreatePopupMenu();
            for (int i=0; i<sizeof(menuLabels)/sizeof(menuLabels[0]); ++i)
              InsertMenu(menu,~0U, MF_BYPOSITION|MF_STRING, unsigned(i+1), menuLabels[i]);
            POINT pp{p.x,p.y};
            ClientToScreen(hwnd,&pp);
            int sel=TrackPopupMenu(menu, TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD,
                                   pp.x,pp.y,0,hwnd,NULL);
            if (sel>0)
              {
                r->handles[handle].reductionOp=Op::ReductionOp(sel-1);
                r->redraw(hwnd);
                r->notifyExcel(true);
              }
            //DestroyMenu(menu);
          }
        else if ((r->handle=r->handleIfMouseOverAxisLabel(p.x, p.y))>=0)
          {
            if (DialogBox(xllModule, MAKEINTRESOURCE(IDD_DIALOG1),hwnd,nameDlgProc)==-1) 
              throw LastError();
          }
        else if ((handle=r->handleIfMouseOver(p.x-r->x, p.y-r->x))>=0 &&
                 handle==int(r->handleIds[0])||handle==int(r->handleIds[1]))
          {
			Handle& h = r->handles[handle];
            POINT pp{p.x,p.y};
            ClientToScreen(hwnd,&pp);

            PopupMenu menu;//=CreatePopupMenu();

            InsertMenu(menu,~0U, MF_BYPOSITION|MF_STRING, 1, _T("Sort by slice label"));
            InsertMenu(menu,~0U, MF_BYPOSITION|MF_STRING|
                       (h.displayFilterCaliper?MF_CHECKED:0),
                       2, _T("Filter slice labels"));
            
            switch (TrackPopupMenu
                    (menu, TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD,
                     pp.x,pp.y,0,hwnd,NULL))
              {
              case 1:
                {
                  PopupMenu l_menu;
                  
                  InsertMenu(l_menu,~0U, MF_BYPOSITION|MF_STRING, 1, 
                             _T("Original Order"));
                  InsertMenu(l_menu,~0U, MF_BYPOSITION|MF_STRING, 2, 
                             _T("Sort by slice label"));
                  InsertMenu(l_menu,~0U, MF_BYPOSITION|MF_STRING, 3, 
                             _T("Reverse sort by slice label"));
                  InsertMenu(l_menu,~0U, MF_BYPOSITION|MF_STRING, 4, 
                             _T("Numerically sort by slice label"));
                  InsertMenu(l_menu,~0U, MF_BYPOSITION|MF_STRING, 5, 
                             _T("Reverse numerically sort by slice label"));
                  // set a check mark against which sort order is
                  // already selected
		  int order = int(h.sliceLabels.order());
		  if (order<5)
		    CheckMenuItem(l_menu,order+1,MF_CHECKED);

                  int sel=TrackPopupMenu
                    (l_menu, TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD,
                     pp.x,pp.y,0,hwnd,NULL);
		  if (sel>0)
		    {
		      h.sliceLabels.order(SortedVector::Order(sel - 1));
                      r->unsortAxis(handle);
		      r->redraw(hwnd);
		      r->notifyExcel(true);
		    }
                }
                break;
              case 2: /* filtering */
                h.displayFilterCaliper=!h.displayFilterCaliper;
                break;
              }
          }
      }
    case WM_LBUTTONUP:
      {
        POINTS& p = (POINTS&)lparam;
        ReleaseCapture();
        rc.onMouseUp(p.x, p.y);
        r->redraw(hwnd);
        r->notifyExcel();
        return 0;
      }
    case WM_KEYDOWN:
      return 0;
    case WM_KEYUP: 
      if (r->handle>=0)
        {
          Handle& handle=r->handles[r->handle];
         switch (wparam)
            {
            case VK_RIGHT: case VK_UP:
              if (handle.sliceIndex < handle.sliceLabels.size()-1)
                handle.sliceIndex++;
              r->updateExcel=true;
              break;
            case VK_LEFT: case VK_DOWN:
              if (handle.sliceIndex>0)
                handle.sliceIndex--;
              r->updateExcel=true;
              break;
            }
          r->redraw(hwnd);
          SetTimer(hwnd,1,1000,refreshExcel);
          // undo cell movement in main application. Excel greedily grabs WM_KEYDOWN, we don't ever see it here
          // This is weird, because modal dialog boxes effectively steal focus from Excel - what am I doing wrong?
          PostMessage(GetParent(hwnd),WM_KEYDOWN, opposite(wparam), lparam);
        }
      return 0;
   case WM_MOUSEMOVE:
      {
        POINTS& p = (POINTS&)lparam;
        if (r->onMouseOver(p.x, p.y)) r->redraw(hwnd);
        r->handle=r->handleIfMouseOver(p.x-r->x,p.y-r->y);
        if (wparam&MK_LBUTTON &&  r->onMouseMotion(p.x, p.y))
          {
            r->updateExcel = true;
            r->redraw(hwnd);
          }
      }
      return 0;
    case WM_SIZE:
      r->x = 0.5*LOWORD(lparam);
      r->y = 0.5*HIWORD(lparam);
      r->rescale(max(0.85*min(r->x, r->y), 1.0));
      r->redraw(hwnd);
      return 0;
    case WM_PAINT:
      r->render();
      return 0;
    case WM_CLOSE:
      delete r;
      return DefWindowProc(hwnd, msg, wparam, lparam);
    default:
      return DefWindowProc(hwnd, msg, wparam, lparam);
    }
  }
  catch (std::exception& ex)
    {
      XLL_ERROR(ex.what());
      return 0;
    }

LRESULT CALLBACK RavelCtl::filterWidgetProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) 
  try {
    RavelCtl* r=(RavelCtl*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (!r) return DefWindowProc(hwnd, msg, wparam, lparam);
    FilterCairo<HDC>& f = static_cast<FilterCairo<HDC>&>(*r);
    //SetupDC<FilterCairo<HDC>> sdc(f,hwnd);
    switch (msg)  {
    case WM_LBUTTONDOWN:
      {
        POINTS& p = (POINTS&)lparam;
        SetCapture(hwnd);
        f.onMouseDown(p.x, p.y);
        return 0;
      }
    case WM_LBUTTONUP:
      {
        POINTS& p = (POINTS&)lparam;
        ReleaseCapture();
        f.onMouseUp(p.x, p.y);
        r->redraw(hwnd);
        r->notifyExcel(true);
        return 0;
      }
   case WM_MOUSEMOVE:
      {
        POINTS& p = (POINTS&)lparam;
        if (wparam&MK_LBUTTON)
          {
            r->onMouse(p.x, p.y);
            r->redraw(hwnd);
          }
      }
      return 0;
    case WM_SIZE:
      f.width = LOWORD(lparam);
      f.height = HIWORD(lparam)-20;
//	  r->renderFilter(hwnd);
	  r->redraw(hwnd);
      return 0;
    case WM_PAINT:
		if (f.width>0 && f.height>0)
      r->renderFilter(hwnd);
      return 0;
    case WM_CLOSE:
      return DefWindowProc(hwnd, msg, wparam, lparam);
    default:
      return DefWindowProc(hwnd, msg, wparam, lparam);
    }
  }
  catch (std::exception& ex)
    {
      XLL_ERROR(ex.what());
      return 0;
    }


// handle reduction operation selections
LRESULT CALLBACK RavelCtl::reductionOpCtlProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  RavelCtl* r = (RavelCtl*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
  switch (msg)
    {
    case WM_COMMAND:
      if (HIWORD(wparam) == BN_CLICKED)
        {
          for (Handle& h: r->handles)
            h.reductionOp = Op::ReductionOp(LOWORD(wparam));
          r->nextRedOp=Op::ReductionOp(LOWORD(wparam));
          r->redraw(r->hwnd);
          r->notifyExcel(true);
        }
      return 0;
    default:
      return DefWindowProc(hwnd, msg, wparam, lparam);
    }
}
		
// handle reduction operation selections
LRESULT CALLBACK RavelCtl::chartSelectorProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  RavelCtl* r = (RavelCtl*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
  switch (msg)
    {
    case WM_COMMAND:
      if (HIWORD(wparam) == BN_CLICKED)
        {
          r->chart = charts[LOWORD(wparam)];
          r->notifyExcel(true);
        }
      return 0;
    default:
      return DefWindowProc(hwnd, msg, wparam, lparam);
    }
}
		
INT_PTR CALLBACK RavelCtl::nameDlgProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  TopLvlWndData* td = (TopLvlWndData*)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);
  RavelCtl* r = (RavelCtl*)GetWindowLongPtr(td->ravelWnd, GWLP_USERDATA);
  switch (msg)
    {
    case WM_INITDIALOG:
      {
        basic_string<TCHAR> previousName=utf_to_utf<TCHAR>(r->handles[r->handle].description);
        SetDlgItemText(hwnd,0,previousName.c_str());
        return TRUE;
      }
    case WM_COMMAND:
      switch (LOWORD(wparam))
        {
        case IDOK:
          {
            const int maxName=40;
            TCHAR name[maxName];
            name[0]=_T('\0');
            GetDlgItemText(hwnd, 0, name, maxName);
            r->handles[r->handle].description=utf_to_utf<char>(name);
            r->notifyExcel(true);
          }
          // fall through
        case IDCANCEL:
          EndDialog(hwnd, NULL);
          return TRUE;
        }
    }
  return FALSE;
}

void RavelCtl::addChart() const
{

  if (!handles.empty())
    {
      SaveRestoreSelection ss;
      // clear selection of chart (which causes problem with later select)
      CallXL(xlcSelect, OPERX(0, 0, 1,1));
      unsigned nsRows = handles[handleIds[1]].collapsed() ? 2 :
        (unsigned)handles[handleIds[1]].sliceLabels.size() + 1;
      unsigned nsCols = handles[handleIds[0]].collapsed() ? 2 :
        (unsigned)handles[handleIds[0]].sliceLabels.size() + 1;
      CallXL(xlcSelect, OPERX(1, 0, nsRows, nsCols));
      try
        {
          CallXL(xlcRun,"'ravel.xlsm'!DisplayRavelChart");
        }
      catch (...) {} // I don't know why the chart occasionally fails
    }
}
	
RavelCtl::~RavelCtl()
{
  if (hwnd) {
    try {
      checkpoint();
      destRavels.erase(sheetId(destSheet));
      sourceRavels.erase(sheetId(sourceSheet));
      sheetsToDelete[sheetId(destSheet)] = SheetInfo{ sheetId(sourceSheet), dirty };
      sheetsToDelete[sheetId(sourceSheet)] = SheetInfo{ sheetId(destSheet), dirty };
      notifyExcel(true);
    }
    catch (...) {}
  }
}

