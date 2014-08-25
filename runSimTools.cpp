#include "runSimTools.hpp"
#include "simToolPath.hpp"
#include "config.hpp"

using namespace std;

string pal::runMadX(string l) {return pal::runSimTool(madx,l);}
string pal::runElegant(string l) {return pal::runSimTool(elegant,l);}

string pal::runSimTool(simulationTool t, string latticeFile)
{
  string simToolFile;
  string log;
  if (t == madx) {
    simToolFile = "libpal.madx";
    log = "madx.log";
  }
  else {
    simToolFile = "libpal.ele";
    log = "elegant.log";
  }

  string inoutPath;
  stringstream cmd;

  // latticeFile path:
  string::size_type f = latticeFile.find_last_of("/");
  if (f != string::npos) {
    inoutPath = latticeFile.substr(0,f+1);
    cout << latticeFile<< " f=" << f <<  " npos=" <<string::npos << endl;
  }
  else
    inoutPath = "./";

  // copy madx/elegant file (if not existing)
  cmd << "cp -n "<< pal::simToolPath() << "/" << simToolFile << " " << inoutPath;
  system(cmd.str().c_str());

  // set lattice filename in simToolFile:
  cmd.str(std::string());
  cmd << "cd "<< inoutPath << "; ";
  if (t == madx) {
    cmd << "sed -i 's!^call, file=\".*\"!call, file=\"";
  }
  else {
    cmd << "sed -i 's!lattice = \".*\"!lattice = \"";
  }
  cmd << latticeFile << "\"!' " << simToolFile;
  system(cmd.str().c_str());
  
  //run madx/elegant:
  stringstream runcmd;
  runcmd << "cd "<< inoutPath << "; ";
  if (t == madx) {
    runcmd << MADXCOMMAND << " < "; 
    cout << "Run MadX...";
  }
  else {
    runcmd << ELEGANTCOMMAND << " ";
    cout << "Run Elegant...";
  }
  runcmd << simToolFile << " >> " << log;
  cout << " (log: " <<inoutPath << log << ")" << endl;
  system(runcmd.str().c_str());

  if (t == madx) {
    //write BPMs as observation points to madx.observe
    cmd.str(std::string());
    cmd << "cd "<< inoutPath << "; "
	<< "grep BPM " << "madx.twiss | awk '{gsub(\"\\\"\",\"\",$2); print \"ptc_observe, place=\"$2\";\"}' > " 
	<< "madx.observe";
    system(cmd.str().c_str());
    
    // 2. madx run -> using madx.observe
    cout << "Run MadX (now can open madx.observe)... (log: " <<inoutPath << log << ")" << endl;
    system(runcmd.str().c_str());
  }

  if (t == madx)  return inoutPath+"madx.twiss";
  else  return inoutPath+"elegant.param";
}
