#include <iostream>

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "AnalysisTreeMaker/TreeMaker/interface/BaseTreeMaker.h"
#include "AnalysisTreeMaker/TreeMaker/interface/BaseFiller.h"
#include "AnalysisSupport/TreeInterface/interface/TreeWrapper.h"
#include "AnalysisTreeMaker/TreeFillers/interface/FillerConstantHelpers.h"

namespace AnaTM {

//--------------------------
AnalysisTreeMaker::AnalysisTreeMaker(const edm::ParameterSet & iConfig)
{

    globalTag              = iConfig.getParameter<std::string>("globalTag" );
    std::string sample     = iConfig.getParameter<std::string>("sample"    );
    std::string typeStr    = iConfig.getParameter<std::string>("type"   );
    realData               = strFind(typeStr,"Run");

    dataEra = FillerConstants::getDataEra(typeStr);
    if(realData){
    		dataRun = FillerConstants::getDataRun(typeStr);
    		dataset = FillerConstants::getDataset(sample);
    } else {
    		mcProcess = FillerConstants::getMCProcess(sample);
    		if(mcProcess==FillerConstants::SIGNAL)
    		    signalType = FillerConstants::getSignalType(sample);
    }
	std::cout << " \033[1;34m~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\033[0m"  << std::endl;
	std::cout << " ++  Setting up AnalysisTreeMaker"<<std::endl;
	std::cout << " ++  globalTag           = " << globalTag                      << std::endl;
	std::cout << " ++  dataEra             = " << FillerConstants::DataEraNames[dataEra]<<std::endl;
	std::cout << " ++  dataset             = ";
	  if      (realData) std::cout << "  \033[31mDATA\033[0m ("<< FillerConstants::DataRunNames[dataRun] <<","<<sample<<")";
	  else  {
	      std::cout << "  \033[1;34mMC\033[0m ("<<     FillerConstants::MCProcessNames[mcProcess];
	      if(mcProcess==FillerConstants::SIGNAL)
	          std::cout << " - "<<FillerConstants::SignalTypeNames[signalType];
	      std::cout <<")";
	  }

	std::cout << std::endl;
	std::cout << " \033[1;34m~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\033[0m"  << std::endl;
	usesResource("TFileService");
}
//--------------------------
AnalysisTreeMaker::~AnalysisTreeMaker(){ for(auto f : initializedFillers) delete f;  if(treeWrapper) delete treeWrapper;}
//--------------------------
BaseFiller * AnalysisTreeMaker::initialize(BaseFiller * filler){
	if(filler->ignore()){
	    delete filler;
	    return 0;
	}
	initializedFillers.push_back(filler);
	return initializedFillers.back();
}
//--------------------------
void AnalysisTreeMaker::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup){
	load    (iEvent, iSetup);
	fill();
	++nEvents;;
}
//--------------------------
void AnalysisTreeMaker::beginJob() {
	// Create and register TTree
	edm::Service<TFileService> fs;
	treeWrapper = new TreeWrapper(fs->make<TTree>("Events", ""),"Events");
	book();
}
//--------------------------
void AnalysisTreeMaker::endJob() {
	std::cout << " \033[1;34m~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\033[0m"  << std::endl;
	std::cout << " \033[1;34m~~\033[0m  Done! There were:" << std::endl;
	if(numEvents())
		std::cout << " \033[1;34m~~\033[0m   " << TString::Format("%6d", numEvents()) << " events processed in this job."                   << std::endl;
	else
		std::cout << " \033[31m~~  WARNING : Either the input file(s) are empty, or some error occurred before processing! \033[0m"          << std::endl;
	std::cout << " \033[1;34m~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\033[0m"  << std::endl;
}
//--------------------------
void AnalysisTreeMaker::book(){ for(auto f : initializedFillers) {f->book(tree());} }
//--------------------------
void AnalysisTreeMaker::load(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
    for(auto f : initializedFillers) {f->load(iEvent, iSetup);f->setLoaded(); }
}
//--------------------------
void AnalysisTreeMaker::fill() {
	for(auto f : initializedFillers) {
	    f->setValues();
	    f->processForTree();
	}
	tree()->fill();
    for(auto f : initializedFillers) {f->reset();}

}

}


