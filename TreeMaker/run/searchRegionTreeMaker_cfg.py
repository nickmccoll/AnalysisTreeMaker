import FWCore.ParameterSet.Config as cms
import re

process = cms.Process('run')

process.options = cms.untracked.PSet(
    allowUnscheduled=cms.untracked.bool(True),
#     wantSummary=cms.untracked.bool(True)
)

# process.SimpleMemoryCheck=cms.Service("SimpleMemoryCheck",
#                                     ignoreTotal=cms.untracked.int32(-1),
#                                       oncePerEventMode=cms.untracked.bool(True),
#                                       moduleMemorySummary=cms.untracked.bool(True),
#                                       )
# process.Timing=cms.Service("Timing")

process.load('FWCore.MessageService.MessageLogger_cfi')
process.MessageLogger.cerr.FwkReport.reportEvery = 100

from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing('analysis')

options.register('isCrab',
                 None,
                 VarParsing.multiplicity.singleton,
                 VarParsing.varType.bool,
                 "Input isCrab")

options.register('sample',
                 "NONE",
                 VarParsing.multiplicity.singleton,
                 VarParsing.varType.string,
                 "Input sample")

options.register('dataRun',
                 "NONE",
                 VarParsing.multiplicity.singleton,
                 VarParsing.varType.string,
                 "Input dataRun")

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

isCrab  = options.isCrab
dataRun = options.dataRun
sample  = options.sample

isRealData = (dataRun != "NONE")

from AnalysisTreeMaker.TreeMaker.treeMaker_cff import *
process.treeMaker = cms.EDAnalyzer('SearchRegionTreeMaker'
                                 , globalTag = cms.string('')
                                 , dataRun = cms.string(dataRun)
                                 , sample = cms.string(sample)
                                 , EventFiller               = cms.PSet(EventFiller)
                                 , METFilterFiller           = cms.PSet(METFilterFiller)
                                 , TriggerFiller             = cms.PSet(TriggerFiller)
                                 , ak4JetFiller              = cms.PSet(ak4JetFiller)
                                 , ak4PuppiJetFiller         = cms.PSet(ak4PuppiJetFiller)
#                                  , ak4PuppiNoLepJetFiller    = cms.PSet(ak4PuppiNoLepJetFiller)
                                 , ak8PuppiNoLepFatJetFiller = cms.PSet(ak8PuppiNoLepFatJetFiller)
                                , ak8PuppiFatJetFiller      = cms.PSet(ak8PuppiFatJetFiller)
                                 , ElectronFiller            = cms.PSet(ElectronFiller)
                                 , MuonFiller                = cms.PSet(MuonFiller)  
                                 , GenParticleFiller         = cms.PSet(GenParticleFiller)                                 
                                 )
setupTreeMakerAndGlobalTag(process,process.treeMaker,isRealData,dataRun)

#==============================================================================================================================#
#==============================================================================================================================#

if isRealData and not isCrab:
    setupJSONFiltering(process)

from AnalysisTreeMaker.TreeMaker.metCorrections_cff import metCorrections
metCorrections(process,process.treeMaker.EventFiller.met,isRealData)
from AnalysisTreeMaker.TreeMaker.jetProducers_cff import defaultJetSequences
defaultJetSequences(process,isRealData,dataRun)
from AnalysisTreeMaker.TreeMaker.eleVIDProducer_cff import eleVIDProducer
eleVIDProducer(process)

if 'signal' in sample:
    process.treeMaker.EventFiller.addPDFWeights = True;


#==============================================================================================================================#
#==============================================================================================================================#
# https://twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETOptionalFiltersRun2#Details_about_the_application_of
process.load('RecoMET.METFilters.BadPFMuonFilter_cfi')
process.BadPFMuonFilter.muons = cms.InputTag("slimmedMuons")
process.BadPFMuonFilter.PFCandidates = cms.InputTag("packedPFCandidates")
process.BadPFMuonFilter.taggingMode = cms.bool(True)

process.load('RecoMET.METFilters.BadChargedCandidateFilter_cfi')
process.BadChargedCandidateFilter.muons = cms.InputTag("slimmedMuons")
process.BadChargedCandidateFilter.PFCandidates = cms.InputTag("packedPFCandidates")
process.BadChargedCandidateFilter.taggingMode = cms.bool(True)

#filter out events that dont pass a chosen trigger in data
if isRealData :
    process.load('AnalysisTreeMaker.TreeMaker.triggerFilter_cff')
    process.p = cms.Path(process.triggerFilter *process.BadPFMuonFilter *process.BadChargedCandidateFilter *process.treeMaker)
else :
    process.p = cms.Path(process.BadPFMuonFilter *process.BadChargedCandidateFilter *process.treeMaker)
