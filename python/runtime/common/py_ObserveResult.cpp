/*******************************************************************************
 * Copyright (c) 2022 - 2025 NVIDIA Corporation & Affiliates.                  *
 * All rights reserved.                                                        *
 *                                                                             *
 * This source code and the accompanying materials are made available under    *
 * the terms of the Apache License 2.0 which accompanies this distribution.    *
 ******************************************************************************/

#include "py_ObserveResult.h"

#include "common/ObserveResult.h"
#include "cudaq/algorithms/observe.h"

namespace py = pybind11;
namespace {
// FIXME(OperatorCpp): Remove this when the operator class is implemented in
// C++
cudaq::spin_op to_spin_op(py::object &obj) {
  if (py::hasattr(obj, "_to_spinop"))
    return obj.attr("_to_spinop")().cast<cudaq::spin_op>();
  return obj.cast<cudaq::spin_op>();
}
} // namespace

namespace cudaq {
/// @brief Bind the `cudaq::observe_result` and `cudaq::async_observe_result`
/// data classes to python as `cudaq.ObserveResult` and
/// `cudaq.AsyncObserveResult`.
void bindObserveResult(py::module &mod) {
  py::class_<observe_result>(
      mod, "ObserveResult",
      "A data-type containing the results of a call to :func:`observe`. "
      "This includes any measurement counts data, as well as the global "
      "expectation value of the user-defined `spin_operator`.\n")
      .def(py::init<double, spin_op, sample_result>())
      .def(py::init(
          [](double exp_val, py::object spin_op, sample_result result) {
            return observe_result(exp_val, to_spin_op(spin_op), result);
          }))
      /// @brief Bind the member functions of `cudaq.ObserveResult`.
      .def("dump", &observe_result::dump,
           "Dump the raw data from the :class:`SampleResult` that are stored "
           "in :class:`ObserveResult` to the terminal.")
      .def("get_spin", &observe_result::get_spin,
           "Return the `SpinOperator` corresponding to this `ObserveResult`.")
      .def(
          "counts", &observe_result::raw_data,
          "Returns a :class:`SampleResult` dictionary with the measurement "
          "results from the experiment. The result for each individual term of "
          "the `spin_operator` is stored in its own measurement register. "
          "Each register name corresponds to the string representation of the "
          "spin term (without any coefficients).\n")
      .def("counts", &observe_result::counts<spin_op>, py::arg("sub_term"),
           R"#(Given a `sub_term` of the global `spin_operator` that was passed 
to :func:`observe`, return its measurement counts.

Args:
  sub_term (:class:`SpinOperator`): An individual sub-term of the 
    `spin_operator`.

Returns:
  :class:`SampleResult`: The measurement counts data for the individual `sub_term`.)#")
      .def(
          "counts",
          [](observe_result &self, py::object sub_term) {
            return self.counts(to_spin_op(sub_term));
          },
          py::arg("sub_term"),
          R"#(Given a `sub_term` of the global `spin_operator` that was passed 
to :func:`observe`, return its measurement counts.

Args:
  sub_term (:class:`SpinOperator`): An individual sub-term of the 
    `spin_operator`.

Returns:
  :class:`SampleResult`: The measurement counts data for the individual `sub_term`.)#")
      .def(
          "expectation",
          [](observe_result &self) { return self.expectation(); },
          "Return the expectation value of the `spin_operator` that was "
          "provided in :func:`observe`.")
      .def(
          "expectation",
          [](observe_result &self, py::object spin_term) {
            return self.expectation(to_spin_op(spin_term));
          },
          py::arg("sub_term"),
          R"#(Return the expectation value of an individual `sub_term` of the 
global `spin_operator` that was passed to :func:`observe`.

Args:
  sub_term (:class:`SpinOperator`): An individual sub-term of the 
    `spin_operator`.

Returns:
  float : The expectation value of the `sub_term` with respect to the 
  :class:`Kernel` that was passed to :func:`observe`.)#")

      .def(
          "expectation_z",
          [](observe_result &self) {
            PyErr_WarnEx(PyExc_DeprecationWarning,
                         "expectation_z() is deprecated, use expectation() "
                         "with the same "
                         "argument structure.",
                         1);
            return self.expectation();
          },
          R"#(Return the expectation value of the `spin_operator` that was
provided in :func:`observe`.

Note:
  `expectation_z` has been deprecated in favor of `expectation`.)#")
      .def(
          "expectation_z",
          [](observe_result &self, py::object spin_term) {
            PyErr_WarnEx(PyExc_DeprecationWarning,
                         "expectation_z() is deprecated, use expectation() "
                         "with the same "
                         "argument structure.",
                         1);
            return self.expectation(to_spin_op(spin_term));
          },
          py::arg("sub_term"),
          R"#(Return the expectation value of an individual `sub_term` of the 
global `spin_operator` that was passed to :func:`observe`.

Note:
  `expectation_z` has been deprecated in favor of `expectation`.

Args:
  sub_term (:class:`SpinOperator`): An individual sub-term of the 
    `spin_operator`.

Returns:
  float : The expectation value of the `sub_term` with respect to the 
  :class:`Kernel` that was passed to :func:`observe`.)#");

  py::class_<async_observe_result>(
      mod, "AsyncObserveResult",
      R"#(A data-type containing the results of a call to :func:`observe_async`. 
      
The `AsyncObserveResult` contains a future, whose :class:`ObserveResult` 
may be returned via an invocation of the `get` method. 

This kicks off a wait on the current thread until the results are available.

See `future <https://en.cppreference.com/w/cpp/thread/future>`_
for more information on this programming pattern.)#")
      .def(py::init([](std::string inJson, py::object op) {
        auto as_spin_op = to_spin_op(op);
        async_observe_result f(&as_spin_op);
        std::istringstream is(inJson);
        is >> f;
        return f;
      }))
      .def("get", &async_observe_result::get,
           py::call_guard<py::gil_scoped_release>(),
           "Returns the :class:`ObserveResult` from the asynchronous observe "
           "execution.")
      .def("__str__", [](async_observe_result &self) {
        std::stringstream ss;
        ss << self;
        return ss.str();
      });
}

} // namespace cudaq
