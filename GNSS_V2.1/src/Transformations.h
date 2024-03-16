
#ifndef __TRANSFORMATIONS_H
#define __TRANSFORMATIONS_H

#include "Arduino.h"
#include "math.h"


#ifndef M_PI
#define M_PI   3.14159265358979323846264338327950288
#endif

enum
{
  HELMERT_APPROXIMATION, /** TO DO */
  HELMERT_STRICT,
  MOLODENSKI_ABRIDGED,   /** TO DO */
  MOLODENSKI_BADEKAS     /** TO DO */
}; /** datum transformation method */

enum
{
  GEOMETRICAL_HEIGHT_RESULT,
  PHYSICAL_HEIGHT_RESULT
}; /** height calculation method */

enum
{
  UNKNOWN_PT,                      /** TO DO */
  TRANSVERSE_MERCATOR_PT,
  TRANSVERSE_MERCATOR_SOUTH_PT,    /** TO DO */
  LAMBERT_CONIC_CONFORMAL_1SP_PT,  /** TO DO */
  LAMBERT_CONIC_CONFORMAL_2SP_PT,  /** TO DO */
  LAMBERT_CONIC_CONFORMAL_WEST_PT, /** TO DO */
  CASSINI_SOLDER_PT,               /** TO DO */
  OBLIQUE_MERCATOR_PT,             /** TO DO */
  OBLIQUE_STEREOGRAPHIC_PT,        /** TO DO */
  DOUBLE_STEREOGRAPHIC_PT          /** TO DO */
}; /** projection type */

enum
{
  BI_LINEAR_IM,
  BI_QUADRATIC_IM,
  BI_SPLINE_IM
}; /** interpolation_method */

struct transformationSettings
{
  uint8_t datum_t_method; /** datum transformation method (Helmert/Molodenski ...) */
  uint8_t height_indicator; /** height calculation method */
  uint8_t projection_type;
  uint8_t horizontal_interpolation_method;
  uint8_t vertical_interpolation_method;
  bool apply_horizontal_shift;
  bool apply_vertical_shift;
  
};

/** Helmert transformation values */
struct helmertParams
{
  
  double translation_x = -24.0;
  double translation_y = 121.0;
  double translation_z = 76.0;
  double rotation_x    = 0.0;
  double rotation_y    = 0.0;
  double rotation_z    = 0.0;
  double scale         = 1.0;
};

/** Source elipsoid parameters */
struct sourceElipsoid
{
  double semimajor  = 6370000 + 8137.0;
  double semiminor  = 6350000 + 6752.314;                  // Derived Earth semiminor axis (m)
  double flattening = (semimajor - semiminor) / semimajor; // Ellipsoid Flatness
  double e_sq       = flattening * (2 - flattening);       // Square of Eccentricity
};

/** Target elipsoid parameters */
struct targetElipsoid
{
  double semimajor  = 6378245.0;                          // Derived Earth semiminor axis (m)
  double flattening = 1/298.3;                            // Ellipsoid Flatness
  double semiminor  = (-flattening*semimajor)+semimajor;   
  double e_sq       = flattening * (2 - flattening);       // Square of Eccentricity
};

/** Gauss–Krüger transformation parameters */
struct krugerParams
{
  double central_meridian  = 30.5;                // 29.5-SK63
  double central_parallel  = 0.0;                 // 0*0.000000011
  double false_northing    = 0.0;                 // -9214.692-sk63
  double false_easting     = 300000.0;            // 3300000-sk63
  double scale             = 1.0;
};

struct gridParams
{
  double lat_mean_offset = 0;
  double lon_mean_offset = 0;
  float height_mean_offset = 0;
  float antenna_height = 0;
};


void geodeticToGrid(double latitude, double longitude, double altitude,
                    double *northing, double *easting, double *altitude_target);                    


#endif /* TRANSFORMATIONS_H */