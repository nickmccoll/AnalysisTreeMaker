
#include "AnalysisTreeMaker/TreeFillers/interface/EventFiller.h"


namespace AnaTM{
//--------------------------------------------------------------------------------------------------
EventFiller::EventFiller(const edm::ParameterSet& fullParamSet, const std::string& psetName,
        edm::ConsumesCollector&& cc, bool isRealData, FillerConstants::DataEra dataEra_input,
        FillerConstants::DataRun dataRun_input,FillerConstants::Dataset dataset_input,
        FillerConstants::MCProcess mcProcess, FillerConstants::SignalType signalType_input
		) :
		BaseFiller(fullParamSet,psetName,"EventFiller"),
		realData (isRealData),
		dataEra_input  (dataEra_input),
		dataRun_input  (dataRun_input),
		dataset_input  (dataset_input),
		mcProcess(mcProcess),
		signalType_input(signalType_input),
		sampParam_input(cfg.getParameter<int>("sampParam")),
		addPDFWeights(cfg.getParameter<bool>("addPDFWeights")),
		addPrefiringWeights(cfg.getParameter<bool>("addPrefiringWeights"))

{
	if(ignore()) return;

	token_vtx   =cc.consumes<reco::VertexCollection>(cfg.getParameter<edm::InputTag>("vertices"));
	token_rho   =cc.consumes<double>                (cfg.getParameter<edm::InputTag>("rho"));
	token_met   =cc.consumes<pat::METCollection>    (cfg.getParameter<edm::InputTag>("met"));
	token_rawMet=cc.consumes<pat::METCollection>    (cfg.getParameter<edm::InputTag>("rawMet"));
	token_vanMet=cc.consumes<pat::METCollection>    (cfg.getParameter<edm::InputTag>("vanMet"));
	token_puppiMet=cc.consumes<pat::METCollection>  (cfg.getParameter<edm::InputTag>("puppiMet"));
	if(addPrefiringWeights){
	    token_prefweight    =cc.consumes<double>(edm::InputTag("prefiringweight:nonPrefiringProb"));
	    token_prefweightup  =cc.consumes<double>(edm::InputTag("prefiringweight:nonPrefiringProbUp"));
	    token_prefweightdown=cc.consumes<double>(edm::InputTag("prefiringweight:nonPrefiringProbDown"));
	}
	if(!isRealData){
		token_puSum   =cc.consumes<std::vector<PileupSummaryInfo>>
		        (cfg.getParameter<edm::InputTag>("puSummaryInfo"));
		token_genEvent=cc.consumes<GenEventInfoProduct>
		        (cfg.getParameter<edm::InputTag>("genEvent"));
	    if(addPDFWeights)
	        token_lheEventInfo=cc.consumes<LHEEventProduct>
	            (cfg.getParameter<edm::InputTag>("lheEvent"));
	}


	data.addSingle(run           ,branchName,"run"           );
	data.addSingle(lumi          ,branchName,"lumi"          );
	data.addSingle(event         ,branchName,"event"         );
	data.addSingle(sampParam     ,branchName,"sampParam"     );
	data.addSingle(goodVtx       ,branchName,"goodVtx"       );
	data.addSingle(npv           ,branchName,"npv"           );
	data.addSingle(rho           ,branchName,"rho"           ,10);
	data.addSingle(met_pt        ,branchName,"met_pt"        );
	data.addSingle(met_phi       ,branchName,"met_phi"       );
	data.addSingle(met_sig       ,branchName,"met_sig"       ,10);
	data.addSingle(met_unclUp_pt ,branchName,"met_unclUp_pt" ,10);
	data.addSingle(met_unclUp_phi,branchName,"met_unclUp_phi",10);
	data.addSingle(met_raw_pt    ,branchName,"met_raw_pt"    ,10);
	data.addSingle(met_raw_phi   ,branchName,"met_raw_phi"   ,10);
    data.addSingle(met_van_pt    ,branchName,"met_van_pt"    ,10);
    data.addSingle(met_van_phi   ,branchName,"met_van_phi"   ,10);
    data.addSingle(met_puppi_pt  ,branchName,"met_puppi_pt"    ,10);
    data.addSingle(met_puppi_phi ,branchName,"met_puppi_phi"   ,10);
	data.addSingle(dataEra       ,branchName,"dataEra"           );
	if(isRealData){
	    data.addSingle(dataset       ,branchName,"dataset"       );
	    data.addSingle(dataRun       ,branchName,"dataRun"       );
	} else {
	    data.addSingle(signalType    ,branchName,"signalType"     );
	    data.addSingle(nTruePUInts   ,branchName,"nTruePUInts"   ,10);
	    data.addSingle(genWeight     ,branchName,"genWeight"        ,10);
	    data.addSingle(process       ,branchName,"process"       );
	    data.addSingle(prefweight    ,branchName,"prefweight"    ,8);
	    data.addSingle(prefweightup  ,branchName,"prefweightup"  ,8);
	    data.addSingle(prefweightdown,branchName,"prefweightdown",8);
	    data.addVector(genWeights    ,branchName,"genWeights_N","genWeights",10);
	}

};
//--------------------------------------------------------------------------------------------------
void EventFiller::load(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
	iEvent.getByToken(token_vtx     ,han_vtx     );
	iEvent.getByToken(token_rho     ,han_rho     );
	iEvent.getByToken(token_met     ,han_met     );
	iEvent.getByToken(token_rawMet  ,han_rawMet  );
	iEvent.getByToken(token_vanMet  ,han_vanMet  );
	iEvent.getByToken(token_puppiMet  ,han_puppiMet  );

	if(addPrefiringWeights){
	    iEvent.getByToken(token_prefweight  ,han_prefweight  );
	    iEvent.getByToken(token_prefweightup  ,han_prefweightup  );
	    iEvent.getByToken(token_prefweightdown  ,han_prefweightdown  );
	}

	if(!realData){
		iEvent.getByToken(token_puSum   ,han_puSum   );
		iEvent.getByToken(token_genEvent,han_genEvent);
		if(addPDFWeights)
		    iEvent.getByToken(token_lheEventInfo     ,han_lheEventInfo     );
	}

	evtCoord.run   = iEvent.run();
	evtCoord.lumi  = iEvent.luminosityBlock();
	evtCoord.event = iEvent.id().event();

	primaryVertex = han_vtx->size() ? &han_vtx->front() : (const reco::Vertex*)(0);
};
//--------------------------------------------------------------------------------------------------
void EventFiller::setValues(){
	  bool hasgoodvtx =  primaryVertex &&
			  ( !primaryVertex->isFake() && primaryVertex->ndof() > 4.0
					  && primaryVertex->position().Rho() <= 2.0
					  && std::fabs(primaryVertex->position().Z()) <= 24.0
					  );
	  run              = evtCoord.run   ;
	  lumi             = evtCoord.lumi  ;
	  event            = evtCoord.event ;
	  sampParam       = sampParam_input;
	  goodVtx          = hasgoodvtx;
	  npv              = ASTypes::convertTo<size16>(han_vtx->size(),"EventFiller::npv" );
	  rho              = *han_rho;
	  met_pt           = han_met->front().pt();
	  met_phi          = han_met->front().phi();
	  met_sig          = han_met->front().metSignificance();
	  met_unclUp_pt    = han_met->front().shiftedPt(pat::MET::UnclusteredEnUp);
	  met_unclUp_phi   = han_met->front().shiftedPhi(pat::MET::UnclusteredEnUp);
	  met_raw_pt       = han_rawMet->front().uncorPt();
	  met_raw_phi      = han_rawMet->front().uncorPhi();
	  met_van_pt       = han_vanMet->front().pt();
	  met_van_phi      = han_vanMet->front().phi();
	  met_puppi_pt     = han_puppiMet->front().pt();
	  met_puppi_phi    = han_puppiMet->front().phi();
	  dataEra            =static_cast<size8>(dataEra_input);
	  if(realData){
		  dataset            =static_cast<size8>(dataset_input);
		  dataRun            =static_cast<size8>(dataRun_input);

	  } else {
		  float   num_true_interactions = 0;
		  for( const auto& psu : *han_puSum ) {
			  if(psu.getBunchCrossing() == 0)
				  num_true_interactions = psu.getTrueNumInteractions();
		  }
		  nTruePUInts         =num_true_interactions;
		  genWeight           =han_genEvent	->weight();
		  process             =static_cast<size8>(mcProcess);
		  if(addPrefiringWeights){
	          prefweight        =*han_prefweight    ;
	          prefweightup      =*han_prefweightup  ;
	          prefweightdown    =*han_prefweightdown;
		  }
		  signalType        = signalType_input;
		    if(addPDFWeights){
		      const auto& pdfWeights = han_lheEventInfo->weights();
		      for(const auto& w: pdfWeights){
		          genWeights->push_back(w.wgt);
		      }
		    }
	  }

};
}
