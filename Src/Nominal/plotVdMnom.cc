
#include <iostream>
#include <fstream>
#include <sstream>

#include <time.h>

#include "TCanvas.h"
#include "TGraph.h"
#include "TAxis.h"

#include "TStyle.h"
#include "TPaveLabel.h"
#include "TLatex.h"
#include "TLegend.h"

using namespace std;

  const int Nvars = 5; //variables to be read: timestamp, B1_H, B1_V, B2_H, B2_V
  const int NdataMax = 30000;
  const int Nsteer = 2;

void readPlotSteer (string stFile, int* iTimeRange, bool* blPdraw, int* iPcolor, string& stTitle, double* dbXaxisRange, double* dbYaxisRange) {
  
  const int Npar = 8;
  const int NsubparsMax = 3;
  
  ifstream fin;
  string stfiline;
  
  fin.open(stFile.c_str());
  bool blSPsubparRead[NsubparsMax]; //boolean flags if all parameters (zero value + min/max time, draw/color) are read.
  int iSubstr, iCurrIdx = -1; //iCurrIdx: 0 - Timestamp, 1-4 - B1_H, B1_V, B2_H, B2_V
  string stSTpars[Npar] = {"Timestamp:", "B1_H:", "B1_V:", "B2_H:", "B2_V:", "Plot info:", "X axis:", "Y axis:"};
  int Nsubpars[Npar] = {      3,            2,       2,       2,       2,         1,          2,         2};
  string stSTtimepars[3] = {"zero value:", "plot min:", "plot max:"};
  string stSTaxispars[2] = {"min:", "max:"};
  
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
          iTimeRange[i] = stoi(stfiline.substr(iSubstr + stSTtimepars[i].size() + 1));
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
        iPcolor[iCurrIdx-1] = stoi(stfiline.substr(iSubstr + 7));
        blSPsubparRead[1] = true;
      }
    } else if (iCurrIdx == 5) {
      iSubstr = stfiline.find(string("title:"));
      if (iSubstr < stfiline.size()) {
        stTitle = stfiline.substr(iSubstr + 7);
        blSPsubparRead[0] = true;
      }
    } else if (iCurrIdx == 6) {
      for (int i=0; i<Nsubpars[iCurrIdx]; i++) {
        iSubstr = stfiline.find(stSTaxispars[i]);
        if (iSubstr < stfiline.size()) {
          dbXaxisRange[i] = stod(stfiline.substr(iSubstr + stSTaxispars[i].size() + 1));
          blSPsubparRead[i] = true;
        }
      }
    } else if (iCurrIdx == 7) {
      for (int i=0; i<Nsubpars[iCurrIdx]; i++) {
        iSubstr = stfiline.find(stSTaxispars[i]);
        if (iSubstr < stfiline.size()) {
          dbYaxisRange[i] = stod(stfiline.substr(iSubstr + stSTaxispars[i].size() + 1));
          blSPsubparRead[i] = true;
        }
      }
    }
    
    if (iCurrIdx > -1) {
      blCheck = blSPsubparRead[0];
      for (int j=1; j<Nsubpars[iCurrIdx]; j++) blCheck = blCheck && blSPsubparRead[j];
      if (blCheck) iCurrIdx = -1;
    }
    
  } //end loop over file lines
  
  return;
}

void readDataSteer (string stFile, string& stDF, string* stDFcName, int* iDFcIdx, double* dbDFcScale) {
  
  ifstream fin;
  string stfiline;
  
  fin.open(stFile.c_str());
  bool blFNread = false, blNeedScale = false;
  int iSubstr, iCurrCidx;
  while(getline(fin, stfiline)) {
    if (!blFNread) {
      iSubstr = stfiline.find(string("Data file:"));
      if (iSubstr < stfiline.size()) {
//        stDF = stfiline.substr(iSubstr + 11);
        stDF = stfiline.substr( iSubstr + 10 + stfiline.substr(iSubstr + 10).find_first_not_of(' ') );
        blFNread = true;
        iCurrCidx = -1;
        blNeedScale = false;
      }
    } else {
      for (int i=0; i<5; i++) {
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
  
  return;
}

void readDataFile (string stFile, int iDFcIdx_in[Nvars], int iTimeRange_in[3], int& NreadPar, Double_t dbreadVar[Nvars][NdataMax]) {
  
  ifstream fin;
  string stfiline;
  
  fin.open(stFile.c_str());
  int iSubstr, iSubstr_next, iCurrCol;
  
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
        for (int i=0; i<Nvars; i++) {
          if (iDFcIdx_in[i] == iCurrCol) {
            dbreadVar[i][NreadPar] = stod(stfiline.substr(iSubstr,iSubstr_next-iSubstr));
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
  
  return;
}

int main(int argc, char **argv) {
  
  if ( argc != (Nsteer + 1) ) {
    printf("program usage:\n plotVdMnom [plot steering file] [data steering file]\n");
    return -1;
  }
  
  string stDFcolName[Nvars] = {"Timestamp", "B1_H", "B1_V", "B2_H", "B2_V"};
  
  string stSteering[Nsteer];
  for (int i=0; i<Nsteer; i++) stSteering[i] = string(argv[i + 1]);
  
  int iPlotTimeRange[3];
  bool blVarDraw[4];
  int iVarColor[4];
  string stPlotTitle;
  double dbXrange[2], dbYrange[2];
  
  readPlotSteer (stSteering[0], iPlotTimeRange, blVarDraw, iVarColor, stPlotTitle, dbXrange, dbYrange);

/*  
  cout << "Plot title: " << stPlotTitle << endl;
  cout << stDFcolName[0] << "Zero value: " << iPlotTimeRange[0] << ", plot from " << iPlotTimeRange[1] << " to " << iPlotTimeRange[2] << "." << endl;
  cout << "X axis from " << dbXrange[0] << " to " << dbXrange[1] << "; Y axis from " << dbYrange[0] << " to " << dbYrange[1] << "." << endl;
  for (int i=0; i<4; i++) cout << stDFcolName[i+1] << " draw: " << blVarDraw[i] << ", color: " << iVarColor[i] << "." << endl;
*/

  string stDataFile;
  int iDFcolIdx[Nvars]; //timestamp, B1_H, B1_V, B2_H, B2_V
  double dbDFcolScale[Nvars];
  
  readDataSteer(stSteering[1], stDataFile, stDFcolName, iDFcolIdx, dbDFcolScale);

/*
  cout << stDataFile << endl;
  for (int i=0; i<Nvars; i++) cout << stDFcolName[i] << "   " << iDFcolIdx[i] << "   " << dbDFcolScale[i] << endl;
*/

  int NvarVal = 0;
  Double_t dbVarIn[Nvars][NdataMax]; //timestamp, B1_H, B1_V, B2_H, B2_V
  
  readDataFile (stDataFile, iDFcolIdx, iPlotTimeRange, NvarVal, dbVarIn);

/*  
  cout << endl << "  NvarVal = " << NvarVal << endl;
  for (int i=21; i<22; i++) {
    for (int j=0; j<Nvars; j++) cout << dbVarIn[j][i] << "   ";
    cout << endl;
  }
*/

  for (int i=0; i<Nvars; i++) {
    for (int j=0; j<NvarVal; j++) {
      dbVarIn[i][j] = dbVarIn[i][j] * dbDFcolScale[i];
    }
  }
  
  TCanvas *canvasmain = new TCanvas("canvasmain","canvasmain",3000,1000);
  canvasmain->Divide(1, 1);
  canvasmain->cd();
  canvasmain->SetTicks(1,1);
  canvasmain->SetTopMargin(0.17);
  canvasmain->SetLeftMargin(0.065);
  canvasmain->SetRightMargin(0.12);
  canvasmain->SetBottomMargin(0.14);
  
  TGraph *tgVdm[Nvars - 1];
  int iFirstTg = -1;
  
  for (int i=0; i<(Nvars - 1); i++) {
    tgVdm[i] = new TGraph(NvarVal, dbVarIn[0], dbVarIn[i+1]);
    tgVdm[i]->SetMarkerStyle(8);
    tgVdm[i]->SetLineColor(iVarColor[i]);
    tgVdm[i]->SetLineWidth(2);
    
    if (iFirstTg == -1) {
      if (blVarDraw[i]) iFirstTg = i;
    }
  }
  
  tgVdm[iFirstTg]->SetTitle("");
  tgVdm[iFirstTg]->GetXaxis()->SetLimits(dbXrange[0], dbXrange[1]);
  tgVdm[iFirstTg]->SetMinimum(dbYrange[0]);
  tgVdm[iFirstTg]->SetMaximum(dbYrange[1]);
  
  
  int NlegEntr = 1;
  tgVdm[iFirstTg]->Draw("AL");
  for (int i=(iFirstTg + 1); i<(Nvars - 1); i++) {
    if (blVarDraw[i]) {
      tgVdm[i]->Draw("L same");
      NlegEntr++;
    }
  }
  
  double dbLegSep = 0.030;
  TLegend* tLeg = new TLegend(0.900,(0.5 - NlegEntr*dbLegSep),0.997,(0.5 + NlegEntr*dbLegSep));  
  tLeg->AddEntry(tgVdm[iFirstTg],stDFcolName[iFirstTg+1].c_str(),"l");
  for (int i=(iFirstTg + 1); i<(Nvars - 1); i++) {
    if (blVarDraw[i]) tLeg->AddEntry(tgVdm[i],stDFcolName[i+1].c_str(),"l");
  }
  
  tLeg->SetTextAlign(12);
  tLeg->SetTextSize(0.05);
  tLeg->SetBorderSize(0);
  tLeg->Draw();
  

  
  TLatex tLabel;
  tLabel.SetNDC(1);
  tLabel.SetTextFont(22);
  tLabel.SetTextSize(0.061);
  tLabel.DrawLatex(0.830,0.040,"t , min");
  tLabel.SetTextAngle(90);
  tLabel.DrawLatex(0.028,0.660,"l , #mu m");
  
  tLabel.SetTextAngle(0);
  tLabel.SetTextSize(0.07);
  tLabel.SetTextAlign(21);
  tLabel.DrawLatex(0.472, 0.900, stPlotTitle.c_str());
  
  canvasmain->Print("VdMscan.eps","eps");
  
  return 0;
}

