#include "geometry.h"
#include <math.h>

#define EARTH_RADIUS 6371000.0 // poluprecnik Zemlje u metrima
#define TO_RAD(x) ((x) * M_PI / 180.0)

double calculateDistance(double lat1, double lon1, double lat2, double lon2) {
    double dLat = TO_RAD(lat2 - lat1);
    double dLon = TO_RAD(lon2 - lon1);
    
    double a = sin(dLat / 2) * sin(dLat / 2) +
               cos(TO_RAD(lat1)) * cos(TO_RAD(lat2)) *
               sin(dLon / 2) * sin(dLon / 2);
               
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    
    return EARTH_RADIUS * c;
}
