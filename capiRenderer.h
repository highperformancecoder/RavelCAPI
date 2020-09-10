/*
  Ravel C API. © Ravelation Pty Ltd 2018
*/
#ifndef CAPIRENDERER_H
#define CAPIRENDERER_H
// Renderer interface. See CairoRenderer for implementation and
// semantics WRT to the Cairo graphics library
struct CAPIRenderer
{
  void (*moveTo)(struct CAPIRenderer*, double x, double y);
  void (*lineTo)(struct CAPIRenderer*, double x, double y);
  void (*relMoveTo)(struct CAPIRenderer*, double x, double y);
  void (*relLineTo)(struct CAPIRenderer*, double x, double y);
  void (*arc)(struct CAPIRenderer*, double x, double y,
              double radius, double start, double end);

  void (*setLineWidth)(struct CAPIRenderer*,double);

  // paths
  void (*newPath)(struct CAPIRenderer*);
  void (*closePath)(struct CAPIRenderer*);
  void (*fill)(struct CAPIRenderer*);
  void (*clip)(struct CAPIRenderer*);
  void (*stroke)(struct CAPIRenderer*);
  void (*strokePreserve)(struct CAPIRenderer*);
  
  // sources
  void (*setSourceRGB)(struct CAPIRenderer*, double r, double g, double b);
    
  // text. Argument is in UTF8 encoding
  void (*showText)(struct CAPIRenderer*, const char*);
  void (*setTextExtents)(struct CAPIRenderer*, const char*);
  double (*textWidth)(struct CAPIRenderer*);
  double (*textHeight)(struct CAPIRenderer*);

  // matrix transformation
  void (*identityMatrix)(struct CAPIRenderer*);
  void (*translate)(struct CAPIRenderer*, double x, double y);
  void (*scale)(struct CAPIRenderer*, double sx, double sy);
  void (*rotate)(struct CAPIRenderer*, double angle); ///< angle in radians

        // context manipulation
  void (*save)(struct CAPIRenderer*);
  void (*restore)(struct CAPIRenderer*);
};
#endif
