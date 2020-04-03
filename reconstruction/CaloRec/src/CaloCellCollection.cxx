

#include "CaloCell/CaloCellCollection.h"

using namespace xAOD;


CaloCellCollection::CaloCellCollection()
{;}


CaloCellCollection::~CaloCellCollection()
{
  release();
}


void CaloCellCollection::release()
{
  for ( auto& pairObj : m_collection ){
    if( pairObj.second )
      delete pairObj.second;
  }

  for (auto& acc : m_collectionAccessor ){
    if(acc)
      delete acc;
  }
  m_collection.clear();
  m_collectionAccessor.clear();
  m_cells.clear();
}


void CaloCellCollection::push_back( xAOD::CaloCell *cell )
{
  m_collection[ cell->hash() ] = cell;
  m_cells.push_back(cell);
}



void CaloCellCollection::push_back( xAOD::CaloCellAccessor *acc )
{
  m_collectionAccessor.push_back(acc);
}



bool CaloCellCollection::retrieve( TVector3 &pos, xAOD::CaloCell *&cell )
{
  // Apply all necessary transformation (x,y,z) to (eta,phi,r) coordinates
  // Get ATLAS coordinates (in transverse plane xy)
  float eta = pos.PseudoRapidity();
  float phi = pos.Phi();
  float r = pos.Perp();
  //std::cout << "("
  //          << pos.X() << ","
  //          << pos.Y() << ","
  //          << pos.Z() << ") = ("
  //          << r << ","
  //          << pos.Theta() << ","
  //          << pos.Eta() << ","
  //          << pos.Phi() << "). Found cell is: ";

  for ( auto& acc : m_collectionAccessor )
  {
    std::string cell_hash;
    if( acc->getHash( eta, phi, r, cell_hash ))
    {
      cell = m_collection[cell_hash];
      //std::cout << "(" << cell->rmin() << "<" << cell->rmax()
      //  << "," << cell->eta() << "±" << cell->deltaEta()
      //  << "," << cell->phi() << "±" << cell->deltaPhi()
      //  << ")" << std::endl;

      return true;
    }
  }
  //std::cout << "None" << std::endl;
  return false;
}


size_t CaloCellCollection::size()
{
  return m_collection.size();
}


void CaloCellCollection::clear()
{
  for ( auto& pairObj : m_collection ){
    pairObj.second->clear();
  }
}


const std::vector< xAOD::CaloCell* >  CaloCellCollection::all()
{
  return m_cells;
}


void CaloCellCollection::Print()
{

 MSG_INFO( "Number of cells in this collection: " << size() );
  for (auto& cell : all() ){
    MSG_INFO( "Cell for layer " << cell->sampling() << " with center in (" << cell->eta() << ", " << cell->phi() << ") and hash " << cell->hash());
  }
}



/*
 * Halper class:
 * Cell Accessor to generate the correct hash given eta/phi and r
 */
CaloCellAccessor::CaloCellAccessor( float etamin, float etamax, float etabins, float phimin, 
                    float phimax, float phibins, float rmin, float rmax, int sampling):
  m_rmin(rmin), m_rmax(rmax), m_sampling(sampling)
{
  MSG_INFO( " Sampling = " << sampling << " Etamin = " << etamin << " Etamax" << etamax );
  MSG_INFO( " phimin = " << phimin << " phimax = " << phimax << " rmin = " << rmin << " rmax = " << rmax);
  float deta = (etamax-etamin)/etabins;
  float dphi = (phimax-phimin)/phibins;
  for (unsigned eta_idx=0 ; eta_idx<etabins+1; ++eta_idx)
    m_eta_bins.push_back( etamin + deta * eta_idx );
  for (unsigned phi_idx=0 ; phi_idx<phibins+1; ++phi_idx)
    m_phi_bins.push_back( phimin + dphi * phi_idx );

}


bool CaloCellAccessor::getHash( float eta, float phi, float r, std::string &cell_hash)
{
  
  if( !(r >= m_rmin && r < m_rmax) )
    return false;


  std::stringstream ss;
  for ( unsigned eta_bin = 0 ; eta_bin < m_eta_bins.size()-1; ++eta_bin ){
    if ( eta > m_eta_bins[eta_bin] && eta <= m_eta_bins[eta_bin+1]){
      for ( unsigned phi_bin = 0 ; phi_bin < m_phi_bins.size()-1; ++phi_bin ){
        if ( phi > m_phi_bins[phi_bin] && phi <= m_phi_bins[phi_bin+1]){
          ss << "layer" << m_sampling << "_eta" << eta_bin << "_phi" << phi_bin;
          cell_hash = ss.str();
          return true;
        }
      }
    }
  }
  return false;
}





