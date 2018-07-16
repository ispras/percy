#include <cstdio>
#include <percy/percy.hpp>
#include <kitty/kitty.hpp>

#define MAX_TESTS 256

using namespace percy;
using kitty::dynamic_truth_table;

/*******************************************************************************
    Verifies that our synthesizers' results are equivalent to each other.
*******************************************************************************/
void check_pd_equivalence(int nr_in, int FI, bool full_coverage)
{
    spec spec;

    bsat_wrapper solver;
    knuth_encoder encoder1(solver);
    partial_dag_encoder encoder2(solver);

    // don't run too many tests.
    auto max_tests = (1 << (1 << nr_in));
    if (!full_coverage) {
        max_tests = std::min(max_tests, MAX_TESTS);
    }
    dynamic_truth_table tt(nr_in);

    chain c1, c2, c2_cegar;

    auto dags = pd_generate(7);

    for (auto i = 1; i < max_tests; i++) {
        kitty::create_from_words(tt, &i, &i+1);

        spec.verbosity = 0;
        spec[0] = tt;
        auto res1 = synthesize(spec, c1, solver, encoder1, SYNTH_STD_CEGAR);
        assert(res1 == success);
        auto sim_tts1 = c1.simulate(spec);
        auto c1_nr_vertices = c1.get_nr_steps();
        assert(c1.satisfies_spec(spec));

        //spec.verbosity = 2;
        auto had_success = false;
        for (auto& dag : dags) {
            auto res2 = pd_synthesize(spec, c2, dag, solver, encoder2);
            if (res2 == success) {
                had_success = true;
                //assert(c2.satisfies_spec(spec));
                break;
            }
        }
        assert(had_success);
        auto sim_tts2 = c2.simulate(spec);
        auto c2_nr_vertices = c2.get_nr_steps();
        assert(c1_nr_vertices == c2_nr_vertices);
        assert(sim_tts1[0] == sim_tts2[0]);
        
        printf("(%d/%d)\r", i+1, max_tests);
        fflush(stdout);
    }
    printf("\n");
}

/// Tests synthesis based on partial DAGs by comparing it to conventional
/// synthesis.  By default, does not check for full equivalence of all n-input functions.
/// Users can specify a arbitrary runtime argument, which removes the limit on
/// the number of equivalence tests.
int main(int argc, char **argv)
{
    bool full_coverage = false;
    if (argc > 1) {
        full_coverage = true;
    }
    if (full_coverage) {
        printf("Doing full equivalence check\n");
    } else {
        printf("Doing partial equivalence check\n");
    }

    check_pd_equivalence(2, 2, full_coverage);
    check_pd_equivalence(3, 2, full_coverage);
    check_pd_equivalence(4, 2, full_coverage);
    
    return 0;
}

