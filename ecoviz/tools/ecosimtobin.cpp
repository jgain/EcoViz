/*
Simple text to binary conversion for EcoViz project files.
(c) P. C. Marais 2024
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <sstream>

using namespace std;

// This program translates input data sources for the EcoSim visualisation tool into binary format.
// It accepts a single elevation map (.elv, text format) and/or a sequence of cohortmaps from the simulation,
// in PDB (.pdb) format. The sequence is assumed to be numbered <basename>N.pdb, where N is a number. N should
// start at . 
// Switches:
// -e <string>     --- .elv file, input (text) elevation map (one only), generates binary .elvb file
// -c <string>     --- the base name for sequence of cohort maps.
//                     input will start at <base>0.pdb,  outputs will be <base><sequence_num>.pdbb
// -n <int>        --- how many PDBs to convert (default = all)
// -v <string>     --- set a new version string for cohort maps (replaces existing one)

// prototypes

// ** helper functions
void parseCommandLine(int agc, char *argv[]);
void printUsage(void);
void printError(string s);
int  getFileSequenceNumber(const string &stem, const string & basename);
// ** conversion functions
void elevationToBin(const string & in, const string & out);
void cohortmapToBin(const string &in, const string & out, const string & verStr);


// globals - ugly but makes life easier

bool elvConv = false;
bool cohortConv = false;
string base = "";
string elvFile = "";
int nFiles = 0;
string version="";
bool newVersion = false;
  

int main(int argc, char *argv[])
{
  parseCommandLine(argc, argv);

  if (elvConv)
    {
      int pos = elvFile.find(".elv");
      if (pos != string::npos)
	{
	  string ofile = elvFile.substr(0,pos) + ".elvb";
	  elevationToBin(elvFile, ofile);
	  cout << " -- Elevation file converted " << elvFile << " converted to " << ofile << endl; 
	}
      else
	printError("elevation conversion failed - invalid extension for name " + elvFile);
    }

  if (cohortConv)
    {
      filesystem::directory_iterator diriter("./");
      vector<int> filenumbers;

      for (auto &entry : diriter)
	{
	  if (entry.path().extension() == ".pdb" && entry.is_regular_file() )
	    {
	      string stem = entry.path().stem();
	      if (stem.find(base) != string::npos)
		{
		  string fname = entry.path().filename();
		  // extract number
		  int sequence = getFileSequenceNumber(stem,base);
		  // 
		  if (sequence == -1)
		    {
		      printError("Cohort conversion error: file has no sequence number - " + fname);
		    }
		  filenumbers.push_back(sequence);   
		}
	    }
	}

      vector<string> inFiles, outFiles;
      sort(filenumbers.begin(), filenumbers.end());
      int filesToProcess = (nFiles > 0 ? min(nFiles, int(filenumbers.size()) ) : int(filenumbers.size())); 
      for (int i = 0; i < filesToProcess; ++i)
	{
	  string fname = base + to_string(i);
	  cohortmapToBin(fname + ".pdb", fname + ".pdbb", version);
	}
       
    }
   
  return 0;
}


void elevationToBin(const string & in, const string & out)
{
  // format: width(int) height(int) step (float)
  // width*height floats
      
    int dx, dy;

    float val, step;
    ifstream infile;
    vector<float> heights;

    infile.open((char *) in.c_str());
    if(infile.is_open())
    {
        infile >> dx >> dy;
        infile >> step;

	for (size_t idx = 0; idx < dx*dy; ++idx)
	  {
	    infile >> val;
	    heights.push_back(val);
	  }
    }
    else
    {
        cerr << "elevationToBin: unable to open file for load" << in << endl;
    }

    // write back

    ofstream ofile;
    ofile.open((char*)out.c_str(), ios::binary);
    if (ofile.is_open())
      {
	ofile.write(reinterpret_cast<const char*>(&dx), sizeof(int));
	ofile.write(reinterpret_cast<const char*>(&dy), sizeof(int));
	ofile.write(reinterpret_cast<const char*>(&step), sizeof(float));
	ofile.write(reinterpret_cast<const char*>(heights.data()), sizeof(float)*heights.size());
	if (!ofile)
	  {
	    cerr<< "elevationToBin: error occured when writing to binary file.\n"; 
	  }
      }
    else
      {
	cerr << "elevationToBin: conversion failed; unable to write to " << out << endl;
      }
}




void cohortmapToBin(const string &in, const string & out, const string & verStr)
{
  if (verStr.size() > 0)
    cout << "New Version string: " << verStr << endl;
  cout << "Converting " << in << " to binary format, writing to " << out << endl;

  std::string lstr;
  std::ifstream ifs(in);

  if (!ifs.is_open())
    throw invalid_argument("Could not open file at " + in);


  string versionNumber;
  ifs >> versionNumber;
  int timestep;
  ifs >> timestep;
  int ntrees_expected;
  ifs >> ntrees_expected;
  string species_id;

  // first part of cohort file
  
  struct cohortA {
  int treeid;
  char code[4]; // 4 byte ASCII tree code
  int x;
  int y;
  float height;
  float radius;
  float dbh;
  int dummy;
};
  
  vector<cohortA> cohortAdata;
  cohortAdata.reserve(ntrees_expected);
 
  for (int i = 0; i < ntrees_expected; ++i)
    {
      cohortA dataA;
       
      ifs >> dataA.treeid;    
      ifs >> species_id; // alpha-numeric species key
      if (species_id.size() != 4)
	printError("Species id " + species_id + " found for " + to_string(i) + "th tree.");
      else
	{
	  dataA.code[0] = species_id[0];
	  dataA.code[1] = species_id[1];
	  dataA.code[2] = species_id[2];
	  dataA.code[3] = species_id[3];
	}
      ifs >> dataA.x;
      ifs >> dataA.y;
      ifs >> dataA.height;
      ifs >> dataA.radius;
      ifs >> dataA.dbh;
      ifs >> dataA.dummy;		

      cohortAdata.push_back(dataA);
    }

  long nBytes = sizeof(cohortA)*cohortAdata.size();

  ofstream ofs;
  ofs.open(out.c_str(), ios::binary);
  if (!ofs.is_open())
    throw invalid_argument("Could not open file at " + out);

  if (verStr.size() > 0)
    versionNumber = verStr;
  
  int slen = versionNumber.length();
  cout << "Version string: " << versionNumber << " of length " << slen << endl;
  ofs.write(reinterpret_cast<const char*>(&slen), sizeof(int));				  
  ofs.write(versionNumber.c_str(), slen); // don't store null
  ofs.write(reinterpret_cast<const char*>(&timestep), sizeof(int));
  ofs.write(reinterpret_cast<const char*>(&ntrees_expected), sizeof(int));
  ofs.write(reinterpret_cast<const char*>(cohortAdata.data()), nBytes);
  if (!ofs)
    throw runtime_error("Something went wrong when writing first part of " + out);
  cohortAdata.clear();

  // second part of cohort file
  
  int ncohorts_expected;

  struct cohortB {
    int xs;
    int ys;
    char code[4];
    float dbh;
    float height;
    float nplants;
  };

  vector<cohortB> cohortBdata;
 
  ifs >> ncohorts_expected;

  cohortBdata.reserve(ncohorts_expected);

  for (int i = 0; i < ncohorts_expected; ++i)
    {
      cohortB dataB;
       
      ifs >> dataB.xs;
      ifs >> dataB.ys;
      ifs >> species_id; // alpha-numeric species key
      if (species_id.size() != 4)
	printError("Species id " + species_id + " found for " + to_string(i) + "th cohort.");
      else
	{
	  dataB.code[0] = species_id[0];
	  dataB.code[1] = species_id[1];
	  dataB.code[2] = species_id[2];
	  dataB.code[3] = species_id[3];
	}
      ifs >> dataB.dbh;
      ifs >> dataB.height;
      ifs >> dataB.nplants;

      cohortBdata.push_back(dataB);
    }

  ifs.close();
  
  long nBytesB = sizeof(cohortB)*cohortBdata.size();
  cout << "Number of bytes in part B of binary file: " << nBytesB << endl;
  ofs.write(reinterpret_cast<const char*>(&ncohorts_expected), sizeof(int));
  ofs.write(reinterpret_cast<const char*>(cohortBdata.data()), nBytesB);
  if (!ofs)
    throw runtime_error("Something went wrong when writing second part of " + out);
  ofs.close();
  
  cout << "Wrote total of (partA = " << nBytes << " and partB = " << nBytesB << ") - total: " <<
    (nBytes+nBytesB) << " bytes\n";
   
}


// ---------------------------------------------------------------------------------------------------

// return the sequence number from the file stem, or -1 there is no number 
int getFileSequenceNumber(const string &stem, const string & basename)
{
  int d;
  string digits = stem.substr(basename.length());
  try {
    d = stoi(digits);
  }
  catch (exception &e) {
    d = -1;
  }
    
  return d;
}

void parseCommandLine(int argc, char *argv[])
{
   // parse command line
  if (argc == 1) printUsage();
   
  for (int i = 1; i < argc; ++i)
    {
      string arg = argv[i];
      if (arg == "-e")
	{
	  if (elvConv || i+1 >= argc) printError("-e can only occur once an must have an argument");
	  elvConv = true;
	  elvFile = argv[++i];
	}
      else if (arg == "-c")
	{
	  if (cohortConv || i+1 >= argc) printError("-c can only occur once an must have an argument");
	  cohortConv = true;
	  base = argv[++i];
	}
      else if (arg == "-n")
	{
	  if (i+1 >= argc)  printError("-n must have an argument");
	  try {
	    nFiles = stoi(argv[++i]);
	  }
	  catch(exception &e) {
	    printError("-n must provide a valid integer");
	  }
	  if (nFiles < 1)
	    printError("-n must be > 0"); 
	}
      else if (arg == "-v")
	{
	  if (i+1 >= argc)  printError("-v must have an argument");
	  newVersion = true;
	  version = argv[++i];
	}
      else if (arg == "-h")
	printUsage();
      else
	printError("inavlid argument, use the -h option for more information");
	
    } 
}

void printUsage(void)
{
 char info[] = "This program translates input data sources for the EcoSim visualisation tool into binary format.\n\
It accepts a single elevation map (.elv, text format) and/or a sequence of cohortmaps from the simulation,\n\
in PDB (.pdb) format. The sequence is assumed to be numbered <basename>N.pdb, where N is a number. N should\n\
start at 0 (or 00 etc).\n\
Switches:\
-e <string>     --- .elv file, input (text) elevation map (one only), generates binary .elvb file\n\
-c <string>     --- the base name for sequence of cohort maps.\n\
                input will start at <base><0>.pdb,  outputs will be <base><sequence_num>.pdbb\n\
-n <int>        --- how many PDBs to convert (default = all)\n\
-v <string>     --- set a new version string for cohort maps (replaces existing one)\n";

cerr<< info;
  exit(0);
}

void printError(string mesg)
{
  cerr << mesg << endl;
  exit(0);
}
