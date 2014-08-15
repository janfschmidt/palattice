#ifndef __POLSIM__ELSASPUREN_HPP_
#define __POLSIM__ELSASPUREN_HPP_

#include <string>

#define NBPMS 32        // Number of BPMs for ELSA-Import (Spuren)
#define NVCORRS 32      // Number of VCs for ELSA-Import (Spuren)

using namespace std;

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

#endif

/*__POLSIM__ELSASPUREN_HPP_*/
