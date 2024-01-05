from c_clause import _test_compute_strings, _tests_groundings, _tests_rules, _test_scoring

def test_c_clause_backend():
    _test_compute_strings()
    _tests_groundings()
    _tests_rules()
    _test_scoring()