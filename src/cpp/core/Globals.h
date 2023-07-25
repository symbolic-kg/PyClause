#include <string>

#ifndef GLOBALS_H
#define GLOBALS_H

extern std::string anyTimeVars;
extern std::string equalityToken;
extern std::string ruleSeparator;
extern std::string atomSeparator;


extern int _cfg_nUnseen;
extern bool _cfg_exactConf;

// ranking parameters

// preselection of candidates based on max
// for noisy-or this will result in slightly incorrect rankings when set low
// for max-plus ranking is correct
extern int _cfg_rnk_numPreselect;
// num candidates to output in ranking
// must not be higher than _cfg_rnk_numPreselect;
extern int _cfg_rnk_topk;

extern bool _cfg_rnk_filterWtrain;
extern bool _cfg_rnk_filterWTest;


#endif