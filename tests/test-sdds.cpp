#include "gtest/gtest.h"
#include "../FunctionOfPos.hpp"
#include "../config.hpp"



//======================================================================================
#ifdef LIBPALATTICE_USE_SDDS_TOOLKIT_LIBRARY
//======================================================================================

#include "SDDS/SDDS.h"

TEST(sdds, Parameter) {
  SDDS_TABLE *t = new SDDS_TABLE;
  if( SDDS_InitializeInput(t,const_cast<char*>(TEST_TWISS_FILE)) != 1 )
    throw pal::SDDSError(TEST_TWISS_FILE);
  
  SDDS_ReadTable(t);
  void *mem = SDDS_GetParameter(t, const_cast<char*>("pCentral"), NULL);
  if (mem == NULL)
    throw pal::SDDSError(TEST_TWISS_FILE);
  
  double p = *static_cast<double *>(mem);
  EXPECT_NEAR(4.500987e+03, p, 0.001);
  
  free(mem);
  ASSERT_EQ(1,SDDS_Terminate(t));
  delete t;
}

TEST(sdds, StringParameter) {
  SDDS_TABLE *t = new SDDS_TABLE;
  if( SDDS_InitializeInput(t,const_cast<char*>(TEST_WATCH_FILE)) != 1 )
    throw pal::SDDSError(TEST_WATCH_FILE);
  
  SDDS_ReadTable(t);
  void *mem = SDDS_GetParameter(t, const_cast<char*>("PreviousElementName"), NULL);
  if (mem == NULL)
    throw pal::SDDSError(TEST_WATCH_FILE);

  std::string s(*static_cast<char **>(mem));
  EXPECT_STREQ("BPM02", s.c_str());

  free(*((char**)mem));
  free(mem);
  ASSERT_EQ(1,SDDS_Terminate(t));
  delete t;
}

TEST(sdds, Column) {
  SDDS_TABLE *t = new SDDS_TABLE;
  if( SDDS_InitializeInput(t,const_cast<char*>(TEST_TWISS_FILE)) != 1 )
    throw pal::SDDSError(TEST_TWISS_FILE);
  
  SDDS_ReadTable(t);
  void *mem = SDDS_GetColumn(t, const_cast<char*>("s"));
  if (mem == NULL)
    throw pal::SDDSError(TEST_TWISS_FILE);

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
  ASSERT_EQ(1,SDDS_Terminate(t));
  delete t;
}

TEST(sdds, ColumnValues) {
 SDDS_TABLE *t = new SDDS_TABLE;
  if( SDDS_InitializeInput(t,const_cast<char*>(TEST_TWISS_FILE)) != 1 )
    throw pal::SDDSError(TEST_TWISS_FILE);

  SDDS_ReadTable(t);
  void *mem;
  double s=0.;
  std::vector<double> x;
  unsigned int length=SDDS_CountRowsOfInterest(t);
  for (auto i=0u; i<length; i++) {
    mem = SDDS_GetValue(t, const_cast<char*>("s"), i, NULL);
    if (mem == NULL) throw pal::SDDSError(TEST_TWISS_FILE);
    s = *static_cast<double *>(mem);
    x.push_back(std::move(*static_cast<double *>(mem)));
    free(mem);
  }
  
  EXPECT_NEAR(164.4008, s, 0.0001);
  EXPECT_EQ(333u, x.size());
  EXPECT_DOUBLE_EQ(0.0, x[0]);
  EXPECT_NEAR(164.4008, x[332],0.0001);

  ASSERT_EQ(1,SDDS_Terminate(t));
  delete t;
}

TEST(sdds, Pages) {
  SDDS_TABLE *t = new SDDS_TABLE;
  if( SDDS_InitializeInput(t,const_cast<char*>(TEST_WATCH_FILE)) != 1 )
    throw pal::SDDSError(TEST_WATCH_FILE);

  // SDDS_ReadTable (or ReadPage) gives first page, has to be repeated for every page.
  // returns -1 if no more pages to read.

  int status = SDDS_ReadPage(t);
  unsigned int i=0;
  while (status != -1) {
    void *mem = SDDS_GetColumn(t, const_cast<char*>("p"));
    if (mem == NULL)
      throw pal::SDDSError(TEST_WATCH_FILE);
    
    double *array = static_cast<double *>(mem);
    unsigned int length=SDDS_CountRowsOfInterest(t); 
    std::vector<double> x(array, array+length);
    EXPECT_EQ(5u, x.size());

    void *parmem = SDDS_GetParameter(t, const_cast<char*>("Pass"), NULL);
    if (parmem == NULL) throw pal::SDDSError(TEST_WATCH_FILE);
    unsigned int turn = *static_cast<unsigned int *>(parmem);
    EXPECT_EQ(i, turn);

    free(mem);
    free(parmem);
    i++;
    status = SDDS_ReadPage(t);
  }
  
  ASSERT_EQ(1,SDDS_Terminate(t));
  delete t;
}

TEST(sdds, FilterParticleId) {
  SDDS_TABLE *t = new SDDS_TABLE;
  if( SDDS_InitializeInput(t,const_cast<char*>(TEST_WATCH_FILE)) != 1 )
    throw pal::SDDSError(TEST_WATCH_FILE);

  // filter single particle id
  // by SDDS_FilterRowsOfInterest AFTER(!) EACH(!) SDDS_ReadPage call
  // Default are all Rows => use Logic Macro SDDS_AND to get only the filtered Rows
  unsigned int id=2u;

  int status = SDDS_ReadPage(t);
  int ret = SDDS_FilterRowsOfInterest(t,const_cast<char*>("particleID"),id,id,SDDS_AND);
  if (ret == -1) throw pal::SDDSError(TEST_WATCH_FILE);
  ASSERT_EQ(1,ret);

  unsigned int i=0;
  while (status != -1) {
    void *mem = SDDS_GetColumn(t, const_cast<char*>("p"));
    if (mem == NULL)
      throw pal::SDDSError(TEST_WATCH_FILE);
    
    double *array = static_cast<double *>(mem);
    unsigned int length=SDDS_CountRowsOfInterest(t); 
    std::vector<double> x(array, array+length);
    EXPECT_EQ(1u, length);
    EXPECT_EQ(1u, x.size());

    void *parmem = SDDS_GetParameter(t, const_cast<char*>("Pass"), NULL);
    if (parmem == NULL) throw pal::SDDSError(TEST_WATCH_FILE);
    unsigned int turn = *static_cast<unsigned int *>(parmem);
    EXPECT_EQ(i, turn);

    free(mem);
    free(parmem);
    i++;
    status = SDDS_ReadPage(t);
    int ret = SDDS_FilterRowsOfInterest(t,const_cast<char*>("particleID"),id,id,SDDS_0_PREVIOUS);
    if (ret == -1) throw pal::SDDSError(TEST_WATCH_FILE);
  }

  ASSERT_EQ(1,SDDS_Terminate(t));
  delete t;
}

TEST(sdds, FilterColumns) {
  SDDS_TABLE *t = new SDDS_TABLE;
  if( SDDS_InitializeInput(t,const_cast<char*>(TEST_ORBIT_FILE)) != 1 )
    throw pal::SDDSError(TEST_ORBIT_FILE);
  
  SDDS_ReadPage(t);
  SDDS_SetColumnFlags(t,0); // unselect all columns first
  if( SDDS_SetColumnsOfInterest(t, SDDS_NAMES_STRING, "s x ") != 1 )
    throw pal::SDDSError(TEST_ORBIT_FILE);
  
  void *mem = SDDS_GetColumn(t, const_cast<char*>("s"));
  if (mem == NULL)
    throw pal::SDDSError(TEST_ORBIT_FILE);

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
  ASSERT_EQ(1,SDDS_Terminate(t));
  delete t;
}


TEST(sdds, FileTwice) {
  SDDS_TABLE *t1 = new SDDS_TABLE;
  SDDS_TABLE *t2 = new SDDS_TABLE;

  unsigned int id=1u;
  
  ASSERT_EQ(1, SDDS_InitializeInput(t1, const_cast<char*>(TEST_WATCH_FILE)) );
  ASSERT_EQ(1, SDDS_InitializeInput(t2, const_cast<char*>(TEST_WATCH_FILE)) );
  ASSERT_NE(0, SDDS_ReadPage(t1));
  ASSERT_NE(0, SDDS_ReadPage(t2));
  ASSERT_EQ(1, SDDS_FilterRowsOfInterest(t1, const_cast<char*>("particleID"),id,id,SDDS_AND));
  ASSERT_EQ(1, SDDS_FilterRowsOfInterest(t2, const_cast<char*>("particleID"),id,id,SDDS_AND));
  
  void *mem1 = SDDS_GetValue(t1, const_cast<char*>("x"), 0, NULL);
  if (mem1 == NULL) throw pal::SDDSError(TEST_WATCH_FILE);
  double x1 = *static_cast<double *>(mem1);
  void *mem2 = SDDS_GetValue(t2, const_cast<char*>("x"), 0, NULL);
  if (mem2 == NULL) throw pal::SDDSError(TEST_WATCH_FILE);
  double x2 = *static_cast<double *>(mem2);
  ASSERT_DOUBLE_EQ(x1,x2);
  std::cout << x1 <<" / "<< x2 <<std::endl;

  ASSERT_NE(0, SDDS_ReadPage(t1));
  ASSERT_NE(0, SDDS_ReadPage(t2));
  ASSERT_EQ(1, SDDS_FilterRowsOfInterest(t1, const_cast<char*>("particleID"),id,id,SDDS_AND));
  ASSERT_EQ(1, SDDS_FilterRowsOfInterest(t2, const_cast<char*>("particleID"),id,id,SDDS_AND));
  
  
  free(mem1); free(mem2);
  ASSERT_EQ(1,SDDS_Terminate(t1));
  ASSERT_EQ(1,SDDS_Terminate(t2));
  delete t1;
  delete t2;
}

//======================================================================================
#endif
//======================================================================================



// the following tests fail without libsdds, because the input files
// are binary sdds files, which cannot be read without libsdds.

TEST(sddsSimTool, Parameter) {
  pal::SimToolInstance elegant(pal::elegant, pal::offline, TEST_PARAM_FILE);
  elegant.setTurns(0);

  double p = elegant.readParameter<double>(elegant.twiss(), "pCentral");
  EXPECT_NEAR(4.500987e+03, p, 0.001);
}

TEST(sddsSimTool, StringParameter) {
  pal::SimToolInstance elegant(pal::elegant, pal::offline, TEST_PARAM_FILE);

  std::string p = elegant.readParameter<string>(elegant.twiss(), "pCentral");
  if(elegant.sddsMode()) {
    EXPECT_STREQ("4500.99", p.c_str());
    std::string p2 = elegant.readParameter<string>(elegant.twiss(), "Stage");
    EXPECT_STREQ("tunes uncorrected", p2.c_str());
  }
  else
    EXPECT_STREQ("4.500986753282873e+03", p.c_str());
}

TEST(sddsSimTool, Table) {
  pal::SimToolInstance elegant(pal::elegant, pal::offline, TEST_PARAM_FILE);

  std::vector<std::string> columnKeys = {"s", "x", "y"};
  pal::SimToolTable tab;
  tab = elegant.readTable(elegant.orbit(), columnKeys);

  EXPECT_EQ(333u, tab.rows());
  EXPECT_EQ(3u, tab.columns());
  EXPECT_NEAR(164.4008, tab.getd(332,"s"), 0.0001);
  EXPECT_DOUBLE_EQ(0.0, tab.get<double>(0,"s"));
  if(elegant.sddsMode())
    EXPECT_STREQ("0", tab.get<std::string>(0,"s").c_str());
  else
    EXPECT_STREQ("0.000000e+00", tab.get<std::string>(0,"s").c_str());
  
  //std::cout << "xp[20]="<<tab.getd(20,"xp") << std::endl;
}

TEST(sddsSimTool, Circumference) {
  pal::SimToolInstance elegant(pal::elegant, pal::offline, TEST_PARAM_FILE);

  EXPECT_NEAR(164.4008, elegant.readCircumference(), 0.0001);
}


TEST(sddsSimTool, Exception) {
  pal::SimToolInstance elegant(pal::elegant, pal::offline, TEST_PARAM_FILE);

  EXPECT_THROW(elegant.readTable(elegant.orbit(), {"s", "x", "yyyy"}), pal::SDDSError);

  std::stringstream s;
  s << "SDDS-Error:\nUnable to process column selection--unrecognized column name yyyy seen (SDDS_SetColumnsOfInterest)\n [" << TEST_ORBIT_FILE << "]";
  
  try {
    elegant.readTable(elegant.orbit(), {"s", "x", "yyyy"});
  }
  catch (SDDSError &e) {
    EXPECT_STREQ(e.what(), s.str().c_str());
  }
}






TEST(sddsFoP, Column) {
  pal::SimToolInstance elegant(pal::elegant, pal::offline, TEST_PARAM_FILE);
  
  pal::FunctionOfPos<double> betax(elegant);
  EXPECT_NEAR(164.4008, betax.circumference(), 0.0001);
  
  betax.readSimToolColumn(elegant, elegant.twiss(), "s", "betax");
  EXPECT_EQ(296u,betax.size());
  betax.print("sdds-betax.dat");
}


TEST(sddsFoP, Orbit) {
  pal::SimToolInstance elegant(pal::elegant, pal::offline, TEST_PARAM_FILE);
  
  pal::FunctionOfPos<pal::AccPair> orbit(elegant);
  EXPECT_NEAR(164.4008, orbit.circumference(), 0.0001);
  
  orbit.simToolClosedOrbit(elegant);
  EXPECT_EQ(296u,orbit.size());
  orbit.print("sdds-orbit.dat");
}


//this test is in online mode, so it also succeeds without sdds,
//because in this case it creates & parses ascii output automatically.
TEST(sddsFoP, Trajectory) {
  pal::SimToolInstance elegant(pal::elegant, pal::online, TEST_LATTICE_FILE);
  elegant.setTurns(10);
  elegant.setNumParticles(12);
  //elegant.verbose = true;
  
  pal::FunctionOfPos<AccPair> traj(elegant);
  EXPECT_NEAR(164.4008, traj.circumference(), 0.0001);

  traj.simToolTrajectory(elegant, 3);
  EXPECT_EQ(10u, traj.turns());
  EXPECT_EQ(33u, traj.samplesInTurn(1));
  EXPECT_EQ(33u, traj.samplesInTurn(10));
  EXPECT_EQ(0u, traj.samplesInTurn(900));
  traj.print("sdds-trajectory.dat");
}




int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();

}
