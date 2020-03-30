
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "TMath.h"

#include "TCanvas.h"
#include "TGraph.h"
#include "TAxis.h"
#include "TLine.h"

#include "TStyle.h"
#include "TPaveLabel.h"
#include "TLatex.h"
#include "TBox.h"
#include "TLegend.h"
#include "TLegendEntry.h"
#include "TList.h"

using namespace std;

  const int Nvars      = 5; //variables to be read: timestamp, B1_H, B1_V, B2_H, B2_V
  const int NvarsBPM   = 9; //variables to be read: timestamp, B1_H_L, B1_H_R, B1_V_L, B1_V_R, B2_H_L, B2_H_R, B2_V_L, B2_V_R
  const int NdataMax   = 30000;
  const int NscansMax  = 30;
  const int Nsteer     = 4;
  const int NoffMax    = 30;
  const int NlabelsMax = 50;

double sciToDub(const string& str) {

   stringstream ss(str);
   double d = 0;
   ss >> d;

   if (ss.fail()) {
      string s = "Unable to format ";
      s += str;
      s += " as a number!";
      throw (s);
   }

   return (d);
}

long sciToLong(const string& str) {
   return (sciToDub(str));
}
  
void readPlotSteer (string stFile, int* iTimeRange, bool* blPdraw, int* iPcolor, string* stTitle, double* dbXaxisRange, double** dbYaxisRange, double& dbNomYcutOut, int& iBPM2color, int iODparsOut[2], int& nLabels, string stLabels[NlabelsMax], double dbLabelsPos[4][NlabelsMax]) {
  
  const int Npar = 13;
  const int NsubparsMax = 3;
  
  ifstream fin;
  string stfiline;
  
  fin.open(stFile.c_str());
  bool blSPsubparRead[NsubparsMax]; //boolean flags if all parameters (zero value + min/max time, draw/color) are read.
  int iSubstr, iCurrIdx = -1; //iCurrIdx: 0 - Timestamp, 1-4 - B1_H, B1_V, B2_H, B2_V
  string stSTpars[Npar] = {"Timestamp:", "B1_H:", "B1_V:", "B2_H:", "B2_V:", "Plot info:", "X axis:", "Y axis BPM:", "Y axis Nominal:", "Nominal data:", "BPM2:", "Orbit Drift:", "Labels:"};
  int    Nsubpars[Npar] = {     3,          2,       2,       2,       2,         3,          2,            2,              2,                1,            1,             2,              1};
  string stSTtimepars[3] = {"zero value:", "plot min:", "plot max:"};
  string stSTaxispars[2] = {"min:", "max:"};
  string stSTplotInfo[3] = {"title:", "top-left:", "top-right:"};
  string stpBPM2sub[1]   = {"color:"};
  string stODpars[2]     = {"min indent:", "max length:"};
  int iNlabelStart = nLabels;
  
  int iTmp, istposTmp;
  string stTmp;
  bool blCheck;
  
  while(getline(fin, stfiline)) {

    for (int i=0; i<Npar; i++) {
      iSubstr = stfiline.find(stSTpars[i]);
      if (iSubstr < stfiline.size()) {
        if (iCurrIdx != -1) {
          cout << "ERROR: New parameter: " << stSTpars[i] << " found while iCurrIdx == " << iCurrIdx << "." << endl;
          exit(1);
        }
        iCurrIdx = i;
        for (int j=0; j<NsubparsMax; j++) blSPsubparRead[j] = false;
      }
    }

    if (iCurrIdx == 0) {
      for (int i=0; i<Nsubpars[iCurrIdx]; i++) {
        iSubstr = stfiline.find(stSTtimepars[i]);
        if (iSubstr < stfiline.size()) {
          iTimeRange[i] = stoi(stfiline.substr(iSubstr + stSTtimepars[i].size() + 0));
          blSPsubparRead[i] = true;
        }
      }
    } else if (iCurrIdx > 0 && iCurrIdx < 5) {
      iSubstr = stfiline.find(string("draw:"));
      if (iSubstr < stfiline.size()) {
        if ( stfiline.find(string("true")) < stfiline.size() ) {
          blPdraw[iCurrIdx-1] = true;
        } else {
          blPdraw[iCurrIdx-1] = false;
        }
        blSPsubparRead[0] = true;
      }
      
      iSubstr = stfiline.find(string("color:"));
      if (iSubstr < stfiline.size()) {
        iPcolor[iCurrIdx-1] = stoi(stfiline.substr(iSubstr + 6));
        blSPsubparRead[1] = true;
      }
    } else if (iCurrIdx == 5) {
        for (int i=0; i<Nsubpars[iCurrIdx]; i++) {
            iSubstr = stfiline.find(stSTplotInfo[i]);
            if (iSubstr < stfiline.size()) {
                iTmp = stfiline.substr(iSubstr + stSTplotInfo[i].size()).find_first_not_of(' ');
                if (iTmp >= 0) {
                    stTitle[i] = stfiline.substr(iSubstr + stSTplotInfo[i].size() +  iTmp);
                } else {
                    stTitle[i] = string("None");
                }
                blSPsubparRead[i] = true;
            }
        }
    } else if (iCurrIdx == 6) {
      for (int i=0; i<Nsubpars[iCurrIdx]; i++) {
        iSubstr = stfiline.find(stSTaxispars[i]);
        if (iSubstr < stfiline.size()) {
          dbXaxisRange[i] = stod(stfiline.substr(iSubstr + stSTaxispars[i].size() + 0));
          blSPsubparRead[i] = true;
        }
      }
    } else if (iCurrIdx > 6 && iCurrIdx < 9) {
      iTmp = iCurrIdx - 7;
      for (int i=0; i<Nsubpars[iCurrIdx]; i++) {
        iSubstr = stfiline.find(stSTaxispars[i]);
        if (iSubstr < stfiline.size()) {
          dbYaxisRange[iTmp][i] = stod(stfiline.substr(iSubstr + stSTaxispars[i].size() + 0));
          blSPsubparRead[i] = true;
        }
      }
    } else if (iCurrIdx == 9) {
      iSubstr = stfiline.find(string("Y cut:"));
      if (iSubstr < stfiline.size()) {
        dbNomYcutOut = stod(stfiline.substr(iSubstr + 6));
        blSPsubparRead[0] = true;
      }
    } else if (iCurrIdx == 10) {
      for (int i=0; i<Nsubpars[iCurrIdx]; i++) {
        iSubstr = stfiline.find(stpBPM2sub[i]);
        if (iSubstr < stfiline.size()) {
          iBPM2color = stoi(stfiline.substr(iSubstr + stpBPM2sub[i].size()));
          blSPsubparRead[i] = true;
        }
      }
    } else if (iCurrIdx == 11) {
      for (int i=0; i<Nsubpars[iCurrIdx]; i++) {
        iSubstr = stfiline.find(stODpars[i]);
        if (iSubstr < stfiline.size()) {
          iODparsOut[i] = stoi(stfiline.substr(iSubstr + stODpars[i].size()));
          blSPsubparRead[i] = true;
        }
      }
    } else if (iCurrIdx == 12) {
        iSubstr = stfiline.find(string("["));
        if (iSubstr < stfiline.size()) {
            stTmp = stfiline.substr(iSubstr + 1);
            
            istposTmp = stTmp.find('"') + 1;
//            if (istposTmp >= stTmp.size()) break; 
            stTmp = stTmp.substr( istposTmp );
            stLabels[nLabels] = stTmp.substr(0, stTmp.find('"'));
            for (int iv = 0; iv < 3; iv++) {
                stTmp = stTmp.substr( stTmp.find(',') + 1 );
                dbLabelsPos[iv][nLabels] = stod( stTmp.substr(0, stTmp.find(',')) );
            }
            stTmp = stTmp.substr( stTmp.find(',') + 1 );
            dbLabelsPos[3][nLabels] = stod( stTmp.substr(0, stTmp.find(']')) );
            nLabels++;
            
        } else if (nLabels > iNlabelStart) {
            blSPsubparRead[0] = true;
        }
    }
    
    if (iCurrIdx > -1) {
      blCheck = blSPsubparRead[0];
      for (int j=1; j<Nsubpars[iCurrIdx]; j++) blCheck = blCheck && blSPsubparRead[j];
      if (blCheck) iCurrIdx = -1;
    }
    
  } //end loop over file lines
  
  fin.close();
  
  return;
}

void readDataSteerBPM (string stFile, string& stDF, string& stDFtitle, string stDFcName[Nvars], int& iNOffOut, long lOfftimeOut[NoffMax], double dbOffsetOut[4][NoffMax], int iDFcIdx[Nvars*2], double dbDFcScale[Nvars]) {
  
  const int Npar = 4;
  const int NsubparsMax = 5;
  const int NsubsubparsMax = 3;
  
  ifstream fin;
  string stfiline;
  
  fin.open(stFile.c_str());
  bool blSPsubparRead[NsubparsMax][NsubsubparsMax]; //boolean flags if all parameters (zero value + min/max time, draw/color) are read.
  int iSubstr, iCurrParIdx = -1, iCurrSubParIdx = -1;
  string stSTpars[Npar] = {"Data file:", "Offsets:", "Columns:", "Title:"};
  int Nsubpars[Npar] = {      0,            2,       NsubparsMax,    0};
  string stSTsubpars[Npar][NsubparsMax] = {{"-","-","-","-","-"}, {"Timestamps:","OffValues:","-","-","-"}, {stDFcName[0],stDFcName[1],stDFcName[2],stDFcName[3],stDFcName[4]}, {"-","-","-","-","-"}};
  int Nsubsubpars[Npar][NsubparsMax] =    {{ 0 , 0 , 0 , 0 , 0 }, { 0 , 0 , 0 , 0 , 0 }, { 2 , 3 , 3 , 3 , 3 }};
  string stSTsubsubpars[Npar][NsubparsMax][NsubsubparsMax] = {{{"-","-","-"}, {"-","-","-"}, {"-","-","-"}, {"-","-","-"}, {"-","-","-"}}, {{"-","-","-"}, {"-","-","-"}, {"-","-","-"}, {"-","-","-"}, {"-","-","-"}}, {{"column:","scale:","-"}, {"L:","R:","scale:"}, {"L:","R:","scale:"}, {"L:","R:","scale:"}, {"L:","R:","scale:"}}, {{"-","-","-"}, {"-","-","-"}, {"-","-","-"}, {"-","-","-"}, {"-","-","-"}}};
  
  bool blSubCheck, blSubSubCheck[NsubparsMax];
  
  int iNOffTmp, istposTmp;
  string stTmp, stTmp2;
  
  while(getline(fin, stfiline)) {

    for (int i=0; i<Npar; i++) {
      iSubstr = stfiline.find(stSTpars[i]);
      if (iSubstr < stfiline.size()) {
        if (iCurrParIdx != -1) {
          cout << "ERROR: New parameter: " << stSTpars[i] << " found while iCurrParIdx == " << iCurrParIdx << "." << endl;
          exit(1);;
        }
        iCurrParIdx = i;
        for (int j=0; j<NsubparsMax; j++) {
          for (int k=0; k<NsubsubparsMax; k++) blSPsubparRead[j][k] = false;
        }
      }
    }

    if (iCurrParIdx == 0) {
      iSubstr = stfiline.find(stSTpars[iCurrParIdx]);
      stDF = stfiline.substr( iSubstr + stSTpars[iCurrParIdx].size() + stfiline.substr(iSubstr + stSTpars[iCurrParIdx].size()).find_first_not_of(' ') );
      blSPsubparRead[0][0] = true;
    } else if (iCurrParIdx == 1) {
      for (int i=0; i<Nsubpars[iCurrParIdx]; i++) {
        iSubstr = stfiline.find(stSTsubpars[iCurrParIdx][i]);
        if (iSubstr < stfiline.size()) {
          iNOffTmp = 0;
          istposTmp = iSubstr + stSTsubpars[iCurrParIdx][i].size();
          stTmp = stfiline.substr( istposTmp + stfiline.substr(istposTmp).find('[') + 1);
          stTmp = stTmp.substr( 0, stTmp.find_last_of("]") );
          if (i==0) {
            while (true) {
              stTmp = stTmp.substr( stTmp.find_first_not_of(' ') );
              istposTmp = stTmp.find(',');
              if (istposTmp != -1) {
                lOfftimeOut[iNOffTmp] = stoi( stTmp.substr( 0, istposTmp) );
                iNOffTmp++;
                stTmp = stTmp.substr( istposTmp + 1 );
              } else {
                lOfftimeOut[iNOffTmp] = stoi( stTmp );
                iNOffTmp++;
                break;
              }
            }
          } else if (i==1) {
            while (true) {
              istposTmp = stTmp.find('[');
              if (istposTmp == -1) break; 
              stTmp = stTmp.substr( istposTmp + 1 );
              istposTmp = stTmp.find(']');
              stTmp2 = stTmp.substr( 0, istposTmp );
              stTmp = stTmp.substr( istposTmp + 1 );
              for (int iOff=0; iOff<3; iOff++) {
                dbOffsetOut[iOff][iNOffTmp] = stod( stTmp2.substr(0, stTmp2.find(',')) );
                stTmp2 = stTmp2.substr( stTmp2.find(',') + 1 );
              }
              dbOffsetOut[3][iNOffTmp] = stod( stTmp2 );
              iNOffTmp++;
            }
          }
          if (iNOffOut < iNOffTmp) {
            for (iNOffOut=iNOffOut; iNOffOut<(iNOffTmp); iNOffOut++) {
              if (i==0) {
                for (int iOff=0; iOff<4; iOff++) dbOffsetOut[iOff][iNOffOut] = 0;
              } else if (i==1) {
                lOfftimeOut[iNOffOut] = 0;
              }
            }
          }
          blSPsubparRead[i][0] = true;
        }
      }
    } else if (iCurrParIdx == 2) {
      for (int i=0; i<Nsubpars[iCurrParIdx]; i++) {
        iSubstr = stfiline.find(stSTsubpars[iCurrParIdx][i]);
        if (iSubstr < stfiline.size()) {
          if (iCurrSubParIdx != -1) {
            cout << "Warning: New  parameter: " << stSTsubpars[iCurrParIdx][i] << " found while iCurrSubParIdx == " << iCurrSubParIdx << "." << endl;
          }
          iCurrSubParIdx = i;
        }
      }
      
      if (iCurrSubParIdx != -1) {
        for (int j=0; j<Nsubsubpars[iCurrParIdx][iCurrSubParIdx]; j++) {
          iSubstr = stfiline.find(stSTsubsubpars[iCurrParIdx][iCurrSubParIdx][j]);
          if (iSubstr < stfiline.size()) {
            if (j < (Nsubsubpars[iCurrParIdx][iCurrSubParIdx] - 1)) {
              iDFcIdx[iCurrSubParIdx*2 + j] = stoi(stfiline.substr(iSubstr + stSTsubsubpars[iCurrParIdx][iCurrSubParIdx][j].size() + 0));
            } else {
              dbDFcScale[iCurrSubParIdx] = stod(stfiline.substr(iSubstr + stSTsubsubpars[iCurrParIdx][iCurrSubParIdx][j].size() + 0));;
            }
            blSPsubparRead[iCurrSubParIdx][j] = true;
          }
        }
      }
    } else if (iCurrParIdx == 3) {
      iSubstr = stfiline.find(stSTpars[iCurrParIdx]);
      stDFtitle = stfiline.substr( iSubstr + stSTpars[iCurrParIdx].size() + stfiline.substr(iSubstr + stSTpars[iCurrParIdx].size()).find_first_not_of(' ') );
      blSPsubparRead[0][0] = true;
    }
    
    
    if (iCurrParIdx > -1) {
      for (int i=0; i<Nsubpars[iCurrParIdx]; i++) {
        blSubSubCheck[i] = blSPsubparRead[i][0];
        if (iCurrSubParIdx == i) {
          for (int j=1; j<Nsubsubpars[iCurrParIdx][iCurrSubParIdx]; j++) blSubSubCheck[i] = blSubSubCheck[i] && blSPsubparRead[i][j];
          if (blSubSubCheck[i]) iCurrSubParIdx = -1;
        }
      }

      blSubCheck = blSPsubparRead[0][0];     
      for (int i=0; i<Nsubpars[iCurrParIdx]; i++) {
        blSubCheck = blSubCheck && blSubSubCheck[i];
      }
      if (blSubCheck) iCurrParIdx = -1;
    }
    
  } //end loop over file lines
  
  fin.close();
  
  return;
}

void readDataSteerNom (string stFile, string& stDF, string* stDFcName, int* iDFcIdx, double* dbDFcScale, int& iNscansOut, string stScanNamesOut[NscansMax], long lScanTimesOut[NscansMax][2]) {
  
  ifstream fin;
  string stfiline;
  
  const int Npar = 3;
  string stPars[Npar] = {"Data file:", "Scan:", "Columns:"};
  int NsubPar[Npar]   = {     0,          2,      Nvars};
  int iCurrParIdx = -1;
  
  string stScanSubPars[2] = {"Names:", "TimeWindows:"};
  int iNscansTmp, istposTmp;
  string stTmp, stTmp2;
  
  fin.open(stFile.c_str());
  bool blNeedScale = false;
  int iSubstr, iCurrCidx;
  while(getline(fin, stfiline)) {
      
    for (int i=0; i<Npar; i++) {
      iSubstr = stfiline.find(stPars[i]);
      if (iSubstr < stfiline.size()) {
        iCurrParIdx = i;
        if (iCurrParIdx == 2) {
          iCurrCidx = -1;
          blNeedScale = false;
        }
      }
    }
      
    if (iCurrParIdx == 0) {
      iSubstr = stfiline.find(stPars[iCurrParIdx]);
      if (iSubstr < stfiline.size()) {
        stDF = stfiline.substr( iSubstr + stPars[iCurrParIdx].size() + stfiline.substr(iSubstr + stPars[iCurrParIdx].size()).find_first_not_of(' ') );
      }
    } else if (iCurrParIdx == 1) {
      for (int i=0; i<NsubPar[iCurrParIdx]; i++) {
        iSubstr = stfiline.find(stScanSubPars[i]);
        if (iSubstr < stfiline.size()) {
          iNscansTmp = 0;
          istposTmp = iSubstr + stScanSubPars[i].size();
          stTmp = stfiline.substr( istposTmp + stfiline.substr(istposTmp).find('[') + 1);
          stTmp = stTmp.substr( 0, stTmp.find_last_of("]") );
//          cout << stTmp << endl;
          if (i==0) {
            while (true) {
              istposTmp = stTmp.find('"') + 1;
              if (istposTmp >= stTmp.size()) break; 
              stTmp = stTmp.substr( istposTmp );
              stScanNamesOut[iNscansTmp] = stTmp.substr( 0, stTmp.find('"'));
              stTmp = stTmp.substr( stTmp.find('"') + 1 );
              iNscansTmp++;
            }
          } else if (i==1) {
            while (true) {
              istposTmp = stTmp.find('[') + 1;
              if (istposTmp >= stTmp.size()) break; 
              stTmp = stTmp.substr( istposTmp );
              lScanTimesOut[iNscansTmp][0] = sciToLong( stTmp.substr(0, stTmp.find(',')) );
              stTmp = stTmp.substr( stTmp.find(',') + 1 );
              lScanTimesOut[iNscansTmp][1] = sciToLong( stTmp.substr(0, stTmp.find(']')) );
              stTmp = stTmp.substr( stTmp.find(']') + 1 );
              iNscansTmp++;
            }
          }
          if (iNscansOut < iNscansTmp) {
              //iNscansOut = iNscansTmp;
            for (iNscansOut=iNscansOut; iNscansOut<(iNscansTmp); iNscansOut++) {
              if (i==0) {
                lScanTimesOut[iNscansOut][0] = 0;
                lScanTimesOut[iNscansOut][1] = 0;
              } else if (i==1) {
                stScanNamesOut[iNscansOut] = string("-");
              }
            }
          }
        }
      }
    } else if (iCurrParIdx == 2) {
      for (int i=0; i<NsubPar[iCurrParIdx]; i++) {
        iSubstr = stfiline.find(stDFcName[i]);
        if (iSubstr < stfiline.size()) {
          if (blNeedScale) {
            cout << "ERROR: Scale was not read for iCurrCidx == " << iCurrCidx << "." << endl;
            exit(1);;
          }
          iCurrCidx = i;
          iDFcIdx[iCurrCidx] = stoi(stfiline.substr(iSubstr + stDFcName[iCurrCidx].size() + 2));
          blNeedScale = true;
        }
      }
      iSubstr = stfiline.find(string("scale:"));
      if (iSubstr < stfiline.size()) {
        if (iCurrCidx < 0) {
          cout << "ERROR: cannot read scale, iCurrCidx == " << iCurrCidx << "." << endl;
          exit(1);
        } else if (!blNeedScale) {
          cout << "ERROR: trying to read scale while blNeedScale == " << blNeedScale << "." << endl;
          exit(1);
        }
        dbDFcScale[iCurrCidx] = stod(stfiline.substr(iSubstr + 7));
        blNeedScale = false;
      }
    }
  }
  
  fin.close();
  
  return;
}

void readDataFileNom (string stFile, int NvarsIn, int* iDFcIdx_in, int iTimeRange_in[3], int& NreadPar, Double_t** dbreadVar) {
  
  ifstream fin;
  string stfiline;
  
  fin.open(stFile.c_str());
  int iSubstr, iSubstr_next, iCurrCol;
  
  int iDFcSign[NvarsIn];
  for (int i=0; i<NvarsIn; i++) {
    if (iDFcIdx_in[i] < 0) {
      iDFcSign[i] = -1;
    } else {
      iDFcSign[i] = 1;
    }
  }
  
  NreadPar = -1;
  while(getline(fin, stfiline)) {
    if (NreadPar == -1) {
      NreadPar++;
    } else {
      iCurrCol = 1;
      iSubstr = 0;
      iSubstr_next = 0;
      while (iSubstr_next > -1) {
        iSubstr_next = stfiline.find(string(","), iSubstr);
        for (int i=0; i<NvarsIn; i++) {
          if ((iDFcIdx_in[i]*iDFcSign[i]) == iCurrCol) {
            dbreadVar[i][NreadPar] = stod(stfiline.substr(iSubstr,iSubstr_next-iSubstr)) * double(iDFcSign[i]);
          }
        }
        
        iSubstr = iSubstr_next+1;
        iCurrCol++;
      } //end of one string reading
      
      if (dbreadVar[0][NreadPar] >= iTimeRange_in[1] && dbreadVar[0][NreadPar] <= iTimeRange_in[2]) {
        dbreadVar[0][NreadPar] = dbreadVar[0][NreadPar] - iTimeRange_in[0];
        NreadPar++;
      }
    }
  } //end of file reading
  
  fin.close();
  
  return;
}

void readDataFileBPM (string stFile, int NvarsIn, int NvarsOut, int* iDFcIdx_in, int iTimeRange_in[3], int& NreadPar, Double_t** dbreadVar) {
  
  Double_t** dbreadVarIn = new Double_t*[NvarsIn];
  for (int i=0; i<NvarsIn; i++)  dbreadVarIn[i] = new Double_t[NdataMax];
  
  readDataFileNom (stFile, NvarsIn, iDFcIdx_in, iTimeRange_in, NreadPar, dbreadVarIn);
  
  for (int i=0; i<NreadPar; i++) {
    dbreadVar[0][i] = dbreadVarIn[0][i];
    for (int j=1; j<NvarsOut; j++) {
      dbreadVar[j][i] = (dbreadVarIn[j*2][i] + dbreadVarIn[j*2+1][i])/2;
    }
  }
  
  for (int i=0;i<NvarsIn;i++) delete [] dbreadVarIn[i];
  delete [] dbreadVarIn;
  
  return;  
}

void sortScans (int& iNscansIn, string stScanNamesIn[NscansMax], long lScanTimesIn[NscansMax][2]) {
  
  long lTimeTmp;
  string stNameTmp;
  bool blIsSwap = true;
  
  while(blIsSwap) {
    blIsSwap = false;
    
    for (int i=0; i<(iNscansIn-1); i++) {
      if (lScanTimesIn[i][0] == 0 || lScanTimesIn[i][1] == 0) {
        for (int j=0; j<2; j++) {
          lTimeTmp = lScanTimesIn[i][j];
          lScanTimesIn[i][j] = lScanTimesIn[iNscansIn-1][j];
          lScanTimesIn[iNscansIn-1][j] = lTimeTmp;
        }
        stNameTmp = stScanNamesIn[i];
        stScanNamesIn[i] = stScanNamesIn[iNscansIn-1];
        stScanNamesIn[iNscansIn-1] = stNameTmp;
        blIsSwap = true;
        iNscansIn--;
      } else if (lScanTimesIn[i][0] > lScanTimesIn[i+1][0]) {
        for (int j=0; j<2; j++) {
          lTimeTmp = lScanTimesIn[i][j];
          lScanTimesIn[i][j] = lScanTimesIn[i+1][j];
          lScanTimesIn[i+1][j] = lTimeTmp;
        }
        stNameTmp = stScanNamesIn[i];
        stScanNamesIn[i] = stScanNamesIn[i+1];
        stScanNamesIn[i+1] = stNameTmp;
        blIsSwap = true;
      }
    }
    if (lScanTimesIn[iNscansIn-1][0] == 0 || lScanTimesIn[iNscansIn-1][1] == 0) {
      iNscansIn--;
    }
  }
  
  return;  
}

void sortDisplData (int NDispVal[2], Double_t dbDispVal[2][2][NdataMax]) {
  
  Double_t dbTmp;
  bool blIsSwap;
  
  for (int k=0; k<2; k++) {
    blIsSwap = true;
    while(blIsSwap) {
      blIsSwap = false;
    
      for (int i=0; i<(NDispVal[k]-1); i++) {
        if (dbDispVal[k][0][i] > dbDispVal[k][0][i+1]) {
          for (int j=0; j<2; j++) {
            dbTmp                = dbDispVal[k][j][i];
            dbDispVal[k][j][i]   = dbDispVal[k][j][i+1];
            dbDispVal[k][j][i+1] = dbTmp;
          }
          blIsSwap = true;
        }
      }
    }
  }
  
  return;  
}

void sortOffsets (int& iNOffIn, long lOfftimeIn[NoffMax], double dbOffsetIn[4][NoffMax]) {
  
  long lTimeTmp;
  double dbOffTmp;
  bool blIsSwap = true;
  
  while(blIsSwap) {
    blIsSwap = false;
    
    for (int i=0; i<(iNOffIn-1); i++) {
      if (lOfftimeIn[i] > lOfftimeIn[i+1]) {
        lTimeTmp = lOfftimeIn[i];
        lOfftimeIn[i] = lOfftimeIn[i+1];
        lOfftimeIn[i+1] = lTimeTmp;
          
        for (int j=0; j<4; j++) {
          dbOffTmp = dbOffsetIn[j][i];
          dbOffsetIn[j][i] = dbOffsetIn[j][i+1];
          dbOffsetIn[j][i+1] = dbOffTmp;
        }
        blIsSwap = true;
      }
    }
  }
  
  return;  
}

void calcOffsetsTime (int iNOffIn, long lOfftimeIn[NoffMax], int iPlotTimeZero, double dbOfftimeOut[NoffMax]) {
  
  double dbTscale = 0.016666667;
  
  for (int i=0; i<iNOffIn; i++) {
    dbOfftimeOut[i] = double(lOfftimeIn[i]-iPlotTimeZero) * dbTscale;
  }
  
  return;
}

int main(int argc, char **argv) {
  
  if ( argc != (Nsteer + 1) ) {
    printf("program usage:\n plotVdMorbitDrift_3p [plot steering file] [BPM data steering file 1] [BPM data steering file 2] [Nominal data steering file]\n");
    return -1;
  }
  
  string stDFcolName[Nvars] = {"Timestamp", "B1_H", "B1_V", "B2_H", "B2_V"};
  
  string stSteering[Nsteer];
  for (int i=0; i<Nsteer; i++) stSteering[i] = string(argv[i + 1]);
  
  int iPlotTimeRange[3]; // [0 - zero value, 1 - plot min, 2 - plot max]
  bool blVarDraw[4];
  int iVarColor[4];
  int iBPM2color;
  string stPlotTitle[3];
  double dbXrange[2];
  Double_t** dbYrange = new Double_t* [2];  //dbYrange[0 -> BPM, 1 -> Nominal][0 -> min, 1 -> max]
  for (int i=0; i<3; i++) dbYrange[i] = new Double_t [2];
  double dbNomYcut; // value used to recognize where Nominal BPM position is 0
  int     iODpars[2]; // Orbit Drift parameters, [0 - min time indent, 1 - max time length]
  double dbODpars[2];
  // Label variables:
  int nLabels = 0;
  string stLabels[NlabelsMax];
  double dbLabelsPos[4][NlabelsMax]; // [0 - font size, 1 - x pos, 2 - y pos 1, 3 - y pos 2 (Nominal / DOROS)]
  
  readPlotSteer (stSteering[0], iPlotTimeRange, blVarDraw, iVarColor, stPlotTitle, dbXrange, dbYrange, dbNomYcut, iBPM2color, iODpars, nLabels, stLabels, dbLabelsPos);
  
//  cout << "Labels read: " << nLabels << endl;
//  for (int il = 0; il < nLabels; il++) {
//      cout << "   " << stLabels[il] << " : " << dbLabelsPos[0][il] << " ; " << dbLabelsPos[1][il] << ", " << dbLabelsPos[2][il] << " ;" << endl;
//  }
  
  string stDataFileNom;
  int iDFcolIdxNom[Nvars]; //timestamp, B1_H, B1_V, B2_H, B2_V
  double dbDFcolScaleNom[Nvars];
  int iNscans = 0;
  string stScanNames[NscansMax];
  long lScanTimeWindows[NscansMax][2]; //[scan] [0-start, 1-end]
  long lScanTimeZones[NscansMax][2]; //[scan] [0-start, 1-end]   <- zones are larger than scan windows, they include some time between scans
  double dbScanTimeWindows[NscansMax][2];
  double dbScanTimeZones[NscansMax][2];
  
  readDataSteerNom(stSteering[3], stDataFileNom, stDFcolName, iDFcolIdxNom, dbDFcolScaleNom, iNscans, stScanNames, lScanTimeWindows);
  sortScans (iNscans, stScanNames, lScanTimeWindows);
  
  if (iPlotTimeRange[0] < lScanTimeWindows[0][0]) {
    lScanTimeZones[0][0] = iPlotTimeRange[0];
  } else {
    lScanTimeZones[0][0] = lScanTimeWindows[0][0];
  }
  for (int i=0; i<(iNscans-1); i++) {
    lScanTimeZones[i][1] = long((lScanTimeWindows[i][1] + lScanTimeWindows[i+1][0])/2);
    lScanTimeZones[i+1][0] = lScanTimeZones[i][1];
  }
  if (iPlotTimeRange[2] > lScanTimeWindows[iNscans-1][1]) {
    lScanTimeZones[iNscans-1][1] = iPlotTimeRange[2];
  } else {
    lScanTimeZones[iNscans-1][1] = lScanTimeWindows[iNscans-1][1];
  }
  
  int iMinLZ, iLZtmp;
  for (int i=0; i<(iNscans); i++) {
    iMinLZ = lScanTimeWindows[i][0] - lScanTimeZones[i][0];
    iLZtmp = lScanTimeZones[i][1] - lScanTimeWindows[i][1];

    if ( abs(iMinLZ - iLZtmp) > 150 ) {
      if (iLZtmp < iMinLZ) iMinLZ = iLZtmp;    
      lScanTimeZones[i][0] = lScanTimeWindows[i][0] - iMinLZ;
      lScanTimeZones[i][1] = lScanTimeWindows[i][1] + iMinLZ;
    }
  }

//Convert Scans time from Timestamp to minutes
  for (int i=0; i<(iNscans); i++) {
    for (int j=0; j<2; j++) {
      dbScanTimeWindows[i][j] = ( double(lScanTimeWindows[i][j] - iPlotTimeRange[0]) )*dbDFcolScaleNom[0];
      dbScanTimeZones[i][j]   = ( double(lScanTimeZones[i][j]   - iPlotTimeRange[0]) )*dbDFcolScaleNom[0];
    }
  }
  
  int NvarValNom = 0;
  Double_t** dbVarInNom = new Double_t* [Nvars];  //timestamp, B1_H, B1_V, B2_H, B2_V
  for (int i=0; i<Nvars; i++) dbVarInNom[i] = new Double_t [NdataMax];
  
  readDataFileNom (stDataFileNom, Nvars, iDFcolIdxNom, iPlotTimeRange, NvarValNom, dbVarInNom);
  
  for (int i=0; i<Nvars; i++) {
    for (int j=0; j<NvarValNom; j++) {
      dbVarInNom[i][j] = dbVarInNom[i][j] * dbDFcolScaleNom[i];
    }
  }
  
//calculate Orbit Drift parameters in minutes
  for (int i=0; i<2; i++) dbODpars[i] = double(iODpars[i]) * dbDFcolScaleNom[0];
  /*
  cout << "Orbit Drift parameters:  " << iODpars[0] << "  " << iODpars[1] << "  |  " << dbODpars[0] << "  " << dbODpars[1] << endl;
  */
  
  string stDataFileBPM[2];  //bpm file 1 and 2
  string stDataFileTitleBPM[2];
  int iNoffBPM[2] = {0, 0};
  long lDFofftimeBPM[2][NoffMax];
  double dbDFofftimeBPM[2][NoffMax], dbDFoffsetBPM[2][4][NoffMax], dbCurrOff;
  int iDFcolIdxBPM[2][Nvars*2]; //timestamp, 0, B1_H_L, B1_H_R, B1_V_L, B1_V_R, B2_H_L, B2_H_R, B2_V_L, B2_V_R
  double dbDFcolScaleBPM[2][Nvars];
  
  for (int k=0; k<2; k++) {
    readDataSteerBPM (stSteering[k+1], stDataFileBPM[k], stDataFileTitleBPM[k], stDFcolName, iNoffBPM[k], lDFofftimeBPM[k], dbDFoffsetBPM[k], iDFcolIdxBPM[k], dbDFcolScaleBPM[k]);
    sortOffsets (iNoffBPM[k], lDFofftimeBPM[k], dbDFoffsetBPM[k]);
    calcOffsetsTime (iNoffBPM[k], lDFofftimeBPM[k], iPlotTimeRange[0], dbDFofftimeBPM[k]);
  }
  
  int NvarValBPM[2] = {0, 0};
  Double_t*** dbVarInBPM = new Double_t** [2];  //bpm file 1 and 2
  for (int j=0; j<2; j++) {
    dbVarInBPM[j] = new Double_t* [Nvars];  //timestamp, B1_H, B1_V, B2_H, B2_V
    for (int i=0; i<Nvars; i++) dbVarInBPM[j][i] = new Double_t [NdataMax];
  }

  for (int k=0; k<2; k++) {
    readDataFileBPM (stDataFileBPM[k], (Nvars*2), Nvars, iDFcolIdxBPM[k], iPlotTimeRange, NvarValBPM[k], dbVarInBPM[k]);
  

// Scale BPM data
    for (int i=0; i<Nvars; i++) {
      for (int j=0; j<NvarValBPM[k]; j++) {
        dbVarInBPM[k][i][j] = dbVarInBPM[k][i][j] * dbDFcolScaleBPM[k][i];
        dbCurrOff = 0;
        for (int iOff=0; iOff<iNoffBPM[k]; iOff++) if (dbVarInBPM[k][0][j] >= dbDFofftimeBPM[k][iOff]) dbCurrOff = dbDFoffsetBPM[k][i-1][iOff];
        if (i > 0) dbVarInBPM[k][i][j] = dbVarInBPM[k][i][j] - dbCurrOff;
      }
    }
  }
  
  
//prepare BPM1 and BPM2 for Nom < 'Y cut'
  int NvarValBPMcut[2][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}}; //Number of selected BPM [0, 1] values for [B1_H, B1_V, B2_H, B2_V]
  Double_t dbVarBPMcut[2][4][2][NdataMax]; // [BMP1, BPM2] [B1_H, B1_V, B2_H, B2_V] [Time, Val] [data point]
  double dbTnomRange[2];
  int iBPMstart[2] = {0, 0}; // [BMP1, BPM2]
  
  int NBPMDispCut[2] = {0, 0}, iTmp;
  string stODXY[3] = {"X", "Y", "summ"};    // [X, Y, summ]
  Double_t dbBPMDispCut[3][2][2][NdataMax]; // [X, Y, summ] [BMP1, BPM2] [Time, Val] [data point]
  bool blTmp;
  Double_t dbDispXtmp, dbDispYtmp, dbSignTmp;
  double dbdT = 0.0000001; //small time window
  
  for (int jn=0; jn<NvarValNom; jn++) {
    if (jn==0) {
      dbTnomRange[0] = dbVarInNom[0][jn];
    } else {
      dbTnomRange[0] = (dbVarInNom[0][jn-1] + dbVarInNom[0][jn])/2;
    }
    if (jn==(NvarValNom-1)) {
      dbTnomRange[1] = dbVarInNom[0][jn];
    } else {
      dbTnomRange[1] = (dbVarInNom[0][jn+1] + dbVarInNom[0][jn])/2;
    }
    
    for (int k=0; k<2; k++) {
      for (int j=iBPMstart[k]; j<NvarValBPM[k]; j++) {
        if (dbVarInBPM[k][0][j] >= dbTnomRange[1]) {
          iBPMstart[k] = j;
          break;
        } else if (dbVarInBPM[k][0][j] >= dbTnomRange[0] && dbVarInBPM[k][0][j] < dbTnomRange[1]) {
          for (int i=0; i<4; i++) {
            if (dbVarInNom[i+1][jn]*dbVarInNom[i+1][jn] < dbNomYcut*dbNomYcut) {
              dbVarBPMcut[k][i][0][NvarValBPMcut[k][i]] = dbVarInBPM[k][0][j];
              dbVarBPMcut[k][i][1][NvarValBPMcut[k][i]] = dbVarInBPM[k][i+1][j];

              NvarValBPMcut[k][i]++;
            }
          }
          
// Evaluate relative displacement for BPM1 and BPM2
          blTmp = true;

          for (int i=0; i<4; i++) blTmp = blTmp && (dbVarInNom[i+1][jn]*dbVarInNom[i+1][jn] < dbNomYcut*dbNomYcut) && (jn == 0 || dbVarInNom[i+1][jn-1]*dbVarInNom[i+1][jn-1] < dbNomYcut*dbNomYcut) && (jn == (NvarValNom-1) || dbVarInNom[i+1][jn+1]*dbVarInNom[i+1][jn+1] < dbNomYcut*dbNomYcut);
          if (blTmp) {
            dbBPMDispCut[2][k][0][NBPMDispCut[k]] = dbVarInBPM[k][0][j];
            dbDispXtmp = dbVarInBPM[k][1][j] - dbVarInBPM[k][3][j];
            dbDispYtmp = dbVarInBPM[k][2][j] - dbVarInBPM[k][4][j];
            dbBPMDispCut[2][k][1][NBPMDispCut[k]] = TMath::Sqrt(dbDispXtmp*dbDispXtmp + dbDispYtmp*dbDispYtmp);
            
            dbBPMDispCut[0][k][0][NBPMDispCut[k]] = dbVarInBPM[k][0][j];
            dbBPMDispCut[0][k][1][NBPMDispCut[k]] = dbDispXtmp;
            dbBPMDispCut[1][k][0][NBPMDispCut[k]] = dbVarInBPM[k][0][j];
            dbBPMDispCut[1][k][1][NBPMDispCut[k]] = dbDispYtmp;
            
            dbSignTmp = 1.;
            for (int iZ=0; iZ<(iNscans-1); iZ++) {
              if (dbBPMDispCut[2][k][0][NBPMDispCut[k]] > (dbScanTimeZones[iZ][0] - dbdT) && dbBPMDispCut[2][k][0][NBPMDispCut[k]] < dbScanTimeZones[iZ][1]) {
                if ( (stScanNames[iZ].find(string("X")) < stScanNames[iZ].size()) || (stScanNames[iZ].find(string("x")) < stScanNames[iZ].size()) ) {
                  if (dbDispXtmp < 0) dbSignTmp = -1.;
                } else if ( (stScanNames[iZ].find(string("Y")) < stScanNames[iZ].size()) || (stScanNames[iZ].find(string("y")) < stScanNames[iZ].size()) ) {
                  if (dbDispYtmp < 0) dbSignTmp = -1.;
                } else {
                  cout << "WARNING: neither 'X' nor 'Y' found in scan name: " << stScanNames[iZ] << ", using X axis for a sign." << endl;
                  if (dbDispXtmp < 0) dbSignTmp = -1.;
                }
                iTmp = iZ;
                break;
              }
            }
            dbBPMDispCut[2][k][1][NBPMDispCut[k]] = dbSignTmp * dbBPMDispCut[2][k][1][NBPMDispCut[k]];
            NBPMDispCut[k]++;
          }
        }
      } //end loop over NvarValBPM
    } //end loop over BPM1/2
  } //end loop over NvarValNom

  for (int i=0; i<3; i++) sortDisplData(NBPMDispCut, dbBPMDispCut[i]);
  
// Evaluate the Orbit Drifts
  double dbODBPMaveVal[3][2][NscansMax][3];     //result of the average of BPM displacements [X, Y, summ] [BPM1/2] [scan N] [0 - beginning, 1 - middle, 2 - end]
  double dbODBPMaveTime[2][NscansMax][3][2]; //time range of the average of BPM displacements [BPM1/2] [scan N] [0 - beginning, 1 - middle, 2 - end] [time range 0 - start, 1 - end]
  double dbODextrVal[3][2][NscansMax][3];        //Orbit Drifts values lineary extrapolated to scan beginning / end [X, Y, summ] [0 - (BPM1 - Nom), 1 - (BPM2 - Nom)] [scan N] [0 - beginning, 1 - middle, 2 - end]
  double dbODextrTime[NscansMax][3];       //Orbit Drifts time lineary extrapolated to scan beginning / end [scan N] [0 - beginning, 1 - middle, 2 - end]
  double dbODdTtmp, dbODtAve[3]; //dOD/dT, for linear extrapolation; beginning / end OD average time
  
  bool   blInODtw;    //in or out of OD estimation window
  int iNODaveTmp;    //number of values for OD average
  
  for (int k=0; k<2; k++) {    //BPM1/2
    for (int iSc=0; iSc<iNscans; iSc++) {    // loop over Scans
      // Evaluate Orbit Drifts before the Scans
      iNODaveTmp = 0;
      for (int iXY=0; iXY<3; iXY++) dbODBPMaveVal[iXY][k][iSc][0] = 0;
      dbODBPMaveTime[k][iSc][0][0] = 0; dbODBPMaveTime[k][iSc][0][1] = 0;
      //Search for the latest BPM candidate value before the scan
      blInODtw = false;
      for (int iDv=(NBPMDispCut[k]-1); iDv>=0; iDv--) {
        if (!blInODtw) {
          if (dbBPMDispCut[2][k][0][iDv] < (dbScanTimeWindows[iSc][0] - dbODpars[0])) {
            if (dbBPMDispCut[2][k][0][iDv] < dbScanTimeZones[iSc][0]) {
//              cout << "ERROR: latest BPM candidate time value before the scan " << stScanNames[iSc] << " is outside scan zone:" << endl;
//              cout << "     BPM t: " << dbBPMDispCut[2][k][0][iDv] << " < " << dbScanTimeZones[iSc][0] << "." << endl;
              break;
            } else {
              dbODBPMaveTime[k][iSc][0][1] = dbBPMDispCut[2][k][0][iDv] + dbdT;
              blInODtw = true;
            }
          }
        }
        if (blInODtw) {
          if (dbBPMDispCut[2][k][0][iDv] > dbScanTimeZones[iSc][0] && dbBPMDispCut[2][k][0][iDv] > (dbODBPMaveTime[k][iSc][0][1] - dbdT - dbODpars[1])) {
            for (int iXY=0; iXY<3; iXY++) dbODBPMaveVal[iXY][k][iSc][0] += dbBPMDispCut[iXY][k][1][iDv];
            iNODaveTmp++;
            dbODBPMaveTime[k][iSc][0][0] = dbBPMDispCut[2][k][0][iDv] - dbdT;
            
            if (iDv == 0) {
              for (int iXY=0; iXY<3; iXY++) dbODBPMaveVal[iXY][k][iSc][0] = dbODBPMaveVal[iXY][k][iSc][0] / iNODaveTmp;
              blInODtw = false;
              break;
            }          
          } else {
            for (int iXY=0; iXY<3; iXY++) dbODBPMaveVal[iXY][k][iSc][0] = dbODBPMaveVal[iXY][k][iSc][0] / iNODaveTmp;
            blInODtw = false;
            break;
          }
        }
      } // end of the loop over BPM data points
      
      
      // Evaluate Orbit Drifts in the middle of the Scans
      iNODaveTmp = 0;
      for (int iXY=0; iXY<3; iXY++) dbODBPMaveVal[iXY][k][iSc][1] = 0;
      dbODBPMaveTime[k][iSc][1][0] = 0; dbODBPMaveTime[k][iSc][1][1] = 0;
      //Search for the first BPM candidate value after the scan starts
      blInODtw = false;
      for (int iDv=0; iDv<NBPMDispCut[k]; iDv++) {
        if (!blInODtw) {
          if (dbBPMDispCut[2][k][0][iDv] > (dbScanTimeWindows[iSc][0] + dbODpars[0])) {
            if (dbBPMDispCut[2][k][0][iDv] > dbScanTimeZones[iSc][1]) {
//              cout << "ERROR: first BPM candidate time value after the scan start " << stScanNames[iSc] << " is outside scan zone:" << endl;
//              cout << "     BPM t: " << dbBPMDispCut[2][k][0][iDv] << " > " << dbScanTimeZones[iSc][1] << "." << endl;
            }
            dbODBPMaveTime[k][iSc][1][0] = dbBPMDispCut[2][k][0][iDv] - dbdT;
            blInODtw = true;
          }
        }
        if (blInODtw) {
          blTmp = true;
          if (dbBPMDispCut[2][k][0][iDv] > (dbScanTimeWindows[iSc][1] + dbODpars[0])) blTmp = dbBPMDispCut[2][k][0][iDv] < (dbODBPMaveTime[k][iSc][1][0] + dbdT + dbODpars[1]);
          if (blTmp) {
            for (int iXY=0; iXY<3; iXY++) dbODBPMaveVal[iXY][k][iSc][1] += dbBPMDispCut[iXY][k][1][iDv];
            iNODaveTmp++;
            dbODBPMaveTime[k][iSc][1][1] = dbBPMDispCut[2][k][0][iDv] + dbdT;
            
            if (iDv == (NBPMDispCut[k]-1)) {
              for (int iXY=0; iXY<3; iXY++) dbODBPMaveVal[iXY][k][iSc][1] = dbODBPMaveVal[iXY][k][iSc][1] / iNODaveTmp;
              blInODtw = false;
              break;  
            }
          } else {
            for (int iXY=0; iXY<3; iXY++) dbODBPMaveVal[iXY][k][iSc][1] = dbODBPMaveVal[iXY][k][iSc][1] / iNODaveTmp;
            blInODtw = false;
            break;
          }
        }
      } // end of the loop over BPM data points
      
      
      // Evaluate Orbit Drifts after the Scans
      iNODaveTmp = 0;
      for (int iXY=0; iXY<3; iXY++) dbODBPMaveVal[iXY][k][iSc][2] = 0;
      dbODBPMaveTime[k][iSc][2][0] = 0; dbODBPMaveTime[k][iSc][2][1] = 0;
      //Search for the first BPM candidate value after the scan
      blInODtw = false;
      for (int iDv=0; iDv<NBPMDispCut[k]; iDv++) {
        if (!blInODtw) {
          if (dbBPMDispCut[2][k][0][iDv] > (dbScanTimeWindows[iSc][1] + dbODpars[0])) {
            if (dbBPMDispCut[2][k][0][iDv] > dbScanTimeZones[iSc][1]) {
//              cout << "ERROR: first BPM candidate time value after the scan " << stScanNames[iSc] << " is outside scan zone:" << endl;
//              cout << "     BPM t: " << dbBPMDispCut[2][k][0][iDv] << " > " << dbScanTimeZones[iSc][1] << "." << endl;
            }
            dbODBPMaveTime[k][iSc][2][0] = dbBPMDispCut[2][k][0][iDv] - dbdT;
            blInODtw = true;
          }
        }
        if (blInODtw) {
          if (dbBPMDispCut[2][k][0][iDv] < (dbODBPMaveTime[k][iSc][2][0] + dbdT + dbODpars[1])) {
            for (int iXY=0; iXY<3; iXY++) dbODBPMaveVal[iXY][k][iSc][2] += dbBPMDispCut[iXY][k][1][iDv];
            iNODaveTmp++;
            dbODBPMaveTime[k][iSc][2][1] = dbBPMDispCut[2][k][0][iDv] + dbdT;
            
            if (iDv == (NBPMDispCut[k]-1)) {
              for (int iXY=0; iXY<3; iXY++) dbODBPMaveVal[iXY][k][iSc][2] = dbODBPMaveVal[iXY][k][iSc][2] / iNODaveTmp;
              blInODtw = false;
              break;  
            }
          } else {
            for (int iXY=0; iXY<3; iXY++) dbODBPMaveVal[iXY][k][iSc][2] = dbODBPMaveVal[iXY][k][iSc][2] / iNODaveTmp;
            blInODtw = false;
            break;
          }
        }
      } // end of the loop over BPM data points
      
    }
  }
  
  for (int k=0; k<2; k++) {    //BPM1/2
    for (int iSc=0; iSc<iNscans; iSc++) {    // loop over Scans
      dbODextrTime[iSc][1] = ((dbODBPMaveTime[0][iSc][1][0] + dbODBPMaveTime[0][iSc][1][1]) / 2 + (dbODBPMaveTime[1][iSc][1][0] + dbODBPMaveTime[1][iSc][1][1]) / 2) / 2;
      
      for (int j=0; j<2; j++) {
        dbODextrTime[iSc][j*2] = dbScanTimeWindows[iSc][j];
        
        dbODtAve[j*2] = (dbODBPMaveTime[k][iSc][j*2][0] + dbODBPMaveTime[k][iSc][j*2][1]) / 2;
      }
      for (int iXY=0; iXY<3; iXY++) {
        dbODextrVal[iXY][k][iSc][1] = dbODBPMaveVal[iXY][k][iSc][1];
        for (int j=0; j<2; j++) {
          dbODdTtmp = (dbODBPMaveVal[iXY][k][iSc][1] - dbODBPMaveVal[iXY][k][iSc][j*2]) / (dbODextrTime[iSc][1] - dbODtAve[j*2]);
          dbODextrVal[iXY][k][iSc][j*2] = dbODBPMaveVal[iXY][k][iSc][j*2] + dbODdTtmp * (dbODextrTime[iSc][j*2] - dbODtAve[j*2]);
        }
      }
    }
  }
  
  double dbODoutVal[3][4][NscansMax][3];        //Orbit Drifts values to save / end [X, Y, summ] [0 - (BPM1 - Nom), 1 - (BPM2 - Nom), 2 - (BPM2 - BPM1), 3 - (BPM2 + BPM1)/2] [scan N] [0 - beginning, 1 - middle, 2 - end]
  double dbODoutTime[4][NscansMax][3];       //Orbit Drifts times to save / end [X, Y, summ] [0 - (BPM1 - Nom), 1 - (BPM2 - Nom), 2 - (BPM2 - BPM1), 3 - (BPM2 + BPM1)/2] [scan N] [0 - beginning, 1 - middle, 2 - end]
  
  // Prepare Orbit Drifts for saving
  for (int iSc=0; iSc<iNscans; iSc++) {
    for (int j=0; j<3; j++) {
      for (int k=0; k<2; k++) {
        dbODoutTime[k][iSc][j] = (dbODBPMaveTime[k][iSc][j][0] + dbODBPMaveTime[k][iSc][j][1]) / 2;
        for (int iXY=0; iXY<3; iXY++) dbODoutVal[iXY][k][iSc][j] = dbODBPMaveVal[iXY][k][iSc][j];
      }
      for (int k=2; k<4; k++) dbODoutTime[k][iSc][j] = dbODextrTime[iSc][j];
      for (int iXY=0; iXY<3; iXY++) {
        dbODoutVal[iXY][2][iSc][j] = dbODextrVal[iXY][1][iSc][j] - dbODextrVal[iXY][0][iSc][j];
        dbODoutVal[iXY][3][iSc][j] = (dbODextrVal[iXY][1][iSc][j] + dbODextrVal[iXY][0][iSc][j]) / 2;
      }
    }
  }
  
/*
  for (int iSc=0; iSc<iNscans; iSc++) {
    cout << stScanNames[iSc] << endl;
    for (int j=0; j<3; j++) {
      for (int k=0; k<2; k++) {
        cout << "  " << stDataFileTitleBPM[k] << endl;
        cout << "      Orbit Drift for T = " << dbODBPMaveTime[k][iSc][j][0] << "-" << dbODBPMaveTime[k][iSc][j][1] << " : " << dbODBPMaveVal[iXY][k][iSc][j] << endl;
        cout << "     extrapolated for T = " << dbScanTimeWindows[iSc][j] << " : " << dbODval[iXY][k][iSc][j] << endl;
      }
      cout << "  Orbit Drifts for BPM2 - BPM1 : " << dbODval[iXY][2][iSc][j] << endl;
    }
  }
*/
  
// write orbit drifts to the files
  ofstream fout;
  string stFoutName;
  
  for (int k=0; k<4; k++) {
    stFoutName = string("OrbitDrifts_");
    if (k<2) {
      stFoutName = stFoutName + stDataFileTitleBPM[k];
    } else if (k==2) {
      stFoutName = stFoutName + stDataFileTitleBPM[1] + string("_m_") + stDataFileTitleBPM[0];
    } else if (k==3) {
      stFoutName = stFoutName + stDataFileTitleBPM[1] + string("_") + stDataFileTitleBPM[0] + string("_ave");
    }
    replace( stFoutName.begin(), stFoutName.end(), ' ', '_');
    stFoutName = stFoutName + string(".txt");
    fout.open(stFoutName.c_str());
    
    fout << endl;
    fout << "Names: [";
    for (int iSc=0; iSc<(iNscans); iSc++) {
      fout << "\"" << stScanNames[iSc] << "\"";
      if (iSc<(iNscans-1)) fout << ", ";
    }
    fout << "]" << endl;
    
    fout << endl << "TimeWindows: [";
    for (int iSc=0; iSc<(iNscans); iSc++) {
// double(<timestamp> - iPlotTimeRange[0]) )*dbDFcolScaleNom[0]
      fout << "[" << int( (dbODoutTime[k][iSc][0]/dbDFcolScaleNom[0] + iPlotTimeRange[0])+0.5) << ", " << int( (dbODoutTime[k][iSc][1]/dbDFcolScaleNom[0] + iPlotTimeRange[0])+0.5) << ", " << int( (dbODoutTime[k][iSc][2]/dbDFcolScaleNom[0] + iPlotTimeRange[0])+0.5) << "]";
      if (iSc<(iNscans-1)) fout << ",";
    }
    fout << "]" << endl;
    
    for (int iXY=0; iXY<2; iXY++) {
      fout << endl << "OrbitDrifts_" << stODXY[iXY] << ": [";
      for (int iSc=0; iSc<(iNscans); iSc++) {
        fout << "[" << dbODoutVal[iXY][k][iSc][0] << ", " << dbODoutVal[iXY][k][iSc][1] << ", " << dbODoutVal[iXY][k][iSc][2] << "]";
        if (iSc<(iNscans-1)) fout << ",";
      }
      fout << "]" << endl;
    }
    
    fout.close();
  }
  
  TCanvas *canvasmain = new TCanvas("canvasmain","canvasmain",3400,2000);
  TCanvas *canvasBPM = new TCanvas("canvasBPM","canvasBPM",3400,1000);      // canvas for the BPM plots
  
  TPad *pad[5];
  pad[0] = new TPad("pad1","Top pad",0.01,0.46,0.99,0.91);
  pad[1] = new TPad("pad2","Bottom pad",0.01,0.01,0.99,0.46);
  pad[2] = new TPad("pad3","BPM1 pad",0.01,0.01,0.99,0.85);
  pad[3] = new TPad("pad3","BPM2 pad",0.01,0.01,0.99,0.85);
  pad[4] = new TPad("pad3","BPM2 pad",0.01,0.01,0.99,0.85);
  
  for (int ip=0; ip<5; ip++) {
    pad[ip]->SetTicks(1,1);
    pad[ip]->SetTopMargin(0.02);
    pad[ip]->SetBottomMargin(0.145);
    pad[ip]->SetLeftMargin(0.06);
    pad[ip]->SetRightMargin(0.12);
  }
  
  canvasmain->cd();
  for (int ip=0; ip<2; ip++) pad[ip]->Draw();
  
  TGraph *tgVdm[Nvars - 1];
  TGraph *tgBPMDisp[3][4];      // [X, Y, summ] [BPM1 / BPM2 / copy BPM1 / copy BPM2]
  
  TLine *tlODaveLine[3][2][iNscans][3];  // Orbit Drift average line [X, Y, summ] [BPM1 / BPM2] [Scan num] [begin / middle / end] 
  TLine *tlODconnectLine[3][2][iNscans][2]; // [X, Y, summ] [BPM1 / BPM2] [Scan num] [begin / end]
  TLine *tlODzoneLine[iNscans][2][2]; // Orbit Drift zone edges [Scan N] [beginning / end] [top / bottom plot]
  
  for (int iSc=0; iSc<iNscans; iSc++) {
    for (int iXY=0; iXY<3; iXY++) {
      for (int k=0; k<2; k++) {
        for (int j=0; j<3; j++) {
          tlODaveLine[iXY][k][iSc][j] = new TLine(dbODBPMaveTime[k][iSc][j][0], dbODBPMaveVal[iXY][k][iSc][j], dbODBPMaveTime[k][iSc][j][1], dbODBPMaveVal[iXY][k][iSc][j]);
          tlODaveLine[iXY][k][iSc][j]->SetLineColor(1);
          tlODaveLine[iXY][k][iSc][j]->SetLineWidth(1);
        }
        for (int j=0; j<2; j++) {
          tlODconnectLine[iXY][k][iSc][j] = new TLine( ((dbODBPMaveTime[k][iSc][0+j][0] + dbODBPMaveTime[k][iSc][0+j][1])/2), dbODBPMaveVal[iXY][k][iSc][0+j], ((dbODBPMaveTime[k][iSc][1+j][0] + dbODBPMaveTime[k][iSc][1+j][1])/2), dbODBPMaveVal[iXY][k][iSc][1+j] );
          tlODconnectLine[iXY][k][iSc][j]->SetLineWidth(1);
        }
      }
      for (int j=0; j<2; j++) {
        tlODconnectLine[iXY][0][iSc][j]->SetLineColor(iVarColor[0]);
        tlODconnectLine[iXY][1][iSc][j]->SetLineColor(iBPM2color);
      }
    }
    
    for (int j=0; j<2; j++) {
      for (int ip=0; ip<2; ip++) {
        tlODzoneLine[iSc][j][ip] = new TLine(dbScanTimeZones[iSc][j], dbYrange[ip][0], dbScanTimeZones[iSc][j], dbYrange[ip][1]);
        tlODzoneLine[iSc][j][ip]->SetLineColor(921);
        tlODzoneLine[iSc][j][ip]->SetLineWidth(1);
      }
    }
  }

  
  string stPlotFile = "VdM_";
  string stPlotSubTitle[2] = {"for Nominal = 0 in", "Nominal beams position in"};
  string stPlotTitleFin;

  string stPlotYLeg[4]  = {"X-plane displacement [#mu m]", "Y-plane displacement [#mu m]", "l , #mu m", "l , #mu m"};
  double dbPlotYLegY[4] = {0.144,          0.144,           0.680,          0.680};
  double dbPlotXLegX[4] = {0.825,          0.825,           0.830,          0.830};  

  double dbLegSep = 0.052;
  TLegend* tLeg;
  TLatex tLabel;
  
  
  for (int i=0; i<(Nvars - 1); i++) {
    tgVdm[i] = new TGraph(NvarValNom, dbVarInNom[0], dbVarInNom[i+1]);
      
    tgVdm[i]->SetMarkerStyle(8);
    tgVdm[i]->SetMarkerSize(1.2);
    tgVdm[i]->SetMarkerColor(iVarColor[i]);
    tgVdm[i]->SetLineColor(iVarColor[i]);
    tgVdm[i]->SetLineWidth(2);
    
    tgVdm[i]->SetTitle("");
    tgVdm[i]->GetXaxis()->SetLimits(dbXrange[0], dbXrange[1]);
    tgVdm[i]->SetMinimum(dbYrange[1][0]);
    tgVdm[i]->SetMaximum(dbYrange[1][1]);
  }
  
//prepare graph for displacement
  for (int iXY=0; iXY<3; iXY++) {
    for (int k=0; k<2; k++) {    //loop over BPM1/2
      tgBPMDisp[iXY][k] = new TGraph(NBPMDispCut[k], dbBPMDispCut[iXY][k][0], dbBPMDispCut[iXY][k][1]);
      tgBPMDisp[iXY][k+2] = new TGraph(NBPMDispCut[k], dbBPMDispCut[iXY][k][0], dbBPMDispCut[iXY][k][1]);
      
      tgBPMDisp[iXY][k]->SetTitle("");
      tgBPMDisp[iXY][k]->GetXaxis()->SetLimits(dbXrange[0], dbXrange[1]);
      
      tgBPMDisp[iXY][k]->GetXaxis()->SetLabelSize(0.06);
      tgBPMDisp[iXY][k]->GetYaxis()->SetLabelSize(0.06);
      
      tgBPMDisp[iXY][k]->SetMinimum(dbYrange[0][0]);
      tgBPMDisp[iXY][k]->SetMaximum(dbYrange[0][1]);
    }
    tgBPMDisp[iXY][0]->SetMarkerStyle(8);
    tgBPMDisp[iXY][0]->SetMarkerSize(1.2);
    tgBPMDisp[iXY][0]->SetMarkerColor(iVarColor[0]);
    
    tgBPMDisp[iXY][1]->SetMarkerStyle(33);
    tgBPMDisp[iXY][1]->SetMarkerSize(1.8);
    tgBPMDisp[iXY][1]->SetMarkerColor(iBPM2color);
    
    tgBPMDisp[iXY][2]->SetMarkerStyle(8);
    tgBPMDisp[iXY][2]->SetMarkerSize(3.6);
    tgBPMDisp[iXY][2]->SetMarkerColor(iVarColor[0]);
    
    tgBPMDisp[iXY][3]->SetMarkerStyle(33);
    tgBPMDisp[iXY][3]->SetMarkerSize(5.4);
    tgBPMDisp[iXY][3]->SetMarkerColor(iBPM2color);
  }
  
  TLine* tLn = new TLine(dbXrange[0], 0., dbXrange[1], 0.);
  int NlegEntr;

    // Draw Orbit Drifts:
  int ip = 0;
  
    for (int iXY=0; iXY<2; iXY++) {
  
        NlegEntr = 0;
        ip = iXY;
        pad[ip]->cd();
    
        tgBPMDisp[iXY][0]->Draw("AP");
    
        for (int iSc=0; iSc<iNscans; iSc++) {
            for (int j=0; j<2; j++) {
                if (dbScanTimeZones[iSc][j] > (dbXrange[0] - dbdT) && dbScanTimeZones[iSc][j] < (dbXrange[1] + dbdT)) {
                    tlODzoneLine[iSc][j][0]->Draw("same");
                }
            }
        }
    
        tLn->Draw("same");
    
        for (int k=0; k<2; k++) {
            tgBPMDisp[iXY][k]->Draw("P same");
            NlegEntr++;
          
            for (int iSc=0; iSc<iNscans; iSc++) {
                if (dbScanTimeWindows[iSc][0] > (dbXrange[0] - dbdT) && dbScanTimeWindows[iSc][1] < (dbXrange[1] + dbdT)) {
                    for (int j=0; j<2; j++) {
                        tlODconnectLine[iXY][k][iSc][j]->Draw("same");
                    }
                    for (int j=0; j<3; j++) {
                        tlODaveLine[iXY][k][iSc][j]->Draw("same");
                    }
                }
            }
        }
  
        tLeg = new TLegend(0.890,(0.5 - NlegEntr*dbLegSep),0.997,(0.5 + NlegEntr*dbLegSep));

        for (int k=0; k<2; k++) {
            tLeg->AddEntry(tgBPMDisp[iXY][k+2],string(stDataFileTitleBPM[k]).c_str(),"p");
        }
        tLeg->SetTextSize(0.065);
        tLeg->SetTextAlign(12);
        tLeg->SetBorderSize(0);
        tLeg->Draw();
  
        tLabel.SetNDC(1);
        tLabel.SetTextAlign(11);
        tLabel.SetTextFont(22);
        tLabel.SetTextSize(0.076);
        tLabel.DrawLatex(dbPlotXLegX[ip],0.025,"t [min]");
        tLabel.SetTextAngle(90);
        tLabel.DrawLatex(0.018,dbPlotYLegY[ip],stPlotYLeg[ip].c_str());
        tLabel.SetTextAngle(0);
        
        tLabel.SetNDC(0);
        tLabel.SetTextAlign(22);
        for (int il = 0; il < nLabels; il++) {
            tLabel.SetTextSize(dbLabelsPos[0][il]);
            tLabel.DrawLatex(dbLabelsPos[1][il], dbLabelsPos[2][il], stLabels[il].c_str());
        }
    }
    
    canvasmain->cd();
    
    if (stPlotTitle[0] != string("None")) {
        tLabel.SetTextAlign(21);
        tLabel.SetTextAngle(0);
        tLabel.SetTextSize(0.043);
        stPlotTitleFin = stDataFileTitleBPM[0] + string(" and ") + stDataFileTitleBPM[1] + string(" Orbit Drifts in ") + stPlotTitle[0];
        tLabel.DrawLatex(0.472, 0.934, stPlotTitleFin.c_str());
    }
    
/*    
    TBox* tbdraw = new TBox(0.676,0.819,0.835,0.864);
    tbdraw->SetFillColor(0);
    tbdraw->Draw("same");
    
    tLabel.SetTextAngle(0);
    tLabel.SetTextSize(0.037);
    tLabel.DrawLatex(0.68,0.832,"#font[22]{CMS} #font[12]{Preliminary}");
*/
    
    canvasmain->Print( string(stPlotFile + "OrbitDrift_XY.eps").c_str(), "eps");
  
  // Draw BPM1:
  TGraph *tgVdmBPM[2][Nvars - 1];   // BPM 1/2; variable
  
  for (int ibpm = 0; ibpm < 2; ibpm++) {
    
    canvasBPM->Clear();
    canvasBPM->cd();
    pad[ibpm + 2]->Draw();
  
    for (int i=0; i<(Nvars - 1); i++) {
        tgVdmBPM[ibpm][i] = new TGraph(NvarValBPM[ibpm], dbVarInBPM[ibpm][0], dbVarInBPM[ibpm][i+1]);
      
        tgVdmBPM[ibpm][i]->SetMarkerStyle(8);
        tgVdmBPM[ibpm][i]->SetMarkerSize(1.2);
        tgVdmBPM[ibpm][i]->SetMarkerColor(iVarColor[i]);
        tgVdmBPM[ibpm][i]->SetLineColor(iVarColor[i]);
        tgVdmBPM[ibpm][i]->SetLineWidth(2);
    
        tgVdmBPM[ibpm][i]->SetTitle("");
        tgVdmBPM[ibpm][i]->GetXaxis()->SetLimits(dbXrange[0], dbXrange[1]);
        if ( stDataFileTitleBPM[ibpm] == string("arcBPM") ) {       // not a clean solution...
            tgVdmBPM[ibpm][i]->SetMinimum(dbYrange[0][0]);
            tgVdmBPM[ibpm][i]->SetMaximum(dbYrange[0][1]);
        } else {
            tgVdmBPM[ibpm][i]->SetMinimum(dbYrange[1][0]);
            tgVdmBPM[ibpm][i]->SetMaximum(dbYrange[1][1]); 
        }
    }
    NlegEntr = 0;
  
    pad[ibpm + 2]->cd();
    tgVdmBPM[ibpm][0]->Draw("AL");
    for (int iSc=0; iSc<iNscans; iSc++) {
        for (int j=0; j<2; j++) {
            if (dbScanTimeZones[iSc][j] > (dbXrange[0] - dbdT) && dbScanTimeZones[iSc][j] < (dbXrange[1] + dbdT)) {
                if ( stDataFileTitleBPM[ibpm] == string("arcBPM") ) {       // not a clean solution...
                    tlODzoneLine[iSc][j][0]->Draw("same");
                } else {
                    tlODzoneLine[iSc][j][1]->Draw("same");
                }
            }
        }
    }
    tLn->Draw("same");
    for (int i=0; i<(Nvars - 1); i++) {
        tgVdmBPM[ibpm][i]->Draw(string("L same").c_str());
        NlegEntr++;
    }
    tLeg = new TLegend(0.900,(0.5 - NlegEntr*dbLegSep),0.997,(0.5 + NlegEntr*dbLegSep));
    for (int i=0; i<(Nvars - 1); i++) {
        tLeg->AddEntry(tgVdmBPM[ibpm][i],stDFcolName[i+1].c_str(),"l");
    }
    tLeg->SetTextSize(0.05);
    tLeg->SetTextAlign(12);
    tLeg->SetBorderSize(0);
    tLeg->Draw();
    tLabel.SetNDC(1);
    tLabel.SetTextAlign(11);
    tLabel.SetTextFont(22);
    tLabel.SetTextSize(0.076);
    tLabel.DrawLatex(dbPlotXLegX[ibpm + 2],0.036,"t [min]");
    tLabel.SetTextAngle(90);
    tLabel.DrawLatex(0.018,dbPlotYLegY[ibpm + 2],stPlotYLeg[ibpm + 2].c_str());
    tLabel.SetTextAngle(0);
    
    tLabel.SetNDC(0);
    tLabel.SetTextAlign(22);
    for (int il = 0; il < nLabels; il++) {
        tLabel.SetTextSize(dbLabelsPos[0][il]);
        if ( stDataFileTitleBPM[ibpm] == string("arcBPM") ) {       // not a clean solution...
            tLabel.DrawLatex(dbLabelsPos[1][il], dbLabelsPos[2][il], stLabels[il].c_str());
        } else {
            tLabel.DrawLatex(dbLabelsPos[1][il], dbLabelsPos[3][il], stLabels[il].c_str());
        }
    }
  
    canvasBPM->cd();
    
    if (stPlotTitle[0] != string("None")) {
        tLabel.SetTextAlign(21);
        tLabel.SetTextSize(0.07);
        stPlotTitleFin = stDataFileTitleBPM[ibpm] + string(" beams position in") + string(" ") + stPlotTitle[0];
        tLabel.DrawLatex(0.472, 0.900, stPlotTitleFin.c_str());
    }

    canvasBPM->Print( string(stPlotFile + stDataFileTitleBPM[ibpm] + ".eps").c_str(), "eps");
  }
  
  // Draw Nominal:
    NlegEntr = 0;
    canvasBPM->Clear();
    canvasBPM->cd();
    pad[4]->Draw();
    pad[4]->cd();
    tgVdm[0]->Draw("AL");
    for (int iSc=0; iSc<iNscans; iSc++) {
        for (int j=0; j<2; j++) {
            if (dbScanTimeZones[iSc][j] > (dbXrange[0] - dbdT) && dbScanTimeZones[iSc][j] < (dbXrange[1] + dbdT)) {
                tlODzoneLine[iSc][j][1]->Draw("same");
            }
        }
    }
    tLn->Draw("same");
    for (int i=0; i<(Nvars - 1); i++) {
        tgVdm[i]->Draw(string("L same").c_str());
        NlegEntr++;
    }
    tLeg = new TLegend(0.900,(0.5 - NlegEntr*dbLegSep),0.997,(0.5 + NlegEntr*dbLegSep));
    for (int i=0; i<(Nvars - 1); i++) {
        tLeg->AddEntry(tgVdm[i],stDFcolName[i+1].c_str(),"l");
    }
    tLeg->SetTextSize(0.05);
    tLeg->SetTextAlign(12);
    tLeg->SetBorderSize(0);
    tLeg->Draw();
    tLabel.SetNDC(1);
    tLabel.SetTextAlign(11);
    tLabel.SetTextFont(22);
    tLabel.SetTextSize(0.061);
    tLabel.DrawLatex(dbPlotXLegX[1],0.025,"t , min");
    tLabel.SetTextAngle(90);
    tLabel.DrawLatex(0.018,dbPlotYLegY[1],stPlotYLeg[1].c_str());
    tLabel.SetTextAngle(0);
    
    tLabel.SetNDC(0);
    tLabel.SetTextAlign(22);
    for (int il = 0; il < nLabels; il++) {
        tLabel.SetTextSize(dbLabelsPos[0][il]);
        tLabel.DrawLatex(dbLabelsPos[1][il], dbLabelsPos[3][il], stLabels[il].c_str());
    }
    
    canvasBPM->cd();
    
    if (stPlotTitle[0] != string("None")) {
        tLabel.SetTextAlign(21);
        tLabel.SetTextSize(0.07);
        stPlotTitleFin = stPlotSubTitle[1] + string(" ") + stPlotTitle[0];
        tLabel.DrawLatex(0.472, 0.900, stPlotTitleFin.c_str());
    }
    
    canvasBPM->Print( string(stPlotFile + "Nominal.eps").c_str(), "eps");
  
// remove pointers
  for (int i=0; i<2; i++) delete [] dbYrange[i];
  delete [] dbYrange;

  for (int i=0; i<Nvars; i++) delete [] dbVarInNom[i];
  delete [] dbVarInNom;
  
  for (int j=0; j<2; j++) {
    for (int i=0; i<Nvars; i++) delete [] dbVarInBPM[j][i];
    delete [] dbVarInBPM[j];
  }
  delete [] dbVarInBPM;
  
  return 0;
}

