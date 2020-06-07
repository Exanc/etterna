#pragma once
#include <string>
#include <array>
#include <vector>

#include "Etterna/Models/NoteData/NoteDataStructures.h"
#include "Etterna/Globals/MinaCalc/Dependent/IntervalHandInfo.h"

using std::pair;
using std::vector;

// ok new plan we will incloop the joomp
struct WideRangeRollMod
{
	const CalcPatternMod _pmod = WideRangeRoll;
	const std::string name = "WideRangeRollMod";

#pragma region params

	float window_param = 5.F;

	float min_mod = 0.25F;
	float max_mod = 1.F;
	float base = 0.15F;
	float scaler = 0.9F;

	float cv_reset = 1.F;
	float cv_threshold = 0.35F;
	float other_cv_threshold = 0.3F;

	const vector<pair<std::string, float*>> _params{
		{ "window_param", &window_param },

		{ "min_mod", &min_mod },
		{ "max_mod", &max_mod },
		{ "base", &base },
		{ "scaler", &scaler },

		{ "cv_reset", &cv_reset },
		{ "cv_threshold", &cv_threshold },
		{ "other_cv_threshold", &other_cv_threshold },
	};
#pragma endregion params and param map

	int window = 0;

	// moving window of longest roll sequences seen in the interval
	CalcMovingWindow<int> _mw_max;

	// we want to keep custom adjusted ms values here
	CalcMovingWindow<float> _mw_adj_ms;

	bool last_passed_check = false;
	int nah_this_file_aint_for_real = 0;
	int max_thingy = 0;
	float hi_im_a_float = 0.F;

	// WE CAN JUST MOVE THE TIMING CHECK FUNCTIONS INTO CALCWINDOW LUL
	vector<float> idk_ms = { 0.F, 0.F, 0.F, 0.F };
	vector<float> seq_ms = { 0.F, 0.F, 0.F };

	float moving_cv = cv_reset;
	float pmod = min_mod;

#pragma region generic functions

	inline void full_reset()
	{
		_mw_max.zero();
		_mw_adj_ms.zero();

		last_passed_check = false;
		nah_this_file_aint_for_real = 0;
		max_thingy = 0;
		hi_im_a_float = 0.F;

		for (auto& v : seq_ms) {
			v = 0.F;
		}
		for (auto& v : idk_ms) {
			v = 0.F;
		}

		moving_cv = cv_reset;
		pmod = neutral;
	}

	inline void setup()
	{
		window =
		  CalcClamp(static_cast<int>(window_param), 1, max_moving_window_size);
	}

#pragma endregion

	inline void zoop_the_woop(const int& pos,
							  const float& div,
							  const float& scaler = 1.F)
	{
		seq_ms[pos] /= div;
		last_passed_check = do_timing_thing(scaler);
		seq_ms[pos] *= div;
	}

	inline void woop_the_zoop(const int& pos,
							  const float& mult,
							  const float& scaler = 1.F)
	{
		seq_ms[pos] *= mult;
		last_passed_check = do_timing_thing(scaler);
		seq_ms[pos] /= mult;
	}

	inline auto do_timing_thing(const float& scaler) -> bool
	{
		_mw_adj_ms(seq_ms[1]);

		if (_mw_adj_ms.get_cv_of_window(window) > other_cv_threshold) {
			return false;
		}

		hi_im_a_float = cv(seq_ms);

		// ok we're pretty sure it's a roll don't bother with the test
		if (hi_im_a_float < 0.12F) {
			moving_cv = (hi_im_a_float + moving_cv + hi_im_a_float) / 3.F;
			return true;
		}
		{
			moving_cv = (hi_im_a_float + moving_cv) / 2.f;
		}

		return moving_cv < cv_threshold / scaler;
	}

	inline auto do_other_timing_thing(const float& scaler) -> bool
	{
		_mw_adj_ms(idk_ms[1]);
		_mw_adj_ms(idk_ms[2]);

		if (_mw_adj_ms.get_cv_of_window(window) > other_cv_threshold) {
			return false;
		}

		hi_im_a_float = cv(idk_ms);

		// ok we're pretty sure it's a roll don't bother with the test
		if (hi_im_a_float < 0.12F) {
			moving_cv = (hi_im_a_float + moving_cv + hi_im_a_float) / 3.F;
			return true;
		}
		{
			moving_cv = (hi_im_a_float + moving_cv) / 2.f;
		}

		return moving_cv < cv_threshold / scaler;
	}

	inline void handle_ccacc_timing_check() { zoop_the_woop(1, 2.5F, 1.25F); }

	inline void handle_roll_timing_check()
	{
		if (seq_ms[1] > seq_ms[0]) {
			zoop_the_woop(1, 2.5F);
		} else {
			seq_ms[0] /= 2.5F;
			seq_ms[2] /= 2.5F;
			last_passed_check = do_timing_thing(1.F);
			seq_ms[0] *= 2.5F;
			seq_ms[2] *= 2.5F;
		}
	}

	inline void handle_ccsjjscc_timing_check(const float& now)
	{
		// translate over the values
		idk_ms[2] = seq_ms[0];
		idk_ms[1] = seq_ms[1];
		idk_ms[0] = seq_ms[2];

		// add the new value
		idk_ms[3] = now;

		// run 2 tests so we can keep a stricter cutoff
		// need to put cv in array thingy mcboop
		// check 1
		idk_ms[1] /= 2.5F;
		idk_ms[2] /= 2.5F;

		do_other_timing_thing(1.25F);

		idk_ms[1] *= 2.5F;
		idk_ms[2] *= 2.5F;

		if (last_passed_check) {
			return;
		}

		// test again
		idk_ms[1] /= 3.F;
		idk_ms[2] /= 3.F;

		do_other_timing_thing(1.25F);

		idk_ms[1] *= 3.F;
		idk_ms[2] *= 3.F;
	}

	inline void complete_seq()
	{
		if (nah_this_file_aint_for_real > 0) {
			max_thingy = nah_this_file_aint_for_real > max_thingy
						   ? nah_this_file_aint_for_real
						   : max_thingy;
		}
		nah_this_file_aint_for_real = 0;
	}

	inline void bibblybop(const meta_type& last_mt)
	{
		// see below
		if (last_mt == meta_enigma) {
			moving_cv = (moving_cv + hi_im_a_float) / 2.F;
		} else if (last_mt == meta_meta_enigma) {
			moving_cv = (moving_cv + hi_im_a_float + hi_im_a_float) / 3.F;
		}

		if (!last_passed_check) {
			complete_seq();
			return;
		}

		++nah_this_file_aint_for_real;

		// if we are here and mt.last == meta enigma, we skipped 1 note
		// before we identified a jumptrillable roll continuation, if meta
		// meta enigma, 2

		// borp it
		if (last_mt == meta_enigma) {
			++nah_this_file_aint_for_real;
		}

		// same but even more-er
		if (last_mt == meta_meta_enigma) {
			nah_this_file_aint_for_real += 2;
		}
	}

	inline void advance_sequencing(const base_pattern_type& bt,
								   const meta_type& mt,
								   const meta_type& last_mt,
								   const float& any_ms,
								   const float& tc_ms)
	{
		// we will let ohjumps through here

		update_seq_ms(bt, any_ms, tc_ms);
		if (bt == base_single_jump || bt == base_jump_single) {
			return;
		}

		if (bt == base_jump_jump) {
			// its an actual jumpjack/jumptrill, don't bother with timing checks
			// disable for now
			if (nah_this_file_aint_for_real > 0) {
				bibblybop(last_mt);
			}
			return;
		}

		// look for stuff thats jumptrillyable.. if that stuff... then leads
		// into more stuff.. that is jumptrillyable... then .... badonk it
		switch (mt) {
			case meta_acca:
				// unlike wrjt we want to complete and reset on these
				complete_seq();
				break;
			case meta_cccccc:
				handle_roll_timing_check();
				bibblybop(last_mt);
				break;
			case meta_ccacc:
				handle_ccacc_timing_check();
				bibblybop(last_mt);
				break;
			case meta_ccsjjscc:
			case meta_ccsjjscc_inverted:
				handle_ccsjjscc_timing_check(any_ms);
				bibblybop(last_mt);
				break;
			case meta_type_init:
			case meta_enigma:
				// this could yet be something we are interested in, but we
				// don't know yet, so just wait and see
				break;
			case meta_meta_enigma:
				// it's been too long... your vision becomes blurry.. your
				// memory fades... why are we here again? what are we trying
				// to do? who are we....
				complete_seq();
				break;
			default:
				assert(0);
				break;
		}
	}

	inline void update_seq_ms(const base_pattern_type& bt, const float& any_ms, const float& tc_ms)
	{
		seq_ms[0] = seq_ms[1]; // last_last
		seq_ms[1] = seq_ms[2]; // last

		// update now
		// for anchors, track tc_ms
		if (bt == base_single_single) {
			seq_ms[2] = tc_ms;
			// for base_left_right or base_right_left, track cc_ms
		} else {
			seq_ms[2] = any_ms;
		}
	}

	inline auto operator()(const ItvHandInfo& itvhi) -> float
	{
		max_thingy = nah_this_file_aint_for_real > max_thingy
					   ? nah_this_file_aint_for_real
					   : max_thingy;

		_mw_max(max_thingy);

		// check taps for _this_ interval, if there's none, and there was a
		// powerful roll mod before, the roll mod will extend into the empty
		// interval at minimum value due to 0/n, and then the smoother will push
		// that push that into the adjecant intervals
		// then check for the window values, perhaps we should also neutral set
		// if a large sequence has just ended on this interval, but that may
		// change too much and the tuning is already looking good anyway
		if (itvhi.get_taps_nowi() == 0 || itvhi.get_taps_windowi(window) == 0 ||
			_mw_max.get_total_for_window(window) == 0) {
			return neutral;
		}

		// really uncertain about the using the total of _mw_max here, but
		// that's what it was, so i'll keep it for now
		float zomg = itvhi.get_taps_windowf(window) /
					 _mw_max.get_total_for_windowf(window);

		pmod *= zomg;
		pmod = CalcClamp(base + fastsqrt(pmod), min_mod, max_mod);

		interval_reset();

		return pmod;
	}

	inline void interval_reset() { max_thingy = 0; }
};