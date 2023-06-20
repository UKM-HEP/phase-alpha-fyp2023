#include <iostream>
#include <cstdlib>
#include <sstream>

#include <TFile.h>
#include <TChain.h>
#include <TTree.h>
#include <TGraph.h>
#include <TCanvas.h>
#include <TRint.h>

/////////////////////////////////////////////////////////////////////
// Input data structure
/////////////////////////////////////////////////////////////////////
struct InputData_t
{
  TChain* tree;
  
  // Run and event numbers
  Int_t     run, event;
  // Event time stamp in seconds (UNIX time) and microseconds
  //ULong64_t tstampS, tstampU;
  // Time width of waveform sampling
  Float_t               dt;
  // Waveform values 
  std::vector<Float_t>* wf[4];
};


/////////////////////////////////////////////////////////////////////
// Sub routines
////////////////////////////////////////////////////////////////////

// Sub: Setup the input branches
void SetupInputBranches(InputData_t& idata)
{
  idata.tree->SetBranchAddress("run",     &idata.run);
  idata.tree->SetBranchAddress("event",   &idata.event);
  //idata.tree->SetBranchAddress("ts.sec",  &idata.tstampS);
  //idata.tree->SetBranchAddress("ts.usec", &idata.tstampU);
  //idata.tree->SetBranchAddress("dt",      &idata.dt);
  idata.dt = 0.8; // fixed now

  for(int ch=0; ch<4; ch++){
    idata.wf[ch] = NULL;
    idata.tree->SetBranchAddress(Form("pwf%c", (char)('A' + ch) ), &idata.wf[ch]);
  }
}


int times = 0, start, end;

// Sub: Process an event data

void ViewEventWaveform(InputData_t& idata, const Long64_t eventNo)
{
  TGraph waveform[4];

  waveform[0].SetNameTitle("Waveform_A", "Waveform A;t;x");
  waveform[1].SetNameTitle("Waveform_B", "Waveform B;t;x");
  waveform[2].SetNameTitle("Waveform_C", "Waveform C;t;x");
  waveform[3].SetNameTitle("Waveform_D", "Waveform D;t;x");

  std::cout << std::endl;
  std::cout << "<<< EVENT " << eventNo << " >>>" << std::endl;
  std::cout << std::endl;

  for(int ch=0; ch<4; ch++){
    // Waveform data of this channel
    std::vector<Float_t>& wf = *(idata.wf[ch]);

    std::cout<< "CH" << ch << " size is " << wf.size() <<std::endl;

    if(0 == wf.size()) continue;

    for (size_t i=0; i<wf.size(); i++) {
      waveform[ch].AddPoint(i, wf[i]);
    }
  }

  std::stringstream wintitle;
  wintitle << "EVENT " << eventNo << " Pico waveforms";
  TCanvas *c1 = new TCanvas("c1", wintitle.str().c_str(), 1024, 768);
  c1->Divide(2,2);
  c1->Draw();
  for (int ch=0; ch<4; ch++) {
    c1->cd(ch+1);
    waveform[ch].Draw();
  }
  c1->Update();
  c1->WaitPrimitive();
  delete c1;

}

/////////////////////////////////////////////////////////////////////
// Main routine (This viewer begins from this part)
/////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
  TRint app("app",0,0);
  if(3 > argc){
    std::cerr << argv[0] << " [event number] [input decoded (root) waveform file name] (another input files if you have...)" << std::endl;
    return 1;
  }
  
  // Load input files
  InputData_t idata;
  idata.tree = new TChain("tr");
  for(int i=2; i<argc; i++){
    idata.tree->Add(argv[i]);
  }
  // Check if the input data were loaded correctly
  if(0 == idata.tree->GetNtrees()){
    std::cerr << "No trees loaded." << std::endl;
    return 2;
  }
  // Set up the input branches
  SetupInputBranches(idata);
  
  // Get target event number
  const long targEventNo(atol(argv[1]));

  //////////////////////////////////////////////////
  // Loop for events
  //////////////////////////////////////////////////
  for(Long64_t entry=0; entry<idata.tree->GetEntries(); entry++){
    if (entry < targEventNo)
      continue;
    // Load input data for this event
    idata.tree->GetEntry(entry);
    // Process the event data
    ViewEventWaveform(idata, entry);
  }

  return 0;
}
