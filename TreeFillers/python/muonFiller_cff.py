import FWCore.ParameterSet.Config as cms
MuonFiller = cms.PSet(
    ignore                          = cms.bool(False),
    branchName                      = cms.string("muon"),
    minPT                           = cms.double(5),
    muons                           = cms.InputTag('slimmedMuons'),    
    pfCandidates = cms.InputTag('packedPFCandidates' )
)