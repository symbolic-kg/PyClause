#include "Globals.h"
#include <string>


// variable ordering of AnyTime Format
std::string anyTimeVars="XABCDEFGHIJKLMNOPY";
std::string equalityToken = "me_myself_i";
std::string ruleSeparator = " <= ";
std::string atomSeparator = ", ";

bool _cfg_exactConf = false;
int _cfg_nUnseen = 5;

//preselection of candidates based on max
int _cfg_rnk_numPreselect=100;
//num candidates to output in ranking
int _cfg_rnk_topk=100;

bool _cfg_rnk_filterWtrain=true;
bool _cfg_rnk_filterWTest=true;