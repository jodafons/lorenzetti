
#include "TruthParticle/TruthParticleConverter.h"
#include "TruthParticle/TruthParticle.h"
//#include "G4Kernel/macros.h"


using namespace xAOD;



bool TruthParticleConverter::convert( const TruthParticle *truth, TruthParticle_t &truth_t )
{
  if(truth){
    truth_t.pdgid = truth->pdgid();
    truth_t.e  = truth->e();
    truth_t.et = truth->et();
    truth_t.eta = truth->eta();
    truth_t.phi = truth->phi();
    return true;
  }
  return false;
}




