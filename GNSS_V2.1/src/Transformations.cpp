
#include "Transformations.h"
#include <Eigen30.h>

using  namespace  Eigen;

helmertParams helmert_params;
sourceElipsoid source_elipsoid;
targetElipsoid target_elipsoid;
krugerParams kruger_params;
gridParams grid_params;
transformationSettings transformation_settings;

double ecef_x_source, ecef_y_source, ecef_z_source;
double ecef_x_target, ecef_y_target, ecef_z_target;
double lat_target, lon_target, height_target;
double northing_target, easting_target;

double dms2dec(float d, float m, float s)
{
    double dec = 0.0;
    dec = d+m/60.0+s/3600.0;
    return dec;
}



/** 
 * Converts WGS-84 Geodetic point (lat, lon, h) to the
 * Earth-Centered Earth-Fixed (ECEF) coordinates (x, y, z).
 */
void geodeticToEcef(double lat, double lon, double h,
                    double* ecef_x, double* ecef_y, double* ecef_z)
{
  // Convert to radians in notation consistent with the paper:
  double lambda = lat * M_PI / 180;
  double phi = lon * M_PI / 180;
  double s = sin(lambda);
  double N = source_elipsoid.semimajor / sqrt(1 - source_elipsoid.e_sq * s * s);

  double sin_lambda = sin(lambda);
  double cos_lambda = cos(lambda);
  double cos_phi = cos(phi);
  double sin_phi = sin(phi);

  *ecef_x = (h + N) * cos_lambda * cos_phi;
  *ecef_y = (h + N) * cos_lambda * sin_phi;
  *ecef_z = (h + (1 - source_elipsoid.e_sq) * N) * sin_lambda;
}



/**
 * Converts the Earth-Centered Earth-Fixed (ECEF) coordinates (x, y, z) to
 * (WGS-84) Geodetic point (lat, lon, h).
 */
void ecefToGeodetic(double x, double y, double z,
                    double *lat, double *lon, double *h)
{
  double eps = target_elipsoid.e_sq / (1.0 - target_elipsoid.e_sq);
  double p = sqrt(x * x + y * y);
  double q = atan2((z * target_elipsoid.semimajor), (p * target_elipsoid.semiminor));
  double sin_q = sin(q);
  double cos_q = cos(q);
  double sin_q_3 = sin_q * sin_q * sin_q;
  double cos_q_3 = cos_q * cos_q * cos_q;
  double phi = atan2((z + eps * target_elipsoid.semiminor * sin_q_3),
                       (p - target_elipsoid.e_sq * target_elipsoid.semimajor * cos_q_3));
  double lambda = atan2(y, x);
  double v = target_elipsoid.semimajor / sqrt(1.0 - target_elipsoid.e_sq * sin(phi) * sin(phi));

  *h = (p / cos(phi)) - v;
  *lat = (phi*180/M_PI);
  *lon = (lambda*180/M_PI);
}



/*
 * Converts the Earth-Centered Earth-Fixed (ECEF) coordinates (x, y, z) to
 * East-North-Up coordinates in a Local Tangent Plane that is centered at the
 * Geodetic point (lat0, lon0, h0).
 */
void ecefToEnu(double x, double y, double z,
               double lat0, double lon0, double h0,
               double *northing, double *easting, double *height)
{
  // Convert to radians in notation consistent with the paper:
  double lambda = lat0 * M_PI / 180;
  double phi = lon0 * M_PI / 180;
  double s = sin(lambda);
  double N = source_elipsoid.semimajor / sqrt(1 - source_elipsoid.e_sq * s * s);

  double sin_lambda = sin(lambda);
  double cos_lambda = cos(lambda);
  double cos_phi = cos(phi);
  double sin_phi = sin(phi);

  double x0 = (h0 + N) * cos_lambda * cos_phi;
  double y0 = (h0 + N) * cos_lambda * sin_phi;
  double z0 = (h0 + (1 - source_elipsoid.e_sq) * N) * sin_lambda;

  double xd, yd, zd;
  xd = x - x0;
  yd = y - y0;
  zd = z - z0;

  // This is the matrix multiplication
  *easting = -sin_phi * xd + cos_phi * yd;
  *northing = -cos_phi * sin_lambda * xd - sin_lambda * sin_phi * yd + cos_lambda * zd;
  *height = cos_lambda * cos_phi * xd + cos_lambda * sin_phi * yd + sin_lambda * zd;
}



void helmertTransformation(double Sx, double Sy, double Sz,
                           double *Tx, double *Ty, double *Tz,
                           double Dx, double Dy, double Dz,
                           double R1, double R2, double R3,
                           double M)
{
  Matrix3d R, Rz, Ry, Rx;
  Matrix <double, 3, 1> XYZ_hp, XYZ_source, XYZ_target;

  XYZ_source << Sx,
                Sy,
                Sz;

  XYZ_hp     << Dx,
                Dy,
                Dz;

  Rz << cos(R3),  sin(R3),  0,
        -sin(R3), cos(R3),  0,
        0,        0,        1;

  Ry << cos(R2),  0,        -sin(R2),
        0,        1,        0,
        sin(R2),  0,        cos(R2);

  Rx << 1,        0,        0,
        0,        cos(R1),  sin(R1),
        0,        -sin(R1), cos(R1);

  XYZ_target = XYZ_hp + M * Rz * Ry * Rx * XYZ_source;

  *Tx = XYZ_target(0, 0);
  *Ty = XYZ_target(1, 0);
  *Tz = XYZ_target(2, 0);
}



inline double BilinearInterpolation(double q11, double q12, double q21, double q22,
                                    double x1, double x2, double y1, double y2, double x, double y)
{
    double x2x1, y2y1, x2x, y2y, yy1, xx1;
    x2x1 = x2 - x1;
    y2y1 = y2 - y1;
    x2x = x2 - x;
    y2y = y2 - y;
    yy1 = y - y1;
    xx1 = x - x1;
    return 1.0 / (x2x1 * y2y1) * (
        q11 * x2x * y2y +
        q21 * xx1 * y2y +
        q12 * x2x * yy1 +
        q22 * xx1 * yy1
    );
}



/*
 * Gauss–Krüger transformation, operation #9807
 */
void geodeticToGrid_9807(double latitude, double longitude,
                    double *northing, double *easting)
{
  const double e2 = target_elipsoid.flattening * (2 - target_elipsoid.flattening); // e2: first eccentricity squared
  const double n  = target_elipsoid.flattening / (2 - target_elipsoid.flattening); // n: 3rd flattening
  const double rectifyingRadius = target_elipsoid.semimajor / (1 + n) *
                                (1 + 0.25*pow(n, 2) + 0.015625*pow(n, 4));

  double A = e2;
  double B = (5 * pow(e2, 2) - pow(e2, 3)) / 6.0;
  double C = (104 * pow(e2, 3) - 45 * pow(e2, 4)) / 120.0;
  double D = (1237 * pow(e2, 4)) / 1260.0;

  // Latitude and longitude are expected to be given in degrees
  // phi and lambda: latitude and longitude in radians
  double phi = latitude * M_PI / 180;
  double lambda = longitude * M_PI / 180;
  double lambda0 = kruger_params.central_meridian * M_PI / 180;

  // deltaLambda: longitude relative to the central meridian
  double deltaLambda = lambda - lambda0;

  // phiStar: conformal latitude
  double phiStar = phi - sin(phi) * cos(phi) * (A + B*pow(sin(phi), 2) +
                   C*pow(sin(phi), 4) + D*pow(sin(phi), 6));

  double xiPrim = atan(tan(phiStar) / cos(deltaLambda));
  double etaPrim = atanh(cos(phiStar) * sin(deltaLambda));

  double beta1 = 1/2.0 * n - 2/3.0 * pow(n, 2) + 5/16.0 * pow(n, 3)     + 41/180.0 * pow(n, 4);
  double beta2 =           13/48.0 * pow(n, 2)  - 3/5.0 * pow(n, 3)   + 557/1440.0 * pow(n, 4);
  double beta3 =                               61/240.0 * pow(n, 3)    - 103/140.0 * pow(n, 4);
  double beta4 =                                                    49561/161280.0 * pow(n, 4);

  *northing = kruger_params.false_northing
            + kruger_params.scale * rectifyingRadius * (xiPrim
            + beta1 * sin(2*xiPrim) * cosh(2*etaPrim)
            + beta2 * sin(4*xiPrim) * cosh(4*etaPrim)
            + beta3 * sin(6*xiPrim) * cosh(6*etaPrim)
            + beta4 * sin(8*xiPrim) * cosh(8*etaPrim));

  *easting = kruger_params.false_easting
           + kruger_params.scale * rectifyingRadius * (etaPrim
           + beta1 * cos(2*xiPrim) * sinh(2*etaPrim)
           + beta2 * cos(4*xiPrim) * sinh(4*etaPrim)
           + beta3 * cos(6*xiPrim) * sinh(6*etaPrim)
           + beta4 * cos(8*xiPrim) * sinh(8*etaPrim));
}

void geodeticToGrid(double latitude, double longitude, double altitude_source,
                    double *northing, double *easting, double *altitude_target)
{    
  geodeticToEcef(latitude, longitude, altitude_source,
                       &ecef_x_source, &ecef_y_source, &ecef_z_source);
  

  helmertTransformation(ecef_x_source, ecef_y_source, ecef_z_source,
                        &ecef_x_target, &ecef_y_target, &ecef_z_target,
                        helmert_params.translation_x, helmert_params.translation_y, helmert_params.translation_z,
                        helmert_params.rotation_x, helmert_params.rotation_y, helmert_params.rotation_z,
                        helmert_params.scale);

  ecefToGeodetic(ecef_x_target, ecef_y_target, ecef_z_target,
                 &lat_target, &lon_target, &height_target);

  geodeticToGrid_9807(lat_target+grid_params.lat_mean_offset, lon_target+grid_params.lon_mean_offset, &northing_target, &easting_target);
  *northing = northing_target;
  *easting  = easting_target;
  *altitude_target = height_target-grid_params.height_mean_offset-grid_params.antenna_height;
}