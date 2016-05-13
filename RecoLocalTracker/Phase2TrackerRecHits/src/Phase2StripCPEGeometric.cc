
#include "RecoLocalTracker/Phase2TrackerRecHits/interface/Phase2StripCPEGeometric.h"
#include "SimTracker/SiPhase2Digitizer/plugins/Phase2TrackerDigitizerFwd.h"
#include "Geometry/TrackerGeometryBuilder/interface/PixelGeomDetUnit.h"
#include "Geometry/CommonTopologies/interface/PixelTopology.h"


Phase2StripCPEGeometric::Phase2StripCPEGeometric(edm::ParameterSet & conf)
{
}


Phase2StripCPEGeometric::LocalValues Phase2StripCPEGeometric::localParameters(
  const Phase2TrackerCluster1D & cluster,
  const GeomDetUnit & detunit) const
{
  const Phase2TrackerGeomDetUnit & det = (const Phase2TrackerGeomDetUnit &) detunit;
  const Phase2TrackerTopology * topo = &det.specificTopology();

  float pitch_x = topo->pitch().first;
  float pitch_y = topo->pitch().second;
  float ix = cluster.center();
  float iy = cluster.column()+0.5; // halfway the column

  LocalPoint lp( topo->localX(ix), topo->localY(iy), 0 );          // x, y, z
  LocalError le( pow(pitch_x, 2) / 12, 0, pow(pitch_y, 2) / 12);   // e2_xx, e2_xy, e2_yy
  return std::make_pair( lp, le );
}
