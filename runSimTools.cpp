#include "runSimTools.hpp"
#include "simToolPath.hpp"
#include "config.hpp"

using namespace std;

string pal::runMadX(string latticeFile)
{
  string madxFile = "libpal.madx";
  string inoutPath;
  stringstream cmd;
  string log = "madx.log";

  // latticeFile path:
  unsigned int f = latticeFile.find_last_of("/");
  if (f != string::npos)
    inoutPath = latticeFile.substr(0,f+1);
  else
    inoutPath = "./";

  // copy madx file (if not existing)
  cmd << "cp -n "<< pal::simToolPath() << "/" << madxFile << " " << inoutPath;
  system(cmd.str().c_str());

  // set lattice filename in madxFile:
  cmd.str(std::string());
  cmd << "cd "<< inoutPath << "; "<<
    "sed -i 's!^call, file=.*!call, file=\"" << latticeFile << "\";!' " << madxFile;
  system(cmd.str().c_str());
  
  //run madx:
  stringstream runcmd;
  runcmd << "cd "<< inoutPath << "; "<< MADXCOMMAND << " < " << madxFile << " >> " << log;
  cout << "Run MadX... (log: " <<inoutPath << log << ")" << endl;
  system(runcmd.str().c_str());

  //write BPMs as observation points to madx.observe
  cmd.str(std::string());
  cmd << "cd "<< inoutPath << "; "
      << "grep BPM " << "madx.twiss | awk '{gsub(\"\\\"\",\"\",$2); print \"ptc_observe, place=\"$2\";\"}' > " 
      << "madx.observe";
  system(cmd.str().c_str());

  // 2. madx run -> using madx.observe
  cout << "Run MadX... (log: " <<inoutPath << log << ")" << endl;
  system(runcmd.str().c_str());

  return inoutPath+"madx.twiss";
}


void pal::runElegant(string latticeFile)
{

}


void pal::runSimTool(simulationTool t, string file)
{
  if (t == madx)
    runMadX(file);
  else //elegant
    runElegant(file);
}
