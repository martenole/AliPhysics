/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes hereby granted      *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

#include <TChain.h>
#include <TTree.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2D.h>
#include <TList.h>
#include <TCanvas.h>
#include <TGeoManager.h>
#include <TRefArray.h>

#include "AliLog.h"
#include "AliAnalysisTask.h"
#include "AliAnalysisManager.h"
#include "AliESDEvent.h"
#include "AliAODEvent.h"
#include "AliVEvent.h"
#include "AliESDInputHandler.h"
#include "AliAODInputHandler.h"
#include "AliESDpid.h"
#include "AliTOFcalib.h"
#include "AliCDBManager.h"
#include "AliRunTag.h"

#include "AliTOFT0maker.h"
#include "AliVCluster.h"
#include "AliESDCaloCluster.h"
#include "AliVCaloCells.h"
#include "AliESDCaloCells.h"
#include "AliAODCaloCluster.h"
#include "AliAODCaloCells.h"
#include "AliEMCALGeometry.h"

#include "AliAnalysisTaskEMCALTimeCalib.h"

/// \cond CLASSIMP
ClassImp(AliAnalysisTaskEMCALTimeCalib) ;
/// \endcond

using std::cout;
using std::endl;

//________________________________________________________________________
/// Constructor
AliAnalysisTaskEMCALTimeCalib::AliAnalysisTaskEMCALTimeCalib(const char *name)
: AliAnalysisTaskSE(name),
  fRunNumber(-1),
  fTOFmaker(0),
  fOutputList(0x0),
  fgeom(0),
  fGeometryName(),
  fMinClusterEnergy(0),
  fMaxClusterEnergy(0),
  fMinNcells(0),
  fMaxNcells(0),
  fMinLambda0(0),
  fMaxLambda0(0),
  fMaxRtrack(0),
  fMinCellEnergy(0),
  fReferenceFileName(),
  fPileupFromSPD(kFALSE),
  fMinTime(0),
  fMaxTime(0),
  fhcalcEvtTime(0),
  fhEvtTimeHeader(0),
  fhEvtTimeDiff(0),
  fhEventType(0),
  fhTOFT0vsEventNumber(0),
  fhTcellvsTOFT0(0),
  fhTcellvsTOFT0HD(0),
  fhTcellvsSM(0),
  fhEneVsAbsIdHG(0),
  fhEneVsAbsIdLG(0),
  fhTimeVsBC(0),
  fhTimeSumSq(),
  fhTimeEnt(),
  fhTimeSum(),
  fhTimeLGSumSq(),
  fhTimeLGEnt(),
  fhTimeLGSum(),
  fhAllAverageBC(),
  fhAllAverageLGBC(),
  fhTimeDsup(),
  fhTimeDsupBC(),
  fhRawTimeVsIdBC(),
  fhRawTimeSumBC(),
  fhRawTimeEntriesBC(),
  fhRawTimeSumSqBC(),
  fhRawTimeVsIdLGBC(),
  fhRawTimeSumLGBC(),
  fhRawTimeEntriesLGBC(),
  fhRawTimeSumSqLGBC()

{
  for(Int_t i = 0; i < kNBCmask; i++) 
  {
    fhAllAverageBC[i]=0;
    fhAllAverageLGBC[i]=0;

    fhTimeSumSq[i]=0;
    fhTimeEnt[i]=0;    
    fhTimeSum[i]=0;

    fhTimeLGSumSq[i]=0;
    fhTimeLGEnt[i]=0;
    fhTimeLGSum[i]=0;

    fhRawTimeVsIdBC[i]=0;
    fhRawTimeSumBC[i]=0;
    fhRawTimeEntriesBC[i]=0;
    fhRawTimeSumSqBC[i]=0;

    fhRawTimeVsIdLGBC[i]=0;
    fhRawTimeSumLGBC[i]=0;
    fhRawTimeEntriesLGBC[i]=0;
    fhRawTimeSumSqLGBC[i]=0;
  }

  //set default cuts for calibration and geometry name
  SetDefaultCuts();

  //T0 TOF time
  PrepareTOFT0maker();

  // Define input and output slots here
  // Input slot #0 works with a TChain
  DefineInput(0, TChain::Class());
  
  // Output slot #0 id reserved by the base class for AOD
  // Output slot #1 writes into a TH1 container
  DefineOutput(1, TList::Class());
  
} // End ctor

//_____________________________________________________________________
/// HKD Move from constructor
/// Use aliTOFT0maker to get proper T0
/// Look the proper source to have more information
/// Modified July 2, 2010 - HKD to take into account
/// the changes in ALiTOFT0maker
//void AliAnalysisTaskEMCALTimeCalib::LocalInit()
//{
//  AliDebug(1,"AliAnalysisTaskEMCALTimeCalib::LocalInit()");
//}

/// Load reference Histograms from file
//_____________________________________________________________________
void AliAnalysisTaskEMCALTimeCalib::LoadReferenceHistos()
{
  if(fReferenceFileName.Length()!=0){
    TFile *myFile = TFile::Open(fReferenceFileName.Data());
    AliInfo(Form("Reference file: %s, pointer %p",fReferenceFileName.Data(),myFile));
    if(myFile==0x0)
      AliDebug(1,"*** NO REFERENCE FILE");
    else 
      {
	AliDebug(1,"*** OK TFILE");
	// connect ref run here
	for(Int_t i = 0; i < kNBCmask; i++)
	  {
	    fhAllAverageBC[i]=(TH1D*) myFile->Get(Form("hAllTimeAvBC%d",i));
	    fhAllAverageLGBC[i]=(TH1D*) myFile->Get(Form("hAllTimeAvLGBC%d",i));
	  }
	
	AliDebug(1,Form("hAllAverage entries %d", (Int_t)fhAllAverageBC[0]->GetEntries() ));
	AliDebug(1,Form("hAllAverage entries2 %d",(Int_t)fhAllAverageBC[2]->GetEntries() ));
	AliDebug(1,Form("hAllAverageLG entries %d", (Int_t)fhAllAverageLGBC[0]->GetEntries() ));
	AliDebug(1,Form("hAllAverageLG entries2 %d",(Int_t)fhAllAverageLGBC[2]->GetEntries() ));
	
      }
  }//end of reference file is provided
} // End of AliAnalysisTaskEMCALTimeCalib::LoadReferenceHistos()

//_____________________________________________________________________
/// Connect ESD or AOD here
/// Called once per file
Bool_t AliAnalysisTaskEMCALTimeCalib::Notify()
{
  AliDebug(1,"AnalysisTaskEMCalTimeCalib::Notify()");
  AliDebug(2,Form("Notify(): EMCal geometry: fgeom = %p, fGeometryName=%s\n ",fgeom,fGeometryName.Data()));

  if (!InputEvent())
  {
    AliFatal("ERROR: InputEvent not set");
    return kFALSE;
  }
  else AliDebug(1,"Good, InputEvent set");

  fRunNumber = InputEvent()->GetRunNumber();
  AliDebug(1,Form("RunNumber %d", fRunNumber));

  // Init EMCAL geometry 
  if (!fgeom) SetEMCalGeometry();
  //Init EMCAL geometry done

  return kTRUE;
}

//_____________________________________________________________________
/// Set the EMCal Geometry
Bool_t AliAnalysisTaskEMCALTimeCalib::SetEMCalGeometry()
{
  AliDebug(1,"AliAnalysisTaskEMCALTimeCalib::SetEMCalGeometry()");
  if(fGeometryName.Length()==0){
    fgeom=AliEMCALGeometry::GetInstanceFromRunNumber(fRunNumber);
    AliInfo(Form("Get EMCAL geometry name <%s> for run %d",fgeom->GetName(),fRunNumber));
  } else {
    fgeom = AliEMCALGeometry::GetInstance(fGeometryName.Data());
    AliInfo(Form("Set EMCAL geometry name to <%s>",fGeometryName.Data()));
  }

  if (!fgeom){
    AliWarning("Make sure the EMCal geometry is set properly !");
  } else {
    AliDebug(1,Form("EMCal geometry properly set: fGeom = %p, fGeometryName=%s",fgeom,fGeometryName.Data()));
  }

  return kTRUE;
}

//_____________________________________________________________________
/// Get T0 time from TOF
void AliAnalysisTaskEMCALTimeCalib::PrepareTOFT0maker()
{
  //method under development
  AliInfo(Form("<D> -- Run # = %d", fRunNumber));
  AliInfo("prepare TOFT0maker!!");
  //cout<<"Run "<< fRunNumber<<" in TOFT0maker"<<endl;


  AliCDBManager * cdb = AliCDBManager::Instance();
  cdb->SetDefaultStorage("raw://");
  cdb->SetRun(fRunNumber);
  
  AliESDpid *extPID=new AliESDpid();

  // Wonder if some have to be declared as private variables??
  // AliESDpid *extPID = new AliESDpid();
  // AliTOFcalib * tofCalib = new AliTOFcalib();
  // tofCalib->SetCalibrateTOFsignal(kTRUE);
  // tofCalib->Init();
  
  fTOFmaker = new AliTOFT0maker(extPID);
  fTOFmaker->SetTimeResolution(115.0); // if you want set the TOF res
  // fTOFmaker = new AliTOFT0maker(extPID,tofCalib);
  // fTOFmaker->SetTimeResolution(130.0);

  //cout<<"extPID "<<extPID<<" fTOFmaker "<<fTOFmaker<<endl;
  
}// End PrepareTOFT0maker

//________________________________________________________________________
/// Create histograms
/// Called once
void AliAnalysisTaskEMCALTimeCalib::UserCreateOutputObjects()
{
  AliDebug(1,"AliAnalysisTaskEMCALTimeCalib::UserCreateOutputObjects()");

  Double_t fineTmin = -500;
  Double_t fineTmax =  400;
  Double_t fineInterval = 0.20;
  Int_t nfinebin = (fineTmax-fineTmin)/fineInterval;
  //cout << "<D> nfinebin = " << nfinebin << endl;
  
  Int_t nChannels = 17664;
  //book histograms
  fhcalcEvtTime = new TH1F("fhcalcEvtTime","calculated event time from T0",nfinebin, fineTmin,fineTmax);
  fhcalcEvtTime->GetXaxis()->SetTitle("T ");
  fhcalcEvtTime->GetYaxis()->SetTitle("Counts (a.u.)");
  
  fhEvtTimeHeader = new TH1F("fhEvtTimeHeader","event time from header",nfinebin, fineTmin,fineTmax);
  fhEvtTimeHeader->GetXaxis()->SetTitle("T ");
  fhEvtTimeHeader->GetYaxis()->SetTitle("Counts (a.u.)");

  fhEvtTimeDiff = new TH1F("fhEvtTimeDiff","event time difference",nfinebin, fineTmin,fineTmax);
  fhEvtTimeDiff->GetXaxis()->SetTitle("#Delta T ");
  fhEvtTimeDiff->GetYaxis()->SetTitle("Counts (a.u.)");

  fhEventType = new TH1F("fhEventType","event type",10, 0.,10.);
  fhEventType ->GetXaxis()->SetTitle("Type ");
  fhEventType ->GetYaxis()->SetTitle("Counts (a.u.)");
  fhTcellvsTOFT0 = new TH2F("hTcellvsTOFT0", " T_cell vs TOFT0", 500,-600.0,+400.0,1200,300.0,900.0);
  fhTcellvsTOFT0HD = new TH2F("hTcellvsTOFT0HD", " T_cell vs TOFT0,HighEnergy", 500,-600.0,+400.0,4000,500.0,700.0);
  fhTcellvsSM = new TH2F("hTcellvsSM", " T_cell vs SM", 20,0,20,300,300,900);
  fhEneVsAbsIdHG = new TH2F("fhEneVsAbsIdHG", "energy vs ID for HG",1000,0,18000,200,0,10);
  fhEneVsAbsIdLG = new TH2F("fhEneVsAbsIdLG", "energy vs ID for LG",1000,0,18000,100,0,20);
  
  for (Int_t i = 0; i < kNBCmask ;  i++)
  {
    //already after correction
    //high gain
    fhTimeSumSq[i] = new TH1F(Form("hTimeSumSq%d", i),
			      Form("cell Sum Square time HG, BC %d ", i),
			      nChannels,0.,(Double_t)nChannels);
    fhTimeSumSq[i]->SetYTitle("Sum Sq Time ");
    fhTimeSumSq[i]->SetXTitle("AbsId");
    
    fhTimeSum[i] = new TH1F(Form("hTimeSum%d", i),
			    Form("cell Sum  time HG, BC %d ", i),
			    nChannels,0.,(Double_t)nChannels);
    fhTimeSum[i]->SetYTitle("Sum  Time ");
    fhTimeSum[i]->SetXTitle("AbsId");
    
    fhTimeEnt[i] = new TH1F(Form("hTimeEnt%d", i),
			    Form("cell Entries HG, BC %d ", i),
			    nChannels,0.,(Double_t)nChannels);
    fhTimeEnt[i]->SetYTitle("Entries for Time ");
    fhTimeEnt[i]->SetXTitle("AbsId");
    
    //low gain 
    fhTimeLGSumSq[i] = new TH1F(Form("hTimeLGSumSq%d", i),
			      Form("cell Sum Square time LG, BC %d ", i),
			      nChannels,0.,(Double_t)nChannels);
    fhTimeLGSumSq[i]->SetYTitle("Sum Sq Time ");
    fhTimeLGSumSq[i]->SetXTitle("AbsId");
    
    fhTimeLGSum[i] = new TH1F(Form("hTimeLGSum%d", i),
			    Form("cell Sum time LG, BC %d ", i),
			    nChannels,0.,(Double_t)nChannels);
    fhTimeLGSum[i]->SetYTitle("Sum  Time ");
    fhTimeLGSum[i]->SetXTitle("AbsId");
    
    fhTimeLGEnt[i] = new TH1F(Form("hTimeLGEnt%d", i),
			    Form("cell Entries LG, BC %d ", i),
			    nChannels,0.,(Double_t)nChannels);
    fhTimeLGEnt[i]->SetYTitle("Entries for Time ");
    fhTimeLGEnt[i]->SetXTitle("AbsId");

    //raw time histograms
    //high gain
    fhRawTimeVsIdBC[i] = new TH2F(Form("RawTimeVsIdBC%d", i),
			      Form("cell raw time vs ID for high gain BC %d ", i),
			      nChannels,0.,(Double_t)nChannels,600,300,900);
    fhRawTimeVsIdBC[i]->SetXTitle("AbsId");
    fhRawTimeVsIdBC[i]->SetYTitle("Time");

    fhRawTimeSumBC[i] = new TH1F(Form("RawTimeSumBC%d", i),
				 Form("sum of cell raw time for high gain BC %d ", i),
				 nChannels,0.,(Double_t)nChannels);
    fhRawTimeSumBC[i]->SetXTitle("AbsId");
    fhRawTimeSumBC[i]->SetYTitle("Sum Time");

    fhRawTimeEntriesBC[i] = new TH1F(Form("RawTimeEntriesBC%d", i),
				     Form("No. entries of cells raw time for high gain BC %d ", i),
				     nChannels,0.,(Double_t)nChannels);
    fhRawTimeEntriesBC[i]->SetXTitle("AbsId");
    fhRawTimeEntriesBC[i]->SetYTitle("Entries for Time ");

    fhRawTimeSumSqBC[i] = new TH1F(Form("RawTimeSumSqBC%d", i),
				   Form("sum of (cell raw time)^2 for high gain BC %d ", i),
				   nChannels,0.,(Double_t)nChannels);
    fhRawTimeSumSqBC[i]->SetXTitle("AbsId");
    fhRawTimeSumSqBC[i]->SetYTitle("Sum Sq Time");

    //low gain
    fhRawTimeVsIdLGBC[i] = new TH2F(Form("RawTimeVsIdLGBC%d", i),
			      Form("cell raw time vs ID for low gain BC %d ", i),
			      nChannels,0.,(Double_t)nChannels,600,300,900);
    fhRawTimeVsIdLGBC[i]->SetXTitle("AbsId");
    fhRawTimeVsIdLGBC[i]->SetYTitle("Time");

    fhRawTimeSumLGBC[i] = new TH1F(Form("RawTimeSumLGBC%d", i),
				 Form("sum of cell raw time for low gain BC %d ", i),
				 nChannels,0.,(Double_t)nChannels);
    fhRawTimeSumLGBC[i]->SetXTitle("AbsId");
    fhRawTimeSumLGBC[i]->SetYTitle("Sum Time");

    fhRawTimeEntriesLGBC[i] = new TH1F(Form("RawTimeEntriesLGBC%d", i),
				     Form("No. entries of cells raw time for low gain BC %d ", i),
				     nChannels,0.,(Double_t)nChannels);
    fhRawTimeEntriesLGBC[i]->SetXTitle("AbsId");
    fhRawTimeEntriesLGBC[i]->SetYTitle("Entries for Time ");

    fhRawTimeSumSqLGBC[i] = new TH1F(Form("RawTimeSumSqLGBC%d", i),
				   Form("sum of (cell raw time)^2 for low gain BC %d ", i),
				     nChannels,0.,(Double_t)nChannels);
    fhRawTimeSumSqLGBC[i]->SetXTitle("AbsId");
    fhRawTimeSumSqLGBC[i]->SetYTitle("Sum Sq Time");


    for (Int_t j = 0; j < kNSM ;  j++) 
    {
      fhTimeDsupBC[j][i]= new TH2F(Form("SupMod%dBC%d",j,i), Form("SupMod %d time_vs_E  BC %d",j,i),500,0.0,20.0,1400,-350.0,350.0);
      fhTimeDsupBC[j][i]->SetYTitle(" Time (ns) "); 
      fhTimeDsupBC[j][i]->SetXTitle(" E (GeV) "); 
    }
  }

  for (Int_t jj = 0; jj < kNSM ;  jj++) 
  {
    fhTimeDsup[jj] =  new TH2F(Form("SupMod%d",jj), Form("SupMod %d time_vs_E ",jj),500,0.0,20.0,1400,-350.0,350.0);
    fhTimeDsup[jj]->SetYTitle(" Time (ns) "); 
    fhTimeDsup[jj]->SetXTitle(" E (GeV) "); 
  }
  
  fhTimeVsBC = new TH2F("TimeVsBC"," SupMod time_vs_BC ", 4001,-0.5,4000.5,400,200.0,1000.0); 
  

  //add histos to list
  fOutputList = new TList();
  
  fOutputList->Add(fhcalcEvtTime);
  fOutputList->Add(fhEvtTimeHeader);
  fOutputList->Add(fhEvtTimeDiff);
  fOutputList->Add(fhEventType);
  fOutputList->Add(fhTcellvsTOFT0);
  fOutputList->Add(fhTcellvsTOFT0HD);
  fOutputList->Add(fhTcellvsSM);
  fOutputList->Add(fhEneVsAbsIdHG);
  fOutputList->Add(fhEneVsAbsIdLG);

  for (Int_t i = 0; i < kNBCmask ;  i++) 
  {
    fOutputList->Add(fhTimeSumSq[i]);
    fOutputList->Add(fhTimeEnt[i]);
    fOutputList->Add(fhTimeSum[i]);

    fOutputList->Add(fhTimeLGSumSq[i]);
    fOutputList->Add(fhTimeLGEnt[i]);
    fOutputList->Add(fhTimeLGSum[i]);

    fOutputList->Add(fhRawTimeVsIdBC[i]);
    fOutputList->Add(fhRawTimeSumBC[i]);
    fOutputList->Add(fhRawTimeEntriesBC[i]);
    fOutputList->Add(fhRawTimeSumSqBC[i]);

    fOutputList->Add(fhRawTimeVsIdLGBC[i]);
    fOutputList->Add(fhRawTimeSumLGBC[i]);
    fOutputList->Add(fhRawTimeEntriesLGBC[i]);
    fOutputList->Add(fhRawTimeSumSqLGBC[i]);

    for (Int_t j = 0; j < kNSM ;  j++){
      fOutputList->Add(fhTimeDsupBC[j][i]);
    }
  }
  
  for (Int_t j = 0; j < kNSM ;  j++)
  {
    fOutputList->Add(fhTimeDsup[j]);
  }
	
  fOutputList->Add(fhTimeVsBC);
  
  fOutputList->SetOwner(kTRUE);
  PostData(1,fOutputList);

  
} // End of AliAnalysisTaskEMCALTimeCalib::UserCreateOuputObjects()

//________________________________________________________________________
/// Main loop executed for each event
void AliAnalysisTaskEMCALTimeCalib::UserExec(Option_t *)
{
  // Called for each event
  AliDebug(2,Form("UserExec: EMCal geometry: fgeom = %p fGeometryName %s",fgeom,fGeometryName.Data()));
  AliVEvent   *event = InputEvent();
  //cout<<"T0TOF "<<event->GetT0TOF()<<endl;//bad idea
  //cout<< fEvent->GetTOFHeader()->GetDefaultEventTimeVal()<<endl;
  AliDebug(2,Form("TOF time from header %f ps",event->GetTOFHeader()->GetDefaultEventTimeVal()));
  fhEvtTimeHeader->Fill(event->GetTOFHeader()->GetDefaultEventTimeVal());

  //fEvent = dynamic_cast<AliESDEvent*>(event);
  if (!event) {
    AliError("ESD not available, exit");
    fhEventType->Fill(0.5);
    return;
  }
  
  if(fPileupFromSPD==kTRUE){
    if(event->IsPileupFromSPD(3,0.8,3.,2.,5.)){
      AliDebug(1,"Event: PileUp skip.");
      fhEventType->Fill(1.5);
      return;
    }	
  }

  TString triggerclasses = event->GetFiredTriggerClasses();
  if(triggerclasses=="") {
    fhEventType->Fill(2.5);
    return;
  }

  Int_t eventType = ((AliVHeader*)event->GetHeader())->GetEventType();
  // physics events eventType=7, select only those
  AliDebug(1,Form("Triggerclasses %s, eventType %d",triggerclasses.Data(),eventType));
  if(eventType != 7) {
    fhEventType->Fill(3.5);
    return;
  }
  
  // Check trigger
  Bool_t bMB  = kFALSE;
  Bool_t bL0  = kFALSE;
  Bool_t bL1G = kFALSE;
  Bool_t bL1J = kFALSE;
  
  if(triggerclasses.Contains("CINT7-B-NOPF-ALLNOTRD") ||
     triggerclasses.Contains("CINT7-I-NOPF-ALLNOTRD") ||
     triggerclasses.Contains("CINT1-I-NOPF-ALLNOTRD") || 
     triggerclasses.Contains("CINT1-B-NOPF-ALLNOTRD") ||
     triggerclasses.Contains("CINT8") ||
     triggerclasses.Contains("CINT7") ||
     triggerclasses.Contains("CPBI2_B1-B-NOPF-ALLNOTRD") )   bMB  = kTRUE;
  
  if(triggerclasses.Contains("CEMC7-B-NOPF-CENTNOTRD") || 
     triggerclasses.Contains("CEMC1-B-NOPF-CENTNOTRD") ||
     triggerclasses.Contains("CEMC7") ||
     triggerclasses.Contains("CEMC8") ||
     triggerclasses.Contains("CEMC8-B-NOPF-CENTNOTRD")   )   bL0  = kTRUE;
  
  if(triggerclasses.Contains("CEMC7EG1-B-NOPF-CENTNOTRD") ||
     triggerclasses.Contains("CEMC7EG2-B-NOPF-CENTNOTRD") ||
     triggerclasses.Contains("CEMC8EG1-B-NOPF-CENTNOTRD") ||
     triggerclasses.Contains("CEMC8EGA") ||
     triggerclasses.Contains("CEMC7EGA") ||
     triggerclasses.Contains("CPBI2EGA")                 )   bL1G = kTRUE;
 
  
  if(triggerclasses.Contains("CEMC7EJ1-B-NOPF-CENTNOTRD") ||
     triggerclasses.Contains("CEMC7EJ2-B-NOPF-CENTNOTRD") ||
     triggerclasses.Contains("CEMC8EJ1-B-NOPF-CENTNOTRD") ||
     triggerclasses.Contains("CEMC7EJE") ||
     triggerclasses.Contains("CEMC8EJE") ||
     triggerclasses.Contains("CPBI2EJE")                 )   bL1J = kTRUE;
  
  if( bL1G || bL1J ||  bL0 ){ fhEventType->Fill(4.5);}
  if( bMB ){ fhEventType->Fill(5.5);}


  //  if(bL1G || bL1J ||  bL0){

  // Prepare TOFT0 maker at the beginning of a run
//  if (event->GetRunNumber() != fRunNumber)
//  {
//    fRunNumber = event->GetRunNumber();
//    //	PrepareTOFT0maker();
//    //  	cout<<"tofT0maker per run"<<fRunNumber<<endl;
//  }// fi Check if run number has changed
  
  // --- Use of AliTOFT0maker
  Double_t calcolot0=0.0;
  if(!AODEvent()){
    Double_t* timeTOFtable;
    timeTOFtable=fTOFmaker->ComputeT0TOF(dynamic_cast<AliESDEvent*>(event));
    AliDebug(2,Form("TOF time %f ps, resolution %f ps, tracks at TOF %f/used %f",timeTOFtable[0],timeTOFtable[1],timeTOFtable[3],timeTOFtable[7]));
    //cout<<"event time "<<timeTOFtable[0]<<" resolution "<<timeTOFtable[1]<<"ps av. ev. time "<<timeTOFtable[2]<<" trks at TOF "<<timeTOFtable[3]<<" calc evnt time "<<timeTOFtable[4]<<" resolution "<<timeTOFtable[5]<<" tracks used "<<timeTOFtable[7]<<endl;
    calcolot0=timeTOFtable[0];
  }

  if (!fhcalcEvtTime) {
    AliWarning("<E> fhcalcEvtTime not available");
    return;
  }// fi no simple histo present
  
  fhcalcEvtTime->Fill(calcolot0);
  if(calcolot0 != 0 && event->GetTOFHeader()->GetDefaultEventTimeVal() != 0 )
    fhEvtTimeDiff->Fill(calcolot0-event->GetTOFHeader()->GetDefaultEventTimeVal());
  
  TRefArray* caloClusters = new TRefArray();
  event->GetEMCALClusters(caloClusters);
  //           	cout << " ###########Bunch Cross nb  = " << event->GetBunchCrossNumber() << endl;
  
  Int_t BunchCrossNumber =event->GetBunchCrossNumber(); 
  
  Float_t offset=0;
  
  Int_t nBC = 0;
  nBC = BunchCrossNumber%4;
  //Int_t nTriggerMask =event->GetTriggerMask();
  //	cout << " nBC " << nBC << " nTriggerMask " << nTriggerMask<< endl;
  Float_t timeBCoffset = 0.; //manual offest
  //	if( nBC%4 ==0 || nBC%4==1) timeBCoffset = 100.; // correction was for LHC11 when BC was not corrected
  
  Int_t nclus = caloClusters->GetEntries();
  AliDebug(1,Form("###########Bunch Cross nb = %d nclus = %d",nBC,nclus ));
  //cout << " ###########Bunch Cross nb  = " << nBC <<" nclus= "<< nclus<< endl;
  //Int_t ntracks = event-> GetNumberOfTracks() ; 
  
  for (Int_t icl = 0; icl < nclus; icl++) {
    //ESD and AOD CaloCells carries the same information
    AliVCluster* clus = (AliVCluster*)caloClusters->At(icl);
    if(!AcceptCluster(clus)) continue;
  
    AliVCaloCells &cells= *(event->GetEMCALCells());
    //cout<<"nCells="<< clus->GetNCells();<<endl;
   
    UShort_t * index = clus->GetCellsAbsId() ;
    
    for(Int_t i = 0; i < clus->GetNCells() ; i++) {
      Int_t absId =   index[i]; // or clus->GetCellNumber(i) ;
      Float_t hkdtime   = cells.GetCellTime(absId) * 1.0e09; // to get ns
      Float_t amp       = cells.GetCellAmplitude(absId) ;
      Bool_t isHighGain = cells.GetCellHighGain(absId);
      //cout<<"cell absID: "<<absId<<" cellTime: "<<hkdtime<<" cellaplit: "<< amp<<endl;	
      Int_t nSupMod, nModule;//,iRCU;
      Int_t iphi, ieta, nIphi, nIeta;


      //main histograms with raw time information 
      if(amp>fMinCellEnergy){
	if(isHighGain){
	  fhRawTimeVsIdBC[nBC]->Fill(absId,hkdtime);
	  fhRawTimeSumBC[nBC]->Fill(absId,hkdtime);
	  fhRawTimeEntriesBC[nBC]->Fill(absId,1.);
	  fhRawTimeSumSqBC[nBC]->Fill(absId,hkdtime*hkdtime);
	}else{
	  fhRawTimeVsIdLGBC[nBC]->Fill(absId,hkdtime);
	  fhRawTimeSumLGBC[nBC]->Fill(absId,hkdtime);
	  fhRawTimeEntriesLGBC[nBC]->Fill(absId,1.);
	  fhRawTimeSumSqLGBC[nBC]->Fill(absId,hkdtime*hkdtime);
	}
      }
      //fgeom->PrintCellIndexes(absId);
      //fgeom->PrintCellIndexes(absId,1);
      
      // GEOMETRY tranformations
      fgeom->GetCellIndex(absId,  nSupMod, nModule, nIphi, nIeta);
      fgeom->GetCellPhiEtaIndexInSModule(nSupMod,nModule,nIphi,nIeta, iphi,ieta);

      // other histograms for cross-check      
      CheckCellRCU(nSupMod,ieta,iphi);//SM, column, row

      fhTcellvsSM->Fill(nSupMod,hkdtime);
      if(isHighGain==kTRUE) {fhEneVsAbsIdHG->Fill(absId,amp);}
      else {fhEneVsAbsIdLG->Fill(absId,amp);}
      
      fhTimeVsBC->Fill(1.*BunchCrossNumber,hkdtime-timeBCoffset);
      if(isHighGain==kTRUE){
	if(fhAllAverageBC[nBC]!=0) {//comming from file after the first iteration
	  offset = fhAllAverageBC[nBC]->GetBinContent(absId);
	}
      } else {
	if(fhAllAverageLGBC[nBC]!=0) {//comming from file after the first iteration
	  offset = fhAllAverageLGBC[nBC]->GetBinContent(absId);
	}
      }
      //if(offset==0)cout<<"offset 0 in SM "<<nSupMod<<endl;
      

      if(amp>0.5) {					
	fhTimeDsup[nSupMod]->Fill(amp,hkdtime-offset);
	fhTimeDsupBC[nSupMod][nBC]->Fill(amp,hkdtime-offset);
      }
      
      if(amp>0.9) {
	fhTcellvsTOFT0HD->Fill(calcolot0, hkdtime);
      }

      fhTcellvsTOFT0->Fill(calcolot0, hkdtime-offset);

      hkdtime = hkdtime-timeBCoffset;//time corrected by manual offset (default=0)
      Float_t hkdtimecorr;
      hkdtimecorr= hkdtime-offset;//time after first iteration

      //main histograms after the first itereation for calibration constants
      //if(hkdtimecorr>=-20. && hkdtimecorr<=20. && amp>0.9 ) {
      if(hkdtimecorr>=fMinTime && hkdtimecorr<=fMaxTime && amp>fMinCellEnergy ) {
	// per cell
//	Float_t entriesTime=fhTimeEnt[nBC]->GetBinContent(absId)+1;
//	Float_t sumTimeSq=(fhTimeSumSq[nBC]->GetBinContent(absId)+(hkdtime*hkdtime));
//	Float_t sumTime=(fhTimeSum[nBC]->GetBinContent(absId)+hkdtime);
//	
//	fhTimeEnt[nBC]->SetBinContent(absId,entriesTime);
//	fhTimeSumSq[nBC]->SetBinContent(absId,sumTimeSq);
//	fhTimeSum[nBC]->SetBinContent(absId,sumTime);

	if(isHighGain){
	  fhTimeEnt[nBC]->Fill(absId,1.);
	  fhTimeSumSq[nBC]->Fill(absId,hkdtime*hkdtime);
	  fhTimeSum[nBC]->Fill(absId,hkdtime);
	}else{
	  fhTimeLGEnt[nBC]->Fill(absId,1.);
	  fhTimeLGSumSq[nBC]->Fill(absId,hkdtime*hkdtime);
	  fhTimeLGSum[nBC]->Fill(absId,hkdtime);
	}


      } // hkdtime:[-20;20]
    } // end icell
  } //end cluster
  
  
  // Post output data.
  //cout<<"Post data and delete caloClusters"<<endl;
  caloClusters->Delete();
  delete caloClusters;
// } // end if trigger type 

  PostData(1, fOutputList);  
} // End of AliAnalysisTaskEMCALTimeCalib::UserExec()

//________________________________________________________________________
/// Draw result to the screen
/// Called once at the end of the query
void AliAnalysisTaskEMCALTimeCalib::Terminate(Option_t *)
{
  fOutputList = dynamic_cast<TList*> (GetOutputData(1));
  
  if(fTOFmaker) delete fTOFmaker;


  if (!fOutputList) 
  {
    AliDebug(1,"ERROR: Output list not available");
    return;
  }
} // End of AliAnalysisTaskEMCALTimeCalib::Terminate

//________________________________________________________________________
/// Selection criteria of good cluster are set here
Bool_t AliAnalysisTaskEMCALTimeCalib::AcceptCluster(AliVCluster* clus)
{
  //fix with noisy EMCAL fee card
  Int_t nCells = clus->GetNCells();
  
  if(clus->IsEMCAL())
  {    
    if ((clus->E() > fMaxClusterEnergy && nCells > fMaxNcells ) || nCells > fMaxNcells){
      AliDebug(1,"very big cluster with enormous energy - cluster rejected");
      return kFALSE;
    }
  }
  
  // remove other than photonlike
  Double_t lambda0=clus->GetM02();
  if (lambda0>fMaxLambda0 || lambda0<fMinLambda0){
    AliDebug(1,"lambda0 failed - cluster rejected");
    return kFALSE;
  }

  // remove matched clusters
  Double_t Dx=clus->GetTrackDx();
  Double_t Dz=clus->GetTrackDz();
  Double_t Rtrack = TMath::Sqrt(Dx*Dx+Dz*Dz);
  if (Rtrack <fMaxRtrack) 
  {
    AliDebug(1,"track matched - cluster rejected");
    return kFALSE;
  }

  if (nCells<fMinNcells) 
  {
    AliDebug(1,"single cell cluster - cluster rejected");
    return kFALSE;
  }

  if(clus->E()<fMinClusterEnergy) 
  {
    AliDebug(1,"cluster energy < 1 GeV- cluster rejected");
    return kFALSE;
  }

  return kTRUE;
}//End AliAnalysisTaskEMCALTimeCalib::AcceptCluster

//________________________________________________________________________
/// Check RCU for cell given by Super Module, column index, row index
Bool_t AliAnalysisTaskEMCALTimeCalib::CheckCellRCU(Int_t nSupMod,Int_t icol,Int_t irow)
{
  Int_t iRCU;
  if(nSupMod < 10 || (nSupMod >= 12 && nSupMod <18) ) 
  {
    if      (0<=irow&&irow<8)                       iRCU=0; // first cable row
    else if (8<=irow&&irow<16 &&  0<=icol&&icol<24) iRCU=0; // first half; 
    //second cable row
    //RCU1
    else if (8<=irow&&irow<16 && 24<=icol&&icol<48) iRCU=1; // second half; 
    //second cable row
    else if (16<=irow&&irow<24)                     iRCU=1; // third cable row
    
    if (nSupMod%2==1) iRCU = 1 - iRCU; // swap for odd=C side, to allow us to cable both sides the same
  } 
  else 
  {
	// Last 2 SM have one single SRU, just assign RCU 0
    iRCU = 0 ;
  }

  //cout<<"RCU:"<<iRCU<<endl;
  if (iRCU<0) 
    AliFatal(Form("Wrong EMCAL/DCAL RCU number = %d\n", iRCU));

  return kTRUE;
}//End AliAnalysisTaskEMCALTimeCalib::CheckCellRCU

//________________________________________________________________________
/// Set default cuts for calibration
void AliAnalysisTaskEMCALTimeCalib::SetDefaultCuts()
{
  fMinClusterEnergy=1.0;//0.5//0.7
  fMaxClusterEnergy=500;
  fMinNcells=2;
  fMaxNcells=200;
  fMinLambda0=0.1;
  fMaxLambda0=0.4;
  fMaxRtrack=0.025;
  fMinCellEnergy=0.4;//0.1//0.4
  fReferenceFileName="";//Reference.root
  fGeometryName="";//EMCAL_COMPLETE12SMV1_DCAL_8SM
  fPileupFromSPD=kFALSE;
  fMinTime=-20.;
  fMaxTime=20.;
}

//________________________________________________________________________
/// Calculate calibration constants
/// input - root file with histograms 
/// output - root file with constants in historams
/// isFinal - flag: kFALSE-first iteration, kTRUE-final iteration
void AliAnalysisTaskEMCALTimeCalib::ProduceCalibConsts(TString inputFile,TString outputFile,Bool_t isFinal)
{
  TFile *file =new TFile(inputFile.Data());
  if(file==0x0) {
    //AliWarning("Input file does not exist!");
    return;
  }

  TList *list=(TList*)file->Get("chistolist");
  if(list==0x0) 
  {
    //AliWarning("List chistolist does not exist in file!");
    return;
  }

  //high gain
  TH1F *h1[4];
  TH1F *h2[4];
  TH1F *h3[4];
  TH1F *hAllTimeAvBC[4];
  TH1F *hAllTimeRMSBC[4];

  //low gain
  TH1F *h4[4];
  TH1F *h5[4];
  TH1F *h6[4];
  TH1F *hAllTimeAvLGBC[4];
  TH1F *hAllTimeRMSLGBC[4];

  if(isFinal==kFALSE){//first itereation
    for(Int_t i=0;i<4;i++){
      h1[i]=(TH1F *)list->FindObject(Form("RawTimeSumBC%d",i));
      h2[i]=(TH1F *)list->FindObject(Form("RawTimeEntriesBC%d",i));
      h3[i]=(TH1F *)list->FindObject(Form("RawTimeSumSqBC%d",i));
      
      h4[i]=(TH1F *)list->FindObject(Form("RawTimeSumLGBC%d",i));
      h5[i]=(TH1F *)list->FindObject(Form("RawTimeEntriesLGBC%d",i));
      h6[i]=(TH1F *)list->FindObject(Form("RawTimeSumSqLGBC%d",i));
    }
  } else {//final iteration
    for(Int_t i=0;i<4;i++){
      h1[i]=(TH1F *)list->FindObject(Form("hTimeSum%d",i));
      h2[i]=(TH1F *)list->FindObject(Form("hTimeEnt%d",i));
      h3[i]=(TH1F *)list->FindObject(Form("hTimeSumSq%d",i));
      
      h4[i]=(TH1F *)list->FindObject(Form("hTimeLGSum%d",i));
      h5[i]=(TH1F *)list->FindObject(Form("hTimeLGEnt%d",i));
      h6[i]=(TH1F *)list->FindObject(Form("hTimeLGSumSq%d",i));
    }
  }
  //AliWarning("Input histograms read.");

  for(Int_t i=0;i<4;i++){
    hAllTimeAvBC[i]=new TH1F(Form("hAllTimeAvBC%d",i),Form("hAllTimeAvBC%d",i),h1[i]->GetNbinsX(),h1[i]->GetXaxis()->GetXmin(),h1[i]->GetXaxis()->GetXmax());
    hAllTimeRMSBC[i]=new TH1F(Form("hAllTimeRMSBC%d",i),Form("hAllTimeRMSBC%d",i),h3[i]->GetNbinsX(),h3[i]->GetXaxis()->GetXmin(),h3[i]->GetXaxis()->GetXmax());

    hAllTimeAvLGBC[i]=new TH1F(Form("hAllTimeAvLGBC%d",i),Form("hAllTimeAvLGBC%d",i),h4[i]->GetNbinsX(),h4[i]->GetXaxis()->GetXmin(),h4[i]->GetXaxis()->GetXmax());
    hAllTimeRMSLGBC[i]=new TH1F(Form("hAllTimeRMSLGBC%d",i),Form("hAllTimeRMSLGBC%d",i),h6[i]->GetNbinsX(),h6[i]->GetXaxis()->GetXmin(),h6[i]->GetXaxis()->GetXmax());
  }
  
  //AliWarning("New histograms booked.");

  for(Int_t i=0;i<4;i++){
    for(Int_t j=1;j<=h1[i]->GetNbinsX();j++){
      //high gain
      if(h2[i]->GetBinContent(j)!=0){
	hAllTimeAvBC[i]->SetBinContent(j,h1[i]->GetBinContent(j)/h2[i]->GetBinContent(j));
	hAllTimeRMSBC[i]->SetBinContent(j,TMath::Sqrt(h3[i]->GetBinContent(j)/h2[i]->GetBinContent(j)) );
      } else {
	hAllTimeAvBC[i]->SetBinContent(j,0.);
	hAllTimeRMSBC[i]->SetBinContent(j,0.);
      }
      //low gain
      if(h5[i]->GetBinContent(j)!=0){
	hAllTimeAvLGBC[i]->SetBinContent(j,h4[i]->GetBinContent(j)/h5[i]->GetBinContent(j));
	hAllTimeRMSLGBC[i]->SetBinContent(j,TMath::Sqrt(h6[i]->GetBinContent(j)/h5[i]->GetBinContent(j)) );
      } else {
	hAllTimeAvLGBC[i]->SetBinContent(j,0.);
	hAllTimeRMSLGBC[i]->SetBinContent(j,0.);
      }

    }
  }

  //AliWarning("Average and rms calculated.");
  TFile *fileNew=new TFile(outputFile.Data(),"recreate");
  for(Int_t i=0;i<4;i++){
    hAllTimeAvBC[i]->Write();
    hAllTimeRMSBC[i]->Write();
    hAllTimeAvLGBC[i]->Write();
    hAllTimeRMSLGBC[i]->Write();
  }

  //AliWarning(Form("Histograms saved in %s file.",outputFile.Data()));

  fileNew->Close();
  delete fileNew;
  file->Close();
  delete file;

  //AliWarning("Pointers deleted. Memory cleaned.");

}
