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
  FREQCOMP *b;

public:
  const unsigned int fmax;   //maximum frequency (fmax is highest included frequency (**))
  const unsigned int turns;  //number of turns in ring

  SPECTRUM(unsigned int f=0, unsigned int t=1) : fmax(f), turns(t) {b = new FREQCOMP[this->size()];}
  ~SPECTRUM() {delete[] b;}
  SPECTRUM(const SPECTRUM &other);
  double freq(unsigned int i) const {return b[i].freq;}
  double amp(unsigned int i) const {return b[i].amp;}
  double phase(unsigned int i) const {return b[i].phase;}
  unsigned int size() const {return (fmax+1)*turns;}      // +1 explained by (**)
  int set(unsigned int i, FREQCOMP tmp);

};

#endif

/*__BSUPPLY__SPECTRUM_HPP_*/
