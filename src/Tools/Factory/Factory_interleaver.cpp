#include "../../Interleaver/Interleaver_LTE.hpp"
#include "../../Interleaver/Interleaver_CCSDS.hpp"
#include "../../Interleaver/Interleaver_NO.hpp"
#include "../../Interleaver/Interleaver_columns.hpp"
#include "../../Interleaver/Interleaver_golden.hpp"
#include "../../Interleaver/Interleaver_random.hpp"

#include "../../Tools/Factory/Factory_interleaver.hpp"

template <typename T>
Interleaver<T>* Factory_interleaver<T>
::build(const t_code_param &code_params, const int &size)
{
	Interleaver<T> *interleaver = nullptr;

	// build the interleaver
	if (code_params.interleaver == "LTE")
		interleaver = new Interleaver_LTE<T>(size);
	else if (code_params.interleaver == "CCSDS")
		interleaver = new Interleaver_CCSDS<T>(size);
	else if (code_params.interleaver == "RANDOM")
		interleaver = new Interleaver_random<T>(size);
	else if (code_params.interleaver == "COLUMNS")
		interleaver = new Interleaver_columns<T>(code_params.K, code_params.M);
	else if (code_params.interleaver == "GOLDEN")
		interleaver = new Interleaver_golden<T>(size);
	else if (code_params.interleaver == "NO")
		interleaver = new Interleaver_NO<T>(size);

	return interleaver;
}

// ==================================================================================== explicit template instantiation 
template struct Factory_interleaver<short>;
template struct Factory_interleaver<int>;
template struct Factory_interleaver<long long>;
// ==================================================================================== explicit template instantiation