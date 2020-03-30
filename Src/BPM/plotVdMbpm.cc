
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <time.h>

#include "TCanvas.h"
#include "TGraph.h"
#include "TAxis.h"

#include "TStyle.h"
//#include "TROOT.h"
#include "TPaveLabel.h"
#include "TLatex.h"
#include "TLegend.h"

using namespace std;

  const int Nvars    = 5; //variables to be read: timestamp, B1_H, B1_V, B2_H, B2_V
  const int NvarsBPM = 9; //variables to be read: timestamp, B1_H_L, B1_H_R, B1_V_L, B1_V_R, B2_H_L, B2_H_R, B2_V_L, B2_V_R
  const int NdataMax = 100000;
  const int Nsteer   = 3;
  const int NoffMax  = 100;

void readPlotSteer (string stFile, int* iTimeRange, bool* blPdraw, int* iPcolor, string* stTitle, double* dbXaxisRange, double* dbYaxisRange, double& dbNomYcutOut) {
  
  const int Npar = 9;
  const int NsubparsMax = 3;
  
  ifstream fin;
  string stfiline;
  
  fin.open(stFile.c_str());
  bool blSPsubparRead[NsubparsMax]; //boolean flags if all parameters (zero value + min/max time, draw/color) are read.
  int iSubstr, iCurrIdx = -1; //iCurrIdx: 0 - Timestamp, 1-4 - B1_H, B1_V, B2_H, B2_V
  string stSTpars[Npar] = {"Timestamp:", "B1_H:", "B1_V:", "B2_H:", "B2_V:", "Plot info:", "X axis:", "Y axis:", "Nominal data:"};
  int Nsubpars[Npar] = {      3,            2,       2,       2,       2,         2,          2,         2,            1};
  string stSTtimepars[3] = {"zero value:", "plot min:", "plot max:"};
  string stSTaxispars[2] = {"min:", "max:"};
  string stSTplotInfo[2] = {"pretitle:", "endtitle:"};
  
  bool blCheck;
  
  while(getline(fin, stfiline)) {

    for (int i=0; i<Npar; i++) {
      iSubstr = stfiline.find(stSTpars[i]);
      if (iSubstr < stfiline.size()) {
        if (iCurrIdx != -1) {
          cout << "ERROR: New parameter: " << stSTpars[i] << " found while iCurrIdx == " << iCurrIdx << "." << endl;
          exit(1);;
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
          stTitle[i] = stfiline.substr(iSubstr + stSTplotInfo[i].size() + stfiline.substr(iSubstr + stSTplotInfo[i].size()).find_first_not_of(' ') );
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
    } else if (iCurrIdx == 7) {
      for (int i=0; i<Nsubpars[iCurrIdx]; i++) {
        iSubstr = stfiline.find(stSTaxispars[i]);
        if (iSubstr < stfiline.size()) {
          dbYaxisRange[i] = stod(stfiline.substr(iSubstr + stSTaxispars[i].size() + 0));
          blSPsubparRead[i] = true;
        }
      }
    } else if (iCurrIdx == 8) {
      iSubstr = stfiline.find(string("Y cut:"));
      if (iSubstr < stfiline.size()) {
        dbNomYcutOut = stod(stfiline.substr(iSubstr + 6));
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

void readDataSteerBPM (string stFile, string& stDF, string stDFcName[Nvars], int& iNOffOut, long lOfftimeOut[NoffMax], double dbOffsetOut[4][NoffMax], int iDFcIdx[Nvars*2], double dbDFcScale[Nvars]) {
  
  const int Npar = 3;
  const int NsubparsMax = 5;
  const int NsubsubparsMax = 3;
  
  ifstream fin;
  string stfiline;
  
  fin.open(stFile.c_str());
  bool blSPsubparRead[NsubparsMax][NsubsubparsMax]; //boolean flags if all parameters (zero value + min/max time, draw/color) are read.
  int iSubstr, iCurrParIdx = -1, iCurrSubParIdx = -1;
  string stSTpars[Npar] = {"Data file:", "Offsets:", "Columns:" };
  int Nsubpars[Npar] = {      0,            2,       NsubparsMax};
  string stSTsubpars[Npar][NsubparsMax] = {{"-","-","-","-","-"}, {"Timestamps:","OffValues:","-","-","-"}, {stDFcName[0],stDFcName[1],stDFcName[2],stDFcName[3],stDFcName[4]}};
  int Nsubsubpars[Npar][NsubparsMax] =    {{ 0 , 0 , 0 , 0 , 0 }, { 0 , 0 , 0 , 0 , 0 }, { 2 , 3 , 3 , 3 , 3 }};
  string stSTsubsubpars[Npar][NsubparsMax][NsubsubparsMax] = {{{"-","-","-"}, {"-","-","-"}, {"-","-","-"}, {"-","-","-"}, {"-","-","-"}}, {{"-","-","-"}, {"-","-","-"}, {"-","-","-"}, {"-","-","-"}, {"-","-","-"}}, {{"column:","scale:","-"}, {"L:","R:","scale:"}, {"L:","R:","scale:"}, {"L:","R:","scale:"}, {"L:","R:","scale:"}}};
  
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

void readDataSteerNom (string stFile, string& stDF, string* stDFcName, int* iDFcIdx, double* dbDFcScale) {
  
  ifstream fin;
  string stfiline;
  
  fin.open(stFile.c_str());
  bool blFNread = false, blNeedScale = false;
  int iSubstr, iCurrCidx;
  while(getline(fin, stfiline)) {
    if (!blFNread) {
      iSubstr = stfiline.find(string("Data file:"));
      if (iSubstr < stfiline.size()) {
        stDF = stfiline.substr( iSubstr + 10 + stfiline.substr(iSubstr + 10).find_first_not_of(' ') );
        blFNread = true;
        iCurrCidx = -1;
        blNeedScale = false;
      }
    } else {
      for (int i=0; i<Nvars; i++) {
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
    printf("program usage:\n plotVdMnom [plot steering file] [BPM data steering file] [Nominal data steering file]\n");
    return -1;
  }
  
  string stDFcolName[Nvars] = {"Timestamp", "B1_H", "B1_V", "B2_H", "B2_V"};
  
  string stSteering[Nsteer];
  for (int i=0; i<Nsteer; i++) stSteering[i] = string(argv[i + 1]);
  
  int iPlotTimeRange[3]; //zero value, plot min, plot max
  bool blVarDraw[4];
  int iVarColor[4];
  string stPlotTitle[2];
  double dbXrange[2], dbYrange[2];
  double dbNomYcut; // value used to recognize where Nominal BPM position is 0
  
  readPlotSteer (stSteering[0], iPlotTimeRange, blVarDraw, iVarColor, stPlotTitle, dbXrange, dbYrange, dbNomYcut);
  
/*  
  cout << "dbNomYcut = " << dbNomYcut << endl << endl;
  cout << "Plot title: " << stPlotTitle[0] << " ; " << stPlotTitle[1] << endl;
  cout << stDFcolName[0] << "Zero value: " << iPlotTimeRange[0] << ", plot from " << iPlotTimeRange[1] << " to " << iPlotTimeRange[2] << "." << endl;
  cout << "X axis from " << dbXrange[0] << " to " << dbXrange[1] << "; Y axis from " << dbYrange[0] << " to " << dbYrange[1] << "." << endl;
  for (int i=0; i<4; i++) cout << stDFcolName[i+1] << " draw: " << blVarDraw[i] << ", color: " << iVarColor[i] << "." << endl;
*/

  string stDataFileNom;
  int iDFcolIdxNom[Nvars]; //timestamp, B1_H, B1_V, B2_H, B2_V
  double dbDFcolScaleNom[Nvars];
  
  readDataSteerNom(stSteering[2], stDataFileNom, stDFcolName, iDFcolIdxNom, dbDFcolScaleNom);
  
  
  string stDataFileBPM;
  int iNoffBPM = 0;
  long lDFofftimeBPM[NoffMax];
  double dbDFofftimeBPM[NoffMax], dbDFoffsetBPM[4][NoffMax], dbCurrOff;
  int iDFcolIdxBRM[Nvars*2]; //timestamp, 0, B1_H_L, B1_H_R, B1_V_L, B1_V_R, B2_H_L, B2_H_R, B2_V_L, B2_V_R
  double dbDFcolScaleBPM[Nvars];
  
  readDataSteerBPM (stSteering[1], stDataFileBPM, stDFcolName, iNoffBPM, lDFofftimeBPM, dbDFoffsetBPM, iDFcolIdxBRM, dbDFcolScaleBPM);
  sortOffsets (iNoffBPM, lDFofftimeBPM, dbDFoffsetBPM);
  calcOffsetsTime (iNoffBPM, lDFofftimeBPM, iPlotTimeRange[0], dbDFofftimeBPM);

/*
  cout << stDataFileBPM << endl;
  for (int j=0; j<iNoffBPM; j++) {
    cout << "Time = " << lDFofftimeBPM[j] << "  =  " << dbDFofftimeBPM[j] << endl;
    for (int i=0; i<4; i++) cout << "   " << stDFcolName[i+1] << "   " << dbDFoffsetBPM[i][j]  << endl;
  }
  for (int i=0; i<Nvars; i++) cout << stDFcolName[i] << "   " << iDFcolIdxBRM[i*2] << "   " << iDFcolIdxBRM[i*2 + 1] << "   " << dbDFcolScaleBPM[i] << endl;
*/

  int NvarValNom = 0;
  Double_t** dbVarInNom = new Double_t* [Nvars];  //timestamp, B1_H, B1_V, B2_H, B2_V
  for (int i=0; i<Nvars; i++) dbVarInNom[i] = new Double_t [NdataMax];
  
  readDataFileNom (stDataFileNom, Nvars, iDFcolIdxNom, iPlotTimeRange, NvarValNom, dbVarInNom);

/*  
  cout << endl << "  NvarValNom = " << NvarValNom << endl;
  for (int i=21; i<22; i++) {
    for (int j=0; j<Nvars; j++) cout << dbVarInNom[j][i] << "   ";
    cout << endl;
  }
*/
  
  for (int i=0; i<Nvars; i++) {
    for (int j=0; j<NvarValNom; j++) {
      dbVarInNom[i][j] = dbVarInNom[i][j] * dbDFcolScaleNom[i];
    }
  }

  int NvarValBPM = 0;
  Double_t** dbVarInBPM = new Double_t* [Nvars];  //timestamp, B1_H, B1_V, B2_H, B2_V
  for (int i=0; i<Nvars; i++) dbVarInBPM[i] = new Double_t [NdataMax];

  readDataFileBPM (stDataFileBPM, (Nvars*2), Nvars, iDFcolIdxBRM, iPlotTimeRange, NvarValBPM, dbVarInBPM);

/*
  cout << endl << "  NvarValBPM = " << NvarValBPM << endl;
  for (int i=0; i<NvarValBPM; i++) {
    for (int j=0; j<Nvars; j++) cout << dbVarInBPM[j][i] << "   ";
    cout << endl;
  }
*/
  
// Scale BPM data
  for (int j=0; j<NvarValBPM; j++) {
    for (int i=0; i<Nvars; i++) {
      dbVarInBPM[i][j] = dbVarInBPM[i][j] * dbDFcolScaleBPM[i];
      dbCurrOff = 0;
      for (int k=0; k<iNoffBPM; k++) if (dbVarInBPM[0][j] >= dbDFofftimeBPM[k]) dbCurrOff = dbDFoffsetBPM[i-1][k];
      if (i > 0) dbVarInBPM[i][j] = dbVarInBPM[i][j] - dbCurrOff;
    }
  }
  
// Evaluation of the new offsets.
  double dbNEWoffsetBPM[4][NoffMax];
  bool blNomAtZero[4] = {false, false, false, false};
  int NvarValBPMcut[4] = {0, 0, 0, 0}; //Number of selected BPM values for B1_H, B1_V, B2_H, B2_V
  Double_t dbVarInBPMcut[4][2][NdataMax]; // [B1_H, B1_V, B2_H, B2_V] [Time, data] [data point]
  
  int iBPMstart = 0;
  bool blAllNomAtZero = false;
  
  bool blNewOffSet[NoffMax];
    for (int iO=0; iO<NoffMax; iO++) blNewOffSet[iO] = false;
  
    double dbTnomRange[2];
    
    for (int i=0; i<(NvarValNom); i++) {
        
        if (i==0) {
            dbTnomRange[0] = dbVarInNom[0][i];
        } else {
            dbTnomRange[0] = (dbVarInNom[0][i-1] + dbVarInNom[0][i])/2;
        }
        if (i==(NvarValNom - 1)) {
            dbTnomRange[1] = dbVarInNom[0][i];
        } else {
            dbTnomRange[1] = (dbVarInNom[0][i+1] + dbVarInNom[0][i])/2;
        }
        
        for (int j=0; j<4; j++) {
            if ( dbVarInNom[j+1][i]*dbVarInNom[j+1][i] < dbNomYcut*dbNomYcut ) {
                blNomAtZero[j] = true;
            } else {
                blNomAtZero[j] = false;
            }
        }
    
        blAllNomAtZero = blNomAtZero[0];
        for (int j=1; j<4; j++) blAllNomAtZero = blAllNomAtZero && blNomAtZero[j];
    
        for (int k=iBPMstart; k<NvarValBPM; k++) {
            if (dbVarInBPM[0][k] >= dbTnomRange[1]) {
                iBPMstart = k;
                break;
            } else if (dbVarInBPM[0][k] >= dbTnomRange[0] && dbVarInBPM[0][k] < dbTnomRange[1]) {
                for (int j=0; j<4; j++) {
                    if (blAllNomAtZero) {
                        dbVarInBPMcut[j][0][NvarValBPMcut[j]] = dbVarInBPM[0][k];
                        dbVarInBPMcut[j][1][NvarValBPMcut[j]] = dbVarInBPM[j+1][k];
                        NvarValBPMcut[j]++;
                    }
                }
        
// fill offsets
                for (int iOff=0; iOff<iNoffBPM; iOff++) {
                    if (dbVarInBPM[0][k] >= dbDFofftimeBPM[iOff] && (!blNewOffSet[iOff])) {
                        for (int j=0; j<4; j++) {
                            dbNEWoffsetBPM[j][iOff] = dbDFoffsetBPM[j][iOff] + dbVarInBPM[j+1][k];
                        }
                        blNewOffSet[iOff] = true;
                    }
                }
        
            }
        }
    }
  

  ofstream fout;
  string stFoutName = stPlotTitle[0];
  replace( stFoutName.begin(), stFoutName.end(), ' ', '_');
  stFoutName = stFoutName + string("_offsets.txt");
  fout.open(stFoutName.c_str());
  
  fout << "Timestamps: [";
  for (int iOff=0; iOff<(iNoffBPM); iOff++) {
    fout << lDFofftimeBPM[iOff];
    if (iOff<(iNoffBPM-1)) fout << ", ";
  }
  fout << "]" << endl;
  
  fout << "OffValues: [";
  for (int iOff=0; iOff<(iNoffBPM); iOff++) {
    fout << "[";
    for (int j=0; j<(Nvars-1); j++) {
      fout << dbNEWoffsetBPM[j][iOff];
      if (j<(Nvars-2)) fout << ", ";
    }
    fout << "]";
    if (iOff<(iNoffBPM-1)) fout << ",";
  }
  fout << "]" << endl;
  
  fout.close();
  
  
  TCanvas *canvasmain = new TCanvas("canvasmain","canvasmain",3000,1000);
  canvasmain->Divide(1, 1);
  canvasmain->cd();
  canvasmain->SetTicks(1,1);
  canvasmain->SetTopMargin(0.17);
  canvasmain->SetLeftMargin(0.065);
  canvasmain->SetRightMargin(0.12);
  canvasmain->SetBottomMargin(0.14);

  
  TGraph *tgVdm[3][Nvars - 1];
  int iFirstTg;
  
  string stPlotFile[3] = {"VdMscan_BPM", "VdMscan_BPM_offset", "VdMscan_Nom"};
  string stPlotSubTitle[3] = {"beams position in", "for Nominal = 0 in", "Nominal beams position in"};
  string stPlotTitleFin;
  string stPlotLegOp[3] = {"l", "p", "l"};
  
  int NlegEntr;
  double dbLegSep = 0.030;
  TLegend* tLeg;
  TLatex tLabel;
  
  for (int iPl=0; iPl<3; iPl++) {
      
    iFirstTg = -1;
    for (int i=0; i<(Nvars - 1); i++) {
      switch(iPl) {
        case 0 : tgVdm[iPl][i] = new TGraph(NvarValBPM, dbVarInBPM[0], dbVarInBPM[i+1]);
                 break;       // and exits the switch
        case 1 : tgVdm[iPl][i] = new TGraph(NvarValBPMcut[i], dbVarInBPMcut[i][0], dbVarInBPMcut[i][1]);
                 break;       // and exits the switch
        case 2 : tgVdm[iPl][i] = new TGraph(NvarValNom, dbVarInNom[0], dbVarInNom[i+1]);
                 break;
      }
      tgVdm[iPl][i]->SetMarkerStyle(8);
      tgVdm[iPl][i]->SetMarkerColor(iVarColor[i]);
      tgVdm[iPl][i]->SetLineColor(iVarColor[i]);
      tgVdm[iPl][i]->SetLineWidth(2);
    
      if (iFirstTg == -1) {
        if (blVarDraw[i]) iFirstTg = i;
      }
    }
  
    tgVdm[iPl][iFirstTg]->SetTitle("");
    tgVdm[iPl][iFirstTg]->GetXaxis()->SetLimits(dbXrange[0], dbXrange[1]);
    tgVdm[iPl][iFirstTg]->SetMinimum(dbYrange[0]);
    tgVdm[iPl][iFirstTg]->SetMaximum(dbYrange[1]);
  
  
    NlegEntr = 1;
    if (iPl == 1) {
      tgVdm[iPl][iFirstTg]->Draw("AP");
    } else {
      tgVdm[iPl][iFirstTg]->Draw("AL");
    }
  
    for (int i=(iFirstTg + 1); i<(Nvars - 1); i++) {
      if (blVarDraw[i]) {
        if (iPl == 1) {
          tgVdm[iPl][i]->Draw("P same");
        } else {
          tgVdm[iPl][i]->Draw("L same");
        }
        NlegEntr++;
      }
    }
  
    tLeg = new TLegend(0.900,(0.5 - NlegEntr*dbLegSep),0.997,(0.5 + NlegEntr*dbLegSep));  
    tLeg->AddEntry(tgVdm[iPl][iFirstTg],stDFcolName[iFirstTg+1].c_str(),stPlotLegOp[iPl].c_str());
    for (int i=(iFirstTg + 1); i<(Nvars - 1); i++) {
      if (blVarDraw[i]) tLeg->AddEntry(tgVdm[iPl][i],stDFcolName[i+1].c_str(),stPlotLegOp[iPl].c_str());
    }
  
    tLeg->SetTextAlign(12);
    tLeg->SetTextSize(0.05);
    tLeg->SetBorderSize(0);
    tLeg->Draw();
  
    tLabel.SetNDC(1);
    tLabel.SetTextFont(22);
    tLabel.SetTextSize(0.061);
    tLabel.DrawLatex(0.830,0.040,"t , min");
    tLabel.SetTextAngle(90);
    tLabel.DrawLatex(0.028,0.660,"l , #mu m");
  
    tLabel.SetTextAngle(0);
    tLabel.SetTextSize(0.07);
    if (iPl < 2) {
      stPlotTitleFin = stPlotTitle[0] + string(" ") + stPlotSubTitle[iPl] + string(" ") + stPlotTitle[1];
    } else {
      stPlotTitleFin = stPlotSubTitle[iPl] + string(" ") + stPlotTitle[1];
    }
    tLabel.SetTextAlign(21);
    tLabel.DrawLatex(0.472, 0.900, stPlotTitleFin.c_str());
    
    
    canvasmain->Print( string(stPlotFile[iPl] + ".eps").c_str(), "eps");
  
  } //end of loop over different plots
  
// remove pointers
  for (int i=0;i<Nvars;i++) delete [] dbVarInNom[i];
  delete [] dbVarInNom;
  
  for (int i=0;i<Nvars;i++) delete [] dbVarInBPM[i];
  delete [] dbVarInBPM;
  
  return 0;
}

