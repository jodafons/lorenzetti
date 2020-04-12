
#from CaloRingerBuilder import *
from Gaugi import GeV
from CaloRec import *
from CaloRingerBuilder import CaloRingerBuilder
from G4Kernel import *
import os
import numpy as np
pi = np.pi

acc = ComponentAccumulator("ComponentAccumulator", VisMacro = "init_vis.mac", NumberOfThreads = 4, OutputFile = 'test')


gun = ParticleGun( "ParticleGun",
                   EventKey   = recordable("EventInfo"),
                   Particle   = 'e+',
                   Energy     = 50*GeV,
                   Sigma      = 2*GeV,
                   EtaMax     = 1.0,
                   )


gun.merge(acc)



calorimeter = CaloCellBuilder("CaloCellBuilder", HistogramPath = "Expert/Cells")
calorimeter.merge(acc)



cluster = CaloClusterMaker( "CaloClusterMaker",
                            CellsKey        = recordable("TruthCells"),
                            EventKey        = recordable("EventInfo"),
                            ClusterKey      = recordable("TruthClusters"),
                            TruthKey        = recordable("Truth"),
                            EtaWindow       = 0.4,
                            PhiWindow       = 0.4,
                            HistogramPath   = "Expert/TruthClusters"
                            )
acc+= cluster



#ringer = CaloRingerBuilder( "CaloRingerBuilder",
#                            RingerKey     = recordable("Rings"),
#                            ClusterKey    = recordable("Clusters"),
#                            DeltaEtaRings = [0.00325, 0.025, 0.050, 0.1, 0.1, 0.2 ],
#                            DeltaPhiRings = [pi/32, pi/128, pi/128, pi/128, pi/32, pi/32, pi/32],
#                            NRings        = [64, 8, 8, 4, 4, 4],
#                            LayerRings    = [1,2,3,4,5,6],
#                            HistogramPath   = "Expert/Ringer/reco"
#                            )
#
#acc+=ringer
#
#
#
#truth_ringer = CaloRingerBuilder( "CaloRingerBuilder",
#                            RingerKey     = recordable("TruthRings"),
#                            ClusterKey    = recordable("TruthClusters"),
#                            DeltaEtaRings = [0.00325, 0.025, 0.050, 0.1, 0.1, 0.2 ],
#                            DeltaPhiRings = [pi/32, pi/128, pi/128, pi/128, pi/32, pi/32, pi/32],
#                            NRings        = [64, 8, 8, 4, 4, 4],
#                            LayerRings    = [1,2,3,4,5,6],
#                            HistogramPath   = "Expert/Ringer/truth"
#                            )
#
#
#acc+= truth_ringer




#acc.run('run_gun.mac')
#acc.run('run_jets.mac')
acc.run('run.mac')
#acc.run()



