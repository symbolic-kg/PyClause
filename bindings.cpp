#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "src/cpp/api/Api.h"
#include "src/cpp/api/RulesHandler.h"
#include "src/cpp/api/RankingHandler.h"
#include "src/cpp/api/QAHandler.h"
#include "src/cpp/api/DataHandler.h"
#include "src/cpp/api/PredictionHandler.h"
#include "src/cpp/core/Types.h"
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
        .def(py::init<std::map<std::string, std::string>>())
        .def("calculate_ranking", &RankingHandler::calculateRanking)
        .def("write_ranking", &RankingHandler::writeRanking)
         .def(
            "get_ranking",
            [](RankingHandler& self, std::string headOrTail, bool return_strings)->py::object{
                if (return_strings){
                    return py::cast(self.getStrRanking(headOrTail));
                }else{
                    return py::cast(self.getRanking(headOrTail));
                }
            }
        )
         .def(
            "get_rules",
            [](RankingHandler& self, std::string headOrTail, bool return_strings)->py::object{
                if (return_strings){
                    return py::cast(self.getStrRules(headOrTail));
                }else{
                    return py::cast(self.getIdxRules(headOrTail));
                }
            }
        )
    ; //class end
    // QAHandler()
    py::class_<QAHandler>(m, "QAHandler") 
        .def(py::init<std::map<std::string, std::string>>())
        .def("calculate_answers", py::overload_cast<std::vector<std::pair<int, int>>&, std::shared_ptr<DataHandler>, std::string>(&QAHandler::calculate_answers))
        .def("calculate_answers", py::overload_cast<std::vector<std::pair<std::string, std::string>>&, std::shared_ptr<DataHandler>, std::string>(&QAHandler::calculate_answers))
        .def(
            "get_answers",
            [](QAHandler& self, bool return_strings)->py::object{
                if (return_strings){
                    return py::cast(self.getStrAnswers());
                }else{
                    return py::cast(self.getIdxAnswers());
                }
            }
        )
        .def(
            "get_rules",
            [](QAHandler& self, bool return_strings)->py::object{
                if (return_strings){
                    return py::cast(self.getStrRules());
                }else{
                    return py::cast(self.getIdxRules());
                }
            }
        )
        .def("set_options", &QAHandler::setOptions)
    ; //class end
    // RulesHandler()
    py::class_<RulesHandler>(m, "RulesHandler") 
        .def(py::init<std::map<std::string, std::string>>())
        .def(
            "calculate_predictions", &RulesHandler::calcRulesPredictions,
            R"pbdoc(
                Given a list of string rules calculate predictions and rule statistics (num_pred, num_true_pred). 
                Option parameters can specify if predictions are stored or if statistics are stored. If only statistics 
                need to be computed, turn of collect_predictions parameter for efficiency.

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
                    }
        )        
        .def("get_statistics", &RulesHandler::getStats)
    ; //class end

    py::class_<DataHandler,  std::shared_ptr<DataHandler>>(m, "DataHandler") 
        .def(py::init<std::map<std::string, std::string>>())
        .def("load_rules", py::overload_cast<std::string>(&DataHandler::loadRules))
        .def("load_rules", py::overload_cast<std::vector<std::string>>(&DataHandler::loadRules))
        .def("load_data", [](DataHandler &self, const std::string &data, const std::string &filter, const std::string &target) { return self.loadData<std::string>(data, filter, target); }, 
           py::arg("data"), py::arg("filter") = "", py::arg("target") = "")
        .def("load_data", [](DataHandler &self, const StringTripleSet &data, const StringTripleSet &filter, const StringTripleSet &target) { return self.loadData<StringTripleSet>(data, filter, target); }, 
            py::arg("data"), py::arg("filter") = StringTripleSet(), py::arg("target") = StringTripleSet())
        .def("load_data", [](DataHandler &self, const TripleSet &data, const TripleSet &filter, const TripleSet &target) { return self.loadData<TripleSet>(data, filter, target); }, 
            py::arg("data"), py::arg("filter") = TripleSet(), py::arg("target") = TripleSet())
        .def("entity_map", &DataHandler::getNodeToIdx)
        .def("relation_map", &DataHandler::getRelationToIdx)
        .def("replace_ent_tokens", &DataHandler::subsEntityStrings)
        .def("replace_rel_tokens", &DataHandler::subsRelationStrings)
        .def("set_entity_index",  &DataHandler::setNodeIndex)
        .def("set_relation_index",  &DataHandler::setRelIndex)
        .def("rule_index",  &DataHandler::getRuleIdx)
    ; // class end

    // PredictionHandler()
    py::class_<PredictionHandler>(m, "PredictionHandler") 
        .def(py::init<std::map<std::string, std::string>>())
        .def("calculate_scores", py::overload_cast<std::string, std::shared_ptr<DataHandler>>(&PredictionHandler::scoreTriples),
               R"pbdoc(
                    Takes as input np.array/list of idx's or a list of string/token triples (tuples or lists)
                    or a path to a file containing tab separarated string/token triples. Entities and relation tokens must
                    be known, e.g., they must have be loaded in some triples in the DataLoader.
                )pbdoc"
            ) 
        .def("calculate_scores", py::overload_cast<std::vector<std::array<int,3>>, std::shared_ptr<DataHandler>>(&PredictionHandler::scoreTriples))
        .def("calculate_scores", py::overload_cast<std::vector<std::array<std::string,3>>, std::shared_ptr<DataHandler>>(&PredictionHandler::scoreTriples))
        .def(
            "get_scores",
            [](PredictionHandler& self, bool return_strings)->py::object{
                        if (return_strings){
                            return py::cast(self.getStrScores()); //TODO kinda spooky why this works, getStrScores is no python function binding
                        }else{
                            return py::cast(self.getIdxScores());
                        }
                    }
        )
        .def("get_explanations", &PredictionHandler::getStrExplanations)
        .def(
            "get_explanations",
            [](PredictionHandler& self, bool return_strings)->py::object{
                        if (return_strings){
                            return py::cast(self.getStrExplanations());
                        }else{
                            return py::cast(self.getIdxExplanations());
                        }
                    }
        )        
    ; // class end
}


