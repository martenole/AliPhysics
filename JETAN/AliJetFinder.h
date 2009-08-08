#ifndef ALIJETFINDER_H
#define ALIJETFINDER_H
 
/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */
 
//---------------------------------------------------------------------
// Jet finder base class
// manages the search for jets 
// Authors: jgcn@mda.cinvestav.mx
//          andreas.morsch@cern.ch
//          magali.estienne@subatech.in2p3.fr
//---------------------------------------------------------------------

//#include <Riostream.h>
//#include <vector>

#include <TObject.h>
#include "AliAODJet.h"
#include "AliJetReader.h"
#include "AliJetHeader.h"
#include "AliJetReaderHeader.h"

class TChain;
class AliAODJet;
class AliAODEvent;

class AliJetFinder : public TObject 
{
 public:
  AliJetFinder();
  virtual ~AliJetFinder();

  // Getters
  virtual AliJetReader *GetReader() {return fReader;}
  virtual AliJetHeader *GetHeader() {return fHeader;}
  // Setters
  virtual void          SetJetReader(AliJetReader* r) {fReader=r;}
  virtual void          SetJetHeader(AliJetHeader* h) {fHeader=h;}
  // Others
  virtual void          AddJet(AliAODJet jet);
  virtual void          WriteRHeaderToFile();  
  // the following have to be implemented for each specific finder
  virtual void          Init() {}
  virtual void          InitTask(TChain* /*tree*/) {}
  virtual void          Reset() {fNAODjets = 0;}
  virtual void          FindJets() {}
  virtual void          FindJetsC(){}
  virtual void          WriteJHeaderToFile() {}
  // some methods to allow steering from the outside
  virtual Bool_t        ProcessEvent();
  virtual Bool_t        ProcessEvent2();
  virtual void          ConnectTree(TTree* tree, TObject* data);
  virtual void          ConnectAOD(AliAODEvent* aod);
  virtual void          ConnectAODNonStd(AliAODEvent* aod,const char* bname);
  virtual void          WriteHeaders();

 protected:
  AliJetFinder(const AliJetFinder& rJetFinder);
  AliJetFinder& operator = (const AliJetFinder& rhsf);
  AliJetReader* fReader;         //  pointer to reader
  AliJetHeader* fHeader;         //  pointer to header
  TClonesArray* fAODjets;        //! reconstructed jets
  Int_t         fNAODjets;       //! number of reconstructed jets
  ClassDef(AliJetFinder,2)
};

#endif
