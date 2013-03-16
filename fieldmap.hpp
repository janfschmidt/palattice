#ifndef __BSUPPLY__FIELDMAP_HPP_
#define __BSUPPLY__FIELDMAP_HPP_

using namespace std;

class FIELD {
public:
  double pos;   //position along ring in meter
  unsigned int turn;
  string name;
  double x;     //[B.x]=1/m (missing factor gamma*m*c/e)
  double z;     //[B.z]=1/m (missing factor gamma*m*c/e)
  double theta; //spin phaseadvance in degree

  FIELD() : pos(0), turn(1), x(0), z(0), theta(0) {}
};



class FIELDMAP {

private:
  FIELD *B;

public:
  const unsigned int n_samp; //number of sampling points along ring
  const unsigned int n_turns;  //number of turns in ring
  const double circumference;   //circumference einlesen!!!!!!!!!!!!!!!!!!!!!!!!
  
  FIELDMAP(unsigned int n=1644, unsigned int t=1) : n_samp(n), n_turns(t), circumference(164.4) {B = new FIELD[this->size()];}
  ~FIELDMAP() {delete[] B;}
  FIELDMAP(const FIELDMAP &other);
  double pos(unsigned int i) const {return B[i].pos;}
  unsigned int turn(unsigned int i) const {return B[i].turn;}
  string name(unsigned int i) const {return B[i].name;}
  double x(unsigned int i) const {return B[i].x;}
  double z(unsigned int i) const {return B[i].z;}
  double theta(unsigned int i) const {return B[i].theta;}
  unsigned int size() const {return n_samp*n_turns;}
  double pos_tot(unsigned int i) const {return pos(i) + (turn(i)-1)*circumference;}
  int set(unsigned int i, FIELD tmp);
  //int out(const char *filename) const;

};

#endif

/*__BSUPPLY__FIELDMAP_HPP_*/
