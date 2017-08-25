#ifndef ANALYSISTREEMAKER_TREEFILLERS_ElectronFILLER_H
#define ANALYSISTREEMAKER_TREEFILLERS_ElectronFILLER_H

#include "AnalysisTreeMaker/TreeMaker/interface/BaseFiller.h"

#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/PatCandidates/interface/VIDCutFlowResult.h"
#include "DataFormats/PatCandidates/interface/Electron.h"

namespace AnaTM{
class EventFiller;
class ElectronFiller : public BaseFiller {
public:
	ElectronFiller(const edm::ParameterSet& fullParamSet, const std::string& psetName, edm::ConsumesCollector&& cc, const EventFiller * eventFiller);
	virtual ~ElectronFiller() {};
	virtual void load(const edm::Event& iEvent, const edm::EventSetup& iSetup);
	virtual void fill();

private:
	size i_pt             ;
	size i_eta            ;
	size i_phi            ;
	size i_q              ;
	size i_scEta          ;
	size i_d0             ;
	size i_dz             ;
	size i_sip3D          ;
	size i_mvaID          ;
	size i_mvaID_cat      ;
	size i_miniIso        ;
	size i_eaRelISO       ;
	size i_id             ;

    float minPT                = 0;

    edm::EDGetTokenT<pat::ElectronCollection>            token_electrons;
    edm::EDGetTokenT<edm::ValueMap<vid::CutFlowResult> > token_cut_veto ;
    edm::EDGetTokenT<edm::ValueMap<vid::CutFlowResult> > token_cut_loose;
    edm::EDGetTokenT<edm::ValueMap<vid::CutFlowResult> > token_cut_med  ;
    edm::EDGetTokenT<edm::ValueMap<vid::CutFlowResult> > token_cut_tight;
    edm::EDGetTokenT<edm::ValueMap<vid::CutFlowResult> > token_cut_heep;
    edm::EDGetTokenT<edm::ValueMap<float              >> token_mva  ;
    edm::EDGetTokenT<edm::ValueMap<int                >> token_mvaCat;
    edm::EDGetTokenT<pat::PackedCandidateCollection>	    token_pfCands;
    edm::EDGetTokenT<double>                             token_miniiso_rho;

    edm::Handle<pat::ElectronCollection>                 han_electrons;
    edm::Handle<edm::ValueMap<vid::CutFlowResult> >      han_cut_veto ;
    edm::Handle<edm::ValueMap<vid::CutFlowResult> >      han_cut_loose;
    edm::Handle<edm::ValueMap<vid::CutFlowResult> >      han_cut_med  ;
    edm::Handle<edm::ValueMap<vid::CutFlowResult> >      han_cut_tight;
    edm::Handle<edm::ValueMap<vid::CutFlowResult> >      han_cut_heep ;
    edm::Handle<edm::ValueMap<float              >>      han_mva  ;
    edm::Handle<edm::ValueMap<int                >>      han_mvaCat;
    edm::Handle<pat::PackedCandidateCollection>			han_pfCands;
    edm::Handle<double>                                  han_miniiso_rho;

    const EventFiller * event;

};
}

#endif