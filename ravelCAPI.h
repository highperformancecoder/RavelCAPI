/*
  Ravel C API. © Ravelation Pty Ltd 2018
*/

#ifndef RAVELCAPI_H
#define RAVELCAPI_H
#include "capiRenderer.h"
#include "capiCivita.h"
#include <cairo/cairo.h>
#include <stdlib.h>

#if defined(__cplusplus) && __cplusplus >= 201103L
#define NOEXCEPT noexcept
#else
#define NOEXCEPT
#endif

struct CAPIRavel;
typedef struct CAPIRavel CAPIRavel;

#include "ravelCAPITypes.h"

struct CAPIRavelDatabase;
typedef struct CAPIRavelDatabase CAPIRavelDatabase;

#ifdef __cplusplus
extern "C" {
#endif
  /// returns number of days until license expires
  int ravel_days_until_expiry() NOEXCEPT;
  /// returns the major version number of this API. This gets bumped
  /// whenever a method is removed or signature changes, but not when
  /// a method is added
  int ravel_capi_version() NOEXCEPT;
  /// returns the what() msg of last exception thrown.
  const char* ravel_lastErr() NOEXCEPT;
  /// returns the release version of this library
  const char* ravel_version() NOEXCEPT;

  /// create a new Ravel control widget
  CAPIRavel* ravel_new(size_t rank) NOEXCEPT;
  /// dispose of ravel created by ravel_new()
  void ravel_delete(CAPIRavel* ravel) NOEXCEPT;
  /// removes all handles
  void ravel_clear(CAPIRavel* ravel) NOEXCEPT;

  /// render ravel widget into a Cairo context
  void ravel_render(CAPIRavel* ravel, CAPIRenderer* cairo) NOEXCEPT;
  /// @{ handle mouse events
  void ravel_onMouseDown(CAPIRavel* ravel, double x, double y) NOEXCEPT;
  void ravel_onMouseUp(CAPIRavel* ravel, double x, double y) NOEXCEPT;
  /// handle mouse motion with button pressed
  /// @ return true if it needs to be rerendered
  int ravel_onMouseMotion(CAPIRavel* ravel, double x, double y) NOEXCEPT;
  /// hande mouse motion without pressed button (tooltips etc)
  /// @ return true if it needs to be rerendered
  int ravel_onMouseOver(CAPIRavel* ravel, double x, double y) NOEXCEPT;
  /// handle mouse movements leaving the ravel
  void ravel_onMouseLeave(CAPIRavel* ravel) NOEXCEPT;
  /// @}
  /// resize a ravel
  void ravel_rescale(CAPIRavel* ravel, double radius) NOEXCEPT;
  /// radius of ravel in screen coordinates
  double ravel_radius(CAPIRavel* ravel) NOEXCEPT;
  /// rank (no. output handles)
  size_t ravel_rank(CAPIRavel* ravel) NOEXCEPT;
  /// descriptive text of the operation of the Ravel (plain English for now)
  /// String buffer remains valid until next Ravel CAPI call.
  const char* ravel_description(CAPIRavel* ravel) NOEXCEPT;
  /// descriptive text of the operation of the Ravel state (plain English for now)
  /// String buffer remains valid until next Ravel CAPI call.
  const char* ravel_state_description(const CAPIRavelState* state) NOEXCEPT;

  /// sets an explanatory message displayed as a tooltip @param
  /// explain a message, if empty, then a default explanatory message
  /// for the item at (x,y) is used
  /// @param x,y location to display the tooltip
  void ravel_setExplain(CAPIRavel* ravel, const char* explain, double x, double y) NOEXCEPT;
  /// resets the explain message
  void ravel_resetExplain(CAPIRavel* ravel) NOEXCEPT;
  /// obtain default explanatory message for item at (x,y)
  const char* ravel_explain(CAPIRavel* ravel, double x, double y) NOEXCEPT;
  
  /// return the handle IDs of the output handles, in order x,y,z, etc.
  /// ids must be ravel_rank() in size
  void ravel_outputHandleIds(CAPIRavel* ravel, size_t ids[]) NOEXCEPT;
  /// set the output handles. \a ids[] must be of length \a rank.
  void ravel_setOutputHandleIds(CAPIRavel* ravel, size_t rank, const size_t ids[]) NOEXCEPT;
  /// number of handles (aka axes)
  unsigned ravel_numHandles(CAPIRavel* ravel) NOEXCEPT;
  /// current handle mouse is over, or -1 if none
  int ravel_selectedHandle(CAPIRavel* ravel) NOEXCEPT;
  /// return the description field for handle \a handle.
  const char* ravel_handleDescription(CAPIRavel* ravel, int handle) NOEXCEPT;
  /// set the description field for \a handle
  void ravel_setHandleDescription(CAPIRavel* ravel, int handle, const char* description) NOEXCEPT;
  /// number of slice labels along axis \a axis
  size_t ravel_numSliceLabels(CAPIRavel* ravel, size_t axis) NOEXCEPT;
  /// number of slice labels along axis \a axis, disregarding calipers
  size_t ravel_numAllSliceLabels(CAPIRavel* ravel, size_t axis) NOEXCEPT;
  /** returns the sliceLabels along axis \a axis 
      \a labels must be at least ravel_numSliceLabels in size.  

      The returned pointers are owned by the ravel object, and remain
      valid until the next call to ravel_sliceLabels. Do not delete or
      free these.
  **/
  void ravel_sliceLabels(CAPIRavel* ravel, size_t axis, const char* labels[]) NOEXCEPT;
  /** returns the sliceLabels along axis \a axis 
      \a labels must be at least ravel_numAllSliceLabels in size.  

      The returned pointers are owned by the ravel object, and remain
      valid until the next call to ravel_sliceLabels. Do not delete or
      free these.
  **/
  void ravel_allSliceLabels(CAPIRavel* ravel, size_t axis, enum RavelOrder order, const char* labels[]) NOEXCEPT;
  /// enable/disable the filter calipers on axis \a axis
  void ravel_displayFilterCaliper(CAPIRavel* ravel, size_t axis, BOOL display) NOEXCEPT;
  /// set the slicer to \a sliceLabel, if it exists
  void ravel_setSlicer(CAPIRavel* ravel, size_t axis, const char* sliceLabel) NOEXCEPT;
  /// set calipers to \a l1, l2
  void ravel_setCalipers(CAPIRavel* ravel, size_t axis, const char* l1, const char* l2) NOEXCEPT;
  /// get current caliper positions
  void ravel_getCaliperPositions(CAPIRavel* ravel, size_t axis, size_t* p1, size_t* p2) NOEXCEPT;
  /// set caliper positions to \a p1, p2. Note: p1 must be less than numSliceLabels, otherwise it is ignored.
  void ravel_setCaliperPositions(CAPIRavel* ravel, size_t axis, size_t p1, size_t p2) NOEXCEPT;
  /// set the ordering on handle \a axis to \a order
  void ravel_orderLabels(CAPIRavel* ravel, size_t axis, enum RavelOrder order) NOEXCEPT;

  /// sets the type of the next reduction operation
  void ravel_nextReduction(CAPIRavel* ravel, enum RavelReductionOp) NOEXCEPT;
  /// set the reduction type for \a handle
  void ravel_handleSetReduction(CAPIRavel* ravel, int handle, enum RavelReductionOp) NOEXCEPT;
  
  /// apply a custom permutation of axis labels (which may be less than the number of labels)
  /// indices is an array of length numIndices
  void ravel_applyCustomPermutation(CAPIRavel* ravel, size_t axis, size_t numIndices, const size_t* indices) NOEXCEPT;
  /// get the current permutation of axis labels. numIndices is the size of the target array, which should be >=numSliceLabels.
  /// elements beyond numSliceLabels are undefined
  void ravel_currentPermutation(CAPIRavel* ravel, size_t axis, size_t numIndices, size_t* indices) NOEXCEPT;
  
  /// add a handle to the Ravel. \a sliceLabels is of length \a numSliceLabels. Ownership of char pointers not passed.
  void ravel_addHandle(CAPIRavel* ravel, const char* description,
                       size_t numSliceLabels, const char* sliceLabels[]) NOEXCEPT;
  
  /// return XML represention of ravel. String buffer remains valid until next Ravel CAPI call.
  const char* ravel_toXML(CAPIRavel* ravel) NOEXCEPT;
  /// populate with XML data. @return true on success
  int ravel_fromXML(CAPIRavel* ravel, const char*) NOEXCEPT;
  /// get the handle state (user modifiable attributes of handle \a
  /// handle). The returned object is valid until the next call to
  /// this function.
  const CAPIRavelHandleState* ravel_getHandleState(CAPIRavel* ravel, size_t handle) NOEXCEPT;
  /// set the handle state
  void ravel_setHandleState(CAPIRavel* ravel, size_t handle, const CAPIRavelHandleState* handleState) NOEXCEPT;
  /// get the ravel state (user modifiable attributes of handle \a
  /// handle). The returned object is valid until the next call to
  /// this function. Never returns NULL.
  const CAPIRavelState* ravel_getRavelState(CAPIRavel* ravel) NOEXCEPT;
  /// set the ravel state
  void ravel_setRavelState(CAPIRavel* ravel, const CAPIRavelState* handleState) NOEXCEPT;
  /// adjust handle slicer mouse is over up or down by n points
  void ravel_adjustSlicer(CAPIRavel* ravel, int) NOEXCEPT; 

  /// redistribute handles according to current state
  void ravel_redistributeHandles(CAPIRavel* ravel) NOEXCEPT;

  /// arrange for the output handle to be sorted in direction \a dir
  /// Has no effect unless \a ravel is rank 1.
  void ravel_sortByValue(CAPIRavel* ravel, const CAPITensor* input, enum RavelOrder dir) NOEXCEPT;

  /// Return a tensor expression representing the application of this ravel on \a arg
  /// Validity of returned object is until next call of \a ravel_hyperSlice, or the lifetime of \a ravel
  const CAPITensor* ravel_hyperSlice(CAPIRavel* ravel, const CAPITensor* arg) NOEXCEPT;

  /// sets handles and slices from \a hc
  /// @param hc is JSON representation of the hypercube. See \a civita::Hypercube::json()
  /// @return true on success. Use ravel_lastErr() to retrieve diagnostic
  int ravel_populateFromHypercube(CAPIRavel* ravel, const char* hc) NOEXCEPT;

  /// Connect to a database table. Note this is tested with mysql, postgresql and sqlite3 only.
  /// @param dbType type of database: see SOCI documentation, but currently: mysql, oracle, postgresql, sqlite3, odbc, firebird, db2.
  /// @param connect connection string for the database in question.
  /// @param table name of table to use
  /// @return database object. NULL is returned in case of error. Error message can be retrieved by ravel_lastErr()
  CAPIRavelDatabase* ravel_connect(const char* dbType, const char* connect, const char* table) NOEXCEPT;

  /// Return all available table names in db. You can pass the empty
  /// string to \a table in ravel_connect, and then call connect again
  /// to reconnect with a new table. Size of the array of strings
  /// returned in \a size, and validitiy of all pointers returned is
  /// until the next call of this, or the lifetime of \a db.
  const char** ravel_dbTableNames(CAPIRavelDatabase* db, size_t* size) NOEXCEPT;
  
  /// close database and destroy the database object
  void ravel_close(CAPIRavelDatabase*) NOEXCEPT;

  /// Create an empty table, using \a spec and the contents of \a filename, which is a CSV file
  /// If the table already exists, it is dropped, prior to creation of the new table.
  /// @return true if successful, false otherwise. Error message can be retrieved by ravel_lastErr()
  BOOL ravel_createTable(CAPIRavelDatabase* db, const char* filename, const CAPIRavelDataSpec* spec) NOEXCEPT;

  /// Load a sequence of \a filenames, terminated by NULL, into the database table specified in \a ravel_connect()
  /// All filenames must have the same CSV structure, described by \a spec, and match the table schema created with \a ravel_createTable()
  /// Duplicate records are not detected in this call - a later call to clean up duplicates must be done later
  BOOL ravel_loadDatabase(CAPIRavelDatabase* db, const char** filenames, const CAPIRavelDataSpec* spec) NOEXCEPT;

  /// de-duplicate records according to the value of \a duplicateKeyAction
  void ravel_deduplicate(CAPIRavelDatabase* db, enum CAPIRavelDuplicateKey duplicateKeyAction, const CAPIRavelDataSpec* spec) NOEXCEPT;

  /// return number of numerical columns in the table: data or value axes
  size_t ravel_dbNumNumericalColumns(CAPIRavelDatabase* db) NOEXCEPT;

  /// populate \a columnNames with the database numerical column names
  /// columnNames must be at least ravel_dbNumNumericalColumns() in size
  /// The returned pointers ar all owned by \a db, and are valid in the next call.
  /// Do not delete or free these.
  void ravel_dbNumericalColumnNames(CAPIRavelDatabase* db, const char* columnNames[])  NOEXCEPT;
  
  /// set extra metadata of \a db: a list of \a axisNames that are
  /// value typed axes, and the horizontal dimension name
  /// @param numAxisNames number of names in the list \a axisNames.
  void ravel_setAxisNames(CAPIRavelDatabase* db, const char** axisNames, size_t numAxisNames, const char* horizontalDimension) NOEXCEPT;
  
  /// Initialises \a ravel with the full hypercube extracted from the database
  /// On error, ravel is left unchanged
  void ravel_dbFullHypercube(CAPIRavel* ravel, CAPIRavelDatabase* db) NOEXCEPT;
  
  /// Return a tensor expression representing the application of this ravel on \a db
  /// Validity of returned object is until next call of \a ravel_hyperSlice, ravel_dbHyperslice, or the lifetime of \a ravel or db
  const CAPITensor* ravel_dbHyperSlice(CAPIRavel* ravel, CAPIRavelDatabase* db) NOEXCEPT;
  
#ifdef __cplusplus
}
#endif
#if defined(CLASSDESC) || defined(ECOLAB_LIB)
#include "ravelCAPI.cd"
#endif
#endif
