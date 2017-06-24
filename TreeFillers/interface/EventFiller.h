#ifndef ANALYSISTREEMAKER_TREEFILLERS_EVENTFILLER_H
#define ANALYSISTREEMAKER_TREEFILLERS_EVENTFILLER_H

#include "AnalysisTreeMaker/TreeMaker/interface/BaseFiller.h"

#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/PatCandidates/interface/MET.h"
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"

namespace AnaTM{

struct EventCoords {
  EventCoords() : run(0),lumi(0),event(0) {}
  size run;
  size lumi;
  ASTypes::size64 event;
};

class EventFiller : public BaseFiller {
public:
	EventFiller(const edm::ParameterSet& fullParamSet, const std::string& psetName, edm::ConsumesCollector&& cc,  bool isRealData);
	virtual ~EventFiller() {};
	virtual void load(const edm::Event& iEvent, const edm::EventSetup& iSetup);
	virtual void fill();

	const reco::Vertex * getPrimVertex() const {return primaryVertex;}

private:
    bool isRealData           = false;

	size i_run                = 0;
	size i_lumi               = 0;
	size i_event              = 0;
	size i_goodVtx            = 0;
	size i_npv                = 0;
	size i_rho                = 0;
	size i_met_pt             = 0;
	size i_met_phi            = 0;
	size i_met_sig            = 0;
	size i_met_unclUp         = 0;
	size i_met_unclDown       = 0;
	size i_met_raw_pt         = 0;
	size i_met_raw_phi        = 0;
	size i_nTruePUInts        = 0;
	size i_weight             = 0;

    edm::EDGetTokenT<reco::VertexCollection>          token_vtx     ;
    edm::EDGetTokenT<double>                          token_rho     ;
    edm::EDGetTokenT<pat::METCollection>              token_met     ;
    edm::EDGetTokenT<pat::METCollection>              token_rawMet  ;
    edm::EDGetTokenT<std::vector<PileupSummaryInfo> > token_puSum   ;
    edm::EDGetTokenT<GenEventInfoProduct>             token_genEvent;

    edm::Handle<reco::VertexCollection>          han_vtx     ;
    edm::Handle<double>                          han_rho     ;
    edm::Handle<std::vector<PileupSummaryInfo> > han_puSum   ;
    edm::Handle<pat::METCollection>              han_met     ;
    edm::Handle<pat::METCollection>              han_rawMet  ;
    edm::Handle<GenEventInfoProduct>             han_genEvent;


    const reco::Vertex * primaryVertex =0;
    EventCoords evtCoord;
    bool hassGoodVertex = false;




};
}



#endif
