#include "ravelControl.h"
#include "splitMerge.h"
#include <Shlwapi.h>

#include <classdesc_epilogue.h>

#ifdef _WIN64
extern "C" int xll_alert_filter();
extern "C" int xll_alert_filter12() { return xll_alert_filter(); }
#endif

namespace
{
#ifdef _WIN64
  HMODULE xllModule=GetModuleHandle(_T("Ravel_x64.xll"));
#else
  HMODULE xllModule=GetModuleHandle(_T("Ravel_Win32.xll"));
#endif
}

MACRO(newRavel)
{
#pragma XLLEXPORT
  try
    {
      RavelCtl& r = RavelCtl::create();

      // selection operations can only be performed on active sheet
      // when macro entered, so we need to call back to excel to
      // colour the sourcesheet
      CallXL(xlcWorkbookActivate, r.sourceSheet);
      CallXL(xlcWorkbookSelect, r.sourceSheet);
      r.notifyExcel();
    }
  catch (const std::exception& ex)
    {
      XLL_ERROR(ex.what(),true);
      return 0;
    }
  return 1;
}

MACRO(getRavelResult)
try
  {
#pragma XLLEXPORT
    IDSHEET id = CallXL(xlSheetId).val.mref.idSheet;
    if (sourceRavels.count(id))
      {
        RavelCtl&r = *sourceRavels[id];
        r.colourAxes();
        r.populateSheet();
        CallXL(xlcWorkbookActivate, r.destSheet);
        CallXL(xlcWorkbookSelect, r.destSheet);
        SSOPERX cell(CallXL(xlSheetId, r.destSheet).val.mref.idSheet, 0, 0);
      }
    if (destRavels.count(id))
      {
        RavelCtl& r = *destRavels[id];
        r.checkSorting();
          
        SaveRestoreSelection ss;
        // clear existing data
        CallXL(xlcSelectLastCell);
        unsigned nsRows = (unsigned short)CallXL(xlfGetCell, 2).val.num;
        unsigned nsCols = (unsigned short)CallXL(xlfGetCell, 3).val.num;
        CallXL(xlSet, OPERX(0, 0, nsRows, nsCols));
        CallXL(xlcSelect, OPERX(0, 0, 1, 1));
        r.populateSheet();
        r.addChart();
      }
    else if (sheetsToDelete.count(id))
      {
        const SheetInfo& si = sheetsToDelete[id];
        if (si.dirty)
          CallXL(xlcClose); // prompts user about saving
        else
          CallXL(xlcClose, false, false); // does not so prompt
        sheetsToDelete.erase(si.other);
        sheetsToDelete.erase(id);
      }
    return 1;
  }
 catch (std::exception& ex)
   {
     XLL_ERROR(ex.what(),true);
     return 0;
   }

static AddInX s_ravelChartType
( 
 FunctionX(_T(XLL_WORD),_T("?xai_ravelChartType"),_T("ravelChartType")).
 FunctionHelp(_T("Get the ravel chart attribute"))
  );

int WINAPI xai_ravelChartType()
{
#pragma XLLEXPORT
  IDSHEET id = CallXL(xlSheetId).val.mref.idSheet;
  if (destRavels.count(id))
    return destRavels[id]->chart;
  else
    return 0;
}

MACRO(markRavelDirty)
{
#pragma XLLEXPORT
  IDSHEET id = CallXL(xlSheetId).val.mref.idSheet;
  if (sourceRavels.count(id))
    sourceRavels[id]->dirty = true;
  return 1;
}
  
enum AxisComment {axis,comment};
int colRowMenu(ColRow colRow, AxisComment axisComment)
  try
    {
      RavelCtl& r = RavelCtl::create();

      unsigned short cl=(unsigned short)CallXL(xlfGetCell, 3).val.num;
      unsigned short rw = (unsigned short)CallXL(xlfGetCell, 2).val.num;
      OPERX sel = CallXL(xlfSelection);
      unsigned nrows = 1, ncols = 1;
      if (sel.xltype == xltypeSRef)
        {
          nrows = sel.val.sref.ref.rwLast - sel.val.sref.ref.rwFirst+1;
          ncols = sel.val.sref.ref.colLast - sel.val.sref.ref.colFirst+1;
        }
      if (axisComment==axis)
        if (colRow==col)
          r.spec.nColAxes = cl;
        else
          r.spec.nRowAxes = rw;
      else
        if (colRow == col)
          for (unsigned i = 0; i < ncols; ++i)
            r.spec.commentCols.insert(cl-1+i);
        else
          for (unsigned i = 0; i < nrows; ++i)
            r.spec.commentRows.insert(rw-1+i);

      try
        {
          r.colourAxes();
          r.loadData();
          r.initRavel(r);
          r.render();
          r.redraw(r.hwnd);
          r.populateSheet();
        }
      catch (RavelError)
        {
          throw; // propagate ravel specific errors to an error display
        }
      catch (std::exception)
        { //ignore errors which may be due to not having a valid configuration yet
        }
      return 1;
    }
  catch (std::exception& ex)
    {
      XLL_ERROR(ex.what(),true);
      return 0;
    }

MACRO(setAxisColumn)
{
#pragma XLLEXPORT
  return colRowMenu(col, axis);
}
MACRO(setAxisRow)
{
#pragma XLLEXPORT
  return colRowMenu(row, axis);
}
MACRO(setCommentColumn)
{
#pragma XLLEXPORT
  return colRowMenu(col, comment);
}
MACRO(setCommentRow)
{
#pragma XLLEXPORT
  return colRowMenu(row, comment);
}

MACRO(selectRavelData) 
{
#pragma XLLEXPORT
  IDSHEET id = CallXL(xlSheetId).val.mref.idSheet;
  if (destRavels.count(id))
    {
      RavelCtl& r=*destRavels[id];
      if (!r.handles.empty())
        {
          unsigned nsRows = r.handles[r.yHandleId()].collapsed() ? 2 :
            (unsigned)r.handles[r.yHandleId()].sliceLabels.size() + 1;
          unsigned nsCols = r.handles[r.xHandleId()].collapsed() ? 2 :
            (unsigned)r.handles[r.xHandleId()].sliceLabels.size() + 1;
          CallXL(xlcSelect, OPERX(1, 0, nsRows, nsCols));
        }
      return 1;
    }
  else
    {
      XLL_ERROR("Not a Ravel output sheet",true);
      return 0;
    }
}

MACRO(restoreRavel) 
try
  {
#pragma XLLEXPORT
    IDSHEET id = CallXL(xlSheetId).val.mref.idSheet;
    if (sourceRavels.count(id)==0 && destRavels.count(id)==0)
      {
        // check if a checkpoint exists
        sheetId(RavelCtl::checkpointName());
        return xai_newRavel();
      }
    return 1;
  }
 catch (...)
   {
     // no checkpoint present, so do nothing
     return 1;
   }

MACRO(onWorkbookClose)
{
#pragma XLLEXPORT
  IDSHEET id = sheetId();
  auto r=sourceRavels.find(id);
  if (r==sourceRavels.end())
    {
      r=destRavels.find(id);
      if (r==destRavels.end())
        return 1; // nothing to clean up
    }
  SendMessage(GetParent(r->second->hwnd),WM_CLOSE, 0,0);
  return 1;
}

int sortBy(ColRow colRow, int dir)
{
  IDSHEET id = sheetId();
  auto i=destRavels.find(id);
  if (i!=destRavels.end())
    {
      RavelCtl& r=*i->second;
      if (colRow==col)
        {
          unsigned short cl=(unsigned short)CallXL(xlfGetCell, 3).val.num;
          if (cl==1)
            r.handles[r.yHandleId()].sliceLabels.order(dir>0?SortedVector::forward:SortedVector::reverse);
          else if (cl>1)
            r.sortBy(r.yHandleId(), cl-2, dir);
        }
      else
        {
          unsigned short rw = (unsigned short)CallXL(xlfGetCell, 2).val.num;
          if (rw==2)
            r.handles[r.xHandleId()].sliceLabels.order(dir>0?SortedVector::forward:SortedVector::reverse);
          else if (rw>2)
            r.sortBy(r.xHandleId(), rw-3, dir);
        }
      r.populateSheet();
      r.addChart();
      r.redraw(r.hwnd);
    }
  return 1;
}

MACRO(sortRowsByColumn)
{
#pragma XLLEXPORT
  return sortBy(col,1);
}

MACRO(sortColumnsByRow)
{
#pragma XLLEXPORT
  return sortBy(row,1);
}

MACRO(reverseSortRowsByColumn)
{
#pragma XLLEXPORT
  return sortBy(col,-1);
}

MACRO(reverseSortColumnsByRow)
{
#pragma XLLEXPORT
  return sortBy(row,-1);
}

MACRO(originalRowOrder)
{
#pragma XLLEXPORT
  IDSHEET id = sheetId();
  auto i=destRavels.find(id);
  if (i!=destRavels.end())
    {
      RavelCtl& r=*i->second;
      r.unsortAxis(r.yHandleId());
      r.handles[r.yHandleId()].sliceLabels.order(SortedVector::none);
      r.populateSheet();
      r.redraw(r.hwnd);
      r.addChart();
    }
  return 1;
}

MACRO(originalColumnOrder)
{
#pragma XLLEXPORT
  IDSHEET id = sheetId();
  auto i=destRavels.find(id);
  if (i!=destRavels.end())
    {
      RavelCtl& r=*i->second;
      r.unsortAxis(r.xHandleId());
      r.handles[r.xHandleId()].sliceLabels.order(SortedVector::none);
      r.populateSheet();
      r.redraw(r.hwnd);
      r.addChart();
    }
  return 1;
}

// implements base operations for the selected sheet
struct XLSplitMerge: public SplitMerge
{
  RavelCtl& r;
  XLSplitMerge(RavelCtl& r) : r(r) { r.getSpreadsheetSize(); }

  std::string get(size_t row, size_t col) const override
  {return str<char>(CallXL(xlCoerce,OPERX(row, col,1,1)));}
  void set(size_t row, size_t col, const std::string& val) override
  {CallXL(xlSet,OPERX(row,col,1,1),val);}
  size_t startRow() const override {return r.spec.nRowAxes;}
  size_t startCol() const override {return r.spec.nColAxes;}
  size_t numRows() const override {return r.nsRows;}
  size_t numCols() const override {return r.nsCols;}
  void insertRows(size_t rw, size_t n) override 
  {
    CallXL(xlcSelect,OPERX(rw,0,n,r.nsCols));
    CallXL(xlcInsert,3);
    if (rw<r.spec.nRowAxes) r.spec.nRowAxes+=n;
    r.nsRows+=n;
  }
  void insertCols(size_t c, size_t n) override 
  {
    CallXL(xlcSelect,OPERX(0,c,r.nsRows,n));
    CallXL(xlcInsert,4);
    if (c<r.spec.nColAxes) r.spec.nColAxes+=n;
    r.nsCols+=n;
  }
  void deleteRows(size_t rw, size_t n) override 
  {
    rw=min(rw, size_t(r.nsRows-1));
    n=min(n, size_t(r.nsRows-rw));
    CallXL(xlcSelect,OPERX(rw,0,n,r.nsCols));
    CallXL(xlcEditDelete,3);
    r.nsRows-=n;
    if (rw<r.spec.nRowAxes)
      r.spec.nRowAxes-=min(n, r.spec.nRowAxes-rw);
  }
  void deleteCols(size_t c, size_t n) override 
  {
    c=min(c, size_t(r.nsCols-1));
    n=min(n, size_t(r.nsCols-c));
    CallXL(xlcSelect,OPERX(0,c,r.nsRows,n));
    CallXL(xlcEditDelete,4);
    r.nsCols -= n;
    if (c<r.spec.nColAxes)
      r.spec.nColAxes-=min(n, r.spec.nColAxes-c);
  }
};


int split(ColRow colRow)
  try
    {
      IDSHEET id = sheetId();
      auto i=sourceRavels.find(id);
      if (i!=sourceRavels.end())
        {
          RavelCtl& r=*i->second;
          XLSplitMerge xlsm(r);
          unsigned rw=0, cl=0;
          OPERX sel = CallXL(xlfSelection);
          if (sel.xltype == xltypeSRef)
            {
              rw=sel.val.sref.ref.rwFirst;
              cl=sel.val.sref.ref.colFirst;
              if (colRow==row && rw<=r.spec.nRowAxes+1)
                xlsm.splitRow(rw);
              else if (colRow==col && cl<=r.spec.nColAxes+1)
                xlsm.splitCol(cl);
              r.loadData();
              r.initRavel(r);
              r.render();
              r.redraw(r.hwnd);
              r.populateSheet();
            }
          return 1;
        }

      XLL_ERROR("You can only split the axes row/columns of a Ravel input sheet",true);
      return 0;
    }
  catch (const RavelError& ex)
    {
      XLL_ERROR(ex.what());
      return 0;
    }
  catch (...)
    {
      // no checkpoint present, so do nothing
      return 1;
    }

MACRO(splitColumn)
{
#pragma XLLEXPORT
  return split(col);
}

MACRO(splitRow)
{
#pragma XLLEXPORT
  return split(row);
}

void merge(ColRow colRow)
  try
    {
      IDSHEET id = sheetId();
      auto i=sourceRavels.find(id);
      if (i!=sourceRavels.end())
        {
          OPERX sel = CallXL(xlfSelection);
          unsigned rw=0, cl=0,nrows = 1, ncols = 1;
          if (sel.xltype == xltypeSRef)
            {
              rw=sel.val.sref.ref.rwFirst;
              cl=sel.val.sref.ref.colFirst;
              nrows = sel.val.sref.ref.rwLast - sel.val.sref.ref.rwFirst+1;
              ncols = sel.val.sref.ref.colLast - sel.val.sref.ref.colFirst+1;
            }

          RavelCtl& r=*i->second;
          XLSplitMerge xlsm(r);

          if (colRow==col)
            xlsm.mergeCols(cl,ncols);
          else
            xlsm.mergeRows(rw,nrows);
          r.loadData();
          r.initRavel(r);
          r.render();
          r.redraw(r.hwnd);
          r.populateSheet();
        }
    }
  catch (const RavelError& ex)
    {
      XLL_ERROR(ex.what());
    }
  catch (...)
    {
    }

MACRO(mergeColumn)
{
#pragma XLLEXPORT
  merge(col);
  return 1;
}

MACRO(mergeRow)
{
#pragma XLLEXPORT
  merge(row);
  return 1;
}

// add in extra menu items for selecting axes
static bool initialised = false;

int init()
{
  // for some bizarre reason, this is called twice on open, but we only want to do this once
  if (!initialised)
    {
      // open ravel.xlsm, which contains VBA support macros
      const int fnameSz=1000;
      TCHAR fname[fnameSz];
      GetModuleFileName(xllModule,fname,fnameSz);
      PathRemoveFileSpec(fname);
      CallXL(xlcOpen,(fname+basic_string<TCHAR>(_T("/ravel.xlsm"))).c_str(),0,true/*read-only*/);

      // add command to column select context menu
      OPERX colCmd(8, 2);
      colCmd(0,0) = OPERX(_T("-"));
      colCmd(1, 0) = OPERX(_T("Make column a Ravel axis"));
      colCmd(1, 1) = OPERX(_T("setAxisColumn"));
      colCmd(2, 0) = OPERX(_T("Ignore column (treat as comment)"));
      colCmd(2, 1) = OPERX(_T("setCommentColumn"));
      colCmd(3, 0) = OPERX(_T("Split"));
      colCmd(3, 1) = OPERX(_T("splitColumn"));
      colCmd(4, 0) = OPERX(_T("Merge"));
      colCmd(4, 1) = OPERX(_T("mergeColumn"));
      colCmd(5, 0) = OPERX(_T("Sort rows by column"));
      colCmd(5, 1) = OPERX(_T("sortRowsByColumn"));
      colCmd(6, 0) = OPERX(_T("Reverse sort rows by column"));
      colCmd(6, 1) = OPERX(_T("reverseSortRowsByColumn"));
      colCmd(7, 0) = OPERX(_T("Original row order"));
      colCmd(7, 1) = OPERX(_T("originalRowOrder"));
      CallXL(xlfAddCommand, 7, 5, colCmd);
      // add command to row select context menu
      OPERX rowCmd(8, 2);
      rowCmd(0, 0) = OPERX(_T("-"));
      rowCmd(1, 0) = OPERX(_T("Make row a Ravel axis"));
      rowCmd(1, 1) = OPERX(_T("setAxisRow"));
      rowCmd(2, 0) = OPERX(_T("Ignore row (treat as comment)"));
      rowCmd(2, 1) = OPERX(_T("setCommentRow"));
      rowCmd(3, 0) = OPERX(_T("Split"));
      rowCmd(3, 1) = OPERX(_T("splitRow"));
      rowCmd(4, 0) = OPERX(_T("Merge"));
      rowCmd(4, 1) = OPERX(_T("mergeRow"));
      rowCmd(5, 0) = OPERX(_T("Sort columns by row"));
      rowCmd(5, 1) = OPERX(_T("sortColumnsByRow"));
      rowCmd(6, 0) = OPERX(_T("Reverse sort columns by row"));
      rowCmd(6, 1) = OPERX(_T("reverseSortColumnsByRow"));
      rowCmd(7, 0) = OPERX(_T("Original column order"));
      rowCmd(7, 1) = OPERX(_T("originalColumnOrder"));
      CallXL(xlfAddCommand, 7, 6, rowCmd);
      // add a command to the workbook context menu
      OPERX cellCmd(1, 2);
      cellCmd(0, 0) = OPERX(_T("Select all ravel data"));
      cellCmd(0, 1) = OPERX(_T("selectRavelData"));
      CallXL(xlfAddCommand, 7, 4, cellCmd);
     
      OPERX ravelCommands(2,2);
      ravelCommands(0,0)=OPERX(_T("Ravel:"));
      ravelCommands(1,0)=OPERX(_T(" Create Ravel"));
      ravelCommands(1,1)=OPERX(_T("newRavel"));
      CallXL(xlfAddCommand,1,1,ravelCommands);

      // very hacky! This allows another thread to signal Excel to reload the Ravel results by sending a F15 key event
      CallXL(xlcOnKey, "{F15}", "getRavelResult");
      //CallXL(xlcOnKey, "{F15}", "'ravel.xlsm'!RavelResults");
      initialised = true;

      CallXL(xlcOnSheet,OPERX(),"restoreRavel");

    }
  return 1;
}
static Auto<Open> s_init(init);

int finalise()
{
  // TODO - remove added commands
  initialised = false;
  return 1;
}
static Auto<Close> s_finalise(finalise);
