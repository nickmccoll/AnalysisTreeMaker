import FWCore.ParameterSet.Config as cms
import re

process = cms.Process('run')

process.options = cms.untracked.PSet(
    allowUnscheduled=cms.untracked.bool(True),
    wantSummary=cms.untracked.bool(True)
)

process.load('FWCore.MessageService.MessageLogger_cfi')
process.MessageLogger.cerr.FwkReport.reportEvery = 100

from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing('analysis')

options.register('crabDataset',
                 None,
                 VarParsing.multiplicity.singleton,
                 VarParsing.varType.string,
                 "Input crabDataset")

options.register('skipEvents',
                 0,
                 VarParsing.multiplicity.singleton,
                 VarParsing.varType.int,
                 "Number of events to skip in processing")

options.parseArguments()

process.TFileService = cms.Service('TFileService',
    fileName=cms.string(options.outputFile)
)

process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(options.maxEvents))

process.source = cms.Source('PoolSource',
    fileNames=cms.untracked.vstring (options.inputFiles),
    skipEvents=cms.untracked.uint32(options.skipEvents)
)


#==============================================================================================================================#
#==============================================================================================================================#
# Import of standard configurations
process.load("Configuration.EventContent.EventContent_cff")
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
#==============================================================================================================================#
#==============================================================================================================================#

isCrab =  True if options.crabDataset else False
datasetName = options.crabDataset if isCrab else options.inputFiles[0]
print datasetName
isRealData = ('/store/data' in datasetName) or (re.match(r'^/[a-zA-Z]+/Run[0-9]{4}[A-Z]', datasetName))



from AnalysisTreeMaker.TreeMaker.treeMaker_cff import *
process.treeMaker = cms.EDAnalyzer('SearchRegionTreeMaker'
                                 , realData = cms.bool(False)
                                 , globalTag = cms.string('')
                                 , EventFiller = cms.PSet(EventFiller)
                                 )
setupTreeMakerAndGlobalTag(process,process.treeMaker,isRealData,datasetName)

#==============================================================================================================================#
#==============================================================================================================================#

if isRealData and not isCrab:
    setupJSONFiltering(process)

from AnalysisTreeMaker.TreeMaker.metCorrections_cff import metCorrections
metCorrections(process,process.treeMaker.EventFiller.met,isRealData)

#==============================================================================================================================#
#==============================================================================================================================#
process.p = cms.Path(process.treeMaker)
