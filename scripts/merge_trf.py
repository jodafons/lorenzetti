#!/usr/bin/env python3

from GaugiKernel          import LoggingLevel, get_argparser_formatter
from G4Kernel             import *
import argparse
import sys,traceback



def build_argparser():
  parser = argparse.ArgumentParser(description = '', add_help = False, formatter_class = get_argparser_formatter() )

  parser.add_argument('-i','--input-file', action='store', dest='input_file', required = True,
                      help = "The event input file generated by the Pythia event generator.")

  parser.add_argument('-p','--pileup-file', action='store', dest='pileup_file', required = True,
                      help = "The event HIT file to be merged (pileup)")

  parser.add_argument('-o','--output-file', action='store', dest='output_file', required = True,
                      help = "The reconstructed event file generated by lzt/geant4 framework.")

  parser.add_argument('--nov','--number-of-events', action='store', dest='number_of_events', required = False, type=int, default=-1,
                      help = "The number of events to apply the reconstruction.")

  parser.add_argument('-l', '--output-level', action='store', dest='output_level', required = False, type=str, default='INFO',
                      help = "The output level messenger.")

  parser.add_argument('-c','--command', action='store', dest='command', required = False, default="''",
                      help = "The preexec command")

  return parser



def run(args):


  outputLevel = LoggingLevel.toC(args.output_level)

  try:

    exec(args.command)

    from GaugiKernel import ComponentAccumulator
    acc = ComponentAccumulator("ComponentAccumulator", args.output_file)


    # the reader must be first in sequence
    from RootStreamBuilder import RootStreamHITReader, recordable

    reader = RootStreamHITReader("HITReader", 
                                  InputFile       = args.input_file,
                                  OutputHitsKey   = recordable("Hits"),
                                  OutputEventKey  = recordable("Events"),
                                  OutputTruthKey  = recordable("Particles"),
                                  OutputSeedsKey  = recordable("Seeds"),
                                  OutputLevel     = outputLevel,
                                )
    reader.merge(acc)

    from CaloCellBuilder import PileupMerge
    pileup = PileupMerge( "PileupMerge", 
                          InputFile       = args.pileup_file,
                          InputHitsKey    = recordable("Hits"),
                          InputEventKey   = recordable("Events"),
                          OutputHitsKey   = "Hits_Merged",
                          OutputEventKey  = "Events_Merged",
                          OutputLevel     = outputLevel
                        )
    acc += pileup

    from RootStreamBuilder import RootStreamHITMaker
    HIT = RootStreamHITMaker( "RootStreamHITMaker",
                               # input from context
                               InputHitsKey    = "Hits_Merged",
                               InputEventKey   = "Events_Merged",
                               InputTruthKey   = recordable("Particles"),
                               InputSeedsKey   = recordable("Seeds"),
                               # output to file
                               OutputHitsKey   = recordable("Hits"),
                               OutputEventKey  = recordable("Events"),
                               OutputLevel     = outputLevel)
    acc += HIT
    acc.run(args.number_of_vents)
    sys.exit(0)

  except  Exception:
    traceback.print_exc()
    sys.exit(1)
    
    
if __name__ == "__main__":
  parser = build_argparser()
  if len(sys.argv)==1:
      print(parser.print_help())
      sys.exit(1)

  args = parser.parse_args()
  run(args)