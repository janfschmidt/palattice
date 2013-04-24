#ifndef __BSUPPLY__SPECTRUM_HPP_
#define __BSUPPLY__SPECTRUM_HPP_

using namespace std;


class FREQCOMP {
public:
  double freq;
  double amp;
  double phase;

  FREQCOMP() : freq(0), amp(0), phase(0) {}
};


class SPECTRUM {

private:
  vector<FREQCOMP> b;

public:
  const unsigned int fmax_rev;   //maximum frequency in rev. harmonics (fmax is highest INCLUDED frequency (**))
  const unsigned int turns;  //number of turns in ring
  const double ampcut; //amlitudes below this are ignored

  SPECTRUM(unsigned int f=0, unsigned int t=1, double a=0) : fmax_rev(f), turns(t), ampcut(a) {}
  ~SPECTRUM() {}
  //SPECTRUM(const SPECTRUM &other);
  double freq(unsigned int i) const {return b[i].freq;}
  double amp(unsigned int i) const {return b[i].amp;}
  double phase(unsigned int i) const {return b[i].phase;}
  unsigned int size() const {return b.size();}         //actual filled values
  unsigned int fmax() const {return fmax_rev*turns;}   //expected/set maximum
  void push_back(FREQCOMP tmp);
  //int evalout(const char *filename) const;

};

#endif

/*__BSUPPLY__SPECTRUM_HPP_*/
