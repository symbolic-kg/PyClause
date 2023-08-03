#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "src/cpp/Api.h"
#include "src/cpp/core/myClass.h"
#include <string>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)





// ***Pybind Examples**** 
int add(int i, int j) {
    return i + j;
}

namespace py = pybind11;


// // ***example bindings***
// PYBIND11_MODULE(rulebackend, m) {
//     py::class_<TripleStorage>(m, "TripleStorage") // TODO is actually not needed and cannot invoked as index is not used 
//         .def(py::init<Index*>())
//         .def("read", &TripleStorage::read)
//     ;
//     #ifdef VERSION_INFO
//     m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
// #else
//     m.attr("__version__") = "dev";
// #endif
// }


//***PyClause backend bindings*****
PYBIND11_MODULE(rules_c, m) {

    //**+ example bindings
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

    // rule handler
    py::class_<RuleHandler>(m, "RuleHandler") 
        .def(py::init<>())
        .def("calculateRanking", &RuleHandler::calculateRanking)
        .def("getRanking", &RuleHandler::getRanking)
    ;
    #ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif


}


