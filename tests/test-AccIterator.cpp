#include "gtest/gtest.h"
#include "../AccLattice.hpp"
#include "../AccIterator.hpp" 

class AccIteratorTest : public ::testing::Test {
public:
  pal::AccLattice lattice;

  AccIteratorTest() : lattice(164.4)
  {
    double pos = 2.0;
    for(std::string num : {"1","2","3","4","5"}) {
      pal::Dipole d("M"+num, 2.5);
      pal::Quadrupole q("Q"+num, 0.5, pal::F, 0.42);
      lattice.mount(pos, d);
      lattice.mount(pos+3., q);
      pos += 5.;
    }
  }

};

TEST_F(AccIteratorTest, Iteration) {
  pal::AccLatticeIterator it = lattice.begin();
  ASSERT_DOUBLE_EQ(2.0, it.pos());
  ASSERT_STREQ("M1", it.element()->name.c_str());

  it++;
  ASSERT_DOUBLE_EQ(5.0, it.pos());
  ASSERT_STREQ("Q1", it.element()->name.c_str());
  
  it.next(); 
  ASSERT_DOUBLE_EQ(7.0, it.pos());
  ASSERT_STREQ("M2", it.element()->name.c_str());

  ++it; 
  ASSERT_DOUBLE_EQ(10.0, it.pos());
  ASSERT_STREQ("Q2", it.element()->name.c_str());

  it--;
  ASSERT_DOUBLE_EQ(7.0, it.pos());
  ASSERT_STREQ("M2", it.element()->name.c_str());
}

TEST_F(AccIteratorTest, TypeIteration) {
  auto it = lattice.begin();
  for (auto s : {"Q1","Q2","Q3","Q4"}) {
    it.next(pal::quadrupole);
    ASSERT_STREQ(s, it.element()->name.c_str());
  }
}

TEST_F(AccIteratorTest, ModifyElement) {
  auto it = lattice.begin();
  it.elementModifier()->name = "NEU";
  ASSERT_STREQ("NEU", it.element()->name.c_str());
}

TEST_F(AccIteratorTest, End) {
  auto it=lattice.begin();
  while(it!=lattice.end()) {
    ++it;
  }
  ASSERT_TRUE(it == lattice.end());
}

TEST_F(AccIteratorTest, Revolve) {
  auto it=lattice.begin();
  for(auto i=0; i<9; i++)
    ++it;
  EXPECT_STREQ("Q5", it.element()->name.c_str());
  it.revolve();
  EXPECT_STREQ("M1", it.element()->name.c_str());
  it.revolve();
  EXPECT_STREQ("Q1", it.element()->name.c_str());
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
