#include "FWCore/Framework/interface/MakerMacros.h"

#include "RecoTracker/TrackProducer/plugins/TrackProducer.h"
#include "RecoTracker/TrackProducer/plugins/TrackRefitter.h"
#include "RecoTracker/TrackProducer/plugins/GsfTrackProducer.h"
#include "RecoTracker/TrackProducer/plugins/DAFTrackProducer.h"
#include "RecoTracker/TrackProducer/plugins/MTFTrackProducer.h"

// DEFINE_SEAL_MODULE();
DEFINE_FWK_MODULE(TrackProducer);
DEFINE_FWK_MODULE(TrackRefitter);
DEFINE_FWK_MODULE(GsfTrackProducer);
DEFINE_FWK_MODULE(DAFTrackProducer);
DEFINE_FWK_MODULE(MTFTrackProducer);
