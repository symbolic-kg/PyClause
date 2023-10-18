#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "src/cpp/api/Api.h"
#include "src/cpp/api/RulesHandler.h"
#include "src/cpp/api/RankingHandler.h"
#include <string>

// *Example* 
int add(int i, int j) {
    return i + j;
}

namespace py = pybind11;


//***PyClause backend bindings + examples *****
PYBIND11_MODULE(c_clause, m) {

    //** example bindings **
    m.doc() = R"pbdoc(
        Pybind11 example plugin
        -----------------------

        .. currentmodule:: python_example

        .. autosummary::
           :toctree: _generate

           add
           subtract
    )pbdoc";

    m.def("add", &add, R"pbdoc(
        Add two numbers

        Some other explanation about the add function.
    )pbdoc");

    m.def("subtract", [](int i, int j) { return i - j; }, R"pbdoc(
        Subtract two numbers

        Some other explanation about the subtract function.
    )pbdoc");

    // **backend bindings**
    py::class_<RankingHandler>(m, "RankingHandler") 
        .def(py::init<std::map<std::string, std::string>>())
        .def("load_datasets", &RankingHandler::loadDatasets)
        .def("load_rules", &RankingHandler::loadRules)
        .def("calc_ranking", &RankingHandler::calculateRanking)
        .def("get_ranking", &RankingHandler::getRanking)
        .def("write_ranking", &RankingHandler::writeRanking)
    ; //class end

    py::class_<RuleHandler>(m, "RuleHandler") 
        .def(py::init<std::string>())
        .def(
            "calcRulesPredictions", &RuleHandler::calcRulesPredictions,
            R"pbdoc(
                Given a string rule calculates all stats and predictions of rules. 
                args: string:rulestring; bool returnPredictions: if true return predictions; bool returnStats: if true returns exact stats.
                returns: tuple where tuple[0] are the predictions , tuple[1][0] are number of exact predictions tuple[1][1] number of true predictions.

            )pbdoc" 
        
        
        )
    ; //class end
    py::class_<RulesHandler>(m, "RulesHandler") 
        .def(py::init<>())
        .def(
            "stats_and_predictions", &RulesHandler::calcRulesPredictions,
            R"pbdoc(
                Given a string rule calculates all stats and predictions of rules. 
                args: string:rulestring; bool returnPredictions: if true return predictions; bool returnStats: if true returns exact stats.
                returns: tuple where tuple[0] are the predictions , tuple[1][0] are number of exact predictions tuple[1][1] number of true predictions.

            )pbdoc" 
        )
        .def("load_data", &RulesHandler::loadData)
    ; //class end
}


