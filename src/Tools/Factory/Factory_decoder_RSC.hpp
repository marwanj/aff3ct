#ifndef FACTORY_DECODER_RSC_HPP
#define FACTORY_DECODER_RSC_HPP

#include "../../Decoder/Decoder.hpp"
#include "../../Decoder/SISO.hpp"

#include "../params.h"

template <typename B, typename R, typename RD>
struct Factory_decoder_RSC
{
	static SISO<R>* build_siso(const t_code_param                    &code_params,
	                           const t_encoder_param                 &enco_params,
	                           const t_channel_param                 &chan_params,
	                           const t_decoder_param                 &deco_params,
	                           const mipp::vector<mipp::vector<int>> &trellis);

	static Decoder<B,R>* build(const t_code_param                    &code_params,
	                           const t_encoder_param                 &enco_params,
	                           const t_channel_param                 &chan_params,
	                           const t_decoder_param                 &deco_params,
	                           const mipp::vector<mipp::vector<int>> &trellis);
};

#endif /* FACTORY_DECODER_RSC_HPP */