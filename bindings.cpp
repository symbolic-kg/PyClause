#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "src/cpp/Api.h"
#include "src/cpp/core/myClass.h"
#include <string>

// *Example* 
int add(int i, int j) {
    return i + j;
}

namespace py = pybind11;


//***PyClause backend bindings + examples *****
PYBIND11_MODULE(rules_c, m) {

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
    py::class_<RankingHandler>(m, "RuleHandler") 
        .def(py::init<>())
        .def("calculateRanking", &RankingHandler::calculateRanking)
        .def("getRanking", &RankingHandler::getRanking)
    ;
}


