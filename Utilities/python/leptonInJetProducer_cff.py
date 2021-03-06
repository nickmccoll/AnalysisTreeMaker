import FWCore.ParameterSet.Config as cms
import re
from collections import OrderedDict
from RecoJets.JetProducers.ECF_cff import ecf

#https://raw.githubusercontent.com/cms-jet/JetToolbox/jetToolbox_102X/python/jetToolbox_cff.py
from PhysicsTools.PatAlgos.tools.helpers import getPatAlgosToolsTask, addToProcessAndTask
def _addProcessAndTask(proc, label, module):
    task = getPatAlgosToolsTask(proc)
    addToProcessAndTask(label, module, proc, task)

def addJetVars(proc,jetSeq,jetAlgo,puLabel,postFix) : #addJetVars(process,process.jetSequence,"ak8","Puppi","wLep")
    match = re.match("([a-zA-Z]+)(\d+)", jetAlgo)
    if not match :
        print "Do not understand"
        print jetAlgo
    
    jetAlgo_rad = match.group(2) #8
    jetAlgo_radFlt = float('0.'+jetAlgo_rad)
    jetAlgo_lc = match.group(1).lower() +jetAlgo_rad #ak8
    jetAlgo_uc = match.group(1).upper()+jetAlgo_rad #AK8
    jetAlgo_cap = jetAlgo_lc.capitalize()+jetAlgo_rad #Ak8
    
    PF = 'PF'
    mod = OrderedDict()
    
    #raw clustered jets
    mod["PFJets"] = jetAlgo_lc+PF+"Jets"+puLabel+postFix
    #Patification of std jets
    mod["PATJets"] = 'patJets'+jetAlgo_uc+PF+puLabel+postFix
    #PAT jets after adding in the new variables
    mod["embeddedPATJets"] = mod["PATJets"] +"withUserData"
    #Selected pat jets
    mod["selectedPATJets"] = 'selectedPatJets'+jetAlgo_uc+PF+puLabel+postFix
    #Groomed PF jets
    mod["pfJetsSoftDrop"] = jetAlgo_lc+PF+"Jets"+puLabel+postFix+"SoftDrop"
    #Patification of groomed PF jets
    mod["patJetsSoftDrop"] = "patJets"+jetAlgo_uc+PF+puLabel+postFix+"SoftDrop"
    #final set of jets all merged together
    mod["packedJets"] = "packedPatJets"+jetAlgo_uc+PF+puLabel+postFix+"SoftDrop"
    mod["ecfNbeta1"] = mod["pfJetsSoftDrop"]+"ecfNbeta1"
    mod["ecfMbeta1"] = mod["pfJetsSoftDrop"]+"ecfMbeta1"
    mod["ecfDbeta1"] = mod["pfJetsSoftDrop"]+"ecfDbeta1"
    mod["ecfUbeta1"] = mod["pfJetsSoftDrop"]+"ecfUbeta1"
    mod["Njettiness"] = "Njettiness"+jetAlgo_uc+puLabel+postFix
    mod["lepInJetV"] = mod["PATJets"]+"LepInJetVars"
    mod["ecfValueMap"] = mod["pfJetsSoftDrop"] + "ecfValueMap"
    mod["lepInJetMVA"] = mod["packedJets"] + "lepInJetMVAValueMap"
    
    # create the ECF variables based on the groomed jets
    ecfProd = ecf.clone( src = cms.InputTag( mod["pfJetsSoftDrop"] ),
                         ecftype = cms.string( "N" ),
                         alpha = cms.double( 1 ),
                         beta = cms.double( 1 )
                         )
    

    
    
    _addProcessAndTask( proc, mod["ecfNbeta1"], ecfProd.clone(ecftype = cms.string( "N" ) ))
    _addProcessAndTask( proc, mod["ecfMbeta1"], ecfProd.clone(ecftype = cms.string( "M" ),
      cuts = cms.vstring( '', '', 'pt>10000'  ) ))
    _addProcessAndTask( proc, mod["ecfDbeta1"], ecfProd.clone(ecftype = cms.string( "D" ),
      cuts = cms.vstring( '', '', 'pt>10000'  ) ))
    _addProcessAndTask( proc, mod["ecfUbeta1"], ecfProd.clone(ecftype = cms.string( "U" ) ))
    
    #add the ECFs to the groomed pat jets
    getattr(proc, mod["patJetsSoftDrop"]).userData.userFloats.src += [
        mod["ecfNbeta1"]+":ecfN2",
        mod["ecfDbeta1"]+":ecfD2",
        mod["ecfMbeta1"]+":ecfM2",
        mod["ecfNbeta1"]+":ecfN3",
        mod["ecfUbeta1"]+":ecfU3",
        mod["ecfUbeta1"]+":ecfU2"
        ]
    
    #match the groomed jets to the ungroomed PFJets        
    _addProcessAndTask(proc, mod["ecfValueMap"],
            cms.EDProducer("RecoJetToPatJetDeltaRValueMapProducer",
                src = cms.InputTag(mod["PFJets"]),
                matched = cms.InputTag(mod["patJetsSoftDrop"]),
                distMax = cms.double(jetAlgo_radFlt),
            values = cms.vstring([
                'userFloat("'+mod["ecfNbeta1"]+':ecfN2")',
                'userFloat("'+mod["ecfDbeta1"]+':ecfD2")',
                'userFloat("'+mod["ecfMbeta1"]+':ecfM2")',
                'userFloat("'+mod["ecfNbeta1"]+':ecfN3")',
                'userFloat("'+mod["ecfUbeta1"]+':ecfU3")',
                'userFloat("'+mod["ecfUbeta1"]+':ecfU2")'
                ]),
            valueLabels = cms.vstring( [
                'ecfN2b1',
                'ecfD2b1',
                'ecfM2b1',
                'ecfN3b1',
                'ecfU3b1',
                'ecfU2b1'
                ]),
             )) 
    
    #add the ECF variables to the pat jets
    getattr(proc, mod["PATJets"]).userData.userFloats.src += [
        mod["ecfValueMap"]+':ecfN2b1',
        mod["ecfValueMap"]+':ecfD2b1',
        mod["ecfValueMap"]+':ecfM2b1',
        mod["ecfValueMap"]+':ecfN3b1',
        mod["ecfValueMap"]+':ecfU3b1',
        mod["ecfValueMap"]+':ecfU2b1'        
        ]
    
    #create the lepton in jets variables
    _addProcessAndTask( proc, mod["lepInJetV"], cms.EDProducer('LepInJetProducer',
                                                    src = cms.InputTag(mod["PATJets"]),
                                                    srcEle = cms.InputTag("slimmedElectrons",'','run'),
                                                    srcMu = cms.InputTag("slimmedMuons")
                                                    ))
    
    #embed the user floats for the lepInJet vars
    _addProcessAndTask( proc, mod["embeddedPATJets"], cms.EDProducer("PATJetUserDataEmbedder",
        src = cms.InputTag( mod["PATJets"]),
        userFloats = cms.PSet(lsf3 = cms.InputTag(mod["lepInJetV"]+":lsf3"),
                              dRLep = cms.InputTag(mod["lepInJetV"]+":dRLep"),
                              ),
        userInts = cms.PSet(muonIdx3SJ = cms.InputTag(mod["lepInJetV"]+":muIdx3SJ"),
                            electronIdx3SJ = cms.InputTag(mod["lepInJetV"]+":eleIdx3SJ"),
                            ),
        ))
    

    
#     jetSeq += getattr(proc, mod["embeddedPATJets"])
#     getattr( proc, 'jetTask', cms.Task() ).add(getattr(proc,mod["embeddedPATJets"]))
    
    #now redirect the next stage of the jet selection to use the embedded version
    getattr(proc, mod["selectedPATJets"]).src = cms.InputTag(mod["embeddedPATJets"]) 
    
    #now compute the MVA
    _addProcessAndTask( proc, mod["lepInJetMVA"], cms.EDProducer('JetBaseMVAValueMapProducer',
        src = cms.InputTag(mod["packedJets"]),
        weightFile =  cms.FileInPath("AnalysisTreeMaker/Utilities/data/lsf3bdt_BDT.weights.xml"),
        name = cms.string("lepInJetMVA"),
        isClassifier = cms.bool(True),
        variablesOrder = cms.vstring(["clf_LSF3","clf_dRLep",
                                      "n2b1:=clf_e3_v2_sdb1/(clf_e2_sdb1*clf_e2_sdb1)",
                                      "m2b1:=clf_e3_v1_sdb1/clf_e2_sdb1",
                                      "d2b1:=clf_e3_sdb1/(clf_e2_sdb1*clf_e2_sdb1*clf_e2_sdb1)",
                                      "n3b1:=clf_e4_v2_sdb1/(clf_e3_v1_sdb1*clf_e3_v1_sdb1)",
                                      "tau21:=clf_Tau2/clf_Tau1",
                                      "tau31:=clf_Tau3/clf_Tau1",
                                      "tau32:=clf_Tau3/clf_Tau2",
                                      "tau43:=clf_Tau4/clf_Tau3",
                                      "u3b1:=clf_e4_v1_sdb1",
                                      "u2b1:=clf_e3_v1_sdb1",
                                      ]),
        variables = cms.PSet(clf_LSF3 = cms.string("userFloat('lsf3')"),
                             clf_dRLep = cms.string("userFloat('dRLep')"),                                                                                                           
                             n2b1 = cms.string('userFloat("'+mod["ecfValueMap"]+':ecfN2b1")'),
                             m2b1 = cms.string('userFloat("'+mod["ecfValueMap"]+':ecfM2b1")'),
                             d2b1 = cms.string('userFloat("'+mod["ecfValueMap"]+':ecfD2b1")'),
                             n3b1 = cms.string('userFloat("'+mod["ecfValueMap"]+':ecfN3b1")'),
                             tau21 = cms.string('userFloat("'+mod["Njettiness"]+':tau2")/userFloat("'+mod["Njettiness"]+':tau1")'),
                             tau31 = cms.string('userFloat("'+mod["Njettiness"]+':tau3")/userFloat("'+mod["Njettiness"]+':tau1")'),
                             tau32 = cms.string('userFloat("'+mod["Njettiness"]+':tau3")/userFloat("'+mod["Njettiness"]+':tau2")'),
                             tau43 = cms.string('userFloat("'+mod["Njettiness"]+':tau4")/userFloat("'+mod["Njettiness"]+':tau3")'),
                             u3b1 = cms.string('userFloat("'+mod["ecfValueMap"]+':ecfU3b1")'),
                             u2b1 = cms.string('userFloat("'+mod["ecfValueMap"]+':ecfU2b1")'),
                             )
        ))
