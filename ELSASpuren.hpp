#ifndef __LIBPALATTICE__ELSASPUREN_HPP_
#define __LIBPALATTICE__ELSASPUREN_HPP_

#include <string>

#define NBPMS 32        // Number of BPMs for ELSA-Import (Spuren)
#define NVCORRS 32      // Number of VCs for ELSA-Import (Spuren)

using namespace std;

namespace pal
{

// ELSA: Closed Orbit
class BPM_MS {
public:
  int ms;
  double x;
  double z;

  BPM_MS() : ms(0), x(0), z(0) {}
};

class BPM {
public:
  std::vector<BPM_MS> time;
  double pos;

  BPM() : pos(0) {}
};

// ELSA Corrector-Kicks
class CORR_MS {
public:
  int ms;
  double kick;

  CORR_MS() : ms(0), kick(0) {}
};

class CORR {
public:
  std::vector<CORR_MS> time;
  double pos;

  CORR() : pos(0) {}
};




class ELSASpuren {
public:
  BPM bpms[NBPMS];         // bpms[0]=BPM01, ELSAbpms[31]=BPM32
  CORR vcorrs[NVCORRS];    // vcorrs[0]=VC01, ELSAvcorrs[31]=VC32
  string spurenFolder;

  ELSASpuren() {}
  ELSASpuren(string _spurenFolder);
  ~ELSASpuren() {}

  void read(string _spurenFolder);
  void import_bpms();
  void import_vcorrs();

};

} //namespace pal

#endif
/*__LIBPALATTICE__ELSASPUREN_HPP_*/
