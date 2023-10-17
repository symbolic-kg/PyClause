#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "src/cpp/api/Api.h"
#include "src/cpp/core/myClass.h"
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

    py::class_<myClass>(m, "myClass")
        .def(py::init<std::string>())
        .def("addOne", &myClass::addOne)
        .def("getNumber", &myClass::getNumber)
        .def("sumRange", &myClass::addRange) //name does not matter
    ;

    // **backend bindings**
    py::class_<RankingHandler>(m, "RankingHandler") 
        .def(py::init<>())
        .def("calculateRanking", &RankingHandler::calculateRanking)
        .def("getRanking", &RankingHandler::getRanking)
    ;

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
    ;
}


