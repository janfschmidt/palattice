#include "gtest/gtest.h"
#include "../AccLattice.hpp"

#include <iterator>

class AccIteratorTest : public ::testing::Test {
public:
  pal::AccLattice lattice;
  pal::AccLattice::iterator it;

  AccIteratorTest() : lattice(30.), it(lattice.begin())
  {
    double pos = 2.0;
    for(std::string num : {"1","2","3","4","5"}) {
      pal::Dipole d("M"+num, 2.5);
      pal::Quadrupole q("Q"+num, 0.5, pal::F, 0.42);
      lattice.mount(pos, d);
      lattice.mount(pos+3., q);
      pos += 5.;
    }
    it = lattice.begin();
  }

};

TEST_F(AccIteratorTest, Initialization) {
  ASSERT_EQ(10u, lattice.size());
  ASSERT_EQ(5u, lattice.size(pal::dipole));
  ASSERT_EQ(5u, lattice.size(pal::quadrupole));
}

TEST_F(AccIteratorTest, Iteration) {
  ASSERT_DOUBLE_EQ(2.0, it.pos());
  ASSERT_STREQ("M1", it.element()->name.c_str());
  ASSERT_STREQ("M1", (*it)->name.c_str());

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
  for (auto s : {"Q1","Q2","Q3","Q4","Q5"}) {
    it.next(pal::quadrupole);
    EXPECT_STREQ(s, it.element()->name.c_str());
  }
  ++it;
  ASSERT_TRUE(it.isEnd());
}

TEST_F(AccIteratorTest, PlaneIteration) {
  ++it; ++it;
  it.element()->plane = pal::V;
  it = lattice.begin();
  it.next(pal::dipole, pal::V);
  ASSERT_STREQ("M2", it.element()->name.c_str());
  it.next(pal::dipole, pal::V);
  ASSERT_TRUE(it.isEnd());
}

TEST_F(AccIteratorTest, Loop) {
  std::vector<std::string> list;
  for (auto it=lattice.begin(); it!=lattice.end(); ++it) {
    list.push_back(it.element()->name);
  }
  EXPECT_STREQ("M1", list[0].c_str());
  EXPECT_STREQ("Q1", list[1].c_str());
  EXPECT_STREQ("M2", list[2].c_str());
  EXPECT_STREQ("M5", list[8].c_str());
  EXPECT_STREQ("Q5", list[9].c_str());
}

TEST_F(AccIteratorTest, RangeLoop) {
  std::vector<std::string> list;
  for (auto ele : lattice) {
    list.push_back(ele->name);
  }
  EXPECT_STREQ("M1", list[0].c_str());
  EXPECT_STREQ("Q1", list[1].c_str());
  EXPECT_STREQ("M2", list[2].c_str());
  EXPECT_STREQ("M5", list[8].c_str());
  EXPECT_STREQ("Q5", list[9].c_str());
}

TEST_F(AccIteratorTest, TypeLoop) {
  std::vector<std::string> list;
  // here it is AccLattice::type_iterator<pal::quadrupole>
  for (auto it=lattice.begin<pal::quadrupole>(); it!=lattice.end(); ++it) {
    list.push_back(it.element()->name);
  }
  EXPECT_EQ(5u, list.size());
  EXPECT_STREQ("Q1", list[0].c_str());
  EXPECT_STREQ("Q5", list[4].c_str());

  pal::AccLattice::type_iterator<pal::quadrupole> it2 = lattice.begin<pal::quadrupole>();
  EXPECT_STREQ("Q1", it2.element()->name.c_str());
  it2++;
  EXPECT_STREQ("Q2", it2.element()->name.c_str());
  it2.next();
  EXPECT_STREQ("Q3", it2.element()->name.c_str());
  
  pal::AccLattice::iterator it3 = lattice.begin(pal::quadrupole);
  EXPECT_STREQ("Q1", it3.element()->name.c_str());
  it3.next(pal::quadrupole);
  EXPECT_STREQ("Q2", it3.element()->name.c_str());
  it3.next();
  EXPECT_STREQ("M3", it3.element()->name.c_str());
}

TEST_F(AccIteratorTest, PlaneLoop) {
  ++it; ++it;
  it.element()->plane = pal::V;
  std::vector<std::string> list;
  // here it is AccTypeIterator<pal::dipole,pal::V>
  for (auto it=lattice.begin<pal::dipole,pal::V>(); it!=lattice.end(); ++it) {
    list.push_back(it.element()->name);
  }
  EXPECT_EQ(1u, list.size());
  EXPECT_STREQ("M2", list[0].c_str());
}


TEST_F(AccIteratorTest, ModifyElement) {
  it.element()->name = "NEU";
  ASSERT_STREQ("NEU", it.element()->name.c_str());
}

TEST_F(AccIteratorTest, End) {
  while(it!=lattice.end()) {
    ++it;
  }
  ASSERT_TRUE(it == lattice.end());
  ASSERT_TRUE(it.isEnd());
  EXPECT_THROW(it.pos(), pal::palatticeError);
  EXPECT_THROW(it.element(), pal::palatticeError);
  EXPECT_THROW(it.pos(pal::Anchor::begin), pal::palatticeError);
  EXPECT_THROW(it.distance(pal::Anchor::begin, 0.), pal::palatticeError);

}

TEST_F(AccIteratorTest, Revolve) {
  for(auto i=0; i<9; i++)
    ++it;
  EXPECT_STREQ("Q5", it.element()->name.c_str());
  it.revolve();
  EXPECT_STREQ("M1", it.element()->name.c_str());
  it.revolve();
  EXPECT_STREQ("Q1", it.element()->name.c_str());

  auto it2 = lattice.begin<pal::quadrupole>();
  EXPECT_STREQ("Q1", it2.element()->name.c_str());
  it2.revolve();  it2.revolve();
  EXPECT_STREQ("Q3", it2.element()->name.c_str());
  it2.revolve();  it2.revolve(); it2.revolve();
  EXPECT_STREQ("Q1", it2.element()->name.c_str());
}

TEST_F(AccIteratorTest, RevolveAndModify) {
  for(auto i=0; i<9; i++)
    ++it;
  EXPECT_STREQ("Q5", it.element()->name.c_str());
  // now add a new magnet after it was created
  lattice.mount(100., pal::Dipole("M6",2.5));
  it.revolve();
  EXPECT_STREQ("M6", it.element()->name.c_str());
  it.revolve();
  EXPECT_STREQ("M1", it.element()->name.c_str());
}

TEST_F(AccIteratorTest, PosAnchor) {
  it++;
  EXPECT_DOUBLE_EQ(5.0, it.pos());
  EXPECT_DOUBLE_EQ(5.0, it.pos(pal::Anchor::begin));
  EXPECT_DOUBLE_EQ(5.25, it.pos(pal::Anchor::center));
  EXPECT_DOUBLE_EQ(5.5, it.pos(pal::Anchor::end));
}

TEST_F(AccIteratorTest, At) {
  EXPECT_TRUE(it.at(2.0));
  EXPECT_TRUE(it.at(4.5));
  EXPECT_TRUE(it.at(3.129724));
  EXPECT_FALSE(it.at(1.999));
  EXPECT_FALSE(it.at(5.));
  ++it; ++it;
  EXPECT_TRUE(it.at(7.0));
  EXPECT_TRUE(it.at(9.4));
  EXPECT_FALSE(it.at(10.0));
}

TEST_F(AccIteratorTest, Distance) {
  EXPECT_NEAR(-1.1, it.distance(pal::Anchor::begin, 0.9), 0.001);
  ++it;
  EXPECT_NEAR(0.05, it.distance(pal::Anchor::center, 5.3), 0.001);
}

TEST_F(AccIteratorTest, DistanceRing) {
  EXPECT_NEAR(1.4, it.distanceRing(pal::Anchor::begin, 3.4), 0.001);
  EXPECT_NEAR(-2.5, it.distanceRing(pal::Anchor::begin, 29.5), 0.001);
}

TEST_F(AccIteratorTest, DistanceNext) {
  EXPECT_NEAR(3., it.distanceNext(pal::Anchor::begin), 0.001);
  for(auto i=0; i<9; i++)
    ++it;  
  EXPECT_NEAR(7., it.distanceNext(pal::Anchor::begin), 0.001);
}


TEST_F(AccIteratorTest, ConvertIterators) {
  pal::AccLattice::const_iterator cit = it;
  EXPECT_STREQ("M1", cit.element()->name.c_str());
  ++cit;
  EXPECT_STREQ("Q1", cit.element()->name.c_str());
  ++cit;
  EXPECT_STREQ("M2", cit.element()->name.c_str());

  pal::AccLattice::type_iterator<pal::quadrupole> qit = it;
  EXPECT_STREQ("M1", qit.element()->name.c_str());
  ++qit;
  EXPECT_STREQ("Q1", qit.element()->name.c_str());
  ++qit;
  EXPECT_STREQ("Q2", qit.element()->name.c_str());

  pal::AccLattice::const_type_iterator<pal::quadrupole> cqit = cit;
  EXPECT_STREQ("M2", cqit.element()->name.c_str());
  ++cqit;
  EXPECT_STREQ("Q2", cqit.element()->name.c_str());
  ++cqit;
  EXPECT_STREQ("Q3", cqit.element()->name.c_str());

  ++cit;
  EXPECT_TRUE(it==it);
  EXPECT_FALSE(it==cit);
  EXPECT_FALSE(it==qit);
  EXPECT_FALSE(it==cqit);

  EXPECT_FALSE(cit==it);
  EXPECT_TRUE(cit==cit);
  EXPECT_TRUE(cit==qit);
  EXPECT_FALSE(cit==cqit);

  EXPECT_FALSE(qit==it);
  EXPECT_TRUE(qit==cit);
  EXPECT_TRUE(qit==qit);
  EXPECT_FALSE(qit==cqit);

  EXPECT_FALSE(cqit==it);
  EXPECT_FALSE(cqit==cit);
  EXPECT_FALSE(cqit==qit);
  EXPECT_TRUE(cqit==cqit);
}

TEST_F(AccIteratorTest, LatticeByPosition) {
  EXPECT_STREQ("M1", lattice[2.0]->name.c_str());
  EXPECT_STREQ("Q1", lattice[5.0]->name.c_str());
  EXPECT_STREQ("Q1", lattice[5.3]->name.c_str());
  EXPECT_STREQ("Q1", lattice[5.5]->name.c_str());
  EXPECT_STREQ("drift", lattice[5.6]->name.c_str());
  EXPECT_STREQ("M2", lattice[7.0]->name.c_str());
  EXPECT_STREQ("Q2", lattice[10.1]->name.c_str());
  EXPECT_STREQ("Q5", lattice.at(25.47).element()->name.c_str());

  EXPECT_STREQ("M1", lattice.at(2.0).element()->name.c_str());
  EXPECT_STREQ("Q1", lattice.at(5.0).element()->name.c_str());
  EXPECT_STREQ("Q1", lattice.at(5.3).element()->name.c_str());
  EXPECT_STREQ("Q1", lattice.at(5.5).element()->name.c_str());
  EXPECT_THROW(lattice.at(5.6).element()->name.c_str(), pal::AccLattice::noMatchingElement);
  EXPECT_STREQ("M2", lattice.at(7.0).element()->name.c_str());
  EXPECT_STREQ("Q5", lattice.at(25.3).element()->name.c_str());
}

TEST_F(AccIteratorTest, LatticeBehind) {
  EXPECT_STREQ("M1", lattice.behind(2.0,pal::Anchor::center).element()->name.c_str());
  EXPECT_STREQ("Q1", lattice.behind(5.3,pal::Anchor::end).element()->name.c_str());
  EXPECT_STREQ("Q1", lattice.behind(4.9,pal::Anchor::begin).element()->name.c_str());
  EXPECT_STREQ("Q1", lattice.behind(5.249,pal::Anchor::center).element()->name.c_str());
  EXPECT_STREQ("M2", lattice.behind(5.25,pal::Anchor::center).element()->name.c_str());
  EXPECT_STREQ("M2", lattice.behind(5.5,pal::Anchor::end).element()->name.c_str());
  EXPECT_STREQ("M2", lattice.behind(6.0,pal::Anchor::begin).element()->name.c_str());
  EXPECT_STREQ("M2", lattice.behind(6.0,pal::Anchor::center).element()->name.c_str());
  EXPECT_STREQ("M2", lattice.behind(6.0,pal::Anchor::end).element()->name.c_str());
  EXPECT_STREQ("Q5", lattice.behind(25.0,pal::Anchor::end).element()->name.c_str());
}

TEST_F(AccIteratorTest, stdIteratorFunctions) {
  auto it2 = it;
  std::advance(it2, 3);
  EXPECT_STREQ("Q2", it2.element()->name.c_str());
  
  EXPECT_EQ(3u, std::distance(it, it2));

  auto it3 = std::next(it, 6);
  EXPECT_STREQ("M1", it.element()->name.c_str());
  EXPECT_STREQ("Q2", it2.element()->name.c_str());
  EXPECT_STREQ("M4", it3.element()->name.c_str());
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
