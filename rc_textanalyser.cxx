#include <iostream>
#include <cstdlib>

#include <TFile.h>
#include <TChain.h>
#include <TTree.h>
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
  ULong64_t tstampS, tstampU;
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

void ProcessEvent(InputData_t& idata)
{
  std::vector<float> T0R;
  std::vector<float> T0L;
  std::vector<float> T1;
  std::vector<float> T2;
  std::vector<float> tT1T2;
  // ===================================================
  // Calculate area of every waveform
  // ===================================================
  // Apply the same calculation for all channels (A -> D)
  for(int ch=0; ch<4; ch++){

    // Print channel banner
    std::cout << ">> Channel #" << ch << std::endl;
    std::cout << std::endl;

    // Waveform data of this channel
    std::vector<Float_t>& wf = *(idata.wf[ch]);

    //std::cout<< ch << wf.size() <<std::endl;

    if(0 == wf.size()) continue;


    // Get baseline
    Double_t base = 0.0;
    const size_t baseSamples = 100;
    for(size_t i=500; i<500 + baseSamples; i++){
      base += wf[i];
    }
    base /= baseSamples;

    std::cout << "Baseline (average of sample #500 to sample #"
              << 500 + baseSamples << ") = " << base << std::endl;
    std::cout << std::endl;
    
    // Value of the area 
    Double_t area = 0;
    
    // Loop for every amplitude (0 to wf.size()-1)
    //    size_t range0 = 650;
    //    size_t range1 = 800;
    size_t range0 = 0;
    size_t range1 = 1000;
    //if(2 == ch){
    //	range0 = 650;
    //	range1 = 800;
    //}

    // Choose minimum waveheight
    Float_t min = wf[0];
    for(size_t i=0; i<wf.size()-1; i++){
      if(wf[i]<min){
	min = wf[i];
      }
    }

    std::cout << "Maximum amplitude = " << -min << std::endl;
    std::cout << std::endl;

    // Threshold
    size_t i=0;
    if(ch==0){
      Float_t thre = -40;
      std::cout << "T0R thresholds indices (thre=" << thre << ") :" << std::endl;
      for(i=0; i<wf.size()-1; i++){
	if(thre < wf[i] && wf[i+1] < thre){
	  T0R.push_back(i+1);
          std::cout << " " << i+1;
	}
      }     
      std::cout << std::endl;
      std::cout << std::endl;
    }    

    if(ch==1){
      Float_t thre = -40;
      std::cout << "T0L thresholds indices (thre=" << thre << ") :" << std::endl;
     for(i=0; i<wf.size()-1; i++){
	if(thre < wf[i] && wf[i+1] < thre){
	  T0L.push_back(i+1);
          std::cout << " " << i+1;
	}
      }
      std::cout << std::endl;
      std::cout << std::endl;
    }
    
    if(ch==2){
      Float_t thre = -40;
      std::cout << "T1 thresholds indices (thre=" << thre << ") :" << std::endl;
      for(i=0; i<wf.size()-1; i++){
	if(thre < wf[i] && wf[i+1] < thre){
	  T1.push_back(i+1);
          std::cout << " " << i+1;
	  //	  std::cout << ch << ":" << T1.back() <<std::endl;

	}
      } 
      std::cout << std::endl;
      std::cout << std::endl;
    }

    if(ch==3){
      Float_t thre = -20;
      std::cout << "T2 thresholds indices (thre=" << thre << ") :" << std::endl;
      for(i=0; i<wf.size()-1; i++){
	if(thre < wf[i] && wf[i+1] < thre){
	  T2.push_back(i+1);
          std::cout << " " << i+1;
	  //	   std::cout << ch << ":" << T2.back() <<std::endl;
	}
      }
      std::cout << std::endl;
      std::cout << std::endl;
    }
    
    //////////////////////////////
    
	for(size_t i=range0; i<range1; i++){
      // Accumulate negative waveform amplitude to the 'area'
      // Suppose the waveform amplitude is negative.
      area += (wf[i] - base); // i-th amplitude of the waveform
    }

    // Constant factor to convert the area into pC
    // 'XXeYY' means XX x 10^YY
    Double_t factor = 1e-3 /*mV*/ * idata.dt * 1e-9 /*nsec*/ / 50 /*ohm*/ / 1e-12 /*into pC*/;
    // Print the value
    std::cout << "Factor * Area = " << (factor * area) << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;
  }

  std::cout << ">>> Timing analysis" << std::endl;
  std::cout << std::endl;
  
  std::cout << "All T1-T0R intervals:" << std::endl;
  for(int j=0; j<T1.size(); j++){
    for(int k=0; k<T0R.size(); k++){
      float a = T1[j] - T0R[k];
      std::cout << " " << a;
      //	std::cout << "a" << a << std::endl;
    }
  }
  std::cout << std::endl;
  std::cout << std::endl;

  // std::cout << "T1"<<T1.size() <<std::endl;
  //std::cout << "T2"<<T2.size() <<std::endl;
  // T1&T2 coincidence
  //
  std::cout << "All T1 times at T1 & T2 coincidences (within 13 time units):" << std::endl;
  for(int j=0; j<T1.size(); j++){
    for(int k=0; k<T2.size(); k++){
      float a = T2[k] - T1[j];
      if(0<a && a<13){
	tT1T2.push_back(T1[j]);
        std::cout << " " << T1[j];
      }
      // std::cout << "a" << a << std::endl;
    }
  }
  std::cout << std::endl;
  std::cout << std::endl;
  //T1T2-T0
  std::cout << "All (T1&T2 coincidence - T0R) intervals:" << std::endl;
  for(int j=0; j<tT1T2.size(); j++){
    for(int k=0; k<T0R.size(); k++){
      float a = tT1T2[j] - T0R[k];
      std::cout << " " << a;
    }
  }
  std::cout << std::endl;
  std::cout << std::endl;
  //T2-T1
  std::cout << "All T2-T1 intervals:" << std::endl;
  for(int j=0; j<T1.size(); j++){
    for(int k=0; k<T2.size(); k++){
      float a = T2[k] - T1[j];
      std::cout << " " << a;
    }
  } 
  std::cout << std::endl;
  std::cout << std::endl;
}

/////////////////////////////////////////////////////////////////////
// Main routine (This analyser begins from this part)
/////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
  TRint app("app",0,0);
  if(3 > argc){
    std::cerr << argv[0] << " <start event #> <input decoded (root) waveform file name> [another input files if you have...]" << std::endl;
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
  
  // Get the desired starting event number
  const Long64_t evtStart = atoll(argv[1]);

  //////////////////////////////////////////////////
  // Loop for events
  //////////////////////////////////////////////////
  for(Long64_t entry=evtStart; entry<idata.tree->GetEntries(); entry++){
    // Load input data for this event
    idata.tree->GetEntry(entry);

    // Print out event banner
    std::cout << "=======================================================================" << std::endl;
    std::cout << "=== EVENT: " << entry << std::endl;
    std::cout << "=======================================================================" << std::endl;
    std::cout << std::endl;

    // Process the event data
    ProcessEvent(idata);

    // Ask for next event or exit
    char buf[16];
    std::cout << "Next event (Y/N) ? ";
    std::cin.getline(buf, 12);
    if (buf[0] == 'N' || buf[0] == 'n')
      break;

    // Post event processing newlines
    std::cout << std::endl;
    std::cout << std::endl;
  }

  // Finish
  std::cout << std::endl;
  std::cout << "No more events." << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;
  return 0;
}
