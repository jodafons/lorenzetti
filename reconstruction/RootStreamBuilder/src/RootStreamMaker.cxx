#include "CaloCell/CaloCellContainer.h"
#include "CaloCluster/CaloClusterContainer.h"
#include "CaloRings/CaloRingsContainer.h"
#include "EventInfo/EventInfoContainer.h"
#include "TruthParticle/TruthParticleContainer.h"

#include "CaloCell/CaloCellConverter.h"
#include "CaloCell/CaloDetDescriptorConverter.h"
#include "CaloCluster/CaloClusterConverter.h"
#include "CaloRings/CaloRingsConverter.h"
#include "EventInfo/EventInfoConverter.h"
#include "TruthParticle/TruthParticleConverter.h"
#include "TTree.h"
#include "RootStreamMaker.h"
#include "GaugiKernel/EDM.h"


using namespace SG;
using namespace Gaugi;



RootStreamMaker::RootStreamMaker( std::string name ) : 
  IMsgService(name),
  Algorithm()
{
  declareProperty( "EventKey"           , m_eventKey="EventInfo"            );
  declareProperty( "TruthKey"           , m_truthKey="Particles"            );
  declareProperty( "CellsKey"           , m_cellsKey="Cells"                );
  declareProperty( "ClusterKey"         , m_clusterKey="Clusters"           );
  declareProperty( "RingerKey"          , m_ringerKey="Rings"               );
  declareProperty( "OutputLevel"        , m_outputLevel=1                   );
  declareProperty( "NtupleName"         , m_ntupleName="physics"            );
  declareProperty( "DumpAllCells"       , m_dumpAllCells=false              );
  declareProperty( "DumpClusterCells"   , m_dumpClusterCells=false          );


}




RootStreamMaker::~RootStreamMaker()
{;}


StatusCode RootStreamMaker::initialize()
{
  setMsgLevel(m_outputLevel);

  if(m_dumpClusterCells)
  {
    m_dumpAllCells=false; // force to be false in case of cluster only
  }

  return StatusCode::SUCCESS;
}


StatusCode RootStreamMaker::bookHistograms( SG::EventContext &ctx ) const
{
  


  auto store = ctx.getStoreGateSvc();

  std::vector<xAOD::CaloCell_t            > container_cells;
  std::vector<xAOD::CaloDetDescriptor_t   > container_descriptor;
  std::vector<xAOD::CaloCluster_t         > container_clus;
  std::vector<xAOD::CaloRings_t           > container_rings;
  std::vector<xAOD::EventInfo_t           > container_event;
  std::vector<xAOD::TruthParticle_t       > container_truth;

  store->cd();
  TTree *tree = new TTree(m_ntupleName.c_str(), "");
  tree->Branch( "EventInfoContainer"          , &container_event     );
  tree->Branch( "TruthParticleContainer"      , &container_truth     );
  tree->Branch( "CaloRingsContainer"          , &container_rings     );
  tree->Branch( "CaloClusterContainer"        , &container_clus      );
  if(m_dumpAllCells || m_dumpClusterCells)
  {
    tree->Branch( "CaloCellContainer"           , &container_cells     );
    tree->Branch( "CaloDetDescriptorContainer"  , &container_descriptor);
  }
  
  store->add( tree );
  
  return StatusCode::SUCCESS;
}


StatusCode RootStreamMaker::pre_execute( EventContext &/*ctx*/ ) const
{
  return StatusCode::SUCCESS;
}


StatusCode RootStreamMaker::execute( EventContext &/*ctx*/, const G4Step * /*step*/ ) const
{
  return StatusCode::SUCCESS;
}


StatusCode RootStreamMaker::finalize()
{
  return StatusCode::SUCCESS;
}


StatusCode RootStreamMaker::post_execute( EventContext &/*ctx*/ ) const
{
  return StatusCode::SUCCESS;
}


StatusCode RootStreamMaker::fillHistograms( EventContext &ctx ) const
{
  return serialize(ctx);
}


template <class T>
void RootStreamMaker::InitBranch(TTree* fChain, std::string branch_name, T* param) const
{
  std::string bname = branch_name;
  if (fChain->GetAlias(bname.c_str()))
     bname = std::string(fChain->GetAlias(bname.c_str()));

  if (!fChain->FindBranch(bname.c_str()) ) {
    MSG_WARNING( "unknown branch " << bname );
    return;
  }
  fChain->SetBranchStatus(bname.c_str(), 1.);
  fChain->SetBranchAddress(bname.c_str(), param);
}




StatusCode RootStreamMaker::serialize( EventContext &ctx ) const
{
  

  auto store = ctx.getStoreGateSvc();

  store->cd();
  TTree *tree = store->tree(m_ntupleName);
 
  std::vector<xAOD::CaloDetDescriptor_t > *container_descriptor = nullptr;
  std::vector<xAOD::CaloCell_t          > *container_cells      = nullptr;
  std::vector<xAOD::CaloCluster_t       > *container_clus       = nullptr;
  std::vector<xAOD::CaloRings_t         > *container_rings      = nullptr;
  std::vector<xAOD::EventInfo_t         > *container_event      = nullptr;
  std::vector<xAOD::TruthParticle_t     > *container_truth      = nullptr;

  MSG_DEBUG( "Link all branches..." );

  InitBranch( tree,  "EventInfoContainer"            , &container_event      );
  InitBranch( tree,  "TruthParticleContainer"        , &container_truth      );
  InitBranch( tree,  "CaloRingsContainer"            , &container_rings      );
  InitBranch( tree,  "CaloClusterContainer"          , &container_clus       );
  if(m_dumpAllCells || m_dumpClusterCells){
    InitBranch( tree,  "CaloCellContainer"             , &container_cells      );
    InitBranch( tree,  "CaloDetDescriptorContainer"    , &container_descriptor );
  }


  { // serialize EventInfo

    SG::ReadHandle<xAOD::EventInfoContainer> event(m_eventKey, ctx);

    if( !event.isValid() ){
      MSG_FATAL( "It's not possible to read the xAOD::EventInfoContainer from this Context" );
    }

    xAOD::EventInfo_t event_t;
    xAOD::EventInfoConverter cnv;
    cnv.convert( (**event.ptr()).front(), event_t);
    container_event->push_back(event_t);
  }
  
  xAOD::cell_links_t       cell_links;
  xAOD::descriptor_links_t descriptor_links;
  xAOD::cluster_links_t    cluster_links;


  
  if(m_dumpAllCells){ // serialize Cells
    SG::ReadHandle<xAOD::CaloCellContainer> container(m_cellsKey, ctx);

    if( !container.isValid() )
    {
      MSG_FATAL("It's not possible to read the xAOD::CaloCellContainer from this Contaxt using this key " << m_cellsKey );
    }

    int link = 0; // decorate all cells 
    for (const auto cell : **container.ptr() ){

      const xAOD::CaloDetDescriptor *descriptor = cell->descriptor();

      cell_links[cell]=link;
      descriptor_links[descriptor]=link;


      { // serialize
        xAOD::CaloCell_t cell_t;
        xAOD::CaloCellConverter cnv;
        cnv.convert(cell, cell_t, link);
        container_cells->push_back(cell_t);
      }

      link++;
    }// loop over cells
  }
  



  { // Serialize Truth Particle

    SG::ReadHandle<xAOD::TruthParticleContainer> container( m_truthKey, ctx );
  
    if( !container.isValid() )
    {
      MSG_FATAL("It's not possible to read the xAOD::TruthParticleContainer from this Context using this key " << m_truthKey );
    }

    for (const auto par : **container.ptr() ){
      xAOD::TruthParticle_t par_t;
      xAOD::TruthParticleConverter cnv;
      cnv.convert( par, par_t );
      container_truth->push_back(par_t);
    }
  
  }



  {// serialize cluster
    
    SG::ReadHandle<xAOD::CaloClusterContainer> container( m_clusterKey, ctx );
  
    if( !container.isValid() )
    {
      MSG_WARNING("It's not possible to read the xAOD::CaloClusterContainer from this Context using this key " << m_ringerKey );
      return StatusCode::SUCCESS;
    }


    int cluster_link = 0;
    for (const auto &clus : **container.ptr() )
    {

      if(m_dumpClusterCells){

        int cell_link=0;
        for(const auto&cell : clus->cells()){
          if(!cell_links.count(cell))
          {
            const xAOD::CaloDetDescriptor *det = cell->descriptor();

            cell_links[cell]=cell_link;
            descriptor_links[det]=cell_link;


            { // serialize cell
              xAOD::CaloCell_t cell_t;
              xAOD::CaloCellConverter cnv;
              cnv.convert(cell, cell_t, cell_link);
              container_cells->push_back(cell_t);
            }

            {
              xAOD::CaloDetDescriptor_t det_t;
              xAOD::CaloDetDescriptorConverter cnv;
              cnv.convert(det, det_t, cell_link);
              container_descriptor->push_back(det_t);
            }

            cell_link++;
          }
        }
      }

      cluster_links[clus] = cluster_link; // decorate the cluster since is used by rings
      xAOD::CaloCluster_t clus_t;
      xAOD::CaloClusterConverter cnv;
      cnv.convert( clus , clus_t , cell_links);
      container_clus->push_back(clus_t);
      cluster_link++;
    }

  }



  {// serialize rings
    SG::ReadHandle<xAOD::CaloRingsContainer> container( m_ringerKey, ctx );

    if( !container.isValid() )
    {
      MSG_WARNING("It's not possible to read the xAOD::CaloRingsContainer from this Context using this key " << m_ringerKey );
      return StatusCode::SUCCESS;

    }

    for (const auto rings : **container.ptr() ){
      xAOD::CaloRings_t rings_t;
      xAOD::CaloRingsConverter cnv;
      cnv.convert( rings , rings_t , cluster_links);
      container_rings->push_back(rings_t);  
    }

  }

  
  tree->Fill();

  return StatusCode::SUCCESS;
 
}


