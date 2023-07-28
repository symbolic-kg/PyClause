#include "Globals.h"
#include <string>


// ranking parameters
// preselection of candidates based on max
// for noisy-or this will result in slightly incorrect rankings when set low
// for max-plus ranking is correct
extern int _cfg_rnk_numPreselect;
// num candidates to output in ranking
// must not be higher than _cfg_rnk_numPreselect;
extern int _cfg_rnk_topk;

extern bool _cfg_rnk_filterWtrain;
// target is the dataset on which ranking is calculated (test)
extern bool _cfg_rnk_filterWTarget;

extern std::string _cfg_rnk_aggrFunc;


//running configurations
extern std::string _cfg_pathTrain;
extern std::string _cfg_pathValid;
extern std::string _cfg_pathTest;

extern std::string _cfg_pathRankFile;

extern std::string _cfg_pathRules;



// variable ordering of AnyTime Format
std::string anyTimeVars="XABCDEFGHIJKLMNOPY";
std::string equalityToken = "me_myself_i";
std::string ruleSeparator = " <= ";
std::string atomSeparator = ", ";

bool _cfg_exactConf = false;
int _cfg_nUnseen = 0;

//general
bool _cfg_verbose = true;


//ranking/application
//preselection of candidates based on max
int _cfg_rnk_numPreselect=100;

//num candidates to output in ranking
int _cfg_rnk_topk=10;

bool _cfg_rnk_filterWtrain=true;
bool _cfg_rnk_filterWTarget=true;
std::string _cfg_rnk_aggrFunc = "maxplus";

// running configurations
std::string _cfg_pathTrain = "/home/patrick/Desktop/PyClause/data/wnrr/train.txt";
std::string _cfg_pathFilter = "/home/patrick/Desktop/PyClause/data/wnrr/valid.txt";
std::string _cfg_pathTarget = "/home/patrick/Desktop/PyClause/data/wnrr/test.txt";

std::string _cfg_pathRankFile = "/home/patrick/Desktop/PyClause/data/wnrr/firstRankingChristian.txt";
std::string _cfg_pathRules = "/home/patrick/Desktop/PyClause/data/wnrr/anyburl-rules-c5-3600";


