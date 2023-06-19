#include <iostream>
#include <cstdlib>

#include <TFile.h>
#include <TChain.h>
#include <TTree.h>
#include <TH1.h>
#include <TRint.h>
#include <TH2.h>

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
// Output data structure
/////////////////////////////////////////////////////////////////////
struct OutputData_t
{
  TFile* file;

  // Distribution of the area of input waveforms
  TH1*   hArea[4];
  TH1*   hAreaZoom[4];
  TH1*   hHeight[4];
  TH2*   hWaveform[4];
  TH1*   hThretime[4];
  TH1*   T1_T0[1];
  TH1*   T1T2[1];
  TH1*   T1T2_T0[1];
  TH1*   T2_T1[1];
  TH2*   T1vsT2[1];
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


// Sub: Setup the output data
void SetupOutput(OutputData_t& odata)
{
  odata.file->cd();
  
  // Define histograms (TH1D) (name, title, number of bins, min, max)
  odata.hArea[0] = new TH1D("hArea_A", "Area of ch.A;Charge (pC)", 2000, -5, 1995);
  odata.hArea[1] = new TH1D("hArea_B", "Area of ch.B;Charge (pC)", 2000, -5, 1995);
  odata.hArea[2] = new TH1D("hArea_C", "Area of ch.C;Charge (pC)", 2000, -50, 1950);
  odata.hArea[3] = new TH1D("hArea_D", "Area of ch.D;Charge (pC)", 2000, -50, 1950);

  odata.hAreaZoom[0] = new TH1D("hAreaZoom_A", "Area of ch.A;Charge (pC)", 250, -12.5, 50);
  odata.hAreaZoom[1] = new TH1D("hAreaZoom_B", "Area of ch.B;Charge (pC)", 250, -12.5, 50);
  odata.hAreaZoom[2] = new TH1D("hAreaZoom_C", "Area of ch.C;Charge (pC)", 250, -12.5, 50);
  odata.hAreaZoom[3] = new TH1D("hAreaZoom_D", "Area of ch.D;Charge (pC)", 250, -12.5, 50);

  odata.hHeight[0] = new TH1D("hHeight_A", "Voltage of ch.A;Count", 500, -50, 950);
  odata.hHeight[1] = new TH1D("hHeight_B", "Voltage of ch.B;Count", 500, -50, 950);
  odata.hHeight[2] = new TH1D("hHeight_C", "Voltage of ch.C;Count", 500, -50, 950);
  odata.hHeight[3] = new TH1D("hHeight_D", "Voltage of ch.D;Count", 500, -50, 950);

  odata.hWaveform[0] = new TH2D("hWaveform_A", "Piled Waveform of ch.A;Charge (pC)", 10000, -5, 9995, 500, -950, 50);
  odata.hWaveform[1] = new TH2D("hWaveform_B", "Piled Waveform of ch.A;Charge (pC)", 10000, -5, 9995, 500, -950, 50);
  odata.hWaveform[2] = new TH2D("hWaveform_C", "Piled Waveform of ch.A;Charge (pC)", 10000, -5, 9995, 500, -950, 50);
  odata.hWaveform[3] = new TH2D("hWaveform_D", "Piled Waveform of ch.A;Charge (pC)", 10000, -5, 9995, 500, -950, 50);


  odata.hThretime[0] = new TH1D("hThretime_A", "T0R;Count", 10000, -10, 9990);
  odata.hThretime[1] = new TH1D("hThretime_B", "T0L;Count", 10000, -10, 9990);
  odata.hThretime[2] = new TH1D("hThretime_C", "T1;Count", 10000, -10, 9990);
  odata.hThretime[3] = new TH1D("hThretime_D", "T2;Count", 10000, -10, 9990);
 

  odata.T1_T0[0] = new TH1D("T1_T0", "T1-T0;Time", 10000, -10, 9990);
 
  odata.T1T2[0] = new TH1D("T1T2", "T1T2;Time", 10000, -10, 9990);

  odata.T1T2_T0[0] = new TH1D("T1T2_T0", "T1T2-T0;Time", 10000, -10, 9990);
 
  odata.T2_T1[0] = new TH1D("T2_T1", "T2-T1;Time", 10000, -5000, 5000);

  odata.T1vsT2[0] = new TH2D("T1vsT2", "T2;T1", 10000, -10, 9990, 10000, -10, 9990);
}

int times = 0, start, end;

// Sub: Process an event data

void ProcessEvent(InputData_t& idata, OutputData_t& odata)
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
    odata.hHeight[ch]->Fill(-min);

    // Threshold
    size_t i=0;
    if(ch==0){
      Float_t thre = -40;
      for(i=0; i<wf.size()-1; i++){
	if(thre < wf[i] && wf[i+1] < thre){
	  odata.hThretime[ch]->Fill(i+1);
	  T0R.push_back(i+1);
	}
      }     
    }    

    if(ch==1){
      Float_t thre = -40;
     for(i=0; i<wf.size()-1; i++){
	if(thre < wf[i] && wf[i+1] < thre){
	  odata.hThretime[ch]->Fill(i+1);
	  T0L.push_back(i+1);
	}
      }
    }
    
    if(ch==2){
      Float_t thre = -40;
      for(i=0; i<wf.size()-1; i++){
	if(thre < wf[i] && wf[i+1] < thre){
	  odata.hThretime[ch]->Fill(i+1);
	  T1.push_back(i+1);
	  //	  std::cout << ch << ":" << T1.back() <<std::endl;

	}
      } 
    }

    if(ch==3){
      Float_t thre = -20;
      for(i=0; i<wf.size()-1; i++){
	if(thre < wf[i] && wf[i+1] < thre){
	  odata.hThretime[ch]->Fill(i+1);
	  T2.push_back(i+1);
	  //	   std::cout << ch << ":" << T2.back() <<std::endl;
	}
      }
    }
    
    ///////////////////////////////
    //Make waveforms
    //std::cout << "size of wf" << wf.size() << std::endl;
    for(size_t i=0; i<wf.size()-1; i++){
      odata.hWaveform[ch]->Fill(i, wf[i]);
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
    // Fill to the histogram
    odata.hArea[ch]->Fill(factor * area);
    odata.hAreaZoom[ch]->Fill(factor * area);
  }
  
  for(int j=0; j<T1.size(); j++){
    for(int k=0; k<T0R.size(); k++){
      float a = T1[j] - T0R[k];
      odata.T1_T0[0]->Fill(a);
      //	std::cout << "a" << a << std::endl;
    }
  }
  // std::cout << "T1"<<T1.size() <<std::endl;
  //std::cout << "T2"<<T2.size() <<std::endl;
  // T1&T2 coincidence
  //
  for(int j=0; j<T1.size(); j++){
    for(int k=0; k<T2.size(); k++){
      float a = T2[k] - T1[j];
      if(0<a && a<13){
	odata.T1T2[0]->Fill(T1[j]);
	tT1T2.push_back(T1[j]);
      }
      // std::cout << "a" << a << std::endl;
    }
  }
  //T1T2-T0
  for(int j=0; j<tT1T2.size(); j++){
    for(int k=0; k<T0R.size(); k++){
      float a = tT1T2[j] - T0R[k];
      odata.T1T2_T0[0]->Fill(a);
    }
  }
  //T2-T1
  for(int j=0; j<T1.size(); j++){
    for(int k=0; k<T2.size(); k++){
      float a = T2[k] - T1[j];
      odata.T2_T1[0]->Fill(a);
    }
  } 
  //T1vsT2
  for(int j=0; j<T1.size(); j++){
    for(int k=0; k<T2.size(); k++){
      odata.T1vsT2[0]->Fill(T1[j],T2[k]);
    }
  }
}

/////////////////////////////////////////////////////////////////////
// Main routine (This analyser begins from this part)
/////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
  TRint app("app",0,0);
  if(3 > argc){
    std::cerr << argv[0] << " [output file name] [input decoded (root) waveform file name] (another input files if you have...)" << std::endl;
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
  
  // Open the output file
  OutputData_t odata;
  // This won't overwrite existing files, so you have to delete them in advance
  odata.file = TFile::Open(argv[1], "create");
  // If the file hasn't opened correctly, ofile has a null pointer (NULL)
  if(NULL == odata.file){ 
    std::cerr << "The output file cannot be opened newly with " << argv[1] << std::endl
	      << "  (This analyser won't overwrite already existing files.)" << std::endl;
    return 3;
  }

  // Setup output data
  SetupOutput(odata);

  //////////////////////////////////////////////////
  // Loop for events
  //////////////////////////////////////////////////
  for(Long64_t entry=0; entry<idata.tree->GetEntries(); entry++){
    // Load input data for this event
    idata.tree->GetEntry(entry);
    // Process the event data
    ProcessEvent(idata, odata);
  }
  odata.file->cd();

  // Close the output file
  odata.file->Write();
  odata.file->Close();
  return 0;
}
