#include "gtest/gtest.h"
#include "../AccLattice.hpp"

#include <sstream>

class AccLatticeTest : public ::testing::Test {
public:
  pal::AccLattice lattice;

  AccLatticeTest() : lattice(60., pal::Anchor::begin)
  {
    double pos = 5.0;
    std::stringstream name;
    for(unsigned int num : {1,2,3,5,6,8,9,10,11}) {
      //dipole
      name.str(std::string());
      name << "M" << num;
      pal::Dipole d(name.str(), 2.5);
      lattice.mount(pos, d);
      //quadrupole
      name.str(std::string());
      if (num%2==0) name << "QF";
      else name << "QD";
      name << num;
      pal::Quadrupole q(name.str(), 0.5, pal::F, 0.42);
      lattice.mount(pos+3., q);
      pos += 5.;
    }
    pal::Sextupole s("QDX", 0.2);
    lattice.mount(pos, s);
  }

};

TEST_F(AccLatticeTest, setFamily) {
  EXPECT_EQ(9u, lattice.size(pal::quadrupole));
  EXPECT_EQ(9u, lattice.size(pal::quadrupole, pal::noplane, pal::F));
  
  lattice.setFamily(pal::D, "QD*");
  EXPECT_EQ(9u, lattice.size(pal::quadrupole));
  EXPECT_EQ(4u, lattice.size(pal::quadrupole, pal::noplane, pal::F));
  EXPECT_EQ(5u, lattice.size(pal::quadrupole, pal::noplane, pal::D));
  EXPECT_TRUE(lattice["QDX"].element()->family == pal::D);

  lattice.setFamily(pal::F, "QD*", pal::sextupole);
  EXPECT_TRUE(lattice["QDX"].element()->family == pal::F);
  EXPECT_EQ(4u, lattice.size(pal::quadrupole, pal::noplane, pal::F));
  EXPECT_EQ(5u, lattice.size(pal::quadrupole, pal::noplane, pal::D));
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

