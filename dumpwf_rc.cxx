#include <iostream>
#include <cstdlib>

#include <TFile.h>
#include <TChain.h>
#include <TTree.h>
//#include <TRint.h>

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

void DumpEventWaveform(InputData_t& idata, const Long64_t eventNo, const int wfNo)
{
  idata.tree->GetEntry(eventNo);
  std::cout << "# event " << eventNo << " waveform " << wfNo << std::endl;

  // Waveform data of the selected channel
  std::vector<Float_t>& wf = *(idata.wf[wfNo]);

  for (size_t i=0; i<wf.size(); i++) {
    std::cout << i << " " << wf[i] << std::endl;
  }
}

/////////////////////////////////////////////////////////////////////
// Main routine (This viewer begins from this part)
/////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
  //TRint app("app",0,0);
  if(4 > argc){
    std::cerr << "Usage: " << argv[0] << " [event number] [waveform channel] [input decoded (root) waveform file name] (another input files if you have...)" << std::endl;
    return 1;
  }
  
  // Load input files
  InputData_t idata;
  idata.tree = new TChain("tr");
  for(int i=3; i<argc; i++){
    idata.tree->Add(argv[i]);
  }
  // Check if the input data were loaded correctly
  if(0 == idata.tree->GetNtrees()){
    std::cerr << "No trees loaded." << std::endl;
    return 2;
  }
  // Set up the input branches
  SetupInputBranches(idata);
  
  // Get target event number and waveform channel number
  const Long64_t targEventNo(atoll(argv[1]));
  const int desiredChannelNo(atoi(argv[2]));

  // Make sure waveform channel number is valid
  if (!(desiredChannelNo >= 0 && desiredChannelNo <= 3)) {
    std::cerr << "Invalid waveform channel number." << std::endl;
    return 3;
  }

  // Now do the waveform dump
  DumpEventWaveform(idata, targEventNo, desiredChannelNo);

  return 0;
}
