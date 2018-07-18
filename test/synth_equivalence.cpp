#include <cstdio>
#include <percy/percy.hpp>
#include <kitty/kitty.hpp>

#define MAX_TESTS 256

using namespace percy;
using kitty::dynamic_truth_table;

/*******************************************************************************
    Verifies that our synthesizers' results are equivalent to each other.
*******************************************************************************/
void check_fence_equivalence(int nr_in, int FI, bool full_coverage)
{
    spec spec;
    spec.add_alonce_clauses = false;
    spec.add_colex_clauses = false;
    spec.add_nontriv_clauses = false;
    spec.add_noreapply_clauses = false;
    spec.add_lex_func_clauses = false;
    spec.add_symvar_clauses = false;
    spec.verbosity = 0;

    bsat_wrapper solver;
    knuth_encoder encoder1(solver);
    knuth_fence_encoder encoder2(solver);

    // don't run too many tests.
    auto max_tests = (1 << (1 << nr_in));
    if (!full_coverage) {
        max_tests = std::min(max_tests, MAX_TESTS);
    }
    dynamic_truth_table tt(nr_in);

    chain c1, c1_cegar, c2, c2_cegar;

    for (auto i = 1; i < max_tests; i++) {
        kitty::create_from_words(tt, &i, &i+1);

        spec[0] = tt;
        auto res1 = synthesize(spec, c1, solver, encoder1, SYNTH_STD);
        assert(res1 == success);
        auto sim_tts1 = c1.simulate();
        auto c1_nr_vertices = c1.get_nr_steps();
        assert(c1.satisfies_spec(spec));

        auto res1_cegar = synthesize(spec, c1_cegar, solver, encoder1, SYNTH_STD_CEGAR);
        assert(res1_cegar == success);
        auto sim_tts1_cegar = c1_cegar.simulate();
        auto c1_cegar_nr_vertices = c1_cegar.get_nr_steps();
        assert(c1_cegar.satisfies_spec(spec));

        auto res2 = synthesize(spec, c2, solver, encoder2, SYNTH_FENCE);
        assert(res2 == success);
        auto sim_tts2 = c2.simulate();
        auto c2_nr_vertices = c2.get_nr_steps();
        assert(c2.satisfies_spec(spec));

        auto res2_cegar = synthesize(spec, c2_cegar, solver, encoder2, SYNTH_FENCE_CEGAR);
        assert(res2_cegar == success);
        auto sim_tts2_cegar = c2_cegar.simulate();
        auto c2_cegar_nr_vertices = c2.get_nr_steps();
        assert(c2_cegar.satisfies_spec(spec));

        assert(c1_nr_vertices == c2_nr_vertices);
        assert(c1_nr_vertices == c1_cegar_nr_vertices);
        assert(c1_cegar_nr_vertices == c2_cegar_nr_vertices);
        assert(sim_tts1[0] == sim_tts2[0]);
        assert(sim_tts1[0] == sim_tts1_cegar[0]);
        assert(sim_tts1_cegar[0] == sim_tts2_cegar[0]);
        
        printf("(%d/%d)\r", i+1, max_tests);
        fflush(stdout);
    }
    printf("\n");
}

int main()
{
    bool full_coverage = false;
    if (full_coverage) {
        printf("Doing full equivalence check\n");
    } else {
        printf("Doing partial equivalence check\n");
    }

    check_fence_equivalence(2, 2, full_coverage);
    check_fence_equivalence(3, 2, full_coverage);
    check_fence_equivalence(4, 2, full_coverage);
    check_fence_equivalence(3, 3, full_coverage);
    check_fence_equivalence(4, 3, full_coverage);
    
    return 0;
}

