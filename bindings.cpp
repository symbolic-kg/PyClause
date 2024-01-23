#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "src/c_clause/api/Api.h"
#include "src/c_clause/api/RulesHandler.h"
#include "src/c_clause/api/RankingHandler.h"
#include "src/c_clause/api/QAHandler.h"
#include "src/c_clause/api/Loader.h"
#include "src/c_clause/api/PredictionHandler.h"
#include "src/c_clause/core/Types.h"
#include "src/c_clause/tests.h"
#include <string>
#include <array>


// **********************************************************************
// ***************** c_clause Python API ********************************
// **********************************************************************


namespace py = pybind11;

PYBIND11_MODULE(c_clause, m) {
    // ***exposed backend functions that are usable in the frontend***

    // RankingHandler()
    py::class_<RankingHandler>(m, "RankingHandler") 
        .def(py::init<std::map<std::string, std::string>>(), py::arg("options"))
        .def("calculate_ranking", &RankingHandler::calculateRanking, py::arg("loader"))
        .def("write_ranking", &RankingHandler::writeRanking, py::arg("path"), py::arg("loader"))
        .def("set_options", &RankingHandler::setOptionsFrontend, py::arg("options"))
        .def(
            "get_ranking",
            [](RankingHandler& self, std::string headOrTail, bool return_strings)->py::object{
                if (return_strings){
                    return py::cast(self.getStrRanking(headOrTail));
                }else{
                    return py::cast(self.getRanking(headOrTail));
                }
            },
            py::arg("direction"), py::arg("as_string")
        )
         .def(
            "get_rules",
            [](RankingHandler& self, std::string headOrTail, bool return_strings)->py::object{
                if (return_strings){
                    return py::cast(self.getStrRules(headOrTail));
                }else{
                    return py::cast(self.getIdxRules(headOrTail));
                }
            },
            py::arg("direction"), py::arg("as_string")
        )
    ; //class end
    // QAHandler()
    py::class_<QAHandler>(m, "QAHandler") 
        .def(py::init<std::map<std::string, std::string>>(), py::arg("options"))
        .def("set_options", &QAHandler::setOptionsFrontend, py::arg("options"))
        .def(
            "calculate_answers",
             py::overload_cast<std::vector<std::pair<int, int>>&, std::shared_ptr<Loader>, std::string>(&QAHandler::calculate_answers),
             py::arg("queries"), py::arg("loader"), py::arg("direction")   
        )
        .def(
            "calculate_answers",
            py::overload_cast<std::vector<std::pair<std::string, std::string>>&, std::shared_ptr<Loader>, std::string>(&QAHandler::calculate_answers),
            py::arg("queries"), py::arg("loader"), py::arg("direction") 
        )
        .def(
            "calculate_answers",
            py::overload_cast<std::string&, std::shared_ptr<Loader>, std::string>(&QAHandler::calculate_answers),
            py::arg("queries"), py::arg("loader"), py::arg("direction") 
        )
        .def(
            "get_answers",
            [](QAHandler& self, bool return_strings)->py::object{
                if (return_strings){
                    return py::cast(self.getStrAnswers());
                }else{
                    return py::cast(self.getIdxAnswers());
                }
            },
            py::arg("as_string")
        )
        .def("write_answers", &QAHandler::writeAnswers, py::arg("path"), py::arg("as_string") = true)
        .def(
            "get_rules",
            [](QAHandler& self, bool return_strings)->py::object{
                if (return_strings){
                    return py::cast(self.getStrRules());
                }else{
                    return py::cast(self.getIdxRules());
                }
            },
            py::arg("as_string")
        )
        .def("write_rules", &QAHandler::writeRules, py::arg("path"), py::arg("as_string") = true)
        .def("set_options", &QAHandler::setOptions)
    ; //class end
    // RulesHandler()
    py::class_<RulesHandler>(m, "RulesHandler") 
        .def(py::init<std::map<std::string, std::string>>(),  py::arg("options"))
        .def("set_options", &RulesHandler::setOptionsFrontend, py::arg("options"))
        .def(
            "calculate_predictions", py::overload_cast<std::vector<std::string>&, std::shared_ptr<Loader>>(&RulesHandler::calcRulesPredictions),
            py::arg("rules"), py::arg("loader"),
            R"pbdoc(
                Given a list of string rules calculate predictions and rule statistics (num_pred, num_true_pred). 
                Option parameters can specify if predictions are stored or if statistics are stored. If only statistics 
                need to be computed, turn off collect_predictions option for efficiency.
            )pbdoc" 
        )
        .def(
            "calculate_predictions", py::overload_cast<std::string&, std::shared_ptr<Loader>>(&RulesHandler::calcRulesPredictions),
            py::arg("rules"), py::arg("loader"),
            R"pbdoc(
                Given a list of rules in a file (list of rules or AnyBURL format) calculate predictions and rule statistics (num_pred, num_true_pred). 
                Option parameters can specify if predictions are stored or if statistics are stored. If only statistics 
                need to be computed, turn off collect_predictions option for efficiency.
            )pbdoc" 
        )
        .def(
            "get_predictions",
            [](RulesHandler& self, bool return_strings)->py::object{
                        if (return_strings){
                            return py::cast(self.getStrPredictions());
                        }else{
                            return py::cast(self.getIdxPredictions());
                        }
                    },
            py::arg("as_string")
        )        
        .def("write_predictions", &RulesHandler::writeRulesPredictions, py::arg("path"), py::arg("flat") = true, py::arg("as_string") = true)
        .def("get_statistics", &RulesHandler::getStats)
        .def("write_statistics", &RulesHandler::writeStats, py::arg("path"))

    ; //class end

    py::class_<Loader,  std::shared_ptr<Loader>>(m, "Loader") 
        .def(py::init<std::map<std::string, std::string>>(), py::arg("options"))
        .def("load_rules", py::overload_cast<std::string>(&Loader::loadRules), py::arg("rules"))
        .def("load_rules", py::overload_cast<std::vector<std::string>>(&Loader::loadRules), py::arg("rules"))
        .def(
            "load_rules",
            py::overload_cast<std::vector<std::string>, std::vector<std::pair<int,int>>>(&Loader::loadRules),
            py::arg("rules"), py::arg("stats")
        )
        .def(
            "load_data",
            [](Loader &self, const std::string &data, const std::string &filter, const std::string &target) { return self.loadData<std::string>(data, filter, target); }, 
            py::arg("data"), py::arg("filter") = "", py::arg("target") = ""
        )
        .def(
            "load_data",
            [](Loader &self, const StringTripleSet &data, const StringTripleSet &filter, const StringTripleSet &target) { return self.loadData<StringTripleSet>(data, filter, target); }, 
            py::arg("data"), py::arg("filter") = StringTripleSet(), py::arg("target") = StringTripleSet()
        )
        .def(
            "load_data",
            [](Loader &self, const TripleSet &data, const TripleSet &filter, const TripleSet &target) { return self.loadData<TripleSet>(data, filter, target); }, 
            py::arg("data"), py::arg("filter") = TripleSet(), py::arg("target") = TripleSet()
        )
        .def("get_entity_index", &Loader::getNodeToIdx)
        .def("get_relation_index", &Loader::getRelationToIdx)
        .def("replace_ent_strings", &Loader::subsEntityStrings, py::arg("new_tokens"))
        .def("replace_rel_strings", &Loader::subsRelationStrings, py::arg("new_tokens"))
        .def("set_entity_index",  &Loader::setNodeIndex, py::arg("index"))
        .def("set_relation_index",  &Loader::setRelIndex, py::arg("index"))
        .def("rule_index",  &Loader::getRuleIdx)
        .def("set_options", &Loader::setOptions, py::arg("options"))
    ; // class end

    // PredictionHandler()
    py::class_<PredictionHandler>(m, "PredictionHandler") 
        .def(py::init<std::map<std::string, std::string>>(), py::arg("options"))
        .def("set_options", &PredictionHandler::setOptionsFrontend, py::arg("options"))
        .def(
            "calculate_scores",
            py::overload_cast<std::string, std::shared_ptr<Loader>>(&PredictionHandler::scoreTriples),
            py::arg("triples"), py::arg("loader"), 
            R"pbdoc(
                    Takes as input np.array/list of idx's or a list of string/token triples (tuples or lists)
                    or a path to a file containing tab separarated string/token triples. Entities and relation tokens must
                    be known, e.g., they must have be loaded in some triples in the Loader.
                )pbdoc"
        ) 
        .def(
            "calculate_scores",
            py::overload_cast<std::vector<std::array<int,3>>, std::shared_ptr<Loader>>(&PredictionHandler::scoreTriples),
            py::arg("triples"), py::arg("loader")
        )
        .def(
            "calculate_scores",
             py::overload_cast<std::vector<std::array<std::string,3>>, std::shared_ptr<Loader>>(&PredictionHandler::scoreTriples),
             py::arg("triples"), py::arg("loader")
        )
        
        .def(
            "get_scores",
            [](PredictionHandler& self, bool return_strings)->py::object{
                        if (return_strings){
                            return py::cast(self.getStrScores());
                        }else{
                            return py::cast(self.getIdxScores());
                        }
                    },
            py::arg("as_string")   
        )
        .def(
            "get_explanations",
            [](PredictionHandler& self, bool return_strings)->py::object{
                        if (return_strings){
                            return py::cast(self.getStrExplanations());
                        }else{
                            return py::cast(self.getIdxExplanations());
                        }
                    },
            py::arg("as_string")
        )    
        .def("write_explanations", &PredictionHandler::writeExplanations, py::arg("path"), py::arg("as_string"))
        .def("write_scores", &PredictionHandler::writeScores, py::arg("path"), py::arg("as_string"))      
    ; // class end

    // backend tests
    m.def("_test_compute_strings", &test_compute_strings);
    m.def("_tests_groundings", &tests_groundings);
    m.def("_tests_rules", &tests);
    m.def("_test_scoring", &testTripleScoring);
}


