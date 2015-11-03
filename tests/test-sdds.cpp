#include "gtest/gtest.h"
#include "../FunctionOfPos.hpp"
#include "SDDS.h"




TEST(sdds, Parameter) {
  SDDS_TABLE *t = new SDDS_TABLE;
  if( SDDS_InitializeInput(t,const_cast<char*>("libpalattice.twi")) != 1 )
    throw sddsi::SDDSFailure();
  
  SDDS_ReadTable(t);
  void *mem = SDDS_GetParameter(t, const_cast<char*>("pCentral"), NULL);
  if (mem == NULL)
    throw sddsi::SDDSFailure();
  
  double p = *static_cast<double *>(mem);
  EXPECT_NEAR(4.500987e+03, p, 0.001);
  
  free(mem);
  delete t;
}

TEST(sdds, StringParameter) {
  SDDS_TABLE *t = new SDDS_TABLE;
  if( SDDS_InitializeInput(t,const_cast<char*>("libpalattice002.w")) != 1 )
    throw sddsi::SDDSFailure();
  
  SDDS_ReadTable(t);
  void *mem = SDDS_GetParameter(t, const_cast<char*>("PreviousElementName"), NULL);
  if (mem == NULL)
    throw sddsi::SDDSFailure();

  std::string s(*static_cast<char **>(mem));
  EXPECT_STREQ("BPM02", s.c_str());

  free(mem);
  delete t;
}

TEST(sdds, Column) {
  SDDS_TABLE *t = new SDDS_TABLE;
  if( SDDS_InitializeInput(t,const_cast<char*>("libpalattice.twi")) != 1 )
    throw sddsi::SDDSFailure();
  
  SDDS_ReadTable(t);
  void *mem = SDDS_GetColumn(t, const_cast<char*>("s"));
  if (mem == NULL)
    throw sddsi::SDDSFailure();

  double *array = static_cast<double *>(mem);
  unsigned int length=SDDS_CountRowsOfInterest(t); 
  std::vector<double> x(array, array+length);

  // for (auto &val : x) {
  //   std::cout << val << std::endl;
  // }
  
  EXPECT_EQ(333u, x.size());
  EXPECT_DOUBLE_EQ(0.0, x[0]);
  EXPECT_NEAR(164.4008, x[332],0.0001);

  free(mem);
  delete t;
}

TEST(sdds, ColumnValues) {
 SDDS_TABLE *t = new SDDS_TABLE;
  if( SDDS_InitializeInput(t,const_cast<char*>("libpalattice.twi")) != 1 )
    throw sddsi::SDDSFailure();

  SDDS_ReadTable(t);
  void *mem;
  double s;
  std::vector<double> x;
  unsigned int length=SDDS_CountRowsOfInterest(t);
  for (auto i=0u; i<length; i++) {
    mem = SDDS_GetValue(t, const_cast<char*>("s"), i, NULL);
    if (mem == NULL) throw sddsi::SDDSFailure();
    s = *static_cast<double *>(mem);
    x.push_back(std::move(*static_cast<double *>(mem)));
  }
  
  EXPECT_NEAR(164.4008, s, 0.0001);
  EXPECT_EQ(333u, x.size());
  EXPECT_DOUBLE_EQ(0.0, x[0]);
  EXPECT_NEAR(164.4008, x[332],0.0001);

  free(mem);
  delete t;
}

TEST(sdds, Pages) {
  SDDS_TABLE *t = new SDDS_TABLE;
  if( SDDS_InitializeInput(t,const_cast<char*>("libpalattice002.w")) != 1 )
    throw sddsi::SDDSFailure();

  // SDDS_ReadTable gives first page, has to be repeated for every page.
  // returns -1 if no more pages to read.

  int status = SDDS_ReadTable(t);
  unsigned int i=0;
  while (status != -1) {
    void *mem = SDDS_GetColumn(t, const_cast<char*>("p"));
    if (mem == NULL)
      throw sddsi::SDDSFailure();
    
    double *array = static_cast<double *>(mem);
    unsigned int length=SDDS_CountRowsOfInterest(t); 
    std::vector<double> x(array, array+length);
    EXPECT_EQ(100u, x.size());

    void *parmem = SDDS_GetParameter(t, const_cast<char*>("Pass"), NULL);
    if (parmem == NULL) throw sddsi::SDDSFailure();
    unsigned int turn = *static_cast<unsigned int *>(parmem);
    EXPECT_EQ(i, turn);

    free(parmem);
    i++;
    status = SDDS_ReadTable(t);
  }

  delete t;
}

TEST(sdds, FilterParticleId) {
  SDDS_TABLE *t = new SDDS_TABLE;
  if( SDDS_InitializeInput(t,const_cast<char*>("libpalattice002.w")) != 1 )
    throw sddsi::SDDSFailure();

  // filter single particle id
  // by SDDS_FilterRowsOfInterest AFTER(!) EACH(!) SDDS_ReadTable call
  // Default are all Rows => use Logic Macro SDDS_AND to get only the filtered Rows
  unsigned int id=2u;

  int status = SDDS_ReadTable(t);
  int ret = SDDS_FilterRowsOfInterest(t,const_cast<char*>("particleID"),id,id,SDDS_AND);
  if (ret == -1) throw sddsi::SDDSFailure();
  ASSERT_EQ(1,ret);

  unsigned int i=0;
  while (status != -1) {
    void *mem = SDDS_GetColumn(t, const_cast<char*>("p"));
    if (mem == NULL)
      throw sddsi::SDDSFailure();
    
    double *array = static_cast<double *>(mem);
    unsigned int length=SDDS_CountRowsOfInterest(t); 
    std::vector<double> x(array, array+length);
    EXPECT_EQ(1u, length);
    EXPECT_EQ(1u, x.size());

    void *parmem = SDDS_GetParameter(t, const_cast<char*>("Pass"), NULL);
    if (parmem == NULL) throw sddsi::SDDSFailure();
    unsigned int turn = *static_cast<unsigned int *>(parmem);
    EXPECT_EQ(i, turn);

    free(parmem);
    i++;
    status = SDDS_ReadTable(t);
    int ret = SDDS_FilterRowsOfInterest(t,const_cast<char*>("particleID"),id,id,SDDS_0_PREVIOUS);
    if (ret == -1) throw sddsi::SDDSFailure();
  }

  delete t;
}

TEST(sdds, FilterColumns) {
  SDDS_TABLE *t = new SDDS_TABLE;
  if( SDDS_InitializeInput(t,const_cast<char*>("libpalattice.clo")) != 1 )
    throw sddsi::SDDSFailure();
  
  SDDS_ReadTable(t);
  SDDS_SetColumnFlags(t,0); // unselect all columns first
  if( SDDS_SetColumnsOfInterest(t, SDDS_NAMES_STRING, "s x ") != 1 )
    throw sddsi::SDDSFailure();
  
  void *mem = SDDS_GetColumn(t, const_cast<char*>("s"));
  if (mem == NULL)
    throw sddsi::SDDSFailure();

  double *array = static_cast<double *>(mem);
  unsigned int length=SDDS_CountRowsOfInterest(t); 
  std::vector<double> s(array, array+length);

  //std::cout <<"s[20]="<< s[20] << std::endl;
  
  EXPECT_EQ(333u, s.size());

  // "Column of interest" not really useful:
  // Other columns are ignored by CountColumnsOfInterest()
  // but not by ColumnCount(). Still all columns are accessible !?!
  EXPECT_EQ(2, SDDS_CountColumnsOfInterest(t));
  EXPECT_EQ(8, SDDS_ColumnCount(t));
  EXPECT_DOUBLE_EQ(0.0, s[0]);

  free(mem);
  delete t;
}




TEST(sddsSimTool, Parameter) {
  pal::SimToolInstance elegant(pal::elegant, pal::offline, "libpalattice.param");
  elegant.set_sddsMode(true);

  double p = elegant.readParameter<double>(elegant.twiss(), "pCentral");
  EXPECT_NEAR(4.500987e+03, p, 0.001);
}

TEST(sddsSimTool, Table) {
  pal::SimToolInstance elegant(pal::elegant, pal::offline, "libpalattice.param");
  elegant.set_sddsMode(true);

  std::vector<std::string> columnKeys = {"s", "x", "y"};
  pal::SimToolTable tab = elegant.readTable(elegant.orbit(), columnKeys);

  EXPECT_EQ(333u, tab.rows());
  EXPECT_EQ(3u, tab.columns());
  EXPECT_NEAR(164.4008, tab.getd(332,"s"), 0.0001);
  EXPECT_DOUBLE_EQ(0.0, tab.get<double>(0,"s"));
  
  std::cout << "xp[20]="<<tab.getd(20,"xp") << std::endl;
}

TEST(sddsSimTool, Circumference) {
  pal::SimToolInstance elegant(pal::elegant, pal::offline, "libpalattice.param");
  elegant.set_sddsMode(true);

  EXPECT_NEAR(164.4008, elegant.readCircumference(), 0.0001);
}


TEST(sddsFoP, SimToolColumn) {
  pal::SimToolInstance elegant(pal::elegant, pal::offline, "libpalattice.param");
  elegant.set_sddsMode(true);
  
  pal::FunctionOfPos<double> betax(elegant);
  EXPECT_NEAR(164.4008, betax.circumference(), 0.0001);
  
  betax.readSimToolColumn(elegant, elegant.twiss(), "s", "betax");
  EXPECT_EQ(333u,betax.size());
  betax.print("sdds-betax.dat");
}

// TEST(sddsFoP, OrbitTest) {
//   pal::SimToolInstance elegant(pal::elegant, pal::offline, "libpalattice.param");
//   elegant.set_sddsMode(true);
  
//   pal::FunctionOfPos<pal::AccPair> orbit(elegant);
//   EXPECT_NEAR(164.4008, orbit.circumference(), 0.0001);
  
//   //orbit.simToolClosedOrbit(elegant);
//   EXPECT_EQ(333u,orbit.size());
//   orbit.print("sdds-orbit.dat");
// }




int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
